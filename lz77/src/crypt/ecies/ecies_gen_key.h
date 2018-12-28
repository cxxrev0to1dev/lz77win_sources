#ifndef ECIES_ECIES_GEN_KEY_H_
#define ECIES_ECIES_GEN_KEY_H_

#if defined(OS_WIN)
#include "crypt/crypt_export.h"
#include "base/basic_incls.h"
#endif

#include "base/basictypes.h"
#include <string>

namespace Crypt {
  template <typename T1, typename T2>
  class ECIESGenKey
  {
  public:
    ECIESGenKey();
    virtual ~ECIESGenKey();
    void ecies_group_init(void);
    void ecies_group_free(void);
    void ecies_key_free(T2 *key);
    T2 * ecies_key_create(void);
    T2 * ecies_key_create_public_hex(char *hex);
    T2 * ecies_key_create_private_hex(char *hex);
    T2 * ecies_key_create_public_octets(unsigned char *octets, size_t length);
    char * ecies_key_public_get_hex(T2 *key);
    char * ecies_key_private_get_hex(T2 *key);
  private:
    T1 * ecies_group(void);
  };
  class ECIESKey
  {
  public:
    CRYPT_EXPORT static ECIESKey* CreateInstance();
    CRYPT_EXPORT static void DeleteInstance();
    CRYPT_EXPORT ECIESKey();
    CRYPT_EXPORT virtual ~ECIESKey();
    CRYPT_EXPORT const char* priv_key() const {
      return priv_key_.c_str();
    }
    CRYPT_EXPORT const char* pub_key() const {
      return pub_key_.c_str();
    }
  private:
    std::string priv_key_;
    std::string pub_key_;
    static ECIESKey* private_instance_;
  };
}

#endif