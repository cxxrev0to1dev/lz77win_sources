#ifndef LZ77LANG_LZ77_LANG_H_
#define LZ77LANG_LZ77_LANG_H_

#include "Lz77Lang/lz77_lang_exports.h"
#include <string>
#include <vector>
#include "Lz77Lang/ui_windows.h"
#include "Lz77Lang/error_msg_table.h"

namespace lang{
  static const wchar_t kResDirName[] = L"res";
#if defined(OS_WIN)
#if defined(OS_WIN_X86)
  static const wchar_t kModuleName[] = L"Lz77Lang.dll";
#else
  static const wchar_t kModuleName[] = L"Lz77Lang_x64.dll";
#endif
#else
  static const wchar_t kModuleName[] = L"Lz77Lang_x64.so";
#endif

  static const char* kConstStrInvokeCommand = "InvokeCommand";
  static const char* kConstStrCommand = "Command";
  static const char* kConstStrPathList = "PathList";
  static const char* kConstStrOp = "Op";
  static const char* kConstStrPath = "Path";
  static const char* kConstStrString = "String";

  enum class WIN_MENU_TABLE
  {
    kMENU_ARCHIVE = 0,
    kMENU_EXTRACT,
    kMENU_SENDTO,
    kMAX_MENU
  };
  
  static const wchar_t* kStrMenuRoot = L"MENU_ROOT";
  static const wchar_t* kStrMenuArchive = L"MENU_ARCHIVE";
  static const wchar_t* kStrMenuExtract = L"MENU_EXTRACT";
  static const wchar_t* kStrMenuSendto = L"MENU_SENDTO";

  class MenuVFTable
  {
  public:
    virtual const std::wstring& MENU_ROOT() = 0;
    virtual const std::wstring& MENU_ARCHIVE() = 0;
    virtual const std::wstring& MENU_EXTRACT() = 0;
    virtual const std::wstring& MENU_SENDTO() = 0;
  };

  class LZ77Language:public MenuVFTable{
  public:
    LZ77_LANG_EXPORT static LZ77Language* GetInstannce();
    LZ77_LANG_EXPORT LZ77Language(void);
    LZ77_LANG_EXPORT virtual ~LZ77Language();
    LZ77_LANG_EXPORT std::uint32_t SubMenuCount();
    LZ77_LANG_EXPORT virtual const std::wstring& MENU_ROOT();
    LZ77_LANG_EXPORT virtual const std::wstring& MENU_ARCHIVE();
    LZ77_LANG_EXPORT virtual const std::wstring& MENU_EXTRACT();
    LZ77_LANG_EXPORT virtual const std::wstring& MENU_SENDTO();
    LZ77_LANG_EXPORT const std::wstring& IndexVectorMenu(int i);
    LZ77_LANG_EXPORT const std::wstring& GetErrorMsg(const std::wstring& msg_index);
    LZ77_LANG_EXPORT void WriteInvokeCommand(const std::wstring& outfile, 
      const std::vector<std::wstring>& files,
      const std::wstring& instruction);
  private:
    void InitVectorMenu();
#if defined(OS_WIN)
    lang::internal::UIWindows* ui_win_;
#endif
    lang::internal::ErrorMsgTable* error_msg_table_;
    std::wstring temp_;
    std::vector<std::wstring> all_sub_menu_;
    bool is_initia_;
  };
}


#endif