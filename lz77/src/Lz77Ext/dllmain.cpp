// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"
#include "Lz77Ext_i.h"
#include "dllmain.h"
#include "compreg.h"
#include "xdlldata.h"

CLz77ExtModule _AtlModule;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
  if (dwReason==DLL_PROCESS_ATTACH){
    CoInitialize(NULL);
  }
  else if (dwReason==DLL_PROCESS_DETACH){
    CoUninitialize();
  }
#ifdef _MERGE_PROXYSTUB
	if (!PrxDllMain(hInstance, dwReason, lpReserved))
		return FALSE;
#endif
	hInstance;
	return _AtlModule.DllMain(dwReason, lpReserved); 
}
