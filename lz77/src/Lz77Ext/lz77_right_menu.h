// lz77_right_menu.h : Declaration of the CLz77RMenu

#pragma once
#include "Lz77Ext_i.h"
#include "resource.h"       // main symbols
#include <comsvcs.h>
#include <ShObjIdl.h>
#include <string>
#include <vector>
#include "Lz77Lang/lz77_lang.h"

using namespace ATL;

extern "C" IMAGE_DOS_HEADER __ImageBase;

#if defined(OS_WIN)
#if defined(OS_WIN_X86)
static const wchar_t kLz77InvokeCmd[] = L"Lz77InvokeCmd.exe";
#else
static const wchar_t kLz77InvokeCmd[] = L"Lz77InvokeCmd_x64.exe";
#endif
#endif


// CLz77RMenu

class ATL_NO_VTABLE CLz77RMenu :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CLz77RMenu, &CLSID_Lz77RMenu>,
  public IShellExtInit,
  public IContextMenu
{
public:
	CLz77RMenu(){ }
  virtual ~CLz77RMenu() {}
BEGIN_COM_MAP(CLz77RMenu)
  COM_INTERFACE_ENTRY(IShellExtInit)
  COM_INTERFACE_ENTRY(IContextMenu)
END_COM_MAP()
public:
  STDMETHOD(Initialize)(LPCITEMIDLIST, LPDATAOBJECT, HKEY);
  STDMETHOD(GetCommandString)(UINT_PTR, UINT, UINT*, LPSTR, UINT);
  STDMETHOD(InvokeCommand)(LPCMINVOKECOMMANDINFO);
  STDMETHOD(QueryContextMenu)(HMENU, UINT, UINT, UINT, UINT);
private:
  std::vector<std::wstring> target_files_;
};