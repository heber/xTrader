#pragma once

// DlgBfLog 对话框
class DlgBfLog : public CDialog
{
	DECLARE_DYNAMIC(DlgBfLog)

public:
	DlgBfLog(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~DlgBfLog();

	void UpdateLog();
// 对话框数据
	enum { IDD = IDD_DLG_BFDETAILS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CListCtrl m_LstLog;
	void InitList();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	afx_msg void OnDestroy();
	afx_msg LRESULT QryBfLogMsg(WPARAM wParam,LPARAM lParam);
};
