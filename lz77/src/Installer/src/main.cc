#include <string>
#include <ShObjIdl.h>
#include <Shlobj.h>
#include <Windows.h>
#include "installer.h"
#include "installer_x64.h"
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"OleAut32.lib")
#pragma comment(lib,"Ole32.lib")
#pragma comment(lib,"Comdlg32.lib")
#pragma comment(lib,"Mpr.lib")
#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"user32.lib")	
#pragma comment(lib,"ws2_32.lib")	
#pragma comment(lib,"Advapi32.lib")
#if defined(OS_WIN_X86)
#pragma comment(lib,"base.lib")
#pragma comment(lib,"compressor.lib")
#pragma comment(lib,"crypt.lib")
#else
#pragma comment(lib,"base_x64.lib")
#pragma comment(lib,"compressor_x64.lib")
#pragma comment(lib,"crypt_x64.lib")
#endif
#include "base/win/registry.h"
#include "base/string_conv.h"
#include "bit7z/include/bit7zlibrary.hpp"
#include "bit7z/include/bitextractor.hpp"
#include "compressor/lib7zip_compressor.h"


extern "C" IMAGE_DOS_HEADER __ImageBase;

void GetDir(std::wstring& dir_path) {
  wchar_t dll_folder[MAX_PATH + 1] = { 0 };
  GetModuleFileNameW((HMODULE)&__ImageBase, dll_folder, MAX_PATH);
  std::wstring dll_dirs(dll_folder);
  std::wstring::size_type pos = std::wstring(dll_dirs).find_last_of(L"\\/");
  dll_dirs = std::wstring(dll_dirs).substr(0, pos);
  dir_path = dll_dirs;
}


bool GetFolder(HWND hwnd, std::wstring& out_dirs) {
  LPITEMIDLIST pidl;
  LPSHELLFOLDER g_psfDesktop;
  if (FAILED(SHGetDesktopFolder(&g_psfDesktop))) {
    return true;
  }
  HRESULT hres = SHGetSpecialFolderLocation(hwnd, CSIDL_PROGRAM_FILES, &pidl);
  if (SUCCEEDED(hres)) {
    wchar_t olePath[MAX_PATH] = { 0 };
    SHGetPathFromIDListW((LPCITEMIDLIST)pidl, olePath);
    out_dirs = olePath;
    CoTaskMemFree(pidl);
  }
  return false;
}
bool GetProgramFilesFolder(std::wstring& out_dirs) {
  return GetFolder(GetDesktopWindow(), out_dirs);
}

void AddCOM(std::wstring lz77_ext_com) {
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
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nShowCmd) {
  std::wstring out_dirs;
  GetProgramFilesFolder(out_dirs);
  out_dirs.append(L"\\LZ77.XYZ");
  CreateDirectoryW(out_dirs.c_str(), nullptr);
  std::string compress_file = std::tmpnam(nullptr);
  compress_file += ".zip";
  FILE* fp = fopen(compress_file.c_str(), "wb");
#if defined(OS_WIN_X86)
  fwrite(hex_c_x86, 1, hex_c_x86_len, fp);
#else
  fwrite(hex_c_x64, 1, hex_c_x64_len, fp);
#endif
  fflush(fp);
  fclose(fp);

  compressor::ArchiveCompressor archive_compressor(nullptr);
  archive_compressor.decompressor(base::StringConv::widen(compress_file.c_str()), out_dirs, L"");
#if defined(OS_WIN_X86)
  const std::wstring lz77_ext_com = out_dirs + L"\\Lz77Ext.dll";
#else
  const std::wstring lz77_ext_com = out_dirs + L"\\Lz77Ext_x64.dll";
#endif
  AddCOM(lz77_ext_com);
  DeleteFileA(compress_file.c_str());
  return 0;
}