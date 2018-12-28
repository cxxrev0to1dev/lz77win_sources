
// CryptAppDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"


// CCryptAppDlg dialog
class CCryptAppDlg : public CDialogEx
{
// Construction
public:
	CCryptAppDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CRYPTAPP_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
  afx_msg void OnBnClickedButtonEncrypt();
  afx_msg void OnBnClickedButtonDecrypt();
  afx_msg void OnDropFiles(HDROP hDropInfo);
  void SetWindowStatus(BOOL is_enable);
  void ClearWindowText();
  static DWORD __stdcall EncryptionThread(void *pArg);
  static DWORD __stdcall DecryptionThread(void *pArg);
  CString m_infile;
  CString m_outfile;
  CEdit m_priv_key;
//  CProgressCtrl m_work_progress;
};
