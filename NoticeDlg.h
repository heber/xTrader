#pragma once

#define IDD_DLG_NOTICE 105
// CNoticeDlg 对话框

class CNoticeDlg : public CDialog
{
	DECLARE_DYNAMIC(CNoticeDlg)

public:
	CNoticeDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CNoticeDlg();
// 对话框数据
	enum { IDD = IDD_DLG_NOTICE };

	CString m_SzMsg;
	CString m_szTitle;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnOK();
	afx_msg void OnCancel();
};
