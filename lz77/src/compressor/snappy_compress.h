#ifndef COMPRESSOR_SNAPPY_COMPRESS_H_
#define COMPRESSOR_SNAPPY_COMPRESS_H_

#include "compressor/vftable.h"

namespace compressor {
  class SnappyCompress
  {
  public:
    SnappyCompress(const std::vector<std::uint8_t>& src
      , const CompressTypeTable& type);
    virtual ~SnappyCompress();
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