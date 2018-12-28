#ifndef COMPRESSOR_ZLIB_COMPRESS_H_
#define COMPRESSOR_ZLIB_COMPRESS_H_

#include "compressor/vftable.h"

namespace compressor{
  class ZLibCompress
  {
  public:
    ZLibCompress() {
      src_.resize(0);
      dst_.resize(0);
    }
    explicit ZLibCompress(const std::vector<std::uint8_t>& src) {
      src_ = src;
    }
    explicit ZLibCompress(const std::string& src) {
      SetSrc(src);
    }
    ZLibCompress(const std::vector<std::uint8_t>& src, size_t decompress_size, const CompressTypeTable& type) :ZLibCompress(src) {
      decompress_size_ = 0;
      decompress_size_ = decompress_size;
      CompressGo(type);
    }
    ZLibCompress(const std::string& src, const CompressTypeTable& type) :ZLibCompress(src) {
      CompressGo(type);
    }
    ~ZLibCompress() {
      src_.resize(0);
      dst_.resize(0);
    }
    void SetSrc(const std::vector<std::uint8_t>& src) {
      src_ = src;
    }
    void SetSrc(const std::string& src) {
      std::copy(src.begin(), src.end(), std::back_inserter(src_));
    }
    const std::vector<std::uint8_t>& dst() {
      return dst_;
    }
    bool CompressGo(const CompressTypeTable& type);
  private:
    bool HTTPGzCompress(const std::vector<std::uint8_t>& src_buf, std::vector<std::uint8_t>& dst_buf);
    bool HTTPGzDecompress(const std::vector<std::uint8_t>& src_buf, std::vector<std::uint8_t>& dst_buf);
    std::uint64_t CalcUpperBoundSize(std::uint64_t len);
    std::vector<std::uint8_t> src_;
    std::vector<std::uint8_t> dst_;
    size_t decompress_size_;
  };
}

#endif