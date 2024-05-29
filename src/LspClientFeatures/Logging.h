// Part of the NPP LSP client
// Created on 24.05.2024 by JamesNiklasson
// Provides log file functionality

#ifndef LOGGING_H
#define LOGGING_H

void logInit(void);
void logExit(void);
void logWrite(const char *msg);
void logHello(void);

#endif