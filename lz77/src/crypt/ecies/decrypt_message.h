#ifndef ECIES_DECRYPT_MESSAGE_H_
#define ECIES_DECRYPT_MESSAGE_H_

#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include "ecies/bignum_key.h"

namespace Crypt {
  class DecryptMessage
  {
  public:
    explicit DecryptMessage(const std::string& private_key);
    explicit DecryptMessage(ECKeyGen& key_gen);
    ~DecryptMessage();
    bool DecryptBytes(const std::vector<std::uint8_t>& bytes);
    void SetPublicR(const std::vector<std::uint8_t>& s3);
    void SetSalt(const std::vector<std::uint8_t>& s1){
      salt_ = s1;
    }
    void SetChecksum(const std::vector<std::uint8_t>& s2){
      checksum_ = s2;
    }
    std::vector<std::uint8_t> clear_text() {
      return clear_text_;
    }
  private:
    std::vector<std::uint8_t> salt_;
    std::vector<std::uint8_t> checksum_;
    std::vector<std::uint8_t> clear_text_;
    BigNumKey key_;
  };
}

#endif