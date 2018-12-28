#ifndef COMPRESSOR_WIN_LIB7Z_ACHIVE_H_
#define COMPRESSOR_WIN_LIB7Z_ACHIVE_H_

#include <string>
#include <vector>
#include <wtypes.h>

#if defined(OS_WIN)
#include "CPP/Common/MyWindows.h"
#include "C/7zTypes.h"
#include "CPP/Common/MyString.h"
#endif

namespace compressor {

  //////////////////////////////////////////////////////////////
  // Archive Creating callback class

  struct CDirItem
  {
    UInt64 Size;
    FILETIME CTime;
    FILETIME ATime;
    FILETIME MTime;
    UString Name;
    FString FullPath;
    UInt32 Attrib;

    bool isDir() const { return (Attrib & FILE_ATTRIBUTE_DIRECTORY) != 0; }
  };

  class Wrapper7zArchive
  {
  public:
    enum class ArchiveErrorTable
    {
      kOK,
      kCreateArchiveFail,
      kGetClassObjectFail,
      kExistErrorFile
    };
    Wrapper7zArchive(const std::vector<std::wstring>& target, const std::wstring& out,const std::wstring& ext,const wchar_t* password);
    virtual ~Wrapper7zArchive();
    const ArchiveErrorTable& archive_error() const {
      return archive_error_;
    }
  private:
    void ArchiveFile(CObjectVector<CDirItem> &dirItems, const std::wstring& ext, const wchar_t* ArchivePackPath);
    void GetArchiveItemFromPath(const wchar_t* strDirPath,const wchar_t* sub_name, CObjectVector<CDirItem> &dirItems);
    void GetArchiveItemFromFileList(CObjectVector<UString> FileList, CObjectVector<CDirItem> &ItemList);

    ArchiveErrorTable archive_error_;
    std::vector<UString> error_files_;
    std::wstring password_;
  };

}

#endif // !COMPRESSOR_LIB7Z_ACHIVE_H_

