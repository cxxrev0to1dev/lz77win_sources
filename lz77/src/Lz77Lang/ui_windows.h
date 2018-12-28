#ifndef LZ77LANG_LANG_UI_WINDOWS_H_
#define LZ77LANG_LANG_UI_WINDOWS_H_

#include <map>
#include <string>
#include <istream>

namespace lang{
  namespace internal{
    class UIWindows
    {
    public:
      explicit UIWindows(std::basic_istream<char>& in);
      virtual ~UIWindows();
      const std::uint32_t& SubMenuCount() const {
        return sub_menu_count_;
      }
      void MENU_ROOT(std::wstring& str);
      void MENU_ARCHIVE(std::wstring& str);
      void MENU_EXTRACT(std::wstring& str);
      void MENU_SENDTO(std::wstring& str);
    private:
      std::map<std::wstring, std::wstring> ui_win_;
      std::uint32_t sub_menu_count_;
    };
  }
}

#endif // !LZ77LANG_UI_WINDOWS_H_
