/**
* Author:    DengTao
* Created:   2017.08.07
*
* (c) Copyright by DengTao.
**/
#include "ecies/bignum_key.h"

#include <openssl/err.h>
#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/bn.h>
#include <openssl/hmac.h>
#include <openssl/engine.h>


namespace Crypt {
  EC_POINT *EC_POINT_mult_BN(const EC_GROUP *group, EC_POINT *P, const EC_POINT *a, const BIGNUM *b, BN_CTX *ctx) {
    EC_POINT *O = EC_POINT_new(group);
    if (P == NULL) P = EC_POINT_new(group);
    for (int i = BN_num_bits(b); i >= 0; i--) {
      EC_POINT_dbl(group, P, P, ctx);
      if (BN_is_bit_set(b, i))
        EC_POINT_add(group, P, P, a, ctx);
      else
        EC_POINT_add(group, P, P, O, ctx);
    }
    return P;
  }
  ECKeyGen::ECKeyGen() {
    pub_key_.clear();
    priv_key_.clear();
    GenerateKeyPairString(pub_key_, priv_key_);
  }
  ECKeyGen::ECKeyGen(bool is_init) {
    if (is_init) {
      GenerateKeyPairString(pub_key_, priv_key_);
    }
  }
  ECKeyGen::~ECKeyGen() {
    pub_key_.resize(0);
    pub_key_.clear();
    priv_key_.resize(0);
    priv_key_.clear();
  }
  void ECKeyGen::ResetRandomSeed() {
    srand(time(NULL));
    int min = 1;
    int max = 100;
    int num = (min + (rand() % (int)(max - min + 1)));
    for (size_t i = num; i; i--) {
      pub_key_.clear();
      priv_key_.clear();
      GenerateKeyPairString(pub_key_, priv_key_);
    }
  }

  void ECKeyGen::GenerateKeyPairString(std::string & pub_key, std::string & priv_key) {
    BN_CTX* ctx = BN_CTX_secure_new();
    EC_KEY *ec_key = EC_KEY_new_by_curve_name(NID_secp521r1);//NID_secp521r1//NID_X9_62_prime256v1
    if (!EC_KEY_generate_key(ec_key) || !EC_KEY_check_key(ec_key))
      return;
    char* key = BN_bn2hex(EC_KEY_get0_private_key(ec_key));
    priv_key = key;
    const EC_POINT *lp = EC_KEY_get0_public_key(ec_key);
    pub_key = EC_POINT_point2hex(EC_KEY_get0_group(ec_key), lp, POINT_CONVERSION_UNCOMPRESSED, ctx);
    OPENSSL_free(key);
    EC_KEY_free(ec_key);
    BN_CTX_free(ctx);
  }
  void ECKeyGen::GenerateKeyPairFile(std::string & pub_key, std::string & priv_key) {
    EC_KEY *ec_key = EC_KEY_new_by_curve_name(NID_secp521r1);
    assert(1 == EC_KEY_generate_key(ec_key));
    assert(1 == EC_KEY_check_key(ec_key));
    BIO * bio = BIO_new(BIO_s_file());
    BIO_write_filename(bio, (void*)pub_key.c_str());
    PEM_write_bio_EC_PUBKEY(bio, ec_key);
    BIO_write_filename(bio, (void*)priv_key.c_str());
    PEM_write_bio_ECPrivateKey(bio, ec_key, NULL, NULL, 0, NULL, NULL);
    BIO_free_all(bio);
    EC_KEY_free(ec_key);
  }
  BigNumKey::BigNumKey() :
    public_handle_s_(0),
    public_handle_r_(0),
    private_handle_s_(0),
    private_handle_r_(0),
    ec_key_handle_(0){
    public_s_.resize(0);
    public_r_.resize(0);
    private_s_.resize(0);
    private_r_.resize(0);
  }
  BigNumKey::~BigNumKey(){
    public_s_.resize(0);
    public_r_.resize(0);
    private_s_.resize(0);
    private_r_.resize(0);
    BN_free(reinterpret_cast<BIGNUM*>(public_handle_r_.get()));
    BN_free(reinterpret_cast<BIGNUM*>(public_handle_s_.get()));
    BN_free(reinterpret_cast<BIGNUM*>(private_handle_r_.get()));
    BN_free(reinterpret_cast<BIGNUM*>(private_handle_s_.get()));
  }
  bool BigNumKey::SetECKey(ECKeyGen& key_gen, bool is_priv) {
    std::string ec_key_str = is_priv ? key_gen.priv_key() : key_gen.pub_key();
    if (is_priv) {
      BIGNUM *priv = NULL;
      BN_hex2bn(&priv, ec_key_str.c_str());
      EC_KEY* eckey = EC_KEY_new_by_curve_name(NID_secp521r1);;
      EC_KEY_set_private_key(eckey, priv);
      ec_key_handle_ = eckey;
    }
    else {
      BIGNUM *priv = NULL;
      BN_hex2bn(&priv, key_gen.priv_key());
      EC_KEY* eckey = EC_KEY_new_by_curve_name(NID_secp521r1);
      EC_KEY_set_private_key(eckey, priv);
      EC_POINT* pub = NULL;
      pub = EC_POINT_hex2point(EC_KEY_get0_group(eckey), ec_key_str.c_str(), pub, NULL);
      EC_KEY_set_public_key(eckey, pub);
      ec_key_handle_ = eckey;
    }
    return true;
  }
  bool BigNumKey::SetECKey(const std::string& private_key){
    BIO *b = BIO_new_mem_buf((void*)private_key.c_str(), private_key.size());
    EVP_PKEY *pkey = NULL;
    if (private_key.find("PRIVATE") != std::string::npos)
      PEM_read_bio_PrivateKey(b, &pkey, NULL, NULL);
    else
      PEM_read_bio_PUBKEY(b, &pkey, NULL, NULL);
    ec_key_handle_ = (BigNumKeyHandle)EVP_PKEY_get1_EC_KEY(pkey);
    BIO_free(b);
    EVP_PKEY_free(pkey);
    return true;
  }
  bool BigNumKey::ECKeyPublicDeriveRS(){
    //////////////////////////////////////////////////////////////////////////
    public_handle_r_ = (BigNumKeyHandle)BN_new();
    public_handle_s_ = (BigNumKeyHandle)BN_new();
    BN_CTX *ctx = BN_CTX_new();
    const EC_GROUP *group = EC_KEY_get0_group(reinterpret_cast<EC_KEY*>(ec_key_handle_.get()));
    const EC_POINT *Kb = EC_KEY_get0_public_key(reinterpret_cast<EC_KEY*>(ec_key_handle_.get()));
    BIGNUM *n = BN_new();
    BIGNUM *r = BN_new();
    EC_POINT *P = NULL;
    EC_POINT *Rp = EC_POINT_new(group);
    BIGNUM *Py = BN_new();
    const EC_POINT *G = EC_GROUP_get0_generator(group);
    int bits, ret = -1;
    EC_GROUP_get_order(group, n, ctx);
    bits = BN_num_bits(n);
    BN_rand(r, bits, -1, 0);
    /* calculate R = rG */
    Rp = EC_POINT_mult_BN(group, Rp, G, r, ctx);
    /* calculate S = Px, P = (Px,Py) = Kb R */
    P = EC_POINT_mult_BN(group, P, Kb, r, ctx);
    if (!EC_POINT_is_at_infinity(group, P)) {
      EC_POINT_get_affine_coordinates_GF2m(group, P, reinterpret_cast<BIGNUM*>(public_handle_s_.get()), Py, ctx);
      EC_POINT_point2bn(group, Rp, POINT_CONVERSION_COMPRESSED, reinterpret_cast<BIGNUM*>(public_handle_r_.get()), ctx);
      ret = 0;
    }
    BN_free(r);
    BN_free(n);
    BN_free(Py);
    EC_POINT_free(P);
    EC_POINT_free(Rp);
    BN_CTX_free(ctx);
    return (ret!=0);
  }
  bool BigNumKey::ECKeyPrivateDeriveRS(){
    //ReversePublicR();
    private_handle_r_ = (BigNumKeyHandle)BN_bin2bn(&public_r_[0], public_r_.size(), BN_new());
    private_handle_s_ = (BigNumKeyHandle)BN_new();
    int ret = -1;
    BN_CTX *ctx = BN_CTX_new();
    BIGNUM *n = BN_new();
    BIGNUM *Py = BN_new();
    const EC_GROUP *group = EC_KEY_get0_group(reinterpret_cast<EC_KEY*>(ec_key_handle_.get()));
    EC_POINT *Rp = EC_POINT_bn2point(group, reinterpret_cast<BIGNUM*>(private_handle_r_.get()), NULL, ctx);
    const BIGNUM *kB = EC_KEY_get0_private_key(reinterpret_cast<EC_KEY*>(ec_key_handle_.get()));
    EC_GROUP_get_order(group, n, ctx);
    /* Calculate S = Px, P = (Px, Py) = R kB */
    EC_POINT *P = EC_POINT_mult_BN(group, NULL, Rp, kB, ctx);
    if (!EC_POINT_is_at_infinity(group, P)) {
      EC_POINT_get_affine_coordinates_GF2m(group, P, reinterpret_cast<BIGNUM*>(private_handle_s_.get()), Py, ctx);
      ret = 0;
    }
    BN_free(n);
    BN_free(Py);
    EC_POINT_free(Rp);
    EC_POINT_free(P);
    BN_CTX_free(ctx);
    return (ret!=0);
  }
  void BigNumKey::PublicR(){
    public_r_.resize(BN_num_bytes(reinterpret_cast<BIGNUM*>(public_handle_r_.get())));
    BN_bn2bin(reinterpret_cast<const BIGNUM*>(public_handle_r_.get()), &public_r_[0]);
  }
  void BigNumKey::PublicS(){
    public_s_.resize(BN_num_bytes(reinterpret_cast<BIGNUM*>(public_handle_s_.get())));
    BN_bn2bin(reinterpret_cast<const BIGNUM*>(public_handle_s_.get()), &public_s_[0]);
  }
  void BigNumKey::PrivateR(){
    private_r_.resize(BN_num_bytes(reinterpret_cast<BIGNUM*>(private_handle_r_.get())));
    BN_bn2bin(reinterpret_cast<BIGNUM*>(private_handle_r_.get()), &private_r_[0]);
  }
  void BigNumKey::PrivateS(){
    private_s_.resize(BN_num_bytes(reinterpret_cast<BIGNUM*>(private_handle_s_.get())));
    BN_bn2bin(reinterpret_cast<BIGNUM*>(private_handle_s_.get()), &private_s_[0]);
  }
  void BigNumKey::ReversePublicR(){
    public_r_.resize(BN_num_bytes(reinterpret_cast<BIGNUM*>(public_handle_r_.get())));
    BN_bn2bin(reinterpret_cast<BIGNUM*>(public_handle_r_.get()), &public_r_[0]);
  }
}