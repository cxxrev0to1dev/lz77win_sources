#ifndef LZ77LANG_LANG_STREAM_H_
#define LZ77LANG_LANG_STREAM_H_

#include <cstdio>
#include <string>

namespace lang {
  namespace internal{

    static std::string ReadFile(const wchar_t* fileName)
    {
      std::string strBuffer;
#if defined(OS_WIN)
      FILE *fp = _wfopen(fileName, L"rb");
#endif
      if (NULL == fp){
        return "";
      }
      char buf[1024];
      while (true){
        const size_t byteRead = fread(buf, 1, sizeof(buf), fp);
        if (0 == byteRead){
          break;
        }
        strBuffer.insert(strBuffer.end(), buf, buf + byteRead);
      }
      fclose(fp);
      return strBuffer;
    }
  }
}

#endif