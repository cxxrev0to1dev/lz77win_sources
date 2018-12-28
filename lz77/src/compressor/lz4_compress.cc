#include "compressor/lz4_compress.h"
#include "lz4-dev/lib/lz4.h"

namespace compressor {
  LZ4Compress::LZ4Compress(
    const std::vector<std::uint8_t>& src, 
    const CompressTypeTable& type):
    src_(src) {
    dst_.resize(0);
    CompressGo(type);
  }
  LZ4Compress::~LZ4Compress(){
    reset();
  }
  bool LZ4Compress::CompressGo(const CompressTypeTable& type) {
    if (type == CompressTypeTable::kCompress) {
      const size_t max_dst_size = LZ4_compressBound(src_.size());
      std::vector<std::uint8_t> dst;
      dst.resize(max_dst_size);
      int bytes_returned = LZ4_compress_default((const char*)&src_[0], (char*)&dst[0], src_.size(), max_dst_size);
      if (bytes_returned<1){
        //fail
        return true;
      }
      dst_.resize(bytes_returned);
      memmove(&dst_[0], &dst[0], bytes_returned);
      //success
      return false;
    }
    else if (type == CompressTypeTable::kUncompress) {
      const size_t max_dst_size = LZ4_compressBound(src_.size());
      std::vector<std::uint8_t> dst;
      dst.resize(max_dst_size);
      int bytes_returned = LZ4_decompress_safe((const char*)&src_[0], (char*)&dst[0], src_.size(), max_dst_size);
      if (bytes_returned<1) {
        //fail
        return true;
      }
      dst_.resize(bytes_returned);
      memmove(&dst_[0], &dst[0], bytes_returned);
      //success
      return false;
    }
    return true;
  }
  void LZ4Compress::reset() {
    src_.resize(0);
    dst_.resize(0);
  }
}