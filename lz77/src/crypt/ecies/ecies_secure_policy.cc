#include "ecies/ecies_secure_policy.h"


namespace Crypt {
  ECIESSecurePolicy::ECIESSecurePolicy(){
  }
  ECIESSecurePolicy::~ECIESSecurePolicy(){
  }
  uint64_t ECIESSecurePolicy::secure_key_length(secure_t *cryptex) {
    secure_head_t *head = (secure_head_t *)cryptex;
    return head->length.key;
  }

  uint64_t ECIESSecurePolicy::secure_mac_length(secure_t *cryptex) {
    secure_head_t *head = (secure_head_t *)cryptex;
    return head->length.mac;
  }

  uint64_t ECIESSecurePolicy::secure_body_length(secure_t *cryptex) {
    secure_head_t *head = (secure_head_t *)cryptex;
    return head->length.body;
  }

  uint64_t ECIESSecurePolicy::secure_orig_length(secure_t *cryptex) {
    secure_head_t *head = (secure_head_t *)cryptex;
    return head->length.orig;
  }

  uint64_t ECIESSecurePolicy::secure_total_length(secure_t *cryptex) {
    secure_head_t *head = (secure_head_t *)cryptex;
    return sizeof(secure_head_t) + (head->length.key + head->length.mac +
      head->length.body);
  }

  void * ECIESSecurePolicy::secure_key_data(secure_t *cryptex) {
    return (char *)cryptex + sizeof(secure_head_t);
  }

  void * ECIESSecurePolicy::secure_mac_data(secure_t *cryptex) {
    secure_head_t *head = (secure_head_t *)cryptex;
    return (char *)cryptex + (sizeof(secure_head_t) + head->length.key);
  }

  void * ECIESSecurePolicy::secure_body_data(secure_t *cryptex) {
    secure_head_t *head = (secure_head_t *)cryptex;
    return (char *)cryptex + (sizeof(secure_head_t) + head->length.key +
      head->length.mac);
  }

  void * ECIESSecurePolicy::secure_alloc(uint64_t key, uint64_t mac, uint64_t orig, uint64_t body) {
    secure_t *cryptex = (secure_t*)malloc(sizeof(secure_head_t) + key + mac + body);
    secure_head_t *head = (secure_head_t *)cryptex;
    head->length.key = key;
    head->length.mac = mac;
    head->length.orig = orig;
    head->length.body = body;
    return cryptex;
  }

  void ECIESSecurePolicy::secure_free(secure_t *cryptex) {
    free(cryptex);
    return;
  }
}
