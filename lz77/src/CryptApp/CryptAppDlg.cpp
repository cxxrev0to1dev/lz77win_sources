
// CryptAppDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CryptApp.h"
#include "CryptAppDlg.h"
#include "afxdialogex.h"
#include "base/base_export.h"
#include "crypt/crypt_export.h"
#include "compressor/compressor_exports.h"
#include "compressor/lib7zip_compressor.h"
#include "crypt/ecies/bignum_key.h"
#include "crypt/ecies/encryption_file.h"
#include "crypt/ecies/decryption_file.h"
#include "crypt/ecies/ecies_gen_key.h"

#define EN_LANGUAGE 1

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
	EnableActiveAccessibility();
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CCryptAppDlg dialog



CCryptAppDlg::CCryptAppDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_CRYPTAPP_DIALOG, pParent)
{
	EnableActiveAccessibility();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCryptAppDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialogEx::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_EDIT_PRIV_KEY, m_priv_key);
  //  DDX_Control(pDX, IDC_PROGRESS1, m_work_progress);
}

BEGIN_MESSAGE_MAP(CCryptAppDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
  ON_BN_CLICKED(IDC_BUTTON_ENCRYPT, &CCryptAppDlg::OnBnClickedButtonEncrypt)
  ON_BN_CLICKED(IDC_BUTTON_DECRYPT, &CCryptAppDlg::OnBnClickedButtonDecrypt)
  ON_WM_DROPFILES()
END_MESSAGE_MAP()


// CCryptAppDlg message handlers

BOOL CCryptAppDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

  //test archive uncompress
  /*
  compressor::ArchiveCompressor archive_compressor;
  archive_compressor.IsSupportedExt(L"D:\\privatespace\\CryptApp\\src\\third_party\\lib7zip\\Test7Zip\\Test7Zip.7z");
  archive_compressor.decompressor(L"D:\\privatespace\\CryptApp\\src\\third_party\\lib7zip\\Test7Zip\\Test7Zip.7z",
    L"D:\\privatespace\\CryptApp\\bin\\Debug\\out_dir", 
    L"test");
  if (!archive_compressor.IsSupportedARCExt(L"bzip2")) {
    archive_compressor.compressor(L"D:\\privatespace\\CryptApp\\src\\third_party\\lib7zip\\Test7Zip\\Test7Zip.tar",
      L"D:\\privatespace\\CryptApp\\src\\third_party\\lib7zip\\Test7Zip\\Test7Zip1.tar.bz2",
      L"");
  }
  */
  DragAcceptFiles(TRUE);
#if defined(EN_LANGUAGE)
  GetDlgItem(IDC_STATIC_IN_FILE)->SetWindowText(_T("Encrypt source file"));
  GetDlgItem(IDC_STATIC_OUT_FILE)->SetWindowText(_T("Encrypted output file"));
  GetDlgItem(IDC_BUTTON_ENCRYPT)->SetWindowText(_T("Encrypt"));
  GetDlgItem(IDC_BUTTON_DECRYPT)->SetWindowText(_T("Decrypt"));
  GetDlgItem(IDC_EDIT_PRIV_KEY)->SetWindowText(_T("File key"));
#else
  GetDlgItem(IDC_STATIC_IN_FILE)->SetWindowText(_T("输入文件"));
  GetDlgItem(IDC_STATIC_OUT_FILE)->SetWindowText(_T("输出文件"));
  GetDlgItem(IDC_BUTTON_ENCRYPT)->SetWindowText(_T("加密"));
  GetDlgItem(IDC_BUTTON_DECRYPT)->SetWindowText(_T("解密"));
  GetDlgItem(IDC_EDIT_PRIV_KEY)->SetWindowText(_T("文件密钥"));
#endif
  ::SetWindowLong(m_hWnd, GWL_STYLE, GetWindowLong(m_hWnd, GWL_STYLE)&~WS_SIZEBOX);
  Crypt::ECKeyGen key_gen;
  key_gen.ResetRandomSeed();
	// TODO: Add extra initialization here
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCryptAppDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCryptAppDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCryptAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CCryptAppDlg::OnBnClickedButtonEncrypt()
{
  _beginthreadex(NULL, 0, (_beginthreadex_proc_type)EncryptionThread, this, 0, NULL);
  // TODO: Add your control notification handler code here
}


void CCryptAppDlg::OnBnClickedButtonDecrypt()
{
  _beginthreadex(NULL, 0, (_beginthreadex_proc_type)DecryptionThread, this, 0, NULL);
  // TODO: Add your control notification handler code here
}


void CCryptAppDlg::OnDropFiles(HDROP hDropInfo)
{
  // TODO: Add your message handler code here and/or call default
  LPTSTR pFilePathName = (LPTSTR)malloc(MAX_PATH);
  ::DragQueryFile(hDropInfo, 0, pFilePathName, MAX_PATH);
  ::DragFinish(hDropInfo);
  m_infile = pFilePathName;
  if (m_infile.Right(m_infile.GetLength() - m_infile.ReverseFind('.') - 1) == "crypt") {
    GetDlgItem(IDC_STATIC_IN_FILE)->SetWindowText(pFilePathName);
    m_outfile = pFilePathName;
    m_outfile.Replace(_T(".crypt"),_T(""));
    GetDlgItem(IDC_BUTTON_ENCRYPT)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_DECRYPT)->EnableWindow(TRUE);
    
  }
  else {
    GetDlgItem(IDC_STATIC_IN_FILE)->SetWindowText(pFilePathName);
    m_outfile = pFilePathName;
    m_outfile.Append(_T(".crypt"));
    GetDlgItem(IDC_BUTTON_DECRYPT)->EnableWindow(FALSE);
    GetDlgItem(IDC_BUTTON_ENCRYPT)->EnableWindow(TRUE);
  }

  GetDlgItem(IDC_STATIC_OUT_FILE)->SetWindowText(m_outfile);
  free(pFilePathName);
  CDialogEx::OnDropFiles(hDropInfo);
}
void CCryptAppDlg::SetWindowStatus(BOOL is_enable) {
  GetDlgItem(IDC_STATIC_IN_FILE)->EnableWindow(is_enable);
  GetDlgItem(IDC_STATIC_OUT_FILE)->EnableWindow(is_enable);
  GetDlgItem(IDC_EDIT_PRIV_KEY)->EnableWindow(is_enable);
  GetDlgItem(IDC_BUTTON_ENCRYPT)->EnableWindow(is_enable);
  GetDlgItem(IDC_BUTTON_DECRYPT)->EnableWindow(is_enable);
}
void CCryptAppDlg::ClearWindowText() {
#if defined(EN_LANGUAGE)
  GetDlgItem(IDC_STATIC_IN_FILE)->SetWindowText(_T("Encrypt source file"));
  GetDlgItem(IDC_STATIC_OUT_FILE)->SetWindowText(_T("Encrypted output file"));
#else
  GetDlgItem(IDC_STATIC_IN_FILE)->SetWindowText(_T("输入文件"));
  GetDlgItem(IDC_STATIC_OUT_FILE)->SetWindowText(_T("输出文件"));
#endif
}
DWORD __stdcall CCryptAppDlg::EncryptionThread(void *pArg) {
  CCryptAppDlg* dlg_this = reinterpret_cast<CCryptAppDlg*>(pArg);
  //Crypt::ECKeyGen key_gen;
  USES_CONVERSION;
  const std::wstring priv_key = A2CW(Crypt::ECIESKey::CreateInstance()->priv_key());
  Crypt::EncryptionFile encrypt_file;
  dlg_this->SetWindowStatus(FALSE);
#if defined(EN_LANGUAGE)
  dlg_this->SetWindowText(_T("Encrypting, please wait..."));
#else
  dlg_this->SetWindowText(_T("正在加密请稍后..."));
#endif
  dlg_this->GetDlgItem(IDC_STATIC_IN_FILE)->GetWindowText(dlg_this->m_infile);
  if (!PathFileExistsW(dlg_this->m_infile.GetBuffer())){
#if defined(EN_LANGUAGE)
    ::MessageBoxW(::GetActiveWindow(), L"Encrypted source file does not exist", L"ERROR", MB_ICONERROR);
#else
    ::MessageBoxW(::GetActiveWindow(), L"加密输入文件不存在", L"ERROR", MB_ICONERROR);
#endif
    dlg_this->SetWindowStatus(TRUE);
    dlg_this->SetWindowText(_T("CryptApp"));
    Crypt::ECIESKey::DeleteInstance();
    return 0;
  }
  dlg_this->GetDlgItem(IDC_STATIC_OUT_FILE)->GetWindowText(dlg_this->m_outfile);
  const std::string src = CT2A(dlg_this->m_infile.GetBuffer());
  const std::string dst = CT2A(dlg_this->m_outfile.GetBuffer());
  encrypt_file.CryptFile(Crypt::ECIESKey::CreateInstance()->pub_key(), src.c_str(), dst.c_str());
  dlg_this->GetDlgItem(IDC_EDIT_PRIV_KEY)->SetWindowText(priv_key.c_str());
  dlg_this->m_priv_key.LineScroll(dlg_this->m_priv_key.GetLineCount());
  dlg_this->SetWindowStatus(TRUE);
  dlg_this->ClearWindowText();
  dlg_this->SetWindowText(_T("CryptApp"));
  Crypt::ECIESKey::DeleteInstance();
  return 0;
}
DWORD __stdcall CCryptAppDlg::DecryptionThread(void *pArg) {
  CCryptAppDlg* dlg_this = reinterpret_cast<CCryptAppDlg*>(pArg);
  Crypt::DecryptionFile decrypt_file;
  dlg_this->SetWindowStatus(FALSE);
#if defined(EN_LANGUAGE)
  dlg_this->SetWindowText(_T("Decrypting, please wait..."));
#else
  dlg_this->SetWindowText(_T("正在解密请稍后..."));
#endif
  dlg_this->GetDlgItem(IDC_STATIC_IN_FILE)->GetWindowText(dlg_this->m_infile);
  dlg_this->GetDlgItem(IDC_STATIC_OUT_FILE)->GetWindowText(dlg_this->m_outfile);
  if (dlg_this->m_infile.Right(dlg_this->m_infile.GetLength() - dlg_this->m_infile.ReverseFind('.') - 1) != "crypt") {
#if defined(EN_LANGUAGE)
    ::MessageBoxW(::GetActiveWindow(), L"Please reselect the decrypted file (.crypt)", L"ERROR", MB_ICONERROR);
#else
    ::MessageBoxW(::GetActiveWindow(),L"请重新选择解密文件",L"ERROR", MB_ICONERROR);
#endif
    dlg_this->SetWindowStatus(TRUE);
    dlg_this->SetWindowText(_T("CryptApp"));
    return 0;
  }
  if (!PathFileExistsW(dlg_this->m_infile.GetBuffer())) {
#if defined(EN_LANGUAGE)
    ::MessageBoxW(::GetActiveWindow(), L"Decrypt source file does not exist", L"ERROR", MB_ICONERROR);
#else
    ::MessageBoxW(::GetActiveWindow(), L"解密输入文件不存在", L"ERROR", MB_ICONERROR);
#endif
    dlg_this->SetWindowStatus(TRUE);
    dlg_this->SetWindowText(_T("CryptApp"));
    return 0;
  }
  CString priv_key;
  dlg_this->GetDlgItem(IDC_EDIT_PRIV_KEY)->GetWindowText(priv_key);
  if (priv_key.GetLength()<=0){
#if defined(EN_LANGUAGE)
    ::MessageBoxW(::GetActiveWindow(), L"Please check File key length", L"ERROR", MB_ICONERROR);
#else
    ::MessageBoxW(::GetActiveWindow(), L"请检查输入密钥长度", L"ERROR", MB_ICONERROR);
#endif
    dlg_this->SetWindowStatus(TRUE);
    dlg_this->SetWindowText(_T("CryptApp"));
    return 0;
  }
  USES_CONVERSION;
  const std::string ppp = CT2A(priv_key.GetBuffer());
  decrypt_file.CryptFile(ppp, CT2A(dlg_this->m_infile.GetBuffer()), CT2A(dlg_this->m_outfile.GetBuffer()));
  dlg_this->SetWindowStatus(TRUE);
  dlg_this->ClearWindowText();
  dlg_this->SetWindowText(_T("CryptApp"));
  return 0;
}