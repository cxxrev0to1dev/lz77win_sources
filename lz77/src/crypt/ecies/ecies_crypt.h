#ifndef ECIES_ECIES_CRYPT_H_
#define ECIES_ECIES_CRYPT_H_

#if defined(OS_WIN)
#include "crypt/crypt_export.h"
#endif
#include "base/basictypes.h"
#include "ecies/ecies_secure_policy.h"

namespace Crypt {
  class ECIESCrypt:public ECIESSecurePolicy
  {
  public:
    CRYPT_EXPORT ECIESCrypt();
    CRYPT_EXPORT virtual ~ECIESCrypt();
    CRYPT_EXPORT void Reset();
    //CRYPT_EXPORT void ecies_encrypt(char *key, unsigned char *data, size_t length);
    //CRYPT_EXPORT void ecies_decrypt(char *key, unsigned char** out, size_t *length);
  private:
    secure_t *cryptex_;
  };
}

#endif