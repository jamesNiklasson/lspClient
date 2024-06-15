// Part of the NPP LSP client
// Created on 24.05.2024 by JamesNiklasson
// For testing

#include "Test.h"
#include "Logging.h"
#include "Defines.h"
#include "..\PluginDefinition.h"
#include "LspMessages.h"
#include "..\nlohmann\json.hpp"
using json = nlohmann::json;

#include <stdio.h>
#include <variant>
#include <windows.h>

extern NppData nppData;

void logDocumentWriteup(void) {
	logWrite("Writeup of currently open documents:");
	int mainViewFiles = (int)::SendMessage(nppData._nppHandle, NPPM_GETNBOPENFILES, (WPARAM)0, (LPARAM)1);
	int secondViewFiles = (int)::SendMessage(nppData._nppHandle, NPPM_GETNBOPENFILES, (WPARAM)0, (LPARAM)2);
	int totalFiles = (int)::SendMessage(nppData._nppHandle, NPPM_GETNBOPENFILES, (WPARAM)0, (LPARAM)0);
	
	char logMessage[BUFFLEN_1];
	sprintf(logMessage, "There are %d files open in the main view and %d files open in the second view. Total %d", mainViewFiles, secondViewFiles, totalFiles);
	logWrite(logMessage);
	
	TCHAR fileNamesBuffer[256][256];
	TCHAR *fileNames[256];
	for (int i = 0; i < 256; i++) {
		fileNames[i] = fileNamesBuffer[i];
	}
	int namesRead = (int)::SendMessage(nppData._nppHandle, NPPM_GETOPENFILENAMES, (WPARAM)fileNames, (LPARAM)totalFiles);
	if (namesRead) {
		for (int i = 0; i < namesRead; i++) {
			char fileName[256];
			wcstombs(fileName, (const wchar_t*)fileNames[i], 256);
			sprintf(logMessage, "file name %d of %d was read was sucessfully: %s", i, namesRead, fileName);
			logWrite(logMessage);
		}
	}
}

void logDocumentPath(void) {
	TCHAR fileNameBuffer[256];
	char logMessage[BUFFLEN_1];
	bool success = (bool)::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, (WPARAM)256, (LPARAM)fileNameBuffer);
	char fileName[256];
	if (success) {
		wcstombs(fileName, (const wchar_t*)fileNameBuffer, 256);
		sprintf(logMessage, "Filenae read sucessfully: %s", fileName);
		logWrite(logMessage);
	} else {
		logWrite("File name read failed");
	}
}

void documentOpenAction(void) {
	logWrite("\nFile opened:");
	logDocumentPath();
}

void testSleep(void) {
	Sleep(1000);
}

void logSomething(void) {
	logWrite("NPP started up");
}

void logSomethingElse(void) {
	logWrite("Document just chaged but different");
}