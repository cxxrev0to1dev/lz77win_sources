#ifndef COMPRESSOR_LIB7ZIP_COMPRESSOR_H_
#define COMPRESSOR_LIB7ZIP_COMPRESSOR_H_

#include "compressor/vftable.h"
#include "lib7zip/Lib7Zip/AskOpenArchivePassword.h"
#include "compressor/compressor_exports.h"


namespace compressor {

  static const char * const kExtractExludeExtensions =
    " 3gp"
    " aac ans ape asc asm asp aspx avi awk"
    " bas bat bmp"
    " c cs cls clw cmd cpp csproj css ctl cxx"
    " def dep dlg dsp dsw"
    " eps"
    " f f77 f90 f95 fla flac frm"
    " gif"
    " h hpp hta htm html hxx"
    " ico idl inc ini inl"
    " java jpeg jpg js"
    " la lnk log"
    " mak manifest wmv mov mp3 mp4 mpe mpeg mpg m4a"
    " ofr ogg"
    " pac pas pdf php php3 php4 php5 phptml pl pm png ps py pyo"
    " ra rb rc reg rka rm rtf"
    " sed sh shn shtml sln sql srt swa"
    " tcl tex tiff tta txt"
    " vb vcproj vbs"
    " wav wma wv"
    " xml xsd xsl xslt"
    " ";

  static const wchar_t *kDoNeedExtractArcName[] = { L"zip", L"tar", L"wim", L"7z", nullptr};
  static const wchar_t *kCompressArchiveTable[] = { L"zip",L"bzip2",L"gzip",L"tar",L"wim",L"xz",L"7z",nullptr };
  static const wchar_t *kCryptARC[] = { L"zip", L"7z", nullptr };
  static const wchar_t* kExtNameISO = L"iso";

  class ArchiveCompressor:public DirUncompressorVFTable,
    public DirCompressorVFTable
  {
  public:
    COMPRESSOR_EXPORT static const wchar_t* ToFixOutExt(const wchar_t* ext_name);
    COMPRESSOR_EXPORT explicit ArchiveCompressor(AskOpenArchivePassword* ask_open_password);
    COMPRESSOR_EXPORT virtual ~ArchiveCompressor();
    COMPRESSOR_EXPORT bool IsSupportedExt(const std::wstring& ext);
    COMPRESSOR_EXPORT bool IsSupportedARCExt(const std::wstring& ext);
    COMPRESSOR_EXPORT bool IsSupportedCryptARC(const std::wstring& ext);
    COMPRESSOR_EXPORT bool IsDoNeedExtractArcName(const std::wstring& ext);
    COMPRESSOR_EXPORT bool TestAttributeFlag(const std::wstring& archive_name);
    COMPRESSOR_EXPORT bool ExtractingExceptionsISO(const std::wstring archive_name,
      const std::wstring& dir);
    COMPRESSOR_EXPORT virtual void decompressor(const std::wstring& archive_name, 
      const std::wstring& dirs, 
      const std::wstring& password);
    COMPRESSOR_EXPORT virtual void compressor(const std::vector<std::wstring>& dirs,
      const std::wstring& archive,
      const std::wstring& password);
    COMPRESSOR_EXPORT const std::wstring& OpResMsg() const {
      return op_res_msg_;
    }
    COMPRESSOR_EXPORT bool IsDecompressOK() const {
      return op_res_msg_.size() == 0;
    }
    COMPRESSOR_EXPORT bool IsPasswordDefined() const {
      return is_password_defined_;
    }
    COMPRESSOR_EXPORT bool IsSignedFile() const {
      return is_signed_file_;
    }
    COMPRESSOR_EXPORT bool IsCompressOK() const;
  private:
    std::vector<std::wstring> exts_;
    std::wstring archive_compress_ext_;
    std::wstring op_res_msg_;
    bool is_password_defined_;
    bool is_signed_file_;
    bool is_compress_ok_;
  };

}

#endif // !COMPRESSOR_LIB7ZIP_COMPRESS_H_
