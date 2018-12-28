#include "compressor/lib7zip_compressor.h"
#include <fstream>
#include <cassert>
#include "base/path.h"
#include "base/string_conv.h"
#include "compressor/lib7zip_compress.h"
#include "bit7z/include/bit7zlibrary.hpp"
#include "bit7z/include/bitextractor.hpp"

#if defined(OS_WIN)
#include "compressor/win/lib7z_achive.h"
#endif // !OS_WIN

namespace compressor {
  static Wrapper7zCompress lib_7zip_compress;
  const wchar_t* ArchiveCompressor::ToFixOutExt(const wchar_t* ext_name) {
    if (!ext_name){
      return nullptr;
    }
    else if (!wcsncmp(ext_name,L"gzip",4)){
      return L"gz";
    }
    else if (!wcsncmp(ext_name, L"bzip2", 4)) {
      return L"bz2";
    }
    else {
      return ext_name;
    }
  }
  ArchiveCompressor::ArchiveCompressor(AskOpenArchivePassword* ask_open_password):is_password_defined_(false){
    exts_.resize(0);
    archive_compress_ext_.resize(0);
    is_signed_file_ = false;
    archive_compress_ext_ = L"7z";
    if (lib_7zip_compress.Init(ask_open_password)) {
      //fail
      return;
    }
    exts_ = lib_7zip_compress.exts();
  }
  ArchiveCompressor::~ArchiveCompressor() {
    is_password_defined_ = false;
    exts_.resize(0);
    archive_compress_ext_.resize(0);
  }
  bool ArchiveCompressor::IsSupportedExt(const std::wstring& ext) {
    WStringArray::iterator it;
    for (it = exts_.begin(); it != exts_.end(); it++) {
      if (ext == *it) {
        //success
        return false;
      }
    }
    return true;
  }
  bool ArchiveCompressor::IsSupportedARCExt(const std::wstring& ext) {
    for (size_t i = 0; kCompressArchiveTable[i] != nullptr; i++){
      if (ext == kCompressArchiveTable[i]) {
        //success
        archive_compress_ext_ = kCompressArchiveTable[i];
        return false;
      }
    }
    return true;
  }
  bool ArchiveCompressor::IsSupportedCryptARC(const std::wstring& ext) {
    for (size_t i = 0; kCryptARC[i] != nullptr; i++) {
      if (ext == kCryptARC[i]) {
        //success
        return false;
      }
    }
    return true;
  }
  bool ArchiveCompressor::IsDoNeedExtractArcName(const std::wstring& ext) {
    for (size_t i = 0; kDoNeedExtractArcName[i] != nullptr; i++) {
      if (ext == kDoNeedExtractArcName[i]) {
        //success
        return true;
      }
    }
    return false;
  }
  bool ArchiveCompressor::TestAttributeFlag(const std::wstring& archive_name) {
    bool fail = lib_7zip_compress.TestAttributeFlag(archive_name);
    is_signed_file_ = lib_7zip_compress.IsSignedFile();
    if (fail) {
      //fail
      is_password_defined_ = lib_7zip_compress.IsPasswordDefined();
      return true;
    }
    return false;
  }
  void ArchiveCompressor::decompressor(const std::wstring& archive_name, 
    const std::wstring& dirs,
    const std::wstring& password) {
    op_res_msg_.resize(0);
    if (lib_7zip_compress.UncompressDirs(archive_name,dirs,password)){
      //fail
      op_res_msg_ = lib_7zip_compress.OpResMsg();
      return;
    }
    return;
  }
  bool ArchiveCompressor::ExtractingExceptionsISO(const std::wstring archive_name,
    const std::wstring& dir) {
    bit7z::Bit7zLibrary lib(L"");
    bit7z::BitExtractor extractor(lib, bit7z::BitFormat::Iso);
    extractor.extract(archive_name, dir);
    return false;
  }

  void ArchiveCompressor::compressor(const std::vector<std::wstring>& dirs,
    const std::wstring& archive,
    const std::wstring& password) {
    is_compress_ok_ = false;
    Wrapper7zArchive archivexxx(dirs, archive, archive_compress_ext_, (password.size()>0)? password.c_str():nullptr);
    assert(archivexxx.archive_error() == Wrapper7zArchive::ArchiveErrorTable::kOK);
    is_compress_ok_ = (archivexxx.archive_error() == Wrapper7zArchive::ArchiveErrorTable::kOK);
  }
  bool ArchiveCompressor::IsCompressOK() const {
    return is_compress_ok_;
  }
}