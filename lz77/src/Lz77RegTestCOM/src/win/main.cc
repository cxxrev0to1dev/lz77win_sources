#include <io.h>
#include <fcntl.h>  
#include <sys/types.h>  
#include <sys/stat.h> 
#include <fstream>
#include <sstream>
#include <process.h>
#include <Windows.h>
#include <commctrl.h>
#include <winerror.h>
#include <ShObjIdl.h>
#include <Shlobj.h>
#include <winreg.h>
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"OleAut32.lib")
#pragma comment(lib,"Ole32.lib")
#pragma comment(lib,"Comdlg32.lib")
#pragma comment(lib,"Mpr.lib")
#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"user32.lib")	
#pragma comment(lib,"ws2_32.lib")	
#pragma comment(lib,"Advapi32.lib")
#include "base/win/registry.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;

void GetDir(std::wstring& dir_path) {
  wchar_t dll_folder[MAX_PATH + 1] = { 0 };
  GetModuleFileNameW((HMODULE)&__ImageBase, dll_folder, MAX_PATH);
  std::wstring dll_dirs(dll_folder);
  std::wstring::size_type pos = std::wstring(dll_dirs).find_last_of(L"\\/");
  dll_dirs = std::wstring(dll_dirs).substr(0, pos);
  dir_path = dll_dirs;
}

int WINAPI wWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPWSTR lpCmdLine,int nShowCmd){
  std::wstring cur_dir;
  GetDir(cur_dir);
#if defined(OS_WIN_X86)
  std::wstring lz77_ext_com = cur_dir + L"\\Lz77Ext.dll";
#else
  std::wstring lz77_ext_com = cur_dir + L"\\Lz77Ext_x64.dll";
#endif
  const wchar_t* key1 = L"SOFTWARE\\Classes\\CLSID\\{0D91A406-64F9-4168-8B6D-F284DC24BFB8}";
  const wchar_t* key2 = L"SOFTWARE\\Classes\\CLSID\\{0D91A406-64F9-4168-8B6D-F284DC24BFB8}\\InprocServer32";
  const wchar_t* hklm_key3 = L"SOFTWARE\\Classes\\*\\shellex\\ContextMenuHandlers\\Lz77Ext";
  const wchar_t* hklm_key4 = L"SOFTWARE\\Classes\\Folder\\ShellEx\\ContextMenuHandlers\\Lz77Ext";
  const wchar_t* guid = L"{0D91A406-64F9-4168-8B6D-F284DC24BFB8}";
  base::win::RegKey RMenu1(HKEY_CURRENT_USER, key1, KEY_CREATE_SUB_KEY | KEY_WRITE);
  RMenu1.DeleteKey(L"InprocServer32");
  if (!RMenu1.Create(HKEY_CURRENT_USER, key2, KEY_CREATE_SUB_KEY | KEY_WRITE)) {
    if (!RMenu1.Open(HKEY_CURRENT_USER, key2, KEY_WRITE)) {
      RMenu1.WriteValue(nullptr, L"");
      RMenu1.WriteValue(nullptr, lz77_ext_com.c_str());
      RMenu1.WriteValue(L"ThreadingModel", L"Apartment");
    }
  }
  if (!RMenu1.Create(HKEY_LOCAL_MACHINE, hklm_key3, KEY_CREATE_SUB_KEY | KEY_WRITE)) {
    RMenu1.WriteValue(nullptr, L"");
    RMenu1.WriteValue(nullptr, guid);
  }
  base::win::RegKey RMenu3(HKEY_LOCAL_MACHINE, key1, KEY_CREATE_SUB_KEY | KEY_WRITE);
  RMenu1.DeleteKey(L"InprocServer32");
  if (!RMenu3.Create(HKEY_LOCAL_MACHINE, key2, KEY_CREATE_SUB_KEY | KEY_WRITE)) {
    if (!RMenu3.Open(HKEY_LOCAL_MACHINE, key2, KEY_ALL_ACCESS)) {
      RMenu3.WriteValue(nullptr, L"");
      RMenu3.WriteValue(nullptr, lz77_ext_com.c_str());
      RMenu3.WriteValue(L"ThreadingModel", L"Apartment");
    }
  }
  if (!RMenu1.Create(HKEY_LOCAL_MACHINE, hklm_key4, KEY_CREATE_SUB_KEY | KEY_WRITE)) {
    if (!RMenu3.Open(HKEY_LOCAL_MACHINE, hklm_key4, KEY_WRITE)) {
      RMenu3.WriteValue(nullptr, L"");
      RMenu3.WriteValue(nullptr, guid);
    }
  }
  return 0;
}