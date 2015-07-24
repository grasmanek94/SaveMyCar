/*
	THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
				http://dev-c.com			
			(C) Alexander Blade 2015
*/

#include "..\..\inc\main.h"
#include "script.hxx"

void KeyboardHandlerFunction(DWORD key, WORD repeats, BYTE scanCode, BOOL isExtended, BOOL isWithAlt, BOOL wasDownBefore, BOOL isUpNow);

BOOL APIENTRY DllMain(HMODULE hInstance, DWORD reason, LPVOID lpReserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		scriptRegister(hInstance, ScriptMain);
		keyboardHandlerRegister(KeyboardHandlerFunction);
		break;
	case DLL_PROCESS_DETACH:
		scriptUnregister(hInstance);
		keyboardHandlerUnregister(KeyboardHandlerFunction);
		break;
	}		
	return TRUE;
}