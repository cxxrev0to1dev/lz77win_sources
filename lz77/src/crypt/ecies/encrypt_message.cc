/**
* Author:    DengTao
* Created:   2017.08.07
*
* (c) Copyright by DengTao.
**/
#include "ecies/encrypt_message.h"

#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/hmac.h>

#include "compressor/zlib_compressor.h"

namespace Crypt {
  EncryptMessage::EncryptMessage(const std::string& private_key){
    key_.SetECKey(private_key);
    salt_.resize(32);
    checksum_.resize(0);
    cipher_text_.resize(0);
    RAND_bytes(&salt_[0], salt_.size());
  }
  EncryptMessage::EncryptMessage(ECKeyGen& key_gen) {
    key_.SetECKey(key_gen,false);
    salt_.resize(32);
    checksum_.resize(0);
    cipher_text_.resize(0);
    RAND_bytes(&salt_[0], salt_.size());
  }
  EncryptMessage::~EncryptMessage(){
    salt_.resize(0);
    checksum_.resize(0);
    cipher_text_.resize(0);
  }
  bool EncryptMessage::EncryptString(const std::string& str){
    std::vector<std::uint8_t> v;
    std::copy(str.begin(), str.end(), std::back_inserter(v));
    return EncryptBytes(v);
  }
  bool EncryptMessage::EncryptBytes(const std::vector<std::uint8_t>& bytes){
    unsigned char *c_out = new unsigned char[bytes.size() * 5]; size_t c_len;
    unsigned char *d_out = new unsigned char[bytes.size() * 5]; size_t d_len;
    assert(c_out != nullptr&&d_out != nullptr);
    memset(c_out, 0, bytes.size() * 5);
    memset(d_out, 0, bytes.size() * 5);
    const std::vector<std::uint8_t> loc_salt = salt();
    key_.ECKeyPublicDeriveRS();
    key_.PublicS();
    std::vector<std::uint8_t> public_s = key_.public_s();
    const EVP_MD *md = EVP_sha1();
    const EVP_CIPHER *cipher = EVP_aes_256_cbc();
    size_t ke_len = EVP_CIPHER_key_length(cipher) + EVP_CIPHER_iv_length(cipher);
    size_t km_len = EVP_MD_block_size(md);
    unsigned char *ke_km = new unsigned char[ke_len + km_len];
    assert(ke_km != nullptr);
    memset(ke_km, 0, ke_len + km_len);
    c_len = 0;
    int outl = 0;
    PKCS5_PBKDF2_HMAC((const char*)(const char*)&public_s[0], public_s.size(), &loc_salt[0], loc_salt.size(), kHMACIter, md, ke_len + km_len, ke_km);
    EVP_CIPHER_CTX *ectx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ectx, cipher, NULL, ke_km, ke_km + EVP_CIPHER_key_length(cipher));

    EVP_EncryptUpdate(ectx, c_out + c_len, &outl, (const unsigned char*)&bytes[0], bytes.size());

    c_len += outl;
    EVP_EncryptFinal_ex(ectx, c_out + c_len, &outl);
    c_len += outl;
    unsigned int len;
    HMAC(md, ke_km + ke_len, km_len, c_out, c_len, d_out, &len);
    d_len = len;
    checksum_.resize(d_len);
    cipher_text_.resize(c_len);
    memmove(&cipher_text_[0], c_out, c_len);
    memmove(&checksum_[0], d_out, d_len);
    key_.ReversePublicR();
    public_r_ = key_.public_r();
    delete[] c_out;
    delete[] d_out;
    delete[] ke_km;
    return false;
  }
}