// Part of the NPP LSP client
// Created on 29.05.2024 by JamesNiklasson
// Contains handlers for NPP notifications

#include "NppActions.h"
#include "Logging.h"
#include "Defines.h"
#include "ServerProcessHandling.h"
#include "..\PluginDefinition.h"
#include "LspMessages.h"
#include "..\nlohmann\json.hpp"
using json = nlohmann::json;

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>
// #include <memory>

extern NppData nppData;
extern ConfigurationOptions configurationOptions;
extern bool pluginConfigWasRead;
extern bool langServerIsRunning;
extern bool langServerIsInitialized;
extern bool listenerThreadIsRunning;
extern HANDLE serverStdinWrite;
extern HANDLE serverProcess;
extern HANDLE listenerThread;
extern OpenTextDocument openTextDocumentsList[];
extern MessageInQueue messageQueue[];
extern int nextMessageId;

void didChangeTextDocument(void) {
	if (!langServerIsInitialized) {
		logWrite("Error: Can't send didChangeTextDocument because Server isn't initialized");
		return;
	}
	
	TCHAR filePathBuffer[MAX_PATH];
	if (!((bool)::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, (WPARAM)MAX_PATH, (LPARAM)filePathBuffer))) {
		logWrite("Error: Read document path from NPP failed");
		::MessageBox(NULL, TEXT("Communication with NPP failed"), TEXT("Error"), MB_OK);
	}
	int fileVersion = -1;
	std::string filePath = stringFromTcharArray(filePathBuffer);
	for (int i = 0; i < MAXOPENTEXTDOCUMENTS; i++) {
		if ((filePath == openTextDocumentsList[i].uri) && openTextDocumentsList[i].open) {
			fileVersion = ++(openTextDocumentsList[i].version);
			break;
		}
	}
	if (fileVersion == -1) {
		logWrite("Error: Can't send didChangeTextDocument because File isn't open");
		return;
	}
	
	int whichScintilla = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, (WPARAM)0, (LPARAM)&whichScintilla);
	if (whichScintilla == -1) {
		logWrite("Error: Read current Scintilla from NPP failed");
		::MessageBox(NULL, TEXT("Communication with NPP failed"), TEXT("Error"), MB_OK);
	}
    HWND curScintilla = (whichScintilla == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
	
	int documentBufferLenght = (int)::SendMessage(curScintilla, SCI_GETTEXT, (WPARAM)0, (LPARAM)NULL);
	char *documentTextBuffer = new char[documentBufferLenght];
	::SendMessage(curScintilla, SCI_GETTEXT, (WPARAM)documentBufferLenght, (LPARAM)documentTextBuffer);
	
	DidChangeTextDocumentParams params = {
		.textDocument = {
			.uri = filePath,
			.version = fileVersion
		},
		.contentChanges = {{
			.text = documentTextBuffer
		}}
	};
	
	NotificationMessage didChangeTextDocumentMessage = {
		.jsonrpc = "2.0",
		.method = "textDocument/didChange",
		.params = params
	};
	
	json didChangeTextDocumentJson = didChangeTextDocumentMessage;
	std::string didChangeTextDocumentString = didChangeTextDocumentJson.dump();
	int contentLenght = (int)didChangeTextDocumentString.length();
	didChangeTextDocumentString.insert(0, "Content-Length: \r\n\r\n");
	didChangeTextDocumentString.insert(16, std::to_string(contentLenght));
	logWrite("\nI'll send this message to the language server");
	logWrite(didChangeTextDocumentString.c_str());
	
	DWORD bytesSent;
	if (!WriteFile(serverStdinWrite, didChangeTextDocumentString.c_str(), (DWORD)(didChangeTextDocumentString.length()), &bytesSent, NULL)) {
		logWrite("Error: Write to server process failed");
		::MessageBox(NULL, TEXT("Couldn't write to language server process"), TEXT("Error"), MB_OK);
		return;
	}
}

void didOpenTextDocument(void) {
	if (!langServerIsInitialized) {
		logWrite("Error: Can't send didOpenTextDocument because Server isn't initialized");
		return;
	}
	
	TCHAR filePathBuffer[MAX_PATH];
	if (!((bool)::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, (WPARAM)MAX_PATH, (LPARAM)filePathBuffer))) {
		logWrite("Error: Read document path from NPP failed");
		::MessageBox(NULL, TEXT("Communication with NPP failed"), TEXT("Error"), MB_OK);
	}
	
	int langType;
	LanguageId langId;
	::SendMessage(nppData._nppHandle, NPPM_GETCURRENTLANGTYPE, (WPARAM)0, (LPARAM)&langType);
	switch (langType) {
		case L_C:
			langId = C;
			break;
		case L_CPP:
			langId = Cpp;
			break;
		case L_CS:
			langId = Csharp;
			break;
		default:
			return;
			break;
	}
	
	int whichScintilla = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, (WPARAM)0, (LPARAM)&whichScintilla);
	if (whichScintilla == -1) {
		logWrite("Error: Read current Scintilla from NPP failed");
		::MessageBox(NULL, TEXT("Communication with NPP failed"), TEXT("Error"), MB_OK);
	}
    HWND curScintilla = (whichScintilla == 0) ? nppData._scintillaMainHandle : nppData._scintillaSecondHandle;
	
	int documentBufferLenght = (int)::SendMessage(curScintilla, SCI_GETTEXT, (WPARAM)0, (LPARAM)NULL);
	char *documentTextBuffer = new char[documentBufferLenght];
	::SendMessage(curScintilla, SCI_GETTEXT, (WPARAM)documentBufferLenght, (LPARAM)documentTextBuffer);
	
	TextDocumentItem textDocument = {
		.uri = stringFromTcharArray(filePathBuffer),
		.languageId = langId,
		.version = 0,
		.text = documentTextBuffer
	};
	
	delete [] documentTextBuffer;
	
	for (int i = 0; i < MAXOPENTEXTDOCUMENTS; i++) {
		if (!(openTextDocumentsList[i].open)) {
			openTextDocumentsList[i].uri = textDocument.uri;
			openTextDocumentsList[i].open = true;
			openTextDocumentsList[i].version = 0;
			break;
		}
	}
	
	DidOpenTextDocumentParams params = {
		.textDocument = textDocument
	};
	
	NotificationMessage didOpenTextDocumentMessage = {
		.jsonrpc = "2.0",
		.method = "textDocument/didOpen",
		.params = params
	};
	
	json didOpenTextDocumentJson = didOpenTextDocumentMessage;
	std::string didOpenTextDocumentString = didOpenTextDocumentJson.dump();
	int contentLenght = (int)didOpenTextDocumentString.length();
	didOpenTextDocumentString.insert(0, "Content-Length: \r\n\r\n");
	didOpenTextDocumentString.insert(16, std::to_string(contentLenght));
	logWrite("\nI'll send this message to the language server");
	logWrite(didOpenTextDocumentString.c_str());
	
	DWORD bytesSent;
	if (!WriteFile(serverStdinWrite, didOpenTextDocumentString.c_str(), (DWORD)(didOpenTextDocumentString.length()), &bytesSent, NULL)) {
		logWrite("Error: Write to server process failed");
		::MessageBox(NULL, TEXT("Couldn't write to language server process"), TEXT("Error"), MB_OK);
		return;
	}
}

void startLangServer(void) {
	if (langServerIsRunning) shutdownLangServer();
	if (langServerIsRunning) return;
	
	if (!pluginConfigWasRead) parseConfigFile();
	if (!pluginConfigWasRead) return;
	
	struct _stat fileInfo;
	if (_stat(configurationOptions.langServerExe.c_str(), &fileInfo)) {
		logWrite("Error: Language server executable not found");
		::MessageBox(NULL, TEXT("Language server executable not found"), TEXT("Error"), MB_OK);
		return;
	}
	
	//start the executable
	HANDLE childInRead = NULL;
	HANDLE childInWrite = NULL;
	HANDLE childOutRead = NULL;
	HANDLE childOutWrite = NULL;
	SECURITY_ATTRIBUTES pipeAttributes;
	ZeroMemory(&pipeAttributes, sizeof(pipeAttributes));
	pipeAttributes.bInheritHandle = true;
	pipeAttributes.nLength = sizeof(pipeAttributes);
	
	if (!CreatePipe(&childInRead, &childInWrite, &pipeAttributes, 0)) {
		logWrite("Error: Failed server stdin creation");
		::MessageBox(NULL, TEXT("Language server startup failed"), TEXT("Error"), MB_OK);
		return;
	}
	if (!CreatePipe(&childOutRead, &childOutWrite, &pipeAttributes, 0)) {
		logWrite("Error: Failed server stdout creation");
		::MessageBox(NULL, TEXT("Language server startup failed"), TEXT("Error"), MB_OK);
		return;
	}
	if (!SetHandleInformation(childInWrite, HANDLE_FLAG_INHERIT, 0)) {
		logWrite("Error: Failed server stdin modification");
		::MessageBox(NULL, TEXT("Language server startup failed"), TEXT("Error"), MB_OK);
		return;
	}
	if (!SetHandleInformation(childOutRead, HANDLE_FLAG_INHERIT, 0)) {
		logWrite("Error: Failed server stdout modification");
		::MessageBox(NULL, TEXT("Language server startup failed"), TEXT("Error"), MB_OK);
		return;
	}
	
	STARTUPINFO processSettings;
	PROCESS_INFORMATION processInfo;
	ZeroMemory(&processSettings, sizeof(processSettings));
	processSettings.dwFlags  = STARTF_USESTDHANDLES;
	processSettings.hStdInput = childInRead;
	processSettings.hStdOutput = childOutWrite;
	processSettings.hStdError = childOutWrite;
	processSettings.cb = sizeof(processSettings);
	ZeroMemory(&processInfo, sizeof(processInfo));
	
	if (!CreateProcessA(configurationOptions.langServerExe.c_str(),
						NULL,
						NULL,
						NULL,
						true,
						CREATE_NEW_CONSOLE,
						NULL,
						NULL,
						(LPSTARTUPINFOA) &processSettings,
						&processInfo)) {
		logWrite("Error: Language server startup failed");
		::MessageBox(NULL, TEXT("Language server startup failed"), TEXT("Error"), MB_OK);
		return;
	}
	serverProcess = processInfo.hProcess;
	serverStdinWrite = childInWrite;
	CloseHandle(processInfo.hThread);
	CloseHandle(childInRead);
	CloseHandle(childOutWrite);
	
	char logMessage[BUFFLEN_1];
	sprintf(logMessage, "\nCreated server process %d", processInfo.dwProcessId);
	logWrite(logMessage);
	langServerIsRunning = true;
	
	//start listener thread
	ListenToServerParams params = {.serverStdoutReadHandle = &childOutRead};
	listenerThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)listenToServer, (void *)&params, 0, NULL);
	while (!listenerThreadIsRunning) Sleep(1);
	
	//setup message queue
	for (int i = 0; i < MESSAGEQUEUELEN; i++) {
		messageQueue[i].outstanding = false;
	}
	
	//setup text document memory
	for (int i = 0; i < MAXOPENTEXTDOCUMENTS; i++) {
		openTextDocumentsList[i].open = false;
	}
	
	initLangServer();
	didOpenTextDocument();
}

void initLangServer(void) {
	if (langServerIsInitialized) {
		::MessageBox(NULL, TEXT("Can't initialize language server"), TEXT("Error"), MB_OK);
		logWrite("Error: Can't initialize language server");
		return;
	}
	
	if (!langServerIsRunning) {
		::MessageBox(NULL, TEXT("Can't initialize language server before it has beeen started"), TEXT("Error"), MB_OK);
		logWrite("Error: Can't initialize language server before it has beeen started");
		return;
	}
	
	//send init message
	InitializeParams initParams = {
		.processId = (int)GetCurrentProcessId(),
		.capabilities = {
			.workspace = {
				.workspaceFolders = false,
				.fileOperations = {
					.dynamicRegistration = false,
					.didCreate = false,
					.willCreate = false,
					.didRename = false,
					.willRename = false,
					.didDelete = false,
					.willDelete = false
				}
			},
			.textDocument = {
				.synchronization = {
					.dynamicRegistration = false,
					.willSave = false,
					.willSaveWaitUntil = false,
					.didSave = false
				},
				.completion = {
					.dynamicRegistration = false,
					.completionItem = {
						.snippetSupport = false,
						.commitCharactersSupport = false,
						.documentationFormat = {PlainText},
						.deprecatedSupport = false,
						.preselectSupport = false,
						.insertReplaceSupport = false,
						.resolveSupport = {
							.properties = {}
						},
						.insertTextModeSupport = {
							.valueSet = {AsIs}
						},
						.labelDetailsSupport = false
					},
					.completionItemKind = {
						.valueSet = {Text}
					},
					.contextSupport = false,
					.insertTextMode = AsIs,
					.completionList = {
						.itemDefaults = {}
					}
				}
			},
			.window = {
				.workDoneProgress = false,
				.showMessage = {
					.messageActionItem = {
						.additionalPropertiesSupport = false
					}
				},
				.showDocument = {
					.support = false
				}
			},
			.general = {
				.staleRequestSupport = {
					.cancel = false,
					.retryOnContentModified = {}
				},
				.positionEncodings = {utf_8, utf_16}
			}
		}
	};
	
	RequestMessage initMessage = {
		.jsonrpc = "2.0",
		.id = nextMessageId,
		.method = "initialize",
		.params = initParams
	};
	
	json initMessageJson = initMessage;
	std::string initMessageString = initMessageJson.dump();
	int contentLenght = (int)initMessageString.length();
	initMessageString.insert(0, "Content-Length: \r\n\r\n");
	initMessageString.insert(16, std::to_string(contentLenght));
	logWrite("\nI'll send this message to the language server");
	logWrite(initMessageString.c_str());

	bool spaceInQueueFound = false;
	for (int i = 0; i < MESSAGEQUEUELEN; i++) {
		if (!(messageQueue[i].outstanding)) {
			spaceInQueueFound = true;
			messageQueue[i].outstanding = true;
			messageQueue[i].messageId = nextMessageId;
			messageQueue[i].method = "initialize";
			nextMessageId++;
			break;
		}
	}
	if (!spaceInQueueFound) {
		logWrite("Error: Message queue is full");
		::MessageBox(NULL, TEXT("Couldn't write to language server process"), TEXT("Error"), MB_OK);
		return;
	}
	
	DWORD bytesSent;
	if (!WriteFile(serverStdinWrite, initMessageString.c_str(), (DWORD)(initMessageString.length()), &bytesSent, NULL)) {
		logWrite("Error: Write to server process failed");
		::MessageBox(NULL, TEXT("Couldn't write to language server process"), TEXT("Error"), MB_OK);
		return;
	}
	
	logWrite("Waiting for Lang server to initialize");
	while (!langServerIsInitialized) Sleep(1);
	logWrite("Lang server initialized");
}

void shutdownLangServer(void) {
	if (!langServerIsRunning) return;
	
	RequestMessage shutdownMessage = {
		.jsonrpc = "2.0",
		.id = nextMessageId,
		.method = "shutdown",
		.params = ""
	};
	
	json shutdownMessageJson = shutdownMessage;
	std::string shutdownMessageString = shutdownMessageJson.dump();
	int contentLenght = (int)shutdownMessageString.length();
	shutdownMessageString.insert(0, "Content-Length: \r\n\r\n");
	shutdownMessageString.insert(16, std::to_string(contentLenght));
	logWrite("\nI'll send this message to the language server");
	logWrite(shutdownMessageString.c_str());

	bool spaceInQueueFound = false;
	for (int i = 0; i < MESSAGEQUEUELEN; i++) {
		if (!(messageQueue[i].outstanding)) {
			spaceInQueueFound = true;
			messageQueue[i].outstanding = true;
			messageQueue[i].messageId = nextMessageId;
			messageQueue[i].method = "shutdown";
			nextMessageId++;
			break;
		}
	}
	if (!spaceInQueueFound) {
		logWrite("Error: Message queue is full");
		::MessageBox(NULL, TEXT("Couldn't write to language server process"), TEXT("Error"), MB_OK);
		return;
	}
	
	DWORD bytesSent;
	if (!WriteFile(serverStdinWrite, shutdownMessageString.c_str(), (DWORD)(shutdownMessageString.length()), &bytesSent, NULL)) {
		logWrite("Error: Write to server process failed");
		::MessageBox(NULL, TEXT("Couldn't write to language server process"), TEXT("Error"), MB_OK);
		return;
	}
	
	logWrite("Waiting for lang server to shut down");
	while (langServerIsRunning) Sleep(1);
	logWrite("Lang server shut down");
	
	logWrite("Waiting for listener thread to shut down");
	while (listenerThreadIsRunning) Sleep(1);
	logWrite("Listener thread shut down");
	
	char pleaseWork[BUFFLEN_1];
	sprintf(pleaseWork, "There is absolutely no reason why this should have to be here but don't delete it, it would break shutting the server down manually");
}

void parseConfigFile(void) {
	TCHAR configFilePath[MAX_PATH];
	TCHAR configFileName[] = TEXT("\\lspClient.json");
	
	//get path for the config file
	bool success = (bool)::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, (WPARAM)((int)MAX_PATH), (LPARAM)configFilePath);
	if (success) {
		_tcscat(configFilePath, configFileName);
	} else {
		logWrite("Error: Couldn't read config file path");
		return;
	}
	
	//check if it exists, open a new one if it doesn't, parse it if it does
	struct _stat fileInfo;
	if (_tstat(configFilePath, &fileInfo)) {
		logWrite("Error: Config file was not found, opening a new one");
		::MessageBox(NULL, TEXT("No config file was found, write a new one and rety the action"), TEXT("Config file not found"), MB_OK);
		openConfigFile();
	} else {
		std::ifstream configFile(configFilePath);
		json configData = json::parse(configFile);
		if (configData.contains("langServerExe")) configurationOptions.langServerExe = (std::string)configData["langServerExe"];
		if (configData.contains("startLangServerOnStartup")) configurationOptions.startLangServerOnStartup = (bool)configData["startLangServerOnStartup"];
		configFile.close();
		char logMessage[BUFFLEN_1];
		sprintf(logMessage, "\nThe server executable path is: %s\nThe start on startup option is %d", configurationOptions.langServerExe.c_str(), configurationOptions.startLangServerOnStartup);
		logWrite(logMessage);
		pluginConfigWasRead = true;
	}
}

void openConfigFile(void) {
	TCHAR configFilePath[MAX_PATH];
	TCHAR configFileName[] = TEXT("\\lspClient.json");
	
	//get path for the config file
	bool success = (bool)::SendMessage(nppData._nppHandle, NPPM_GETPLUGINSCONFIGDIR, (WPARAM)((int)MAX_PATH), (LPARAM)configFilePath);
	if (success) {
		_tcscat(configFilePath, configFileName);
	} else {
		logWrite("Error: Couldn't read config file path");
		return;
	}
	
	//if it doesn't exitst, create a new one with minimal json in it
	struct _stat fileInfo;
	if (_tstat(configFilePath, &fileInfo)) {
		logWrite("Error: Config file was not found, creating a new one");
		FILE *configFile = _tfopen(configFilePath, TEXT("a"));
		fprintf(configFile, "{}");
		fclose(configFile);
	}
	
	//open the file
	success = (bool)::SendMessage(nppData._nppHandle, NPPM_DOOPEN, (WPARAM)0, (LPARAM)configFilePath);
	if (!success) {
		logWrite("Error: Couldn't open config file");
	}
}