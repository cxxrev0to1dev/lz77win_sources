#ifndef ECIES_BIGNUM_KEY_H_
#define ECIES_BIGNUM_KEY_H_

#include <cassert>
#include <cstdint>
#include <vector>
#include <string>

#if defined(OS_WIN)
#include "crypt/crypt_export.h"
#endif
#include "base/basictypes.h"

namespace Crypt{
  const int kHMACIter = 2000;
  class BigNumKeyHandle {
    void* id;
  public:
    BigNumKeyHandle(void* h) : id(h) { }
    void reset(void* h) {
      id = h;
    }
    void* get() {
      return id;
    }
    BigNumKeyHandle& operator =(void* r) {
      id = r;
      return *this;
    }
    friend bool operator ==(BigNumKeyHandle l, BigNumKeyHandle r) {
      return l.id == r.id;
    }
  };
  class ECKeyGen
  {
  public:
    CRYPT_EXPORT ECKeyGen();
    CRYPT_EXPORT explicit ECKeyGen(bool is_init);
    CRYPT_EXPORT virtual ~ECKeyGen();
    CRYPT_EXPORT void ResetRandomSeed();
    CRYPT_EXPORT const char* pub_key() const {
      return pub_key_.c_str();
    }
    CRYPT_EXPORT const char* priv_key() const {
      return priv_key_.c_str();
    }
    CRYPT_EXPORT void set_pub_key(const std::string key) {
      pub_key_ = key;
    }
    CRYPT_EXPORT void set_priv_key(const std::string key) {
      priv_key_ = key;
    }
  private:
    void GenerateKeyPairString(std::string & pub_key, std::string & priv_key);
    void GenerateKeyPairFile(std::string & pub_key, std::string & priv_key);
    std::string pub_key_;
    std::string priv_key_;
  };

  class BigNumKey
  {
  public:
    BigNumKey();
    ~BigNumKey();
    bool SetECKey(ECKeyGen& key_gen, bool is_priv);
    bool SetECKey(const std::string& private_key);
    bool ECKeyPublicDeriveRS();
    bool ECKeyPrivateDeriveRS();
    void PublicR();
    void PublicS();
    void PrivateR();
    void PrivateS();
    void ReversePublicR();
    std::vector<std::uint8_t> public_s(){
      return public_s_;
    }
    std::vector<std::uint8_t> public_r(){
      return public_r_;
    }
    void SetPublicR(const std::vector<std::uint8_t>& r){
      public_r_ = r;
    }
    std::vector<std::uint8_t> private_s(){
      return private_s_;
    }
    std::vector<std::uint8_t> private_r(){
      return private_r_;
    }
    BigNumKeyHandle public_handle_s(){
      return public_handle_s_;
    }
    BigNumKeyHandle public_handle_r(){
      return public_handle_r_;
    }
    BigNumKeyHandle private_handle_s(){
      return private_handle_s_;
    }
    BigNumKeyHandle private_handle_r(){
      return private_handle_r_;
    }
    BigNumKeyHandle ec_key_handle(){
      return ec_key_handle_;
    }
  private:
    std::vector<std::uint8_t> public_s_;
    std::vector<std::uint8_t> public_r_;
    std::vector<std::uint8_t> private_s_;
    std::vector<std::uint8_t> private_r_;
    BigNumKeyHandle public_handle_s_;
    BigNumKeyHandle public_handle_r_;
    BigNumKeyHandle private_handle_s_;
    BigNumKeyHandle private_handle_r_;
    BigNumKeyHandle ec_key_handle_;
  };
}

#endif