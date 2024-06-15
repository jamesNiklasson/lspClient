// Part of the NPP LSP client
// Created on 29.05.2024 by JamesNiklasson
// Contains handlers for NPP notifications

#ifndef NPPACTIONS_H
#define NPPACTIONS_H

void didChangeTextDocument(void);
void didOpenTextDocument(void);
void startLangServer(void);
void initLangServer(void);
void sendOpenDocuments(void);
void shutdownLangServer(void);
void parseConfigFile(void);
void openConfigFile(void);

#endif