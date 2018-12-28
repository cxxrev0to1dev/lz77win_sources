#ifndef ECIES_ECIES_SECURE_POLICY_H_
#define ECIES_ECIES_SECURE_POLICY_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef _MSC_VER
#include <inttypes.h>
#else
#include <stdint.h>
#endif

namespace Crypt {
  typedef struct {

    struct {
      uint64_t key;
      uint64_t mac;
      uint64_t orig;
      uint64_t body;
    } length;

  } secure_head_t;
  typedef char * secure_t;

  class ECIESSecurePolicy
  {
  public:
    ECIESSecurePolicy();
    virtual ~ECIESSecurePolicy();
  protected:
    void secure_free(secure_t *cryptex);
    void * secure_key_data(secure_t *cryptex);
    void * secure_mac_data(secure_t *cryptex);
    void * secure_body_data(secure_t *cryptex);
    uint64_t secure_key_length(secure_t *cryptex);
    uint64_t secure_mac_length(secure_t *cryptex);
    uint64_t secure_body_length(secure_t *cryptex);
    uint64_t secure_orig_length(secure_t *cryptex);
    uint64_t secure_total_length(secure_t *cryptex);
    void * secure_alloc(uint64_t key, uint64_t mac, uint64_t orig, uint64_t body);
  };
}

#endif