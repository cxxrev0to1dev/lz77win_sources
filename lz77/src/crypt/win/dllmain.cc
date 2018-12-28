/**
* Author:    DengTao
* Created:   2017.08.05
*
* (c) Copyright by DengTao.
**/
#include <Windows.h>
#include <openssl/ssl.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#pragma comment(lib,"libcrypto.lib")
#pragma comment(lib,"libssl.lib")
#include "base/basictypes.h"
#if (defined(_WIN32)||defined(WIN32)) 
BOOL APIENTRY DllMain(HMODULE hModule,DWORD  ul_reason_for_call,LPVOID lpReserved){
  if(ul_reason_for_call== DLL_PROCESS_ATTACH){
    WSADATA WSAData;
    WSAStartup(0x101, &WSAData);
    DisableThreadLibraryCalls(hModule);
    SSL_library_init();
    ERR_load_crypto_strings();
    SSL_load_error_strings();
    OpenSSL_add_all_algorithms();
  }
	return TRUE;
}
#endif
