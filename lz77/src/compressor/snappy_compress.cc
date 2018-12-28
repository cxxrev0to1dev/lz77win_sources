#include "compressor/snappy_compress.h"
#include "snappy/snappy-c.h"

namespace compressor {
  SnappyCompress::SnappyCompress(
    const std::vector<std::uint8_t>& src, 
    const CompressTypeTable& type) :
    src_(src) {
    dst_.resize(0);
    CompressGo(type);
  }
  SnappyCompress::~SnappyCompress(){
    reset();
  }
  bool SnappyCompress::CompressGo(const CompressTypeTable& type) {
    if (type==CompressTypeTable::kCompress){
      size_t output_length = snappy_max_compressed_length(src_.size());
      dst_.resize(output_length);
      if (snappy_compress((const char*)&src_[0], src_.size(), (char*)&dst_[0], &output_length)
        == SNAPPY_OK) {
        //success
        return false;
      }
    }
    else if(type == CompressTypeTable::kUncompress){
      size_t output_length = 0;
      if (snappy_uncompressed_length((const char*)&src_[0], src_.size(), &output_length)
        != SNAPPY_OK) {
        //fail
        return true;
      }
      dst_.resize(output_length);
      if (snappy_uncompress((const char*)&src_[0], src_.size(), (char*)&dst_[0], &output_length)
        == SNAPPY_OK) {
        //success
        return false;
      }
    }
    //fail
    return true;
  }
  void SnappyCompress::reset() {
    src_.resize(0);
    dst_.resize(0);
  }
}