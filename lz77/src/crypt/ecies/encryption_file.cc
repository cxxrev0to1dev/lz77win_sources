#include "ecies/encryption_file.h"
#include "ecies/encrypt_message.h"
#include "compressor/zlib_compressor.h"

namespace Crypt {
  EncryptionFile::EncryptionFile(){
    FileStream::Close();
  }
  EncryptionFile::~EncryptionFile(){
    FileStream::Close();
    Close();
  }
  void EncryptionFile::CryptFile(const std::string& pubk, const char* src, const char* dst) {
    if (!FileStream::Open(src))
      return;
    out_file_.open(dst, std::ios::binary | std::ios::out);
    if (!out_file_.is_open())
      return;
    pubk_ = pubk;
    while (FileStream::IsEOF()){
      FileStream::Read();
    }
  }
  size_t EncryptionFile::ReadingProcess(const uint8_t* bytes, const size_t len) {
    Crypt::ECKeyGen key_gen(false);
    key_gen.set_pub_key(pubk_);
    std::vector<uint8_t> my_vector(bytes, &bytes[len]);
    Crypt::EncryptMessage encrypt_message(key_gen);

#ifndef NO_COMPRESSOR
    compressor::ZLibCompressor xxxxx;
    xxxxx.compressor(my_vector);
    const std::vector<std::uint8_t>& compress_bytes = xxxxx.dst();
    encrypt_message.EncryptBytes(compress_bytes);
#else
    encrypt_message.EncryptBytes(my_vector);
#endif
    std::vector<std::uint8_t> salt = encrypt_message.salt();
    std::vector<std::uint8_t> checksum = encrypt_message.checksum();
    std::vector<std::uint8_t> ciphertext = encrypt_message.cipher_text();
    std::vector<std::uint8_t> public_key = encrypt_message.public_r();
    size_t public_key_length = public_key.size();
    out_file_.write(reinterpret_cast<const char*>(&public_key_length), sizeof(size_t));
    out_file_.write(reinterpret_cast<const char*>(public_key.data()), public_key_length);
    out_file_.flush();
    size_t salt_length = salt.size();
    out_file_.write(reinterpret_cast<const char*>(&salt_length), sizeof(size_t));
    out_file_.write(reinterpret_cast<const char*>(salt.data()), salt_length);
    out_file_.flush();
    size_t checksum_length = checksum.size();
    out_file_.write(reinterpret_cast<const char*>(&checksum_length), sizeof(size_t));
    out_file_.write(reinterpret_cast<const char*>(checksum.data()), checksum_length);
    out_file_.flush();
    size_t ciphertext_length = ciphertext.size();
    size_t decompress_size = len;
    out_file_.write(reinterpret_cast<const char*>(&decompress_size), sizeof(size_t));
    out_file_.write(reinterpret_cast<const char*>(&ciphertext_length), sizeof(size_t));
    out_file_.write(reinterpret_cast<const char*>(ciphertext.data()), ciphertext_length);
    out_file_.flush();
    return 0;
  }
  void EncryptionFile::Close() {
    if (out_file_.is_open()) {
      out_file_.close();
      out_file_.clear();
    }
  }
}