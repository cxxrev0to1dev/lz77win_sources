#include "base/file_stream.h"
#include <vector>

namespace base {
  FileStream::FileStream():processed_size_(0), file_size_(0){
    big_file_.clear();
  }
  FileStream::~FileStream(){
    Close();
  }
  bool FileStream::Open(const char* filename) {
    big_file_.open(filename, std::ios::binary | std::ios::in);
    bool is_open_file = big_file_.is_open();
    if (!file_size_&&is_open_file) {
      big_file_.seekg(0, big_file_.end);
      file_size_ = big_file_.tellg();
      big_file_.seekg(0, big_file_.beg);
    }
    return is_open_file;
  }
  bool FileStream::IsEOF(){
    return (file_size_!=processed_size_);
  }
  void FileStream::Close() {
    if (big_file_.is_open()) {
      big_file_.close();
      big_file_.clear();
    }
  }
  void FileStream::Read() {
    size_t align_size = ((file_size_ - processed_size_) >= kFileChunk) ? kFileChunk : (file_size_ - processed_size_);
    std::unique_ptr<uint8_t[]> buffer(new uint8_t[align_size]);
    if (big_file_.read((char*)buffer.get(), align_size)) {
      this->ReadingProcess(buffer.get(), align_size);
      processed_size_ += align_size;
    } 
    else {
      this->ReadingProcess(buffer.get(), align_size);
    }
  }
  size_t FileStream::ReadingStream(uint8_t* bytes, const size_t len) {
    if (big_file_.read((char*)bytes, len)) {
      processed_size_ += len;
      return len;
    }
    else {
      return (big_file_.gcount() - processed_size_);
    }
  }
}