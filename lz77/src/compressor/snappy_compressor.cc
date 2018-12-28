#include "compressor/snappy_compressor.h"

#include <zlib.h>
#include "base/basic_incls.h"
#include "base/base_export.h"
#include "compressor/snappy_compress.h"

namespace compressor {
  SnappyCompressor::SnappyCompressor() {
    reset();
  }
  SnappyCompressor::~SnappyCompressor(){
    reset();
  }
  void SnappyCompressor::compressor(const std::vector<std::uint8_t>& src) {
    reset();
    compressor(src, dst_);
  }
  void SnappyCompressor::decompressor(const std::vector<std::uint8_t>& src) {
    reset();
    decompressor(src, dst_);
  }
  void SnappyCompressor::reset() {
    dst_.resize(0);
  }
  void SnappyCompressor::compressor(const std::vector<std::uint8_t>& src, std::vector<std::uint8_t>& dst) {
    SnappyCompress zzzz(src, CompressTypeTable::kCompress);
    dst = zzzz.dst();
  }
  void SnappyCompressor::decompressor(const std::vector<std::uint8_t>& src, std::vector<std::uint8_t>& dst) {
    SnappyCompress xxxx(src, CompressTypeTable::kUncompress);
    dst = xxxx.dst();
  }
}