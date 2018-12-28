#include "ecies/ecies_crypt.h"

#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/hmac.h>
#include <openssl/engine.h>
#include <openssl/ossl_typ.h>

#include "ecies/ecies_gen_key.h"

#define ECIES_CIPHER EVP_aes_256_cbc()
#define ECIES_HASHER EVP_sha512()


namespace Crypt {
  void * ecies_key_derivation(const void *input, size_t ilen, void *output,
    size_t *olen) {

    if (*olen < SHA512_DIGEST_LENGTH) {
      return NULL;
    }

    *olen = SHA512_DIGEST_LENGTH;
    return SHA512((const unsigned char*)input, ilen, (unsigned char*)output);
  }

  ECIESCrypt::ECIESCrypt(){
  }
  ECIESCrypt::~ECIESCrypt(){
    Reset();
  }
  void ECIESCrypt::Reset() {
    if (cryptex_) {
      secure_free(cryptex_);
      cryptex_ = nullptr;
    }
  }
}
