#include "compressor/zlib_compressor.h"

#include <zlib.h>
#include "base/basic_incls.h"
#include "base/base_export.h"
#include "compressor/zlib_compress.h"

namespace compressor {
  ZLibCompressor::ZLibCompressor() {
    reset();
  }
  ZLibCompressor::~ZLibCompressor(){
    reset();
  }
  void ZLibCompressor::compressor(const std::vector<std::uint8_t>& src) {
    reset();
    compressor(src, dst_);
  }
  void ZLibCompressor::decompressor(const std::vector<std::uint8_t>& src, size_t decompress_size) {
    reset();
    SetDecompressSize(decompress_size);
    decompressor(src, dst_);
  }
  void ZLibCompressor::reset() {
    dst_.resize(0);
    decompress_size_ = 0;
  }
  void ZLibCompressor::compressor(const std::vector<std::uint8_t>& src, std::vector<std::uint8_t>& dst) {
    ZLibCompress zzzz(src, 0, CompressTypeTable::kCompress);
    dst = zzzz.dst();
  }
  void ZLibCompressor::decompressor(const std::vector<std::uint8_t>& src, std::vector<std::uint8_t>& dst) {
    ZLibCompress xxxx(src, decompress_size_, CompressTypeTable::kUncompress);
    dst = xxxx.dst();
  }
}