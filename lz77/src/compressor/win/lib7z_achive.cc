#include "compressor/win/lib7z_achive.h"

#ifdef USE_STATIC_7Z_COMPONENT
#include "compressor/lib7z_exports.h"
#endif // !USE_STATIC_7Z_COMPONENT

#include "base/string_conv.h"
#if defined(OS_WIN)
#include "CPP/Common/MyWindows.h"

#include "CPP/Common/Defs.h"
#include "CPP/Common/MyInitGuid.h"

#include "CPP/Common/IntToString.h"
#include "CPP/Common/StringConvert.h"

#include "CPP/Windows/DLL.h"
#include "CPP/Windows/FileDir.h"
#include "CPP/Windows/FileFind.h"
#include "CPP/Windows/FileName.h"
#include "CPP/Windows/NtCheck.h"
#include "CPP/Windows/PropVariant.h"
#include "CPP/Windows/PropVariantConv.h"

#include "CPP/7zip/Common/FileStreams.h"

#include "CPP/7zip/Archive/IArchive.h"

#include "CPP/7zip/IPassword.h"
#include "C/7zVersion.h"

#include "CPP/7zip/ICoder.h"
#endif

DEFINE_GUID(CLSID_CFormat7z,
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);
DEFINE_GUID(CLSID_CFormatXz,
  0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x0C, 0x00, 0x00);

#define CLSID_Format CLSID_CFormat7z


namespace compressor {

  using namespace NWindows;
  using namespace NFile;
  using namespace NDir;

  static const wchar_t * const kEmptyFileAlias = L"[Content]";


  static HRESULT GetProp(
    Func_GetHandlerProperty getProp,
    Func_GetHandlerProperty2 getProp2,
    UInt32 index, PROPID propID, NCOM::CPropVariant &prop)
  {
    if (getProp2)
      return getProp2(index, propID, &prop);;
    return getProp(propID, &prop);
  }

  static HRESULT GetProp_Bool(
    Func_GetHandlerProperty getProp,
    Func_GetHandlerProperty2 getProp2,
    UInt32 index, PROPID propID, bool &res)
  {
    res = false;
    NCOM::CPropVariant prop;
    RINOK(GetProp(getProp, getProp2, index, propID, prop));
    if (prop.vt == VT_BOOL)
      res = VARIANT_BOOLToBool(prop.boolVal);
    else if (prop.vt != VT_EMPTY)
      return E_FAIL;
    return S_OK;
  }

  static HRESULT GetProp_UInt32(
    Func_GetHandlerProperty getProp,
    Func_GetHandlerProperty2 getProp2,
    UInt32 index, PROPID propID, UInt32 &res, bool &defined)
  {
    res = 0;
    defined = false;
    NCOM::CPropVariant prop;
    RINOK(GetProp(getProp, getProp2, index, propID, prop));
    if (prop.vt == VT_UI4)
    {
      res = prop.ulVal;
      defined = true;
    }
    else if (prop.vt != VT_EMPTY)
      return E_FAIL;
    return S_OK;
  }

  static HRESULT GetProp_String(
    Func_GetHandlerProperty getProp,
    Func_GetHandlerProperty2 getProp2,
    UInt32 index, PROPID propID, UString &res)
  {
    res.Empty();
    NCOM::CPropVariant prop;
    RINOK(GetProp(getProp, getProp2, index, propID, prop));
    if (prop.vt == VT_BSTR)
      res.SetFromBstr(prop.bstrVal);
    else if (prop.vt != VT_EMPTY)
      return E_FAIL;
    return S_OK;
  }

  static HRESULT GetProp_RawData(
    Func_GetHandlerProperty getProp,
    Func_GetHandlerProperty2 getProp2,
    UInt32 index, PROPID propID, CByteBuffer &bb)
  {
    bb.Free();
    NCOM::CPropVariant prop;
    RINOK(GetProp(getProp, getProp2, index, propID, prop));
    if (prop.vt == VT_BSTR)
    {
      UINT len = ::SysStringByteLen(prop.bstrVal);
      bb.CopyFrom((const Byte *)prop.bstrVal, len);
    }
    else if (prop.vt != VT_EMPTY)
      return E_FAIL;
    return S_OK;
  }

  class CArchiveUpdateCallback :
    public IArchiveUpdateCallback2,
    public ICryptoGetTextPassword2,
    public CMyUnknownImp
  {
  public:
    MY_UNKNOWN_IMP2(IArchiveUpdateCallback2, ICryptoGetTextPassword2)

      // IProgress
      STDMETHOD(SetTotal)(UInt64 size);
    STDMETHOD(SetCompleted)(const UInt64 *completeValue);

    // IUpdateCallback2
    STDMETHOD(GetUpdateItemInfo)(UInt32 index,
      Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive);
    STDMETHOD(GetProperty)(UInt32 index, PROPID propID, PROPVARIANT *value);
    STDMETHOD(GetStream)(UInt32 index, ISequentialInStream **inStream);
    STDMETHOD(SetOperationResult)(Int32 operationResult);
    STDMETHOD(GetVolumeSize)(UInt32 index, UInt64 *size);
    STDMETHOD(GetVolumeStream)(UInt32 index, ISequentialOutStream **volumeStream);

    STDMETHOD(CryptoGetTextPassword2)(Int32 *passwordIsDefined, BSTR *password);

  public:
    CRecordVector<UInt64> VolumesSizes;
    UString VolName;
    UString VolExt;

    FString DirPrefix;
    const CObjectVector<CDirItem> *DirItems;

    bool PasswordIsDefined;
    UString Password;
    bool AskPassword;

    bool m_NeedBeClosed;

    FStringVector FailedFiles;
    CRecordVector<HRESULT> FailedCodes;

    CArchiveUpdateCallback() : PasswordIsDefined(false), AskPassword(false), DirItems(0) {};

    ~CArchiveUpdateCallback() { Finilize(); }
    HRESULT Finilize();

    void Init(const CObjectVector<CDirItem> *dirItems)
    {
      DirItems = dirItems;
      m_NeedBeClosed = false;
      FailedFiles.Clear();
      FailedCodes.Clear();
    }
  };

  STDMETHODIMP CArchiveUpdateCallback::SetTotal(UInt64 /* size */)
  {
    return S_OK;
  }

  STDMETHODIMP CArchiveUpdateCallback::SetCompleted(const UInt64 * /* completeValue */)
  {
    return S_OK;
  }

  STDMETHODIMP CArchiveUpdateCallback::GetUpdateItemInfo(UInt32 /* index */,
    Int32 *newData, Int32 *newProperties, UInt32 *indexInArchive)
  {
    if (newData)
      *newData = BoolToInt(true);
    if (newProperties)
      *newProperties = BoolToInt(true);
    if (indexInArchive)
      *indexInArchive = (UInt32)(Int32)-1;
    return S_OK;
  }

  STDMETHODIMP CArchiveUpdateCallback::GetProperty(UInt32 index, PROPID propID, PROPVARIANT *value)
  {
    NCOM::CPropVariant prop;

    if (propID == kpidIsAnti)
    {
      prop = false;
      prop.Detach(value);
      return S_OK;
    }

    {
      const CDirItem &dirItem = (*DirItems)[index];
      switch (propID)
      {
      case kpidPath:  prop = dirItem.Name; break;
      case kpidIsDir:  prop = dirItem.isDir(); break;
      case kpidSize:  prop = dirItem.Size; break;
      case kpidAttrib:  prop = dirItem.Attrib; break;
      case kpidCTime:  prop = dirItem.CTime; break;
      case kpidATime:  prop = dirItem.ATime; break;
      case kpidMTime:  prop = dirItem.MTime; break;
      }
    }
    prop.Detach(value);
    return S_OK;
  }

  HRESULT CArchiveUpdateCallback::Finilize()
  {
    if (m_NeedBeClosed)
    {
      //PrintNewLine();
      m_NeedBeClosed = false;
    }
    return S_OK;
  }

  static void GetStream2(const wchar_t *name)
  {
    //Print("Compressing  ");
    if (name[0] == 0)
      name = kEmptyFileAlias;
    //Print(name);
  }

  STDMETHODIMP CArchiveUpdateCallback::GetStream(UInt32 index, ISequentialInStream **inStream)
  {
    RINOK(Finilize());

    const CDirItem &dirItem = (*DirItems)[index];
    GetStream2(dirItem.Name);

    if (dirItem.isDir())
      return S_OK;

    {
      CInFileStream *inStreamSpec = new CInFileStream;
      CMyComPtr<ISequentialInStream> inStreamLoc(inStreamSpec);
      FString path = DirPrefix + dirItem.FullPath;
      if (!inStreamSpec->Open(path))
      {
        DWORD sysError = ::GetLastError();
        FailedCodes.Add(sysError);
        FailedFiles.Add(path);
        // if (systemError == ERROR_SHARING_VIOLATION)
        {
          //PrintNewLine();
          //PrintError("WARNING: can't open file");
          // Print(NError::MyFormatMessageW(systemError));
          return S_FALSE;
        }
        // return sysError;
      }
      *inStream = inStreamLoc.Detach();
    }
    return S_OK;
  }

  STDMETHODIMP CArchiveUpdateCallback::SetOperationResult(Int32 /* operationResult */)
  {
    m_NeedBeClosed = true;
    return S_OK;
  }

  STDMETHODIMP CArchiveUpdateCallback::GetVolumeSize(UInt32 index, UInt64 *size)
  {
    if (VolumesSizes.Size() == 0)
      return S_FALSE;
    if (index >= (UInt32)VolumesSizes.Size())
      index = VolumesSizes.Size() - 1;
    *size = VolumesSizes[index];
    return S_OK;
  }

  STDMETHODIMP CArchiveUpdateCallback::GetVolumeStream(UInt32 index, ISequentialOutStream **volumeStream)
  {
    wchar_t temp[16];
    ConvertUInt32ToString(index + 1, temp);
    UString res = temp;
    while (res.Len() < 2)
      res.InsertAtFront(L'0');
    UString fileName = VolName;
    fileName += '.';
    fileName += res;
    fileName += VolExt;
    COutFileStream *streamSpec = new COutFileStream;
    CMyComPtr<ISequentialOutStream> streamLoc(streamSpec);
    if (!streamSpec->Create(us2fs(fileName), false))
      return ::GetLastError();
    *volumeStream = streamLoc.Detach();
    return S_OK;
  }

  STDMETHODIMP CArchiveUpdateCallback::CryptoGetTextPassword2(Int32 *passwordIsDefined, BSTR *password)
  {
    if (!PasswordIsDefined)
    {
      if (AskPassword)
      {
        // You can ask real password here from user
        // Password = GetPassword(OutStream);
        // PasswordIsDefined = true;
        // PrintError("Password is not defined");
        return E_ABORT;
      }
    }
    *passwordIsDefined = BoolToInt(PasswordIsDefined);
    return StringToBstr(Password, password);
  }

  Wrapper7zArchive::Wrapper7zArchive(const std::vector<std::wstring>& dirs,
    const std::wstring& out, 
    const std::wstring& ext, 
    const wchar_t* password){
    archive_error_ = ArchiveErrorTable::kOK;
    error_files_.resize(0);
    password_.resize(0);
    if (password){
      password_ = password;
    }
    CObjectVector<UString> fileList;
    std::vector<std::wstring>::const_iterator it = dirs.begin();
    for (;it!=dirs.end();it++){
      fileList.Add(it->c_str());
    }
    CObjectVector<CDirItem> ItemList;
    GetArchiveItemFromFileList(fileList, ItemList);
    ArchiveFile(ItemList, ext, out.c_str());
  }
  Wrapper7zArchive::~Wrapper7zArchive(){
    error_files_.resize(0);
    password_.resize(0);
  }

  //example
  /*
  CObjectVector<CDirItem> ItemList;
  GetArchiveItemFromPath("C:\\Users\\kr\\Desktop\\VPN",ItemList);
  ArchiveFile(ItemList,L"dirPath.7Z");

  CObjectVector<CDirItem> ItemList;
  CObjectVector<AString> fileList;
  fileList.Add("C:\\Users\\kr\\Desktop\\VPN\\VPN.txt");
  fileList.Add("C:\\Users\\kr\\Desktop\\VPN.txt");
  fileList.Add("C:\\Users\\kr\\Desktop\\VPN\\VPN3.txt");
  GetArchiveItemFromFileList(fileList,ItemList);
  ArchiveFile(ItemList,L"File.7Z");
  */
  void Wrapper7zArchive::ArchiveFile(CObjectVector<CDirItem> &dirItems, const std::wstring& ext, const wchar_t* ArchivePackPath) {
    UString archiveName = ArchivePackPath;
    COutFileStream *outFileStreamSpec = new COutFileStream;
    CMyComPtr<IOutStream> outFileStream = outFileStreamSpec;
    if (!outFileStreamSpec->Create(archiveName, true)){
      archive_error_ = ArchiveErrorTable::kCreateArchiveFail;
      return;
    }
    UInt32 numFormats = 1;
    GetNumberOfFormats(&numFormats);
    GUID classID = {0};
    for (UInt32 i = 0; i < numFormats; i++){
      UString name = L"";
      GetProp_String(GetHandlerProperty, GetHandlerProperty2, i, NArchive::NHandlerPropID::kName, name);
      {
        NCOM::CPropVariant prop;
        if (GetProp(GetHandlerProperty, GetHandlerProperty2, i, NArchive::NHandlerPropID::kClassID, prop) != S_OK)
          continue;
        if (prop.vt != VT_BSTR)
          continue;
        bool UpdateEnabled = false;
        GetProp_Bool(GetHandlerProperty, GetHandlerProperty2, i, NArchive::NHandlerPropID::kUpdate, UpdateEnabled);
        classID = *(const GUID *)prop.bstrVal;
        if (UpdateEnabled&& ext == name.GetBuf()) {
          break;
        }
        classID = { 0 };
        prop.Clear();
      }

    }
    CMyComPtr<IOutArchive> outArchive;
    if (CreateObject(&classID, &IID_IOutArchive, (void **)&outArchive) != S_OK) {
      archive_error_ = ArchiveErrorTable::kGetClassObjectFail;
      return;
    }
    CArchiveUpdateCallback *updateCallbackSpec = new CArchiveUpdateCallback;
    CMyComPtr<IArchiveUpdateCallback2> updateCallback(updateCallbackSpec);
    updateCallbackSpec->Init(&dirItems);
    if (password_.length()){
      updateCallbackSpec->PasswordIsDefined = true;
      updateCallbackSpec->Password = password_.c_str();
    }
    HRESULT result = outArchive->UpdateItems(outFileStream, dirItems.Size(), updateCallback);
    updateCallbackSpec->Finilize();

    if (result != S_OK){
      return;
    }
    if (updateCallbackSpec->FailedFiles.Size()) {
      archive_error_ = ArchiveErrorTable::kGetClassObjectFail;
      for (uint32_t i = 0; i < updateCallbackSpec->FailedFiles.Size(); i++) {
        //error_files_.push_back(updateCallbackSpec->FailedFiles[i]);
      }
    }

    return;
  }
  void Wrapper7zArchive::GetArchiveItemFromPath(const wchar_t* strDirPath, 
    const wchar_t* sub_name, 
    CObjectVector<CDirItem> &dirItems) {
    NFile::NFind::CFileInfo fi;
    NFile::NFind::CFindFile TEST;
    UString dirPath;
    dirPath = strDirPath;
    dirPath += "\\*.*";
    TEST.FindFirst(dirPath, fi);
    while (TEST.FindNext(fi))
    {
      if (fi.IsDots())
      {
        continue;
      }
      CDirItem di;
      di.Attrib = fi.Attrib;
      di.Size = fi.Size;
      di.CTime = fi.CTime;
      di.ATime = fi.ATime;
      di.MTime = fi.MTime;
      di.Name = sub_name;
      di.Name += L"\\";
      di.Name += fi.Name;
      di.FullPath = strDirPath;
      di.FullPath += L"\\";
      di.FullPath += fi.Name;
      dirItems.Add(di);
      if (fi.Attrib&FILE_ATTRIBUTE_DIRECTORY) {
        UString sub_dirPath;
        sub_dirPath = di.FullPath;
        UString sub_1 = sub_name;
        sub_1 += L"\\";
        sub_1 += fi.Name;
        GetArchiveItemFromPath(sub_dirPath, sub_1, dirItems);
      }
    }
  }
  void Wrapper7zArchive::GetArchiveItemFromFileList(CObjectVector<UString> FileList, CObjectVector<CDirItem> &ItemList) {
    NFile::NFind::CFileInfo fi;
    for (uint32_t i = 0;i < FileList.Size();i++)
    {
      fi.Find(FileList[i]);
      if (fi.Attrib&FILE_ATTRIBUTE_DIRECTORY){
        //FIXME:exist bug?
        GetArchiveItemFromPath(FileList[i], fi.Name, ItemList);
        continue;
      }
      else {
        CDirItem di;
        di.Attrib = fi.Attrib;
        di.Size = fi.Size;
        di.CTime = fi.CTime;
        di.ATime = fi.ATime;
        di.MTime = fi.MTime;
        di.Name = fi.Name;
        di.FullPath = FileList[i];
        ItemList.Add(di);
      }
    }
    return;
  }

}