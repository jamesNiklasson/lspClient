//this file is part of notepad++
//Copyright (C)2022 Don HO <don.h@free.fr>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include "LspClientFeatures\Defines.h"
#include "LspClientFeatures\Test.h"
#include "LspClientFeatures\Logging.h"
#include "LspClientFeatures\NppActions.h"

extern FuncItem funcItem[nbFunc];
extern NppData nppData;
extern OpenTextDocument openTextDocumentsList[]; 
extern bool shuttingDown;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  reasonForCall, LPVOID /*lpReserved*/)
{
	try {

		switch (reasonForCall)
		{
			case DLL_PROCESS_ATTACH:
				pluginInit(hModule);
				break;

			case DLL_PROCESS_DETACH:
				pluginCleanUp();
				break;

			case DLL_THREAD_ATTACH:
				break;

			case DLL_THREAD_DETACH:
				break;
		}
	}
	catch (...) { return FALSE; }

    return TRUE;
}


extern "C" __declspec(dllexport) void setInfo(NppData notpadPlusData)
{
	nppData = notpadPlusData;
	commandMenuInit();
}

extern "C" __declspec(dllexport) const TCHAR * getName()
{
	return NPP_PLUGIN_NAME;
}

extern "C" __declspec(dllexport) FuncItem * getFuncsArray(int *nbF)
{
	*nbF = nbFunc;
	return funcItem;
}

extern "C" __declspec(dllexport) void beNotified(SCNotification *notifyCode)
{
	switch (notifyCode->nmhdr.code) 
	{
		case NPPN_SHUTDOWN:
		{
			shutdownLangServer();
			commandMenuCleanUp();
		}
		break;
		
		case NPPN_BEFORESHUTDOWN:
			shuttingDown = true;
			break;
			
		case NPPN_BUFFERACTIVATED:
		{
			if (shuttingDown) return;
			TCHAR filePathBuffer[MAX_PATH];
			if (!((bool)::SendMessage(nppData._nppHandle, NPPM_GETFULLCURRENTPATH, (WPARAM)MAX_PATH, (LPARAM)filePathBuffer))) {
				logWrite("Error: Read document path from NPP failed");
				::MessageBox(NULL, TEXT("Communication with NPP failed"), TEXT("Error"), MB_OK);
			}
			std::string filePath = stringFromTcharArray(filePathBuffer);
			for (int i = 0; i < MAXOPENTEXTDOCUMENTS; i++) {
				if ((filePath == openTextDocumentsList[i].uri) && openTextDocumentsList[i].open) {
					return;
				}
			}
			didOpenTextDocument();
		} break;
		
		case NPPN_GLOBALMODIFIED:
			if (shuttingDown) return;
			// didChangeTextDocument(); 
			break;
		
		case SCN_MODIFIED:
			if (shuttingDown) return;
			if ((notifyCode->modificationType) & (0x01 | 0x02)) {
				didChangeTextDocument();
			}
			break;
			
		case NPPN_READY:
			logSomething();
			break;

		default:
			return;
	}
}

// Here you can process the Npp Messages 
// I will make the messages accessible little by little, according to the need of plugin development.
// Please let me know if you need to access to some messages :
// https://github.com/notepad-plus-plus/notepad-plus-plus/issues
//
extern "C" __declspec(dllexport) LRESULT messageProc(UINT /*Message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{/*
	if (Message == WM_MOVE)
	{
		::MessageBox(NULL, "move", "", MB_OK);
	}
*/
	return TRUE;
}

#ifdef UNICODE
extern "C" __declspec(dllexport) BOOL isUnicode()
{
    return TRUE;
}
#endif //UNICODE
