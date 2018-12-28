#ifndef COMPRESSOR_LZ4_COMPRESS_H_
#define COMPRESSOR_LZ4_COMPRESS_H_

#include "compressor/vftable.h"

namespace compressor {
  class LZ4Compress
  {
  public:
    LZ4Compress(const std::vector<std::uint8_t>& src
      , const CompressTypeTable& type);
    virtual ~LZ4Compress();
    const std::vector<std::uint8_t>& dst() {
      return dst_;
    }
  private:
    bool CompressGo(const CompressTypeTable& type);
    void reset();
    std::vector<std::uint8_t> src_;
    std::vector<std::uint8_t> dst_;
  };
}

#endif
