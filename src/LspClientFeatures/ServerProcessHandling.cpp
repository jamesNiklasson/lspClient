// Part of the NPP LSP client
// Created on 02.06.2024 by JamesNiklasson
// Contains functions for handling the language server process

#include "ServerProcessHandling.h"
#include "Defines.h"
#include "Logging.h"
#include "LspMessages.h"
#include "..\nlohmann\json.hpp"
using json = nlohmann::json;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <variant>
#include <string>

extern bool langServerIsRunning;
extern bool listenerThreadIsRunning;
extern bool langServerIsInitialized;
extern MessageInQueue messageQueue[];
extern HANDLE serverStdinWrite;
extern HANDLE serverProcess;

void handleInitializeResponse(json result) {
	logWrite("Hello from the initialize response handler");
	langServerIsInitialized = true;
}

void handleShutdownResponse(json result) {
	logWrite("Hello from the shutdown response handler");
	
	NotificationMessage exitMessage = {
		.jsonrpc = "2.0",
		.method = "exit",
		.params = ""
	};
	
	json exitMessageJson = exitMessage;
	std::string exitMessageString = exitMessageJson.dump();
	int contentLenght = (int)exitMessageString.length();
	exitMessageString.insert(0, "Content-Length: \r\n\r\n");
	exitMessageString.insert(16, std::to_string(contentLenght));
	logWrite("\nI'll send this message to the language server");
	logWrite(exitMessageString.c_str());
	
	DWORD bytesSent;
	if (!WriteFile(serverStdinWrite, exitMessageString.c_str(), (DWORD)(exitMessageString.length()), &bytesSent, NULL)) {
		logWrite("Error: Write to server process failed");
		::MessageBox(NULL, TEXT("Couldn't write to language server process"), TEXT("Error"), MB_OK);
		return;
	}
	
	DWORD serverExitProcess;
	while (true) {
		if (!GetExitCodeProcess(serverProcess, &serverExitProcess)) {
			logWrite("Failed to get server exit code");
			return;
		}
		if (serverExitProcess != STILL_ACTIVE) break;
	}
	if (serverExitProcess != 0) logWrite("Server exited with error");
	
	langServerIsInitialized = false;
	langServerIsRunning = false;
	
	CloseHandle(serverProcess);
	CloseHandle(serverStdinWrite);
}

bool compareStringToEndOfCyclicBuffer(char *string, char *buffer, int bufferPosition, int bufferLenght) {
	int stringLength = (int)strlen(string);
	for (int i = stringLength - 1; i >= 0; i--) {
		if (string[i] != buffer[(bufferLenght + bufferPosition + i + 1 - stringLength) % bufferLenght]) return false;
	}
	return true;
}

void listenToServer(void *rawParams) {
	ListenToServerParams *params = (ListenToServerParams *)rawParams;
	if (!langServerIsRunning) {
		logWrite("Error: Tried to start listening to server, but server hasn't been started");
		return;
	}
	
	listenerThreadIsRunning = true;
	
	//main loop runs once per message
	while (true) {
		if (!langServerIsRunning) break;
		
		//Read one byte at a time from the server stdout until the content length field is found
		char headerBuffer[BUFFLEN_1];
		int i = 0;
		DWORD bytesRecieved;
		while (true) {
			if (!ReadFile(*(params -> serverStdoutReadHandle), headerBuffer + i, 1, &bytesRecieved, NULL)) {
				logWrite("Error: Read from language server stdout failed 1");
				::MessageBox(NULL, TEXT("Couldn't read from language server process"), TEXT("Error"), MB_OK);
				listenerThreadIsRunning = false;
				return;
			}
			if (!bytesRecieved) continue;
			if (!headerBuffer[i]) continue;
			if (compareStringToEndOfCyclicBuffer((char *)"Content-Length: ", headerBuffer, i, BUFFLEN_1)) break;
			i = (i + 1) % BUFFLEN_1;
		}
		logWrite("\nFound content length field in server response");
		
		//Read the content length of the message
		i = 0;
		while (i < BUFFLEN_1) {
			if (!ReadFile(*(params -> serverStdoutReadHandle), headerBuffer + i, 1, &bytesRecieved, NULL)) {
				logWrite("Error: Read from language server stdout failed 2");
				::MessageBox(NULL, TEXT("Couldn't read from language server process"), TEXT("Error"), MB_OK);
				listenerThreadIsRunning = false;
				return;
			}
			if (headerBuffer[i] == '\r') break;
			i++;
		}
		headerBuffer[i] = 0;
		int contentLength;
		sscanf(headerBuffer, "%d", &contentLength);
		char logMessage[BUFFLEN_1];
		sprintf(logMessage, "Message length of message read as %d", contentLength);
		logWrite(logMessage);
		
		//Read until the message begins
		i = 0;
		while (true) {
			if (!ReadFile(*(params -> serverStdoutReadHandle), headerBuffer + i, 1, &bytesRecieved, NULL)) {
				logWrite("Error: Read from language server stdout failed 3");
				::MessageBox(NULL, TEXT("Couldn't read from language server process"), TEXT("Error"), MB_OK);
				listenerThreadIsRunning = false;
				return;
			}
			if (!bytesRecieved) continue;
			if (!headerBuffer[i]) continue;
			if (compareStringToEndOfCyclicBuffer((char *)"\n\r\n", headerBuffer, i, BUFFLEN_1)) break;
			i = (i + 1) % BUFFLEN_1;
		}
		
		//Read the content string
		i = 0;
		char *contentString = new char[contentLength + 1];
		while (i < contentLength) {
			if (!ReadFile(*(params -> serverStdoutReadHandle), contentString + i, 1, &bytesRecieved, NULL)) {
				logWrite("Error: Read from language server stdout failed 4");
				::MessageBox(NULL, TEXT("Couldn't read from language server process"), TEXT("Error"), MB_OK);
				delete [] contentString;
				listenerThreadIsRunning = false;
				return;
			}
			if (!bytesRecieved) continue;
			if (!contentString[i]) continue;
			i++;
		}
		contentString[i] = 0;
		logWrite("Content from message:");
		logWrite(contentString);
		json contentData = json::parse(contentString);
		delete [] contentString;
		
		//check to see if this message has been awaited
		MessageId messageId = (int)contentData["id"];
		bool messageExpected = false;
		for (i = 0; i < MESSAGEQUEUELEN; i++) {
			if (std::holds_alternative<int>(messageId) && (std::get<int>(messageId) == messageQueue[i].messageId) && messageQueue[i].outstanding) {
				messageQueue[i].outstanding = false;
				messageExpected = true;
				sprintf(logMessage, "Reply to message number %d was recieved", std::get<int>(messageId));
				logWrite(logMessage);
				break;
			}
		}
		
		// call the correct message handler
		std::string method;
		if (messageExpected) {
			method = messageQueue[i].method;
		} else if (contentData.contains("method")) {
			method = contentData["method"];
		}
		
		if (method == "initialize") {
			if (contentData.contains("result")) handleInitializeResponse(contentData["result"]);
		} else if (method == "shutdown") {
			if (contentData.contains("result")) handleShutdownResponse(contentData["result"]);
		}
	}
	logWrite("Listener thread ended");
	listenerThreadIsRunning = false;
}