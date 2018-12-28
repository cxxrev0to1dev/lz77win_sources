#include "compressor/lib7zip_compress.h"
#include <filesystem>
#include "base/path.h"
#include "base/string_conv.h"
#include "Lz77Lang/lz77_lang.h"
#include <CTPL/ctpl_stl.h>
#include <thread>
#include <iostream>
#include <thread>
#include <vector>
#include <utility>
#include <algorithm>
#include <functional>
#include <mutex>
#include <future>

#if defined(OS_WIN)
#include "base/win/cpu.h"
#endif
/*
#include "lib7zip/Lib7Zip/lib7zip.h"
#include "CPP/7zip/Archive/IArchive.h"
*/
namespace compressor {

  static int GetCpuUsage() {
    static base::CPU cpu;
    static std::mutex cpu_lock;
    while (!cpu_lock.try_lock()){
      Sleep(200);
    }
    int sys;
    TKTime upTime;
    cpu.GetUsage(&sys, &upTime);
    cpu_lock.unlock();
    return sys;
  }

  static const wchar_t * const kEmptyFileAlias = L"[Content]";

  Wrapper7zCompress::Wrapper7zCompress():archive_(nullptr){
    exts_.resize(0);
    error_file_msg_.clear();
    is_signed_file_ = false;
  }
  Wrapper7zCompress::~Wrapper7zCompress() {
    exts_.resize(0);
    error_file_msg_.clear();
    is_signed_file_ = false;
  }
  bool Wrapper7zCompress::Init(AskOpenArchivePassword* ask_open_password) {
    if (!lib_.Initialize()) {
      //fail
      return true;
    }
    if (!lib_.GetSupportedExts(exts_)) {
      //fail
      return true;
    }
    lib_.SetAskOpenArchivePassword(ask_open_password);
    //success
    return false;
  }
  bool Wrapper7zCompress::UncompressDirs(const std::wstring& archive_name,
    const std::wstring& dirs,
    const std::wstring& password) {
    is_password_defined_ = false;
    is_signed_file_ = false;
    error_file_msg_.clear();
    uint32_t numItems = -1;
    base::Path path(archive_name);
    if (!path.IsFile()) {
      return true;
    }
    Wrapper7zMultiVolumes* volumes = nullptr;
    Wrapper7zInStream* stream = nullptr;
    if (Is7zMultiVolumesExt(path.ext())){
      volumes = new Wrapper7zMultiVolumes(archive_name);
      if (lib_.OpenMultiVolumeArchive(volumes, &archive_, true)) {
        archive_->GetItemCount(&numItems);
        archive_->SetRootDir(dirs.c_str());
        if (password.length() > 0) {
          archive_->SetArchivePassword(password);
        }
        is_signed_file_ = true; //success
      }
      else {
        if (lib_.GetLastError() == lib7zip::LIB7ZIP_NEED_PASSWORD) {
          if (lib_.OpenMultiVolumeArchive(volumes, &archive_, password, true)) {
            is_signed_file_ = true; //success
            archive_->GetItemCount(&numItems);
            archive_->SetRootDir(dirs.c_str());
          }
        }
      }
    }
    else {
      stream = new Wrapper7zInStream(archive_name, path.ext());
      if (lib_.OpenArchive(stream, &archive_,true)) {
        archive_->GetItemCount(&numItems);
        archive_->SetRootDir(dirs.c_str());
        if (password.length() > 0) {
          archive_->SetArchivePassword(password);
        }
        is_signed_file_ = true; //success
      }
      else {
        if (lib_.GetLastError() == lib7zip::LIB7ZIP_NEED_PASSWORD) {
          if (lib_.OpenArchive(stream, &archive_, password, true)) {
            is_signed_file_ = true; //success
            archive_->GetItemCount(&numItems);
            archive_->SetRootDir(dirs.c_str());
          }
        }
      }
    }
    bool fail_res = (numItems==-1);
    std::thread::hardware_concurrency();
    std::mutex lock;
    C7ZipArchiveItem * archive_item = NULL;
    archive_->GetItemInfo(0, &archive_item);
    std::wstring root_dir = dirs + L"\\";
    base::Path::mkpath(root_dir.c_str());
    if (!archive_->Extract(archive_item, nullptr)) {
      fail_res = true;
    }
    const std::map <std::wstring,std::wstring> error_msgs = archive_->ErrorFileMsg();
    std::map<std::wstring, std::wstring>::const_iterator it;
    for (it = error_msgs.begin();it != error_msgs.end();it++) {
      error_file_msg_[it->first] = lang::LZ77Language::GetInstannce()->GetErrorMsg(it->second);
    }
#ifdef RELEASE_VERSION
    for (uint32_t i = 0;i < numItems && !fail_res;i++) {
      C7ZipArchiveItem * archive_item = NULL;
      if (archive_->GetItemInfo(i, &archive_item)) {
        const unsigned int index = archive_item->GetArchiveIndex();
        const std::wstring rpath = archive_item->GetFullPath();
        const bool is_dir = archive_item->IsDir();
        std::experimental::filesystem::path out_path(dirs);
        std::experimental::filesystem::path full_path = out_path / rpath;
        if (out_path == rpath) {
          full_path = out_path;
        }
        std::wstring xxx = full_path.c_str();
        if (!is_dir) {
          if (rpath == kEmptyFileAlias) {
            xxx = out_path;
          }
          Wrapper7zOutStream out(xxx, path.ext());
          if (!out.IsOpen()) {
            for (uint32_t try_j = 0;try_j < 2; try_j++) {
              base::Path::mkpath(xxx.c_str());//FIXME
              out.Open(xxx, path.ext());
              if (out.IsOpen()) {
                break;
              }
            }
            if (!out.IsOpen()) {
              //fail
              fail_res = true;
              break;
            }
          }
          while (!lock.try_lock()) {
#if defined(OS_WIN)
            Sleep(0);
#endif
          }
          if (!archive_->Extract(archive_item, &out)) {
            //fail
            lock.unlock();
            error_file_msg_[full_path] = lang::LZ77Language::GetInstannce()->GetErrorMsg(archive_->OpResMsg());
            continue;
          }
          lock.unlock();
        }
        else {
          if (base::Path::mkpath(xxx.c_str())) {//FIXME
            continue;
          }
        }
      }
    }
#endif
    if (volumes!=nullptr){
      delete volumes;
      volumes = nullptr;
    }
    if (stream != nullptr) {
      delete stream;
      stream = nullptr;
    }
    if (archive_!=nullptr){
      delete archive_;
      archive_ = nullptr;
    }
    if (!error_file_msg_.empty()) {
      fail_res = true;
    }
    return fail_res;
  }
  bool Wrapper7zCompress::TestAttributeFlag(const std::wstring& archive_name) {
    is_password_defined_ = false;
    is_signed_file_ = false;
    uint32_t numItems = -1;
    base::Path path(archive_name);
    if (!path.IsFile()) {
      return true;
    }
    Wrapper7zMultiVolumes* volumes = nullptr;
    Wrapper7zInStream* stream = nullptr;
    if (Is7zMultiVolumesExt(path.ext())) {
      volumes = new Wrapper7zMultiVolumes(archive_name);
      if (lib_.OpenMultiVolumeArchive(volumes, &archive_, true)) {
        is_signed_file_ = true; //success
        archive_->GetItemCount(&numItems);
      }
    }
    else {
      stream = new Wrapper7zInStream(archive_name, path.ext());
      if (lib_.OpenArchive(stream, &archive_, true)) {
        is_signed_file_ = true; //success
        archive_->GetItemCount(&numItems);
      }
    }
    bool success = false;
    if (archive_&&numItems != -1) {
      const std::string archive_ext = base::StringConv::narrow(path.ext());
      for (unsigned int i = 0;i < numItems;i++) {
        C7ZipArchiveItem * archive_item = NULL;
        if (!archive_->GetItemInfo(i, &archive_item)) {
          continue;
        }
        archive_item->GetBoolProperty(lib7zip::kpidEncrypted, is_password_defined_);
        if (is_password_defined_) {
          if (lib_.GetAskOpenArchivePassword()) {
            lib_.GetAskOpenArchivePassword()->AskPasswordUI();
          }
          break;//fail
        }
        bool is_dir = false;
        archive_item->GetBoolProperty(lib7zip::kpidIsDir, is_dir);
        if (!is_dir){
          break;
        }
        
      }
      success = is_password_defined_;
    }
    else {
      //open archive need password
      if (lib_.GetLastError()==lib7zip::LIB7ZIP_NEED_PASSWORD){
        is_signed_file_ = true; //success
        is_password_defined_ = true;
      }
      //fail
      success = true;
    }
    if (volumes != nullptr) {
      delete volumes;
      volumes = nullptr;
    }
    if (stream != nullptr) {
      delete stream;
      stream = nullptr;
    }
    if (archive_ != nullptr) {
      delete archive_;
      archive_ = nullptr;
    }
    if (!error_file_msg_.empty()){
      success = true;
    }
    return success;
  }
  const std::wstring& Wrapper7zCompress::OpResMsg() {
    return base::StringConv::GetMapW(error_file_msg_);
  }
  bool Wrapper7zCompress::Is7zMultiVolumesExt(const std::wstring& ext) {
    return (wcsncmp(ext.c_str(), k7zFmtMulVolume, 
      sizeof(k7zFmtMulVolume) - sizeof(wchar_t)) == 0);
  }
}