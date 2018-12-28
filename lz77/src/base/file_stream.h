#ifndef BASE_FILE_STREAM_H_
#define BASE_FILE_STREAM_H_

#include <cstdint>
#include <fstream>
#include <memory>
#include "base/base_export.h"
#include "base/basictypes.h"

namespace base{
  static const size_t kFileChunk = 1024 * 1024 * 16;
  class FileStream
  {
  public:
    BASE_EXPORT FileStream();
    BASE_EXPORT virtual ~FileStream();
  protected:
    BASE_EXPORT bool Open(const char* filename);
    BASE_EXPORT bool IsEOF();
    BASE_EXPORT void Close();
    BASE_EXPORT void Read();
    BASE_EXPORT virtual size_t ReadingProcess(const uint8_t* bytes,const size_t len) = 0;
    BASE_EXPORT virtual size_t ReadingStream(uint8_t* bytes, const size_t len);
  private:
    std::ifstream big_file_;
    size_t processed_size_;
    size_t file_size_;
  };
}

#endif