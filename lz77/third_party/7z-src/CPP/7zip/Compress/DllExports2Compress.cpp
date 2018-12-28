// DllExports2Compress.cpp

#include "StdAfx.h"

#include "../../Common/MyInitGuid.h"

#include "../ICoder.h"

#include "../Common/RegisterCodec.h"

extern "C"
BOOL WINAPI DllMainFIXME1(
  #ifdef UNDER_CE
  HANDLE
  #else
  HINSTANCE
  #endif
  /* hInstance */, DWORD /* dwReason */, LPVOID /*lpReserved*/)
{
  return TRUE;
}

STDAPI CreateCoder(const GUID *clsid, const GUID *iid, void **outObject);


STDAPI CreateObjectFIXME3(const GUID *clsid, const GUID *iid, void **outObject)
{
  return CreateCoder(clsid, iid, outObject);
}