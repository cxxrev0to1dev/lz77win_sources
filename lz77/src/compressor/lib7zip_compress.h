#ifndef COMPRESSOR_LIB7ZIP_COMPRESS_H_
#define COMPRESSOR_LIB7ZIP_COMPRESS_H_

#include "lib7zip/Lib7Zip/lib7zip.h"
#include "base/basic_incls.h"
#include "compressor/lib7zip_wrapper.h"
#include "lib7zip/Lib7ZIP/AskOpenArchivePassword.h"
#include <map>
#include <string>


namespace compressor {

  static const wchar_t k7zFmtMulVolume[] = L".001";

  static const char * const kNoOpenAsExtensions =
    " 7z arj bz2 cab chm cpio flv gz lha lzh lzma rar swm tar tbz2 tgz wim xar xz z zip ";

  class Wrapper7zCompress
  {
  public:
    Wrapper7zCompress();
    virtual ~Wrapper7zCompress();
    bool Init(AskOpenArchivePassword* ask_open_password);
    bool UncompressDirs(const std::wstring& archive_name, 
      const std::wstring& dirs,
      const std::wstring& password);
    bool TestAttributeFlag(const std::wstring& archive_name);
    const std::wstring& OpResMsg();
    const WStringArray& exts() const {
      return exts_;
    }
    bool IsPasswordDefined() const {
      return is_password_defined_;
    }
    bool IsSignedFile() const {
      return is_signed_file_;
    }
    C7ZipArchive* GetArchive() {
      return archive_;
    }
  private:
    bool Is7zMultiVolumesExt(const std::wstring& ext);
    C7ZipArchive* archive_;
    WStringArray exts_;
    C7ZipLibrary lib_;
    bool is_password_defined_;
    bool is_signed_file_;
    std::map<std::wstring, std::wstring> error_file_msg_;
  };

}

#endif // !COMPRESSOR_LIB7ZIP_COMPRESS_H_
