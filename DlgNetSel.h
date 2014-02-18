#pragma once
#include "XListCtrl/XListCtrl.h"

class DlgNetSel : public CDialog
{
	DECLARE_DYNAMIC(DlgNetSel)

public:
	DlgNetSel(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~DlgNetSel();

// 对话框数据
	enum { IDD = IDD_DLG_NETSET };

	static UINT TestSvr(LPVOID pParam);
public:
	void InitLists();
	void InitCombo();
	void GetDelay();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_bSockType;
	CWinThread* m_pTdTest;
	CWinThread* m_pMdTest;
	CComboBox m_CombSockLst;
	CString m_szSkAddr;
	CString m_szSkPort;
	CString m_szSkUser;
	CString m_szSkPwd;
	CXListCtrl m_LstTds;
	CXListCtrl m_LstMds;
	CString m_szGrpName;
	int m_iIdx;
	CStringArray m_szArTs,m_szArMd;
private:
	afx_msg void OnBnClkChkSock();
	afx_msg void OnBnClickedOk();
	afx_msg void OnNMClickLstTrade(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickLstMd(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();
	afx_msg void OnOK();
	afx_msg void OnCancel();
};

struct TESTSVRPARAM
{
	PVOID pDlg;
	CString szSvr;
	__int64 iDelayNs;
};