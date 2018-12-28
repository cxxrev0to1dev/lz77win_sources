#ifndef COMPRESSOR_WRAPPER_7Z_ARCHIVE_H_
#define COMPRESSOR_WRAPPER_7Z_ARCHIVE_H_

#include "lib7zip/Lib7Zip/lib7zip.h"
#include "base/string_conv.h"
#include <mutex>

namespace compressor {

  class Wrapper7zInStream : public C7ZipInStream
  {
  private:
    FILE * m_pFile;
    std::wstring m_strFileName;
    wstring m_strFileExt;
    uint64_t m_nFileSize;
  public:
    explicit Wrapper7zInStream(std::wstring fileName) :
      m_strFileName(fileName),
      m_strFileExt(L"001")
    {
#if defined(OS_WIN)
      m_pFile = _wfopen(fileName.c_str(), L"rb");
#else
      m_pFile = wfopen(fileName.c_str(), L"rb");
#endif
      if (m_pFile) {
        fseek(m_pFile, 0, SEEK_END);
        m_nFileSize = _ftelli64(m_pFile);
        fseek(m_pFile, 0, SEEK_SET);
      }
    }
    Wrapper7zInStream(std::wstring fileName,const std::wstring& ext) :
      m_strFileName(fileName),
      m_strFileExt(ext)
    {

      //wprintf(L"fileName.c_str(): %s\n", fileName.c_str());
#if defined(OS_WIN)
      m_pFile = _wfopen(fileName.c_str(), L"rb");
#else
      m_pFile = wfopen(fileName.c_str(), L"rb");
#endif
      if (m_pFile) {
        fseek(m_pFile, 0, SEEK_END);
        m_nFileSize = _ftelli64(m_pFile);
        fseek(m_pFile, 0, SEEK_SET);

        int pos = m_strFileName.find_last_of(L".");
        if (pos != m_strFileName.npos) {
          m_strFileExt = m_strFileName.substr(pos + 1);
        }
        //wprintf(L"Ext:%ls\n", m_strFileExt.c_str());
      }
      else {
        //wprintf(L"fileName.c_str(): %s cant open\n", fileName.c_str());
      }
    }

    virtual ~Wrapper7zInStream()
    {
      fclose(m_pFile);
    }

  public:
    virtual wstring GetExt() const
    {
      //wprintf(L"GetExt:%ls\n", m_strFileExt.c_str());
      return m_strFileExt;
    }

    virtual int Read(void *data, unsigned int size, unsigned int *processedSize)
    {
      if (!m_pFile)
        return 1;

      int count = fread(data, 1, size, m_pFile);
      //wprintf(L"Read:%d %d\n", size, count);

      if (count >= 0) {
        fflush(m_pFile);
        if (processedSize != NULL)
          *processedSize = count;

        return 0;
      }

      return 1;
    }

    virtual int Seek(__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition)
    {
      if (!m_pFile)
        return 1;

      int result = _fseeki64(m_pFile, offset, seekOrigin);

      if (!result) {
        if (newPosition)
          *newPosition = _ftelli64(m_pFile);

        return 0;
      }

      return result;
    }

    virtual int GetSize(unsigned __int64 * size)
    {
      if (size)
        *size = m_nFileSize;
      return 0;
    }
  };

  class Wrapper7zOutStream : public C7ZipOutStream
  {
  private:
    FILE * m_pFile;
    std::wstring m_strFileName;
    wstring m_strFileExt;
    uint64_t m_nFileSize;
#if defined(COMPRESSOR_MULTI_THREAD)
    std::mutex* wlock_;
#endif
  public:
    Wrapper7zOutStream(std::wstring fileName, const std::wstring& ext) :
      m_strFileName(fileName),
      m_strFileExt(ext){
      m_pFile = nullptr;
      Open(fileName, ext);
    }
    bool IsOpen() const {
      return (m_pFile != nullptr);
    }

    void Open(std::wstring fileName, const std::wstring& ext) {
      if (IsOpen()) {
        fflush(m_pFile);
        fclose(m_pFile);
        m_pFile = nullptr;
      }
      m_strFileName = fileName;
      m_strFileExt = ext;
#if defined(OS_WIN)
      m_pFile = _wfopen(fileName.c_str(), L"wb");
#else
      m_pFile = wfopen(fileName.c_str(), L"wb");
#endif
      m_nFileSize = 0;
      if (m_pFile) {
        fseek(m_pFile, 0, SEEK_END);
        m_nFileSize = _ftelli64(m_pFile);
        fseek(m_pFile, 0, SEEK_SET);
        int pos = fileName.find_last_of(L".");
        if (pos != fileName.npos) {
          m_strFileExt = m_strFileName.substr(pos + 1);
        }
      }
    }

    virtual ~Wrapper7zOutStream()
    {
      if (IsOpen()) {
        fclose(m_pFile);
        m_pFile = nullptr;
      }
        
    }

  public:
    int GetFileSize() const
    {
      return m_nFileSize;
    }

    virtual int Write(const void *data, unsigned int size, unsigned int *processedSize)
    {
      int count = fwrite(data, 1, size, m_pFile);
      //wprintf(L"Write:%d %d\n", size, count);

      if (count >= 0)
      {
        //fflush(m_pFile);
        if (processedSize != NULL)
          *processedSize = count;

        m_nFileSize += count;
        return 0;
      }
      return 1;
    }

    virtual int Seek(__int64 offset, unsigned int seekOrigin, unsigned __int64 *newPosition)
    {
      int result = _fseeki64(m_pFile, (long)offset, seekOrigin);

      if (!result)
      {
        if (newPosition)
          *newPosition = _ftelli64(m_pFile);
        return 0;
      }
      return result;
    }

    virtual int SetSize(unsigned __int64 size)
    {
      //wprintf(L"SetFileSize:%ld\n", size);
      return 0;
    }
  };

  class Wrapper7zMultiVolumes : public C7ZipMultiVolumes
  {
  private:
    FILE * m_pFile;
    wstring m_strFileName;
    uint64_t m_nFileSize;
    wstring m_strCurVolume;
    bool m_done;

  public:
    Wrapper7zMultiVolumes(wstring fileName) :
      m_strFileName(fileName),
      m_pFile(NULL),
      m_done(false)
    {
    }

    virtual ~Wrapper7zMultiVolumes()
    {
      if (m_pFile)
        fclose(m_pFile);
    }

  public:
    virtual wstring GetFirstVolumeName() {
      m_strCurVolume = m_strFileName;
      MoveToVolume(m_strCurVolume);
      return m_strCurVolume;
    }

    virtual bool MoveToVolume(const wstring& volumeName) {
      m_strCurVolume = volumeName;
      //wprintf(L"move to volume:%ls\n", volumeName.c_str());

      if (m_pFile)
        fclose(m_pFile);
      m_pFile = NULL;
      //wprintf(L"narrow volume:%s\n", f.c_str());
#if defined(OS_WIN)
      m_pFile = _wfopen(volumeName.c_str(), L"rb");
#else
      m_pFile = wfopen(volumeName.c_str(), L"rb");
#endif
      if (!m_pFile)
        m_done = true;
      else {
        fseek(m_pFile, 0, SEEK_END);
        m_nFileSize = _ftelli64(m_pFile);
        fseek(m_pFile, 0, SEEK_SET);
      }

      return !m_done;
    }

    virtual C7ZipInStream* OpenCurrentVolumeStream() {
      return new Wrapper7zInStream(m_strCurVolume);
    }

    virtual unsigned __int64 GetCurrentVolumeSize() {
      //wprintf(L"get current volume size:%ls\n", m_strCurVolume.c_str());
      return m_nFileSize;
    }
  };


}

#endif // !COMPRESSOR_WRAPPER_7Z_ARCHIVE_H_
