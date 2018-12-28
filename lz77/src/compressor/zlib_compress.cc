#include "compressor/zlib_compress.h"

#include <zlib.h>
#include "base/basic_incls.h"
#include "base/base_export.h"

namespace compressor {
  bool  ZLibCompress::CompressGo(const CompressTypeTable& type) {
    std::vector<std::uint8_t> tmp;
    z_stream defstream;
    defstream.zalloc = Z_NULL;
    defstream.zfree = Z_NULL;
    defstream.opaque = Z_NULL;
    defstream.avail_in = (uInt)src_.size();
    defstream.next_in = (Bytef*)&src_[0];
    if (type == CompressTypeTable::kCompress) {

      tmp.resize(CalcUpperBoundSize(src_.size()));
      defstream.avail_out = (uInt)tmp.size();
      defstream.next_out = (Bytef *)&tmp[0];

      deflateInit(&defstream, Z_BEST_COMPRESSION);
      int len = deflate(&defstream, Z_FINISH);
      deflateEnd(&defstream);
      dst_.resize((std::uint64_t)defstream.next_out - (std::uint64_t)&tmp[0]);
      memmove(&dst_[0], &tmp[0], dst_.size());
      return false;
    }
    else if (type == CompressTypeTable::kUncompress) {

      tmp.resize(decompress_size_);
      defstream.avail_out = (uInt)tmp.size();
      defstream.next_out = (Bytef *)&tmp[0];

      inflateInit(&defstream);
      inflate(&defstream, Z_NO_FLUSH);
      inflateEnd(&defstream);
      dst_.resize((std::uint64_t)defstream.next_out - (std::uint64_t)&tmp[0]);
      memmove(&dst_[0], &tmp[0], dst_.size());
      return false;
    }
    else if (type == CompressTypeTable::kCompressHTTPGz) {
      HTTPGzCompress(src_, tmp);
      dst_.resize(tmp.size());
      memmove(&dst_[0], &tmp[0], dst_.size());
      return false;
    }
    else if (type == CompressTypeTable::kUncompressHTTPGz) {
      HTTPGzDecompress(src_, tmp);
      dst_.resize(tmp.size());
      memmove(&dst_[0], &tmp[0], dst_.size());
      return false;
    }
    return true;
  }
  bool ZLibCompress::HTTPGzCompress(const std::vector<std::uint8_t>& src_buf, std::vector<std::uint8_t>& dst_buf) {
    z_stream c_stream;
    int err = 0;
    if (src_buf.size()) {
      std::vector<std::uint8_t> tmp_buf;
      tmp_buf.resize(dst_buf.size());
      c_stream.zalloc = NULL;
      c_stream.zfree = NULL;
      c_stream.opaque = NULL;
      if (deflateInit2(&c_stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        return true;
      }
      c_stream.next_in = (Bytef*)&src_buf[0];
      c_stream.avail_in = src_buf.size();
      c_stream.next_out = &tmp_buf[0];
      c_stream.avail_out = tmp_buf.size();
      while (c_stream.avail_in != 0 && c_stream.total_out < tmp_buf.size()) {
        if (deflate(&c_stream, Z_NO_FLUSH) != Z_OK) {
          return true;
        }
      }
      if (c_stream.avail_in != 0) {
        return true;
      }
      for (;;) {
        if ((err = deflate(&c_stream, Z_FINISH)) == Z_STREAM_END) {
          break;
        }
        if (err != Z_OK) {
          return true;
        }
      }
      if (deflateEnd(&c_stream) != Z_OK) {
        return true;
      }
      dst_buf.resize(c_stream.total_out);
      memmove(&dst_buf[0], &tmp_buf[0], dst_buf.size());
      return  false;
    }
    return true;
  }
  bool ZLibCompress::HTTPGzDecompress(const std::vector<std::uint8_t>& src_buf, std::vector<std::uint8_t>& dst_buf) {
    std::vector<std::uint8_t> tmp_buf;
    tmp_buf.resize(dst_buf.size());
    int err = 0;
    z_stream d_stream = { 0 };
    static char dummy_head[2] =
    {
      0x8 + 0x7 * 0x10,
      (((0x8 + 0x7 * 0x10) * 0x100 + 30) / 31 * 31) & 0xFF,
    };
    d_stream.zalloc = (alloc_func)0;
    d_stream.zfree = (free_func)0;
    d_stream.opaque = (voidpf)0;
    d_stream.next_in = (Bytef*)&src_buf[0];
    d_stream.avail_in = 0;
    d_stream.next_out = (Bytef*)&tmp_buf[0];
    if (inflateInit2(&d_stream, -MAX_WBITS) != Z_OK) {
      return -1;
    }
    while (d_stream.total_out < tmp_buf.size() && d_stream.total_in < src_buf.size()) {
      d_stream.avail_in = d_stream.avail_out = 1; /* force small buffers */
      if ((err = inflate(&d_stream, Z_NO_FLUSH)) == Z_STREAM_END) {
        break;
      }
      if (err != Z_OK) {
        if (err == Z_DATA_ERROR) {
          d_stream.next_in = (Bytef*)dummy_head;
          d_stream.avail_in = sizeof(dummy_head);
          if ((err = inflate(&d_stream, Z_NO_FLUSH)) != Z_OK) {
            return -1;
          }
        }
        else return -1;
      }
    }
    if (inflateEnd(&d_stream) != Z_OK) {
      return -1;
    }
    dst_buf.resize(d_stream.total_out);
    memmove(&dst_buf[0], &tmp_buf[0], dst_buf.size());
    return  false;
  }
  std::uint64_t ZLibCompress::CalcUpperBoundSize(std::uint64_t len) {
    return (compressBound(len) * 3);
  }
}