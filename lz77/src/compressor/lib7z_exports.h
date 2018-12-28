#ifndef COMPRESSOR_LIB7Z_EXPORTS_H_
#define COMPRESSOR_LIB7Z_EXPORTS_H_

#include "C/7zVersion.h"
#include "CPP/Common/ComTry.h"
#include "CPP/Windows/PropVariant.h"


STDAPI GetMethodProperty(UInt32 codecIndex, PROPID propID, PROPVARIANT *value);
STDAPI GetNumberOfMethods(UINT32 *numCodecs);

STDAPI GetHandlerProperty2(UInt32 formatIndex, PROPID propID, PROPVARIANT *value);
STDAPI GetHandlerProperty(PROPID propID, PROPVARIANT *value);
STDAPI GetNumberOfFormats(UINT32 *numFormats);
STDAPI CreateObject(const GUID *clsid, const GUID *iid, void **outObject);
STDAPI SetLargePageMode();

#define __STR2__(x) #x  
#define __STR1__(x) __STR2__(x)  
#define __LOC__ __FILE__ "("__STR1__(__LINE__)") : Warning Msg: " 

#if USE_STATIC_7Z_COMPONENT
#pragma message(__LOC__"static 7z src lib")
#else
#pragma message(__LOC__"dynamic 7z lib")
#endif // USE_STATIC_7Z_COMPONENT


#endif