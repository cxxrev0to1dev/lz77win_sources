#include "base/string_conv.h"
#include <algorithm>
#if defined(OS_WIN)
#include <Windows.h>
#endif


namespace base {
  static StringConv string_conv;
  const std::wstring& StringConv::widen(const std::string& str) {
#if defined(OS_WIN)
    string_conv.StrToWStr(str);
    return string_conv.wdst();
#else
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
#endif
  }

  const std::string& StringConv::narrow(const std::wstring& str) {
#if defined(OS_WIN)
    string_conv.WStrToStr(str);
    return string_conv.dst();
#else
    std::ostringstream stm;
    stm.imbue(std::locale("C"));
    const std::ctype<char>& ctfacet =
      std::use_facet< std::ctype<char> >(stm.getloc());
    for (size_t i = 0; i < str.size(); ++i)
      stm << ctfacet.narrow(str[i], 0);
    stm << ctfacet.narrow('\0');
    return stm.str();
#endif
  }
  const std::wstring& StringConv::GetMapW(const std::map<std::wstring, std::wstring>& mapxxx) {
    return string_conv.GetMap(mapxxx);
  }
  StringConv::StringConv(){
    wdst_.resize(0);
    dst_.resize(0);
  }
  StringConv::~StringConv(){
    wdst_.resize(0);
    dst_.resize(0);
  }
  void StringConv::WStrToStr(const std::wstring& str) {
#if defined(OS_WIN)
    int len = WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0, NULL, NULL);
    if (len > 0){
      dst_.resize(len);
      WideCharToMultiByte(CP_UTF8, 0, str.c_str(), str.length(), &dst_[0], len, NULL, NULL);
      dst_.erase(std::remove(dst_.begin(), dst_.end(), '\0'), dst_.end());
    }
#endif
  }
  void StringConv::StrToWStr(const std::string& str) {
#if defined(OS_WIN)
    int len = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    if (len > 0) {
      wdst_.resize(len);
      MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &wdst_[0], len);
      wdst_.erase(std::remove(wdst_.begin(), wdst_.end(), '\0'), wdst_.end());
#endif
    }
  }
  const std::wstring& StringConv::GetMap(const std::map<std::wstring, std::wstring>& mapxxx) {
    std::wstring result;
    result.resize(0);
    wdst_.resize(0);
    std::for_each(mapxxx.begin(),
      mapxxx.end(),
      [&result](const std::map<std::wstring, std::wstring>::value_type& p) {
      result += p.first;
      result += L":";
      result.append(p.second.c_str(), p.second.size());
      result += L"\n";
    });
    wdst_ = result;
    return wdst_;
  }
}