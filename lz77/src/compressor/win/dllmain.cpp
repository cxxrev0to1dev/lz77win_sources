// dllmain.cpp : Defines the entry point for the DLL application.
#include "base/basictypes.h"
#include "base/basic_incls.h"
#include "compressor/compressor_exports.h"

//7z FIXME
int g_CodePage = -1;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

