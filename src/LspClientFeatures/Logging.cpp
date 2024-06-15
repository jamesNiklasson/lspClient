// Part of the NPP LSP client
// Created on 24.05.2024 by JamesNiklasson
// Provides log file functionality

#include "Logging.h"
#include <stdio.h>
#include <windows.h>
#include <tchar.h>

char logFilePath[] = "C:\\Users\\nikla\\Downloads\\lspClient.log";
FILE *logFile = NULL;

void logInit(void) {
	if (logFile) {
		printf("Tried to open logfile, but file has already been opened!\n");
		return;
	}
	logFile = fopen(logFilePath, "a");
	if (logFile) {
		logWrite("\nLogfile opened");
		return;
	}
	printf("Logfile couldn't be opened!\n");
}

void logExit(void) {
	if (logFile) {
		logWrite("Logfile closed");
		fclose(logFile);
		logFile = NULL;
		return;
	}
	printf("Tried to close logfile, but file is not open!\n");
}

void logWrite(const char *msg) {
	if (logFile) {
		fprintf(logFile, "%s\n", msg);
		return;
	}
	printf("Tried to write to logfile, but file is not open!\n");
}

void logWrite(const TCHAR *msg) {
	if (logFile) {
		_ftprintf(logFile, TEXT("%s\n"), msg);
		return;
	}
	printf("Tried to write to logfile, but file is not open!\n");
}

void logHello(void) {
	logWrite("Hello World");
}