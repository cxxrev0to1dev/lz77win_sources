#ifndef BASE_BASIC_INCLS_H_
#define BASE_BASIC_INCLS_H_

#if (defined(_WIN32)||defined(WIN32)) 
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <SDKDDKVer.h>
#include <WinSock2.h>
#include <ws2ipdef.h>
#include <WS2tcpip.h>
#pragma comment(lib,"OleAut32.lib")
#pragma comment(lib,"Ole32.lib")
#pragma comment(lib,"Comdlg32.lib")
#pragma comment(lib,"Mpr.lib")
#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"user32.lib")	
#pragma comment(lib,"ws2_32.lib")	
#pragma comment(lib,"Advapi32.lib")
#pragma comment(lib,"zlib.lib")
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#if (defined(_WIN32)||defined(WIN32)) 
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#ifndef S_ISDIR
#define S_ISDIR(x) (((x) & S_IFMT) == S_IFDIR)
#endif
#else
#include <sys/stat.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#endif

#if (defined(_WIN32)||defined(WIN32))
#define stat _stat
#define fstat _fstat
#define open _open
#define close _close
#define O_RDONLY _O_RDONLY
#endif

#if (defined(_WIN32)||defined(WIN32))
#define EVENT__HAVE_OPENSSL
#endif

typedef union{
  struct sockaddr_storage ss;
  struct sockaddr sa;
  struct sockaddr_in in;
  struct sockaddr_in6 i6;
} sock_hop;

#endif