/**
* Author:    DengTao
* Created:   2017.08.07
*
* (c) Copyright by DengTao.
**/
#include "ecies/decrypt_message.h"

#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/hmac.h>

namespace Crypt {
  DecryptMessage::DecryptMessage(const std::string& private_key){
    key_.SetECKey(private_key);
    salt_.resize(0);
    checksum_.resize(0);
  }
  DecryptMessage::DecryptMessage(ECKeyGen& key_gen) {
    key_.SetECKey(key_gen, true);
    salt_.resize(0);
    checksum_.resize(0);
  }
  DecryptMessage::~DecryptMessage(){
    salt_.resize(0);
    checksum_.resize(0);
  }
  bool DecryptMessage::DecryptBytes(const std::vector<std::uint8_t>& bytes){
    key_.ECKeyPrivateDeriveRS();
    key_.PrivateS();
    std::vector<std::uint8_t> private_s = key_.private_s();
    const EVP_MD *md = EVP_sha1();
    const EVP_CIPHER *cipher = EVP_aes_256_cbc();
    size_t ke_len = EVP_CIPHER_key_length(cipher) + EVP_CIPHER_iv_length(cipher);
    size_t km_len = EVP_MD_block_size(md);
    unsigned char *ke_km = new unsigned char[ke_len + km_len];
    memset(ke_km, 0, ke_len + km_len);
    unsigned char *dc_out = new unsigned char[bytes.size() * 5];
    assert(ke_km != nullptr&&dc_out != nullptr);
    memset(dc_out, 0, bytes.size() * 5);
    size_t dc_len = 0;
    int outl = 0;

    PKCS5_PBKDF2_HMAC((const char*)&private_s[0], private_s.size(), &salt_[0], salt_.size(), kHMACIter, md, ke_len + km_len, ke_km);

    unsigned char *dv_out = new unsigned char[km_len];
    assert(dv_out != nullptr);
    unsigned int dv_len;
    HMAC(md, ke_km + ke_len, km_len, &bytes[0], bytes.size(), dv_out, &dv_len);

    if (checksum_.size() != dv_len || memcmp(dv_out, &checksum_[0], dv_len) != 0){
      printf("MAC verification failed\n");
      return true;
    }

    EVP_CIPHER_CTX *ectx = EVP_CIPHER_CTX_new();

    EVP_DecryptInit_ex(ectx, cipher, NULL, ke_km, ke_km + EVP_CIPHER_key_length(cipher));
    EVP_DecryptUpdate(ectx, dc_out + dc_len, &outl, &bytes[0], bytes.size());
    dc_len += outl;
    EVP_DecryptFinal_ex(ectx, dc_out + dc_len, &outl);
    dc_len += outl;
    dc_out[dc_len] = 0;
    clear_text_.resize(dc_len);
    memmove(&clear_text_[0], dc_out, dc_len);
    delete[] dc_out;
    delete[] ke_km;
    delete[] dv_out;
    return 0;
  }
  void DecryptMessage::SetPublicR(const std::vector<std::uint8_t>& s3){
    key_.SetPublicR(s3);
  }
}