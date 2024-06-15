// Part of the NPP LSP client
// Created on 24.05.2024 by JamesNiklasson
// Provides log file functionality

#ifndef LOGGING_H
#define LOGGING_H

#include <tchar.h>

void logInit(void);
void logExit(void);
void logWrite(const char *msg);
void logWrite(const TCHAR *msg);
void logHello(void);

#endif