#ifndef BASE_STRING_CONV_H_
#define BASE_STRING_CONV_H_

#include <locale>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <codecvt>
#include <map>
#include "base/base_export.h"
#include "base/basictypes.h"

namespace base {
  class StringConv
  {
  public:
    BASE_EXPORT static const std::wstring& widen(const std::string& str);
    BASE_EXPORT static const std::string& narrow(const std::wstring& str);
    BASE_EXPORT static const std::wstring& GetMapW(const std::map<std::wstring, std::wstring>& mapxxx);
    BASE_EXPORT StringConv();
    BASE_EXPORT virtual ~StringConv();
    BASE_EXPORT void WStrToStr(const std::wstring& str);
    BASE_EXPORT void StrToWStr(const std::string& str);
    BASE_EXPORT const std::wstring& GetMap(const std::map<std::wstring, std::wstring>& map);
    BASE_EXPORT const std::wstring& wdst() const {
      return wdst_;
    }
    BASE_EXPORT const std::string& dst() const {
      return dst_;
    }
  private:
    std::wstring wdst_;
    std::string dst_;
  };
  /*
  static std::wstring widen(const std::string& str) {
    std::wostringstream wstm;
    std::wbuffer_convert<std::codecvt_utf8<wchar_t>> conv(std::cout.rdbuf());
    std::wostream out(&conv);
    wstm.imbue(std::locale(out.getloc(), new std::time_put_byname<wchar_t>("en_US.utf8")));
    const std::ctype<wchar_t>& ctfacet =
      std::use_facet< std::ctype<wchar_t> >(wstm.getloc());
    for (size_t i = 0; i < str.size(); ++i)
      wstm << ctfacet.widen(str[i]);
    wstm << ctfacet.widen(L'\0\0');
    return wstm.str();
  }

  static std::string narrow(const std::wstring& str) {
    std::ostringstream stm;
    stm.imbue(std::locale("C"));
    const std::ctype<char>& ctfacet =
      std::use_facet< std::ctype<char> >(stm.getloc());
    for (size_t i = 0; i < str.size(); ++i)
      stm << ctfacet.narrow(str[i], 0);
    stm << ctfacet.narrow('\0');
    return stm.str();
  }
  */
}

#endif