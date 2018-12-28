#ifndef COMPRESSOR_LZ4_COMPRESSOR_H_
#define COMPRESSOR_LZ4_COMPRESSOR_H_

#include "compressor/vftable.h"
#include "compressor/compressor_exports.h"

namespace compressor {
  class LZ4Compressor:
    public CompressorVFTable,
    public DecompressorVFTable
  {
  public:
    COMPRESSOR_EXPORT LZ4Compressor();
    COMPRESSOR_EXPORT virtual ~LZ4Compressor();
    COMPRESSOR_EXPORT void compressor(const std::vector<std::uint8_t>& src);
    COMPRESSOR_EXPORT void decompressor(const std::vector<std::uint8_t>& src);
    COMPRESSOR_EXPORT const std::vector<std::uint8_t>& dst() {
      return dst_;
    }
  private:
    void reset();
    virtual void compressor(const std::vector<std::uint8_t>& src, std::vector<std::uint8_t>& dst);
    virtual void decompressor(const std::vector<std::uint8_t>& src, std::vector<std::uint8_t>& dst);
    std::vector<std::uint8_t> dst_;
  };
}

#endif