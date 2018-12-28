#ifndef ECIES_ENCRYPT_MESSAGE_H_
#define ECIES_ENCRYPT_MESSAGE_H_

#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>
#include "ecies/bignum_key.h"

namespace Crypt {
  class EncryptMessage
  {
  public:
    explicit EncryptMessage(const std::string& private_key);
    explicit EncryptMessage(ECKeyGen& key_gen);
    ~EncryptMessage();
    bool EncryptString(const std::string& str);
    bool EncryptBytes(const std::vector<std::uint8_t>& bytes);
    std::vector<std::uint8_t> salt() const{
      return salt_;
    }
    std::vector<std::uint8_t> checksum() const{
      return checksum_;
    }
    std::vector<std::uint8_t> cipher_text() const{
      return cipher_text_;
    }
    std::vector<std::uint8_t> public_r() const{
      return public_r_;
    }
  private:
    std::vector<std::uint8_t> salt_;
    std::vector<std::uint8_t> checksum_;
    std::vector<std::uint8_t> cipher_text_;
    std::vector<std::uint8_t> public_r_;
    BigNumKey key_;
  };
}

#endif