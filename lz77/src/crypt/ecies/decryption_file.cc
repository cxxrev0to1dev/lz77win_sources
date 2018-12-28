#include "ecies/decryption_file.h"
#include "ecies/encrypt_message.h"
#include "ecies/decrypt_message.h"
#include "compressor/zlib_compressor.h"

namespace Crypt {
  DecryptionFile::DecryptionFile(){
    FileStream::Close();
  }
  DecryptionFile::~DecryptionFile(){
    FileStream::Close();
    Close();
  }
  void DecryptionFile::CryptFile(const std::string& privk, const char* src, const char* dst) {
    if (!FileStream::Open(src))
      return;
    out_file_.open(dst, std::ios::binary | std::ios::out);
    if (!out_file_.is_open())
      return;
    privk_ = privk;
    while (FileStream::IsEOF()) {
      Crypt::ECKeyGen key_gen(false);
      key_gen.set_priv_key(privk_);
      std::vector<std::uint8_t> public_key;
      size_t public_key_length = 0;
      FileStream::ReadingStream((uint8_t*)&public_key_length, sizeof(size_t));
      public_key.resize(public_key_length);
      FileStream::ReadingStream((uint8_t*)&public_key[0], public_key_length);
      size_t salt_length = 0;
      std::vector<std::uint8_t> salt;
      FileStream::ReadingStream((uint8_t*)&salt_length,sizeof(size_t));
      salt.resize(salt_length);
      FileStream::ReadingStream((uint8_t*)&salt[0], salt_length);
      size_t checksum_length = 0;
      std::vector<std::uint8_t> checksum;
      FileStream::ReadingStream((uint8_t*)&checksum_length, sizeof(size_t));
      checksum.resize(checksum_length);
      FileStream::ReadingStream((uint8_t*)&checksum[0], checksum_length);
      size_t ciphertext_length = 0;
      size_t decompress_size = 0;
      std::vector<std::uint8_t> ciphertext;
      FileStream::ReadingStream((uint8_t*)&decompress_size, sizeof(size_t));
      FileStream::ReadingStream((uint8_t*)&ciphertext_length, sizeof(size_t));
      ciphertext.resize(ciphertext_length);
      FileStream::ReadingStream((uint8_t*)&ciphertext[0], ciphertext_length);

      Crypt::DecryptMessage decrypt_message(key_gen);
      decrypt_message.SetSalt(salt);
      decrypt_message.SetChecksum(checksum);
      decrypt_message.SetPublicR(public_key);
      decrypt_message.DecryptBytes(ciphertext);

      std::vector<std::uint8_t> data = decrypt_message.clear_text();
#ifndef NO_COMPRESSOR
      compressor::ZLibCompressor xxxxx;
      xxxxx.decompressor(data, decompress_size);
      const std::vector<std::uint8_t>& decompress_bytes = xxxxx.dst();
      out_file_.write(reinterpret_cast<const char*>(&decompress_bytes[0]), decompress_bytes.size());
#else
      out_file_.write(reinterpret_cast<const char*>(&data[0]), data.size());
#endif
      out_file_.flush();
    }
  }
  size_t DecryptionFile::ReadingProcess(const uint8_t* bytes, const size_t len) {
    return 0;
  }
  void DecryptionFile::Close() {
    if (out_file_.is_open()) {
      out_file_.close();
      out_file_.clear();
    }
  }
}