// lz77_right_menu.cpp : Implementation of CLz77RMenu

#include "stdafx.h"
#include "lz77_right_menu.h"
#include <ShlObj.h>
#include <cstdint>
#include <winerror.h>
#include "base/string_conv.h"


//MOD:FIXME:LdrpInitializeThread crash

size_t ExecuteProcess(std::wstring FullPathToExe, std::wstring Parameters, size_t SecondsToWait)
{
  size_t iMyCounter = 0, iReturnVal = 0, iPos = 0;
  DWORD dwExitCode = 0;
  std::wstring sTempStr = L"";

  /* - NOTE - You should check here to see if the exe even exists */

  /* Add a space to the beginning of the Parameters */
  if (Parameters.size() != 0)
  {
    if (Parameters[0] != L' ')
    {
      Parameters.insert(0, L" ");
    }
  }

  /* The first parameter needs to be the exe itself */
  sTempStr = FullPathToExe;
  iPos = sTempStr.find_last_of(L"\\");
  sTempStr.erase(0, iPos + 1);
  Parameters = sTempStr.append(Parameters);

  /* CreateProcessW can modify Parameters thus we allocate needed memory */
  wchar_t * pwszParam = new wchar_t[Parameters.size() + 1];
  if (pwszParam == 0)
  {
    return 1;
  }
  const wchar_t* pchrTemp = Parameters.c_str();
  wcscpy_s(pwszParam, Parameters.size() + 1, pchrTemp);

  /* CreateProcess API initialization */
  STARTUPINFOW siStartupInfo;
  PROCESS_INFORMATION piProcessInfo;
  memset(&siStartupInfo, 0, sizeof(siStartupInfo));
  memset(&piProcessInfo, 0, sizeof(piProcessInfo));
  siStartupInfo.cb = sizeof(siStartupInfo);

  if (CreateProcessW(FullPathToExe.c_str(),
    pwszParam, 0, 0, false,
    CREATE_DEFAULT_ERROR_MODE, 0, 0,
    &siStartupInfo, &piProcessInfo) != false)
  {
    /* Watch the process. */
    dwExitCode = WaitForSingleObject(piProcessInfo.hProcess, (SecondsToWait * 1000));
  }
  else
  {
    /* CreateProcess failed */
    iReturnVal = GetLastError();
  }

  /* Free memory */
  delete[]pwszParam;
  pwszParam = 0;

  /* Release handles */
  CloseHandle(piProcessInfo.hProcess);
  CloseHandle(piProcessInfo.hThread);

  return iReturnVal;
}

// CLz77RMenu
STDMETHODIMP CLz77RMenu::Initialize(LPCITEMIDLIST pidlFolder, LPDATAOBJECT pDataObj, HKEY hProgID){
  STGMEDIUM   medium = { TYMED_HGLOBAL };
  FORMATETC   fe = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
  target_files_.resize(0);
  if (SUCCEEDED(pDataObj->GetData(&fe, &medium))) {
    uint32_t file_count = DragQueryFile((HDROP)medium.hGlobal, 0xFFFFFFFF, NULL, 0);
    for (uint32_t i = 0;i < file_count;i++) {
      wchar_t szPath[MAX_PATH] = {0};
      DragQueryFile((HDROP)medium.hGlobal, i, szPath, sizeof(szPath));
      target_files_.push_back(szPath);
    }
  }
  ReleaseStgMedium(&medium);
  return S_OK;
}


STDMETHODIMP CLz77RMenu::QueryContextMenu(HMENU hmenu, UINT uMenuIndex, UINT uidFirstCmd, UINT uidLastCmd, UINT uFlags)
{
  UINT uCmdId = uidFirstCmd;
  if (uFlags & CMF_DEFAULTONLY) {
    return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, 0);
  }
  HMENU hSubMenu = CreateMenu();
  if (hSubMenu) {
    for (size_t i = 0; i < (size_t)lang::WIN_MENU_TABLE::kMAX_MENU && 
      i < lang::LZ77Language::GetInstannce()->SubMenuCount(); 
      i++){
      const std::wstring menu_str = lang::LZ77Language::GetInstannce()->IndexVectorMenu(i);
      InsertMenuW(hSubMenu, i, MF_STRING | MF_BYPOSITION, uCmdId++, menu_str.c_str());
    }
  }
  const std::wstring menu_str = lang::LZ77Language::GetInstannce()->MENU_ROOT();
  InsertMenuW(hmenu, uMenuIndex, MF_BYPOSITION | MF_STRING | MF_POPUP, (UINT_PTR)hSubMenu, menu_str.c_str());
  return MAKE_HRESULT(SEVERITY_SUCCESS, FACILITY_NULL, uCmdId - uidFirstCmd + 1);
}

STDMETHODIMP CLz77RMenu::GetCommandString(UINT_PTR idCmd, UINT uFlags, UINT* pwReserved, LPSTR pszName, UINT cchMax) {
  return S_OK;
}

STDMETHODIMP CLz77RMenu::InvokeCommand(LPCMINVOKECOMMANDINFO pCmdInfo) {
  if (0 != HIWORD(pCmdInfo->lpVerb)) {
    return E_INVALIDARG;
  }
  //
  wchar_t dll_folder[MAX_PATH + 1] = {0};
  GetModuleFileNameW((HMODULE)&__ImageBase, dll_folder, MAX_PATH);
  std::wstring dll_dirs(dll_folder);
  std::wstring::size_type pos = std::wstring(dll_dirs).find_last_of(L"\\/");
  dll_dirs = std::wstring(dll_dirs).substr(0, pos);
  dll_dirs.append(L"\\");
  dll_dirs.append(kLz77InvokeCmd);
  //
  const std::wstring&  invoke_command_xml = base::StringConv::widen(std::tmpnam(nullptr));
  std::vector<std::wstring> target_files = target_files_;
  target_files_.resize(0);
  //
  switch (LOWORD(pCmdInfo->lpVerb)) {
  case (DWORD)lang::WIN_MENU_TABLE::kMENU_ARCHIVE:
    lang::LZ77Language::GetInstannce()->WriteInvokeCommand(invoke_command_xml,
      target_files, lang::kStrMenuArchive);
    break;
  case (DWORD)lang::WIN_MENU_TABLE::kMENU_EXTRACT:
    lang::LZ77Language::GetInstannce()->WriteInvokeCommand(invoke_command_xml,
      target_files, lang::kStrMenuExtract);
    break;
  case (DWORD)lang::WIN_MENU_TABLE::kMENU_SENDTO:
    lang::LZ77Language::GetInstannce()->WriteInvokeCommand(invoke_command_xml,
      target_files, lang::kStrMenuSendto);
    break;
  default:
    return E_INVALIDARG;
    break;
  }
  size_t result = ExecuteProcess(dll_dirs, invoke_command_xml, 0);
  return (result ? E_INVALIDARG : S_OK);
}