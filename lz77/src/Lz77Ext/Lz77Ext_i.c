

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 8.00.0603 */
/* at Sat Jul 07 09:47:53 2018
 */
/* Compiler settings for Lz77Ext.idl:
    Oicf, W1, Zp8, env=Win64 (32b run), target_arch=AMD64 8.00.0603 
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
/* @@MIDL_FILE_HEADING(  ) */

#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IComponentRegistrar,0xa817e7a2,0x43fa,0x11d0,0x9e,0x44,0x00,0xaa,0x00,0xb6,0x77,0x0a);


MIDL_DEFINE_GUID(IID, IID_ILz77RMenu,0x2433BF14,0x8909,0x4BD3,0xA4,0x1B,0x38,0x73,0x7E,0xE8,0xF6,0x0B);


MIDL_DEFINE_GUID(IID, LIBID_Lz77ExtLib,0xA14759C8,0x63CE,0x4016,0xAD,0x8E,0xDB,0xD7,0x03,0x3B,0xD3,0x9D);


MIDL_DEFINE_GUID(CLSID, CLSID_CompReg,0x0D91A406,0x64F9,0x4168,0x8B,0x6D,0xF2,0x84,0xDC,0x24,0xBF,0xB8);


MIDL_DEFINE_GUID(CLSID, CLSID_Lz77RMenu,0xE1621F46,0xCB37,0x4E27,0xB1,0x59,0xCA,0xD6,0xF6,0xA4,0x45,0x95);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



