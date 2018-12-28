#ifndef COMPRESSOR_VFTABLE_H_
#define COMPRESSOR_VFTABLE_H_

#include <cstdint>
#include <vector>
#include <string>
#include <algorithm>
#include <iterator>

class CompressorVFTable {
protected:
  virtual void compressor(const std::vector<std::uint8_t>& src, std::vector<std::uint8_t>& dst) = 0;
};

class DecompressorVFTable {
protected:
  virtual void decompressor(const std::vector<std::uint8_t>& src, std::vector<std::uint8_t>& dst) = 0;
};

class DirUncompressorVFTable
{
protected:
  virtual void decompressor(const std::wstring& archive_name,
    const std::wstring& dirs,
    const std::wstring& password) = 0;
};

class DirCompressorVFTable
{
protected:
  virtual void compressor(const std::vector<std::wstring>& dirs,
    const std::wstring& archive,
    const std::wstring& password) = 0;
};

namespace compressor {
  enum class CompressTypeTable { kCompress, kUncompress, kCompressHTTPGz, kUncompressHTTPGz };
}

#endif // !COMPRESSOR_VFTABLE_H_
