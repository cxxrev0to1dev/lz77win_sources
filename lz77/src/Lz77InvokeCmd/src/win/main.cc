#include <io.h>
#include <fcntl.h>  
#include <sys/types.h>  
#include <sys/stat.h> 
#include <fstream>
#include <sstream>
#include <process.h>
#include <Windows.h>
#include <commctrl.h>
#include <winerror.h>
#include <ShObjIdl.h>
#include <Shlobj.h>
#include "Lz77Lang/lz77_lang.h"
#include "base/string_conv.h"
#include "base/path.h"
#include "compressor/lib7zip_compressor.h"
#include "resource.h"
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"OleAut32.lib")
#pragma comment(lib,"Ole32.lib")
#pragma comment(lib,"Comdlg32.lib")
#pragma comment(lib,"Mpr.lib")
#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"user32.lib")	
#pragma comment(lib,"ws2_32.lib")	
#pragma comment(lib,"Advapi32.lib")

#include "rapidxml/rapidxml.hpp"  
#include "rapidxml/rapidxml_utils.hpp"//rapidxml::file  
#include "rapidxml/rapidxml_print.hpp"//rapidxml::print
#include "lib7zip/Lib7Zip/AskOpenArchivePassword.h"
#include "base/gen_pwd.h"

/*
#if defined(OS_WIN_X86)
#pragma comment(lib,"libmikmod.lib")
#else
#pragma comment(lib,"libmikmod_x64.lib")
#endif // WIN32
#if defined(_WIN32)
#define MikMod_Sleep(ns) Sleep(ns / 1000)
#elif defined(_MIKMOD_AMIGA)
void amiga_sysinit(void);
void amiga_usleep(unsigned long timeout);
#define MikMod_Sleep(ns) amiga_usleep(ns)
#else
#include <unistd.h>
#define MikMod_Sleep(ns) usleep(ns)
#endif
#include "mikmod.h"

int Player(const char* xm_path) {
  MikMod_InitThreads();

  MikMod_RegisterAllDrivers();

  MikMod_RegisterAllLoaders();
  md_mode |= DMODE_SOFT_MUSIC | DMODE_NOISEREDUCTION;
  if (MikMod_Init("")) {
    fprintf(stderr, "Could not initialize sound, reason: %s\n",
      MikMod_strerror(MikMod_errno));
    return 2;
  }

  MODULE* module = Player_Load(xm_path, 64, 0);
  if (module) {
    printf("Playing %s (%d chn)\n", module->songname, (int)module->numchn);
    Player_Start(module);
    while (Player_Active()) {
      MikMod_Sleep(10000);
      MikMod_Update();
    }

    Player_Stop();
    Player_Free(module);
  }
  else
    fprintf(stderr, "Could not load module, reason: %s\n",
      MikMod_strerror(MikMod_errno));

  MikMod_Exit();
  return 0;
}
*/

extern "C" IMAGE_DOS_HEADER __ImageBase;

void GetDir(std::wstring& dir_path) {
  wchar_t dll_folder[MAX_PATH + 1] = { 0 };
  GetModuleFileNameW((HMODULE)&__ImageBase, dll_folder, MAX_PATH);
  std::wstring dll_dirs(dll_folder);
  std::wstring::size_type pos = std::wstring(dll_dirs).find_last_of(L"\\/");
  dll_dirs = std::wstring(dll_dirs).substr(0, pos);
  dir_path = dll_dirs;
}

void Read(const wchar_t* path, std::string& out) {
  std::ifstream is(path, std::ifstream::binary);
  if (is) {
    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);
    char* buffer = new char[length + sizeof(char)];
    memset(buffer, 0, length + sizeof(char));
    is.read(buffer, length);
    if (is) {
      out = buffer;
    }
    is.close();
    delete[] buffer;
  }
}

bool GetSendtoFolder(HWND hwnd,std::wstring& out_dirs){
  LPITEMIDLIST pidl;
  LPSHELLFOLDER g_psfDesktop;
  if (FAILED(SHGetDesktopFolder(&g_psfDesktop))) {
    return true;
  }
  HRESULT hres = SHGetSpecialFolderLocation(hwnd, CSIDL_SENDTO, &pidl);
  if (SUCCEEDED(hres)) {
    wchar_t olePath[MAX_PATH] = { 0 };
    SHGetPathFromIDListW((LPCITEMIDLIST)pidl, olePath);
    out_dirs = olePath;
    CoTaskMemFree(pidl);
  }
  return false;
}
bool CreateLink(LPCWSTR szPath, LPCWSTR szLink){
  CoInitialize(NULL);
  IShellLink* psl;
  IPersistFile* ppf;
  HRESULT hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
    IID_IShellLink, (void**)&psl);
  if (FAILED(hres))
    return true;
  psl->SetPath(szPath);
  hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
  if (FAILED(hres))
    return true;
  hres = ppf->Save(szLink, STGM_READWRITE);
  ppf->Release();
  psl->Release();
  CoUninitialize();
  return false;
}
bool GetUserSelectOutDir(std::wstring& out_dir) {
  out_dir.resize(0);
  std::wstring lpszTitle = lang::LZ77Language::GetInstannce()->GetErrorMsg(L"kArchiveExtractOutDir");
  bool result_fail = true;
  BROWSEINFOW sInfo = { 0 };
  sInfo.pidlRoot = 0;
  sInfo.lpszTitle = lpszTitle.c_str();
  sInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN | BIF_NEWDIALOGSTYLE;
  sInfo.lpfn = NULL;
  LPITEMIDLIST lpidlBrowse = ::SHBrowseForFolderW(&sInfo);
  if (lpidlBrowse != NULL) {
    wchar_t szFolderPath[MAX_PATH] = { 0 };
    if (::SHGetPathFromIDListW(lpidlBrowse, szFolderPath)) {
      out_dir = szFolderPath;
      result_fail = false;
    }
  }
  if (lpidlBrowse != NULL) {
    ::CoTaskMemFree(lpidlBrowse);
  }
  return result_fail;
}

void SetDlgCenter(HWND hWnd) {
  RECT rc, rc1, rctomove;
  int width = GetSystemMetrics(SM_CXSCREEN);
  int height = GetSystemMetrics(SM_CYSCREEN);
  rc.left = 0;
  rc.top = 0;
  rc.right = width;
  rc.bottom = height;
  GetClientRect(hWnd, &rc1);
  rctomove.left = (rc.right - rc.left) / 2 - (rc1.right - rc1.left) / 2;
  rctomove.right = (rc.right - rc.left) / 2 + (rc1.right - rc1.left) / 2;
  rctomove.top = (rc.bottom - rc.top) / 2 - (rc1.bottom - rc1.top) / 2;
  rctomove.bottom = (rc.bottom - rc.top) / 2 + (rc1.bottom - rc1.top) / 2;
  ::SetWindowPos(hWnd, HWND_TOPMOST, rctomove.left, rctomove.top, rc1.right - rc1.left, rc1.bottom - rc1.top, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);
}
wchar_t in_password[4096] = { 0 };
wchar_t out_arc_fmt[128] = { 0 };
bool need_extract = false;
INT_PTR CALLBACK PwdDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  int iXpos = 0;
  int iYpos = 0;
  switch (uMsg)
  {
  case WM_INITDIALOG:
    SetDlgCenter(hWnd);
    ::SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE)&~WS_SIZEBOX);
    SetWindowTextW(hWnd, lang::LZ77Language::GetInstannce()->GetErrorMsg(L"kArchiveExtractPasswordDlgTitle").c_str());
    break;
  case WM_CLOSE:
    if (!in_password[0]){
      ExitProcess(0);
    }
    EndDialog(hWnd, 0);
    break;
  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDOK:
      if (GetDlgItemTextW(hWnd, IDC_EDIT_PWD, in_password, 4095) == 0){
        break;
      }
      EndDialog(hWnd, 0);
    }
    break;
  }
  return 0;
}

static void SetComoHight(HWND hWnd, HWND hCombo) {
  RECT rt;
  POINT p1;
  GetWindowRect(hCombo, &rt);
  p1.x = rt.left;
  p1.y = rt.top;
  int width = rt.right - rt.left;
  int height = (rt.bottom - rt.top) * 5;
  ScreenToClient(hWnd, &p1);
  MoveWindow(hCombo, p1.x, p1.y, width, height, TRUE);
}

static void InitComoArchiveFmt(HWND hWnd) {
  HWND hCombo = GetDlgItem(hWnd, IDC_COMBO_ARCHIVE_FMT);
  if (need_extract)
  {
    for (size_t i = 0; compressor::kCompressArchiveTable[i]; i++) {
      SendMessageW(hCombo, CB_ADDSTRING, i,
        (LPARAM)compressor::kCompressArchiveTable[i]);
    }
  }
  else {
    for (size_t i = 0; compressor::kDoNeedExtractArcName[i]; i++) {
      SendMessageW(hCombo, CB_ADDSTRING, i,
        (LPARAM)compressor::kDoNeedExtractArcName[i]);
    }
  }
  SetComoHight(hWnd, hCombo);
  SendMessage(hCombo, CB_SETCURSEL, 0, 0);
}

INT_PTR CALLBACK ArcDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  int iXpos = 0;
  int iYpos = 0;
  switch (uMsg)
  {
  case WM_INITDIALOG:
    InitComoArchiveFmt(hWnd);
    SetDlgCenter(hWnd);
    ::SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE)&~WS_SIZEBOX);
    SetWindowTextW(hWnd, lang::LZ77Language::GetInstannce()->GetErrorMsg(L"kArchiveOutFmtTitle").c_str());
    break;
  case WM_CLOSE:
    ExitProcess(0);
    EndDialog(hWnd, 0);
    break;
  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDC_BUTTON_OK:
      SendMessage(GetDlgItem(hWnd, IDC_COMBO_ARCHIVE_FMT), CB_GETLBTEXT, 
        SendMessage(GetDlgItem(hWnd, IDC_COMBO_ARCHIVE_FMT), CB_GETCURSEL, 0, 0), 
        LPARAM(out_arc_fmt));
      EndDialog(hWnd, 0);
      break;
    }
    break;
  }
  return 0;
}

static bool TestCreateFileFail(const std::wstring& file) {
  std::wofstream f(file, std::ios::out | std::ios::binary);
  if (f.is_open()) {
    f.close();
    f.clear();
    DeleteFileW(file.c_str());
    return false;
  }
  return true;
}

static bool FindExt(const char *p, const std::wstring &name)
{
  int dotPos = name.find_last_of(L".");
  if (dotPos < 0 || dotPos == (int)name.length() - 1)
    return false;

  std::string s;

  for (unsigned pos = dotPos + 1;; pos++)
  {
    wchar_t c = name[pos];
    if (c == 0)
      break;
    if (c >= 0x80)
      return false;
    s += (char)tolower((char)c);
  }

  for (unsigned i = 0; p[i] != 0;)
  {
    unsigned j;
    for (j = i; p[j] != ' '; j++);
    if (s.length() == j - i && memcmp(p + i, s.c_str(), s.length()) == 0)
      return true;
    i = j + 1;
  }

  return false;
}

static bool DoNeedExtract(const std::wstring &name)
{
  return !FindExt(compressor::kExtractExludeExtensions, name);
}
/*
unsigned __stdcall bar(void * x){
  int* exit_status = (int*)x;
  std::wstring dirs;
  GetDir(dirs);
  std::string xxxx = base::StringConv::narrow(dirs);
  xxxx += "\\res\\music.xm";
  while (true){
    if (*exit_status == 1){
      _endthreadex(0);
    }
    Player(xxxx.c_str());
  }
  return 0;
}
*/
unsigned __stdcall PwdInputThread(void* x) {
  DialogBoxW((HINSTANCE)x, MAKEINTRESOURCE(IDD_PWD_DLG), NULL, (DLGPROC)PwdDlgProc);
  return 0;
}

class AskOpenArchivePasswordUI:public AskOpenArchivePassword
{
public:
  AskOpenArchivePasswordUI() {
    password_.resize(0);
    is_ask_pwd_ok_ = false;
    root_dir_.resize(0);
  }
  virtual ~AskOpenArchivePasswordUI() {
    password_.resize(0);
    is_ask_pwd_ok_ = false;
  }
  void Set(HINSTANCE hinstance) {
    hinstance_ = hinstance;
  }
  bool IsAskPwdOK() const {
    return is_ask_pwd_ok_;
  }
  virtual void AskPasswordUI() {
    if (!is_ask_pwd_ok_){
      DialogBoxW(hinstance_, MAKEINTRESOURCE(IDD_PWD_DLG), NULL, (DLGPROC)PwdDlgProc);
      if (in_password[0])
      {
        SetPassword(in_password);
        is_ask_pwd_ok_ = true;
      }
    }
  }
  virtual const wchar_t* GetPassword() {
    return password_.c_str();
  }
  virtual void SetPassword(const wchar_t* pwd) {
    if (pwd&&pwd[0]) {
      password_ = pwd;
    }
  }
private:
  std::wstring root_dir_;
  std::wstring password_;
  HINSTANCE hinstance_;
  bool is_ask_pwd_ok_;
};
int WINAPI wWinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPWSTR lpCmdLine,int nShowCmd){
  //Sleep(15000);
  InitCommonControls();
  int exit_status = 0;
  //HANDLE play_thread = (HANDLE)_beginthreadex(nullptr, 0, bar, &exit_status, 0, nullptr);
  std::string xml_lang;
  Read(lpCmdLine, xml_lang);
  std::istringstream is(xml_lang);

  rapidxml::file<> fdoc(is);
  rapidxml::xml_document<> doc;
  doc.parse<0>(fdoc.data());
  std::string command;
  std::vector<std::wstring> path_list;
  for (size_t i = 0; i < 1; i++){
    const rapidxml::xml_node<> *invoke_command_node = doc.first_node(lang::kConstStrInvokeCommand);
    if (!invoke_command_node){
      break;
    }
    const rapidxml::xml_node<> *command_node = invoke_command_node->first_node(lang::kConstStrCommand);
    if (!command_node) {
      break;
    }
    rapidxml::xml_attribute<char>* op_node = command_node->first_attribute(lang::kConstStrOp);
    if (!op_node) {
      break;
    }
    command = op_node->value();
    rapidxml::xml_node<> *path_list_node = command_node->first_node(lang::kConstStrPathList);
    if (!path_list_node) {
      break;
    }
    rapidxml::xml_node<> *datum = (path_list_node == nullptr ? nullptr : path_list_node->first_node(lang::kConstStrPath));
    while (NULL != datum) {
      rapidxml::xml_attribute<char>* attr = datum->first_attribute(lang::kConstStrString);
      std::wstring t = base::StringConv::widen(attr->value());
      path_list.push_back(t);
#if defined(OS_WIN_X86)
      struct _stat status;
#else
      struct _stat64i32 status;
#endif
      _wstat(t.c_str(), &status);
      if (((status.st_mode & S_IFDIR) == 0) && DoNeedExtract(t)){
        need_extract = true;
      }
      datum = datum->next_sibling();
    }
  }
  if (!command.empty() && !path_list.empty()) {
    std::wstring wcommand = base::StringConv::widen(command);
    std::wstring src_path = path_list[0];
    base::Path src_path_parse(src_path);
    if (wcommand.compare(lang::kStrMenuArchive) == 0) {
      for (int i =0;i<1;i++){
        DialogBoxW(hInstance, MAKEINTRESOURCE(IDD_ARC_DLG), NULL, (DLGPROC)ArcDlgProc);
        compressor::ArchiveCompressor archive_compressor(nullptr);
        if (!archive_compressor.IsSupportedARCExt(out_arc_fmt)) {
          std::wstring password;
          if (!archive_compressor.IsSupportedCryptARC(out_arc_fmt)) {
            password = base::StringConv::widen(base::GenPwd::GetInstance()->GetPasscode());
            password = password.substr(0, password.size() - sizeof(wchar_t));
          }
          std::wstring out_arc;
          if (archive_compressor.IsDoNeedExtractArcName(out_arc_fmt)) {//FIXME
            out_arc = src_path_parse.FullDir();
          }
          else {
            out_arc = src_path;
          }
          out_arc.append(L".");
          out_arc.append(compressor::ArchiveCompressor::ToFixOutExt(out_arc_fmt));
          if (TestCreateFileFail(out_arc)) {
            MessageBoxW(GetDesktopWindow(), out_arc.c_str(), L"UAC ERROR", MB_ICONERROR);
            break;
          }
          archive_compressor.compressor(path_list, out_arc, password);
          if (!archive_compressor.IsCompressOK()) {
            MessageBoxW(GetDesktopWindow(), out_arc.c_str(), L"Fail", MB_ICONERROR);
            break;
          }
          else {
            if (password.length() && _waccess(out_arc.c_str(), 0) == 0) {
              std::wstring readme_file = out_arc + L".ReadMe.txt";
              std::wofstream f(readme_file, std::ios::out | std::ios::binary);
              f << "Password£º";
              f << password;
              f << "\r\n";
              f.flush();
              f.close();
              MessageBoxW(GetDesktopWindow(), L"OK", L"OK", MB_ICONINFORMATION);
            }
          }
        }
      }
    }
    else if (wcommand.compare(lang::kStrMenuExtract) == 0) {
      std::wstring out_dir;
      out_dir = src_path_parse.FullDir();
      for (size_t i = 0; i < 1; i++) {
        AskOpenArchivePasswordUI ask_open_archive_password_ui;
        ask_open_archive_password_ui.Set(hInstance);
        compressor::ArchiveCompressor archive_compressor(&ask_open_archive_password_ui);
        if (archive_compressor.TestAttributeFlag(src_path)) {
          if (!archive_compressor.IsSignedFile()) {
            std::wstring sss = lang::LZ77Language::GetInstannce()->GetErrorMsg(L"kFileTypeNotSupport");
            ::MessageBoxW(GetDesktopWindow(), sss.c_str(), L"ERROR", MB_ICONERROR);
            break;
          }
        }
        std::wstring pwd;
        if (!ask_open_archive_password_ui.IsAskPwdOK()&&archive_compressor.IsPasswordDefined()){
          DialogBoxW(hInstance, MAKEINTRESOURCE(IDD_PWD_DLG), NULL, (DLGPROC)PwdDlgProc);
          if (in_password[0]){
            pwd = in_password;
          }
        }
        else {
          pwd = ask_open_archive_password_ui.GetPassword();
        }
        archive_compressor.decompressor(src_path, out_dir, pwd);
        if (!archive_compressor.IsDecompressOK()) {
          const std::wstring op_msg = archive_compressor.OpResMsg();
          ::MessageBoxW(GetDesktopWindow(), op_msg.c_str(), L"ERROR", MB_ICONERROR);
        }
        MessageBoxW(GetDesktopWindow(), L"OK", L"OK", MB_ICONINFORMATION);
      }
    }
    else if (wcommand.compare(lang::kStrMenuSendto) == 0) {
      std::wstring sendto_dirs;
      GetSendtoFolder(GetDesktopWindow(), sendto_dirs);
      sendto_dirs.append(L"\\");
      sendto_dirs.append(src_path_parse.fname());
      sendto_dirs.append(L".lnk");
      if (_waccess(src_path.c_str(), 0)==0 && _waccess(sendto_dirs.c_str(), 0) != 0){
        if (CreateLink(src_path.c_str(), sendto_dirs.c_str())) {
          MessageBoxA(GetDesktopWindow(), command.c_str(), "ERROR", MB_ICONERROR);
        }
      }
    }
    else {
      MessageBoxW(GetDesktopWindow(), L"what?", L"what?", MB_ICONERROR);
    }
  }
  DeleteFileW(lpCmdLine);
  /*
  exit_status = 1;
  WaitForSingleObject(play_thread, 2000);
  CloseHandle(play_thread);
  */
  return 0;
}