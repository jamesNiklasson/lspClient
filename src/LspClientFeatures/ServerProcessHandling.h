// Part of the NPP LSP client
// Created on 02.06.2024 by JamesNiklasson
// Contains functions for handling the language server process

#include <windows.h>

typedef struct {
	HANDLE *serverStdoutReadHandle;
} ListenToServerParams;

void listenToServer(void *rawParams);