#include "base/path.h"
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(OS_WIN)
#include <windows.h>
#endif
namespace base {
  void ModulePathDir(const wchar_t* mod,std::wstring& out) {
#if defined(OS_WIN)
    wchar_t buffer[_MAX_PATH] = {0};
    GetModuleFileNameW(GetModuleHandleW(mod), buffer, _MAX_PATH);
    std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
    out = std::wstring(buffer).substr(0, pos);
#endif
  }
#if defined(OS_WIN)
  static int do_mkdir(const wchar_t *path)
  {
    struct _stat            st;
    int             status = 0;

    if (_wstat(path, &st) != 0)
    {
      /* Directory does not exist. EEXIST for race condition */
      if (_wmkdir(path) != 0 && errno != EEXIST)
        status = -1;
    }
    else if (((st.st_mode & S_IFDIR) == 0))
    {
      errno = ENOTDIR;
      status = -1;
    }

    return(status);
  }

  /**
  ** mkpath - ensure all directories in path exist
  ** Algorithm takes the pessimistic view and works top-down to ensure
  ** each directory in path exists, rather than optimistically creating
  ** the last element and working backwards.
  */
  BASE_EXPORT int Path::mkpath(const wchar_t *path)
  {
    wchar_t*pp;
    wchar_t *sp;
    int status;
    wchar_t *copypath = wcsdup(path);

    status = 0;
    pp = wcschr(copypath, L'\\');
    while (status == 0 && (sp = wcschr(pp, L'\\')) != 0)
    {
      if (sp != pp)
      {
        /* Neither root nor double slash in path */
        *sp = '\0';
        status = do_mkdir(copypath);
        *sp = L'\\';
      }
      pp = sp + 1;
    }
    free(copypath);
    return (status);
  }
#else
  static int do_mkdir(const char *path, mode_t mode)
  {
    Stat            st;
    int             status = 0;

    if (stat(path, &st) != 0)
    {
      /* Directory does not exist. EEXIST for race condition */
      if (mkdir(path, mode) != 0 && errno != EEXIST)
        status = -1;
    }
    else if (!S_ISDIR(st.st_mode))
    {
      errno = ENOTDIR;
      status = -1;
    }

    return(status);
  }

  /**
  ** mkpath - ensure all directories in path exist
  ** Algorithm takes the pessimistic view and works top-down to ensure
  ** each directory in path exists, rather than optimistically creating
  ** the last element and working backwards.
  */
  int mkpath(const char *path, mode_t mode)
  {
    char           *pp;
    char           *sp;
    int             status;
    char           *copypath = STRDUP(path);

    status = 0;
    pp = copypath;
    while (status == 0 && (sp = strchr(pp, '/')) != 0)
    {
      if (sp != pp)
      {
        /* Neither root nor double slash in path */
        *sp = '\0';
        status = do_mkdir(copypath, mode);
        *sp = '/';
      }
      pp = sp + 1;
    }
    if (status == 0)
      status = do_mkdir(path, mode);
    FREE(copypath);
    return (status);
  }
#endif
  Path::Path() {
    Reset();
  }
  Path::Path(const std::wstring& path){
    Reset();
    Split(path);
  }
  Path::~Path(){
    Reset();
  }
  void Path::Split(const std::wstring& path) {
    wchar_t drive[_MAX_DRIVE] = { 0 };
    wchar_t dir[_MAX_DIR] = { 0 };
    wchar_t fname[_MAX_FNAME] = { 0 };
    wchar_t ext[_MAX_EXT] = { 0 };
    Reset();
    is_dir_ = DirsExists(path.c_str());
    if (!is_dir_){
#if defined(OS_WIN)
      //is_file_ == true  success
      is_file_ = (_waccess(path.c_str(), 0) == 0);
#endif
    }
#if defined(OS_WIN)
    if (_wsplitpath_s(path.c_str(), drive, _MAX_DRIVE, dir, _MAX_DIR,
      fname, _MAX_FNAME, ext, _MAX_EXT) != 0) {
      return;
    }
#endif
    drive_ = drive;
    dir_ = dir;
    fname_ = fname;
    ext_ = ext;

    full_dir_ = drive_;
    full_dir_ += dir_;
    full_dir_ += fname_;
    full_path_ = path;
  }
  bool Path::MkdirFullDir() {
#if defined(OS_WIN)
    if (_wmkdir(full_dir_.c_str()) == 0) {
#endif
      //success
      return false;
    }
    //fail
    return true;
  }
  const std::wstring& Path::MakePath(const wchar_t* name, const wchar_t* ext) {
    full_path_ = full_dir_;
    if (name) {
      full_path_.append(L"\\");
      full_path_.append(name);
      if (ext) full_path_.append(ext);
    }
    return full_path_;
  }
  const std::wstring& Path::AppendPath(const wchar_t* name, const wchar_t* ext) {
    if (full_path_.empty()){
      full_path_ = full_dir_;
    }
    if (name) {
      full_path_.append(L"\\");
      full_path_.append(name);
      if (ext) full_path_.append(ext);
    }
    return full_path_;
  }
  void Path::InitModulePathDir(const wchar_t* mod) {
    std::wstring temp;
    ModulePathDir(mod, temp);
    full_dir_ = temp;
  }
  bool Path::DirsExists(const wchar_t* absolutePath) {
#if defined(OS_WIN)
    if (_waccess(absolutePath, 0) == 0) {
      struct _stat status;
      _wstat(absolutePath, &status);
      return ((status.st_mode & S_IFDIR) != 0);
    }
#endif
    //fail
    return false;
  }
  void Path::Reset() {
    drive_.resize(0);
    dir_.resize(0);
    fname_.resize(0);
    ext_.resize(0);
    full_dir_.resize(0);
    full_path_.resize(0);
    is_dir_ = false;
    is_file_ = false;
  }
}