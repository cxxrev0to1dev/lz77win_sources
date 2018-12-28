#ifndef COMPRESSOR_ZLIB_COMPRESSOR_H_
#define COMPRESSOR_ZLIB_COMPRESSOR_H_

#include "compressor/vftable.h"
#include "compressor/compressor_exports.h"

namespace compressor {
  class ZLibCompressor:
    public CompressorVFTable,
    public DecompressorVFTable
  {
  public:
    COMPRESSOR_EXPORT ZLibCompressor();
    COMPRESSOR_EXPORT virtual ~ZLibCompressor();
    COMPRESSOR_EXPORT void compressor(const std::vector<std::uint8_t>& src);
    COMPRESSOR_EXPORT void decompressor(const std::vector<std::uint8_t>& src, size_t decompress_size);
    COMPRESSOR_EXPORT const std::vector<std::uint8_t>& dst() {
      return dst_;
    }
  private:
    void SetDecompressSize(size_t decompress_size) {
      decompress_size_ = decompress_size;
    }
    void reset();
    virtual void compressor(const std::vector<std::uint8_t>& src, std::vector<std::uint8_t>& dst);
    virtual void decompressor(const std::vector<std::uint8_t>& src, std::vector<std::uint8_t>& dst);
    std::vector<std::uint8_t> dst_;
    size_t decompress_size_;
  };
}

#endif