#include "ecies/ecies_gen_key.h"
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
  static EC_GROUP *eliptic = NULL;
  static const int kECIES_CURVE = NID_secp521r1;
  template <typename T1, typename T2>
  ECIESGenKey<T1,T2>::ECIESGenKey(){
  }
  template <typename T1, typename T2>
  ECIESGenKey<T1, T2>::~ECIESGenKey(){
  }
  
  template <typename T1, typename T2>
  void ECIESGenKey<T1, T2>::ecies_group_init(void) {

    EC_GROUP *group;

    if (!(group = EC_GROUP_new_by_curve_name(kECIES_CURVE))) {
      printf("EC_GROUP_new_by_curve_name failed. {error = %s}\n", ERR_error_string(ERR_get_error(), NULL));
    }

    else if (EC_GROUP_precompute_mult(group, NULL) != 1) {
      printf("EC_GROUP_precompute_mult failed. {error = %s}\n", ERR_error_string(ERR_get_error(), NULL));
      EC_GROUP_free(group);
    }

    EC_GROUP_set_point_conversion_form(group, POINT_CONVERSION_COMPRESSED);
    eliptic = group;

    return;
  }
  template <typename T1, typename T2>
  void ECIESGenKey<T1, T2>::ecies_group_free(void) {

    EC_GROUP *group = eliptic;
    eliptic = NULL;
    if (group) {
      EC_GROUP_free(group);
    }
    return;
  }
  template <typename T1, typename T2>
  void ECIESGenKey<T1, T2>::ecies_key_free(T2 *key) {
    EC_KEY_free(key);
    return;
  }
  template <typename T1, typename T2>
  T2 * ECIESGenKey<T1, T2>::ecies_key_create(void) {

    EC_GROUP *group;
    EC_KEY *key = NULL;

    if (!(key = EC_KEY_new())) {
      printf("EC_KEY_new failed. {error = %s}\n",
        ERR_error_string(ERR_get_error(), NULL));
      return NULL;
    }

    if (!(group = ecies_group())) {
      EC_KEY_free(key);
      return NULL;
    }

    if (EC_KEY_set_group(key, group) != 1) {
      printf("EC_KEY_set_group failed. {error = %s}\n",
        ERR_error_string(ERR_get_error(), NULL));
      EC_GROUP_free(group);
      EC_KEY_free(key);
      return NULL;
    }

    EC_GROUP_free(group);

    if (EC_KEY_generate_key(key) != 1) {
      printf("EC_KEY_generate_key failed. {error = %s}\n",
        ERR_error_string(ERR_get_error(), NULL));
      EC_KEY_free(key);
      return NULL;
    }

    return key;
  }
  template <typename T1, typename T2>
  T2 * ECIESGenKey<T1, T2>::ecies_key_create_public_octets(unsigned char *octets, size_t length) {

    EC_GROUP *group;
    EC_KEY *key = NULL;
    EC_POINT *point = NULL;

    if (!(key = EC_KEY_new())) {
      printf("EC_KEY_new failed. {error = %s}\n",
        ERR_error_string(ERR_get_error(), NULL));
      return NULL;
    }

    if (!(group = ecies_group())) {
      EC_KEY_free(key);
      return NULL;
    }

    if (EC_KEY_set_group(key, group) != 1) {
      printf("EC_KEY_set_group failed. {error = %s}\n",
        ERR_error_string(ERR_get_error(), NULL));
      EC_GROUP_free(group);
      EC_KEY_free(key);
      return NULL;
    }

    if (!(point = EC_POINT_new(group))) {
      printf("EC_POINT_new failed. {error = %s}\n",
        ERR_error_string(ERR_get_error(), NULL));
      EC_GROUP_free(group);
      EC_KEY_free(key);
      return NULL;
    }

    if (EC_POINT_oct2point(group, point, octets, length, NULL) != 1) {
      printf("EC_POINT_oct2point failed. {error = %s}\n",
        ERR_error_string(ERR_get_error(), NULL));
      EC_GROUP_free(group);
      EC_KEY_free(key);
      return NULL;
    }

    if (EC_KEY_set_public_key(key, point) != 1) {
      printf("EC_KEY_set_public_key failed. {error = %s}\n",
        ERR_error_string(ERR_get_error(), NULL));
      EC_GROUP_free(group);
      EC_POINT_free(point);
      EC_KEY_free(key);
      return NULL;
    }

    EC_GROUP_free(group);
    EC_POINT_free(point);

    if (EC_KEY_check_key(key) != 1) {
      printf("EC_KEY_check_key failed. {error = %s}\n",
        ERR_error_string(ERR_get_error(), NULL));
      EC_KEY_free(key);
      return NULL;
    }

    return key;
  }
  template <typename T1, typename T2>
  T2 * ECIESGenKey<T1, T2>::ecies_key_create_public_hex(char *hex) {

    EC_GROUP *group;
    EC_KEY *key = NULL;
    EC_POINT *point = NULL;

    if (!(key = EC_KEY_new())) {
      printf("EC_KEY_new\n");
      printf("%s\n", ERR_error_string(ERR_get_error(), NULL));
      return NULL;
    }

    if (!(group = EC_GROUP_new_by_curve_name(ECIES_CURVE))) {
      printf("EC_GROUP_new_by_curve_name failed. {error = %s}\n",
        ERR_error_string(ERR_get_error(), NULL));
      EC_KEY_free(key);
      return NULL;
    }

    EC_GROUP_set_point_conversion_form(group, POINT_CONVERSION_COMPRESSED);

    if (EC_KEY_set_group(key, group) != 1) {
      printf("EC_KEY_set_group\n");
      printf("%s\n", ERR_error_string(ERR_get_error(), NULL));
      EC_GROUP_free(group);
      EC_KEY_free(key);
      return NULL;
    }

    if (!(point = EC_POINT_hex2point(group, hex, NULL, NULL))) {
      printf("EC_POINT_hex2point\n");
      printf("%s\n", ERR_error_string(ERR_get_error(), NULL));
      EC_KEY_free(key);
      return NULL;
    }

    if (EC_KEY_set_public_key(key, point) != 1) {
      printf("EC_KEY_set_public_key\n");
      printf("%s\n", ERR_error_string(ERR_get_error(), NULL));
      EC_GROUP_free(group);
      EC_POINT_free(point);
      EC_KEY_free(key);
      return NULL;
    }

    EC_GROUP_free(group);
    EC_POINT_free(point);

    if (EC_KEY_check_key(key) != 1) {
      printf("EC_KEY_check_key\n");
      printf("%s\n", ERR_error_string(ERR_get_error(), NULL));
      EC_KEY_free(key);
      return NULL;
    }

    return key;
  }
  template <typename T1, typename T2>
  char * ECIESGenKey<T1, T2>::ecies_key_public_get_hex(T2 *key) {

    char *hex;
    const EC_POINT *point;
    const EC_GROUP *group;

    if (!(point = EC_KEY_get0_public_key(key))) {
      printf("EC_KEY_get0_public_key\n");
      printf("%s\n", ERR_error_string(ERR_get_error(), NULL));
      return NULL;
    }

    if (!(group = EC_KEY_get0_group(key))) {
      printf("EC_KEY_get0_group\n");
      printf("%s\n", ERR_error_string(ERR_get_error(), NULL));
      return NULL;
    }

    if (!(hex = EC_POINT_point2hex(group, point,
      POINT_CONVERSION_COMPRESSED, NULL))) {
      printf("EC_POINT_point2hex\n");
      printf("%s\n", ERR_error_string(ERR_get_error(), NULL));
      return NULL;
    }

    //printf("PUB: %s\n", hex);
    return hex;
  }
  template <typename T1, typename T2>
  T2 * ECIESGenKey<T1, T2>::ecies_key_create_private_hex(char *hex) {

    EC_GROUP *group;
    BIGNUM *bn = NULL;
    EC_KEY *key = NULL;

    if (!(key = EC_KEY_new())) {
      printf("EC_KEY_new\n");
      printf("%s\n", ERR_error_string(ERR_get_error(), NULL));
      return NULL;
    }

    if (!(group = EC_GROUP_new_by_curve_name(ECIES_CURVE))) {
      printf("EC_GROUP_new_by_curve_name failed. {error = %s}\n",
        ERR_error_string(ERR_get_error(), NULL));
      EC_KEY_free(key);
      return NULL;
    }

    EC_GROUP_set_point_conversion_form(group, POINT_CONVERSION_COMPRESSED);

    if (EC_KEY_set_group(key, group) != 1) {
      printf("EC_KEY_set_group\n");
      printf("%s\n", ERR_error_string(ERR_get_error(), NULL));
      EC_GROUP_free(group);
      EC_KEY_free(key);
      return NULL;
    }

    EC_GROUP_free(group);

    if (!(BN_hex2bn(&bn, hex))) {
      printf("BN_hex2bn\n");
      printf("%s\n", ERR_error_string(ERR_get_error(), NULL));
      EC_KEY_free(key);
      return NULL;
    }

    if (EC_KEY_set_private_key(key, bn) != 1) {
      printf("EC_KEY_set_public_key\n");
      printf("%s\n", ERR_error_string(ERR_get_error(), NULL));
      EC_KEY_free(key);
      BN_free(bn);
      return NULL;
    }

    BN_free(bn);

    return key;
  }
  template <typename T1, typename T2>
  char * ECIESGenKey<T1, T2>::ecies_key_private_get_hex(T2 *key) {

    char *hex;
    const BIGNUM *bn;

    if (!(bn = EC_KEY_get0_private_key(key))) {
      printf("EC_KEY_get0_private_key\n");
      printf("%s\n", ERR_error_string(ERR_get_error(), NULL));
      return NULL;
    }

    if (!(hex = BN_bn2hex(bn))) {
      printf("BN_bn2hex\n");
      printf("%s\n", ERR_error_string(ERR_get_error(), NULL));
      return NULL;
    }

    //printf("PRIV: %s\n", hex);
    return hex;
  }
  // This is not thread-safe! You'll need to use a mutex if you want this to work in a multi-threaded world.
  // But its worth noting that duplicating the group instead of allocating it on each pass reduced the execution time by 50%!
  template <typename T1, typename T2>
  T1 * ECIESGenKey<T1, T2>::ecies_group(void) {

    EC_GROUP *group;

    if (eliptic) {
      return EC_GROUP_dup(eliptic);
    }

    if (!(group = EC_GROUP_new_by_curve_name(kECIES_CURVE))) {
      printf("EC_GROUP_new_by_curve_name failed. {error = %s}\n", ERR_error_string(ERR_get_error(), NULL));
      return NULL;
    }

    else if (EC_GROUP_precompute_mult(group, NULL) != 1) {
      printf("EC_GROUP_precompute_mult failed. {error = %s}\n", ERR_error_string(ERR_get_error(), NULL));
      EC_GROUP_free(group);
      return NULL;
    }

    EC_GROUP_set_point_conversion_form(group, POINT_CONVERSION_COMPRESSED);

    return EC_GROUP_dup(group);
  }
  ECIESKey* ECIESKey::private_instance_ = NULL;
  ECIESKey* ECIESKey::CreateInstance(){
    if(!private_instance_){
      private_instance_ = new ECIESKey;
      ECIESGenKey<EC_GROUP, EC_KEY>* gen_key = new ECIESGenKey<EC_GROUP, EC_KEY>;
      char *hex_pub = NULL, *hex_priv = NULL;
      EC_KEY *key = NULL;
      gen_key->ecies_group_init();
      if (!(key = gen_key->ecies_key_create())) {
        return nullptr;
      }
      if (!(hex_pub = gen_key->ecies_key_public_get_hex(key)) || !(hex_priv = gen_key->ecies_key_private_get_hex(key))) {
        if (key) {
          gen_key->ecies_key_free(key);
        }
        return nullptr;
      }
      private_instance_->priv_key_ = hex_priv;
      private_instance_->pub_key_ = hex_pub;
      OPENSSL_free(hex_pub);
      OPENSSL_free(hex_priv);
      delete gen_key;
    }
    return private_instance_;
  }
  void ECIESKey::DeleteInstance() {
    if (private_instance_) {
      delete private_instance_;
      private_instance_ = nullptr;
    }
  }
  ECIESKey::ECIESKey(){
    priv_key_.resize(0);
    pub_key_.resize(0);
  }

  ECIESKey::~ECIESKey(){
    priv_key_.resize(0);
    pub_key_.resize(0);
  }
}