// Part of the NPP LSP client
// Created on 24.05.2024 by JamesNiklasson
// Contains defines and data types

#include "Defines.h"
#include "Logging.h"
#include <windows.h>
#include <tchar.h>
#include <string>

ConfigurationOptions configurationOptions = {
	.langServerExe = "",
	.startLangServerOnStartup = false
};

bool pluginConfigWasRead = false;
bool listenerThreadIsRunning = false;
bool langServerIsRunning = false;
bool langServerIsInitialized = false;
bool shuttingDown = false;

HANDLE serverStdinWrite = NULL;
HANDLE serverProcess = NULL;
HANDLE listenerThread = NULL;

MessageInQueue messageQueue[MESSAGEQUEUELEN];
int nextMessageId = 1;

OpenTextDocument openTextDocumentsList[MAXOPENTEXTDOCUMENTS];

std::string stringFromTcharArray(TCHAR *a) {
	#ifdef _UNICODE
	logWrite("Lol ur fucked because ur using wide chars");
	int stringLength = (int)_tcslen(a);
	char *buffer = new char[stringLength + 1];
	wcstombs(buffer, (const wchar_t *)a, stringLength + 1);
	std::string result(buffer);
	delete [] buffer;
	return result;
	#else
	std::string result((char *)a);
	return result;
	#endif
}