
#pragma once

#define IDD_LOGIN 104

/////////////////////////////////////////////////////////////////////////////
// LoginDlg dialog

class LoginDlg : public CDialog
{
// Construction
public:
	LoginDlg(CWnd* pParent = NULL);   // standard constructor

	~LoginDlg();

	void LoadConfig();
	void SaveConfig();
	//int DoLogin();
	void ProgressUpdate(LPCTSTR szMsg,const int nPercentDone);
	static UINT LoginThread(LPVOID pParam);
// Dialog Data
	//{{AFX_DATA(LoginDlg)
	enum { IDD = IDD_LOGIN };
	MyCombo m_ComboIsp,m_ComboBkr;
	CString	m_szUser;
	CString	m_szPass;
	//CString m_szDefTds,m_szDefMds;
	//int		m_iPort;
	BOOL	m_bSave;
	int		m_iSel1,m_iSel2;
	CProgressCtrl	m_prgs;
	CStatic	m_staInfo;
	CString m_szBrkName;
	CStringArray m_szArTs,m_szArMd;
	//}}AFX_DATA

public:
	CWinThread *m_pLogin;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(LoginDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

protected:
	HICON m_hIcon;
// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(LoginDlg)
	afx_msg void OnLogin();
	afx_msg void OnQuit();
	afx_msg void OnNetset();
	afx_msg void OnSave();
	afx_msg void OnSelISP();
	afx_msg void OnSelBkr();
	afx_msg void OnDestroy();
	afx_msg void OnChangeUser();
	afx_msg void OnChangePass();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClkExtra();
};
