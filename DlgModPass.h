
#pragma once
/////////////////////////////////////////////////////////////////////////////
// DlgModPass dialog

class DlgModPass : public CDialog
{
// Construction
public:
	DlgModPass(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(DlgModPass)
	enum { IDD = IDD_DLG_MODPASS };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA


	int		m_itype;
	CString m_szNewPass,m_szOldPass,m_szNewCfm;

	CWinThread *m_pModPass;
	static UINT ModPwdThread(LPVOID pParam);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(DlgModPass)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//virtual BOOL OnInitDialog();
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(DlgModPass)
	afx_msg void OnTradepass();
	afx_msg void OnAccpass();
	afx_msg void OnChangeOripass();
	afx_msg void OnChangeNewpass();
	afx_msg void OnChangeNewpasscfm();
	afx_msg void OnDestroy();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
