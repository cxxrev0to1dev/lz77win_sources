#ifndef BASE_PATH_H_
#define BASE_PATH_H_

#include <string>
#include "base/base_export.h"
#include "base/basictypes.h"

namespace base{
  class Path
  {
  public:
    BASE_EXPORT static int mkpath(const wchar_t *path);
    BASE_EXPORT Path();
    BASE_EXPORT explicit Path(const std::wstring& path);
    BASE_EXPORT virtual ~Path();
    BASE_EXPORT void Split(const std::wstring& path);
    BASE_EXPORT bool MkdirFullDir();
    BASE_EXPORT const std::wstring& MakePath(const wchar_t* name,const wchar_t* ext);
    BASE_EXPORT const std::wstring& AppendPath(const wchar_t* name, const wchar_t* ext);
    BASE_EXPORT void InitModulePathDir(const wchar_t* mod);
    BASE_EXPORT bool IsDir() const {
      return is_dir_;
    }
    BASE_EXPORT bool IsFile() const {
      return is_file_;
    }
    BASE_EXPORT const wchar_t* drive() const {
      return drive_.c_str();
    }
    BASE_EXPORT const wchar_t* dir() const {
      return dir_.c_str();
    }
    BASE_EXPORT const wchar_t* fname() const {
      return fname_.c_str();
    }
    BASE_EXPORT const wchar_t* ext() const {
      return ext_.c_str();
    }
    BASE_EXPORT const wchar_t* FullDir() const {
      return full_dir_.c_str();
    }
    BASE_EXPORT const wchar_t* FullPath() const {
      return full_path_.c_str();
    }
  private:
    bool DirsExists(const wchar_t* absolutePath);
    void Reset();
    std::wstring drive_;
    std::wstring dir_;
    std::wstring fname_;
    std::wstring ext_;
    std::wstring full_dir_;
    std::wstring full_path_;
    bool is_dir_;
    bool is_file_;
  };
}

#endif