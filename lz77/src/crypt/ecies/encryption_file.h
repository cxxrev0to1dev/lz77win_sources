#ifndef ECIES_ENCRYPTION_FILE_H_
#define ECIES_ENCRYPTION_FILE_H_

#if defined(OS_WIN)
#include "crypt/crypt_export.h"
#endif
#include "base/basictypes.h"
#include "base/file_stream.h"

namespace Crypt {
  class EncryptionFile :public base::FileStream
  {
  public:
    CRYPT_EXPORT EncryptionFile();
    CRYPT_EXPORT virtual ~EncryptionFile();
    CRYPT_EXPORT void CryptFile(const std::string& pubk,const char* src, const char* dst);
  protected:
    virtual size_t ReadingProcess(const uint8_t* bytes, const size_t len);
  private:
    void Close();
    std::ofstream out_file_;
    std::string pubk_;
  };
}

#endif