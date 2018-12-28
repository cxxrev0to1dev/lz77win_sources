#include "compressor/lz4_compressor.h"

#include <zlib.h>
#include "base/basic_incls.h"
#include "base/base_export.h"
#include "compressor/lz4_compress.h"

namespace compressor {
  LZ4Compressor::LZ4Compressor() {
    reset();
  }
  LZ4Compressor::~LZ4Compressor(){
    reset();
  }
  void LZ4Compressor::compressor(const std::vector<std::uint8_t>& src) {
    reset();
    compressor(src, dst_);
  }
  void LZ4Compressor::decompressor(const std::vector<std::uint8_t>& src) {
    reset();
    decompressor(src, dst_);
  }
  void LZ4Compressor::reset() {
    dst_.resize(0);
  }
  void LZ4Compressor::compressor(const std::vector<std::uint8_t>& src, std::vector<std::uint8_t>& dst) {
    LZ4Compress zzzz(src, CompressTypeTable::kCompress);
    dst = zzzz.dst();
  }
  void LZ4Compressor::decompressor(const std::vector<std::uint8_t>& src, std::vector<std::uint8_t>& dst) {
    LZ4Compress xxxx(src, CompressTypeTable::kUncompress);
    dst = xxxx.dst();
  }
}