#ifndef LZ77LANG_ERROR_MSG_TABLE_H_
#define LZ77LANG_ERROR_MSG_TABLE_H_

#include <map>
#include <string>
#include <istream>

namespace lang{
  namespace internal{
    class ErrorMsgTable
    {
    public:
      explicit ErrorMsgTable(std::basic_istream<char>& in);
      virtual ~ErrorMsgTable();
      const std::wstring& GetErrorMsg(std::wstring msg_index);
    private:
      std::map<std::wstring, std::wstring> error_msg_table_;
      std::wstring temp_;
    };
  }
}

#endif // !LZ77LANG_UI_WINDOWS_H_
