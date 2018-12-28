// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "Lz77Lang/lz77_lang.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
  //MOD:TEST
  //lang::LZ77Language lang;
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
    
    break;;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

