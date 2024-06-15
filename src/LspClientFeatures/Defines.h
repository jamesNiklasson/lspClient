// Part of the NPP LSP client
// Created on 24.05.2024 by JamesNiklasson
// Contains defines and data types

#ifndef DEFINES_H
#define DEFINES_H

#include "LspMessages.h"

#include <string>
#include <tchar.h>

#define BUFFLEN_1 1024
#define MESSAGEQUEUELEN 100
#define MAXOPENTEXTDOCUMENTS 100

typedef struct {
	std::string langServerExe;
	bool startLangServerOnStartup;
} ConfigurationOptions;

typedef struct {
	int messageId;
	bool outstanding;
	std::string method;
} MessageInQueue;

typedef struct {
	std::string uri;
	bool open;
	int version;
} OpenTextDocument;

std::string stringFromTcharArray(TCHAR *a);

#endif