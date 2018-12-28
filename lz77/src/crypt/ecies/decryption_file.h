#ifndef ECIES_DECRYPTION_FILE_H_
#define ECIES_DECRYPTION_FILE_H_

#if defined(OS_WIN)
#include "crypt/crypt_export.h"
#endif
#include "base/basictypes.h"
#include "base/file_stream.h"


namespace Crypt {
  class DecryptionFile :public base::FileStream
  {
  public:
    CRYPT_EXPORT DecryptionFile();
    CRYPT_EXPORT virtual ~DecryptionFile();
    CRYPT_EXPORT void CryptFile(const std::string& privk, const char* src, const char* dst);
  protected:
    virtual size_t ReadingProcess(const uint8_t* bytes, const size_t len);
  private:
    void Close();
    std::ofstream out_file_;
    std::string privk_;
  };
}

#endif