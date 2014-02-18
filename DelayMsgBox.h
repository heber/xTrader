#pragma once

class CDelayMsgBox : public CWnd
{
	DECLARE_DYNAMIC(CDelayMsgBox)

public:
	CDelayMsgBox(CWnd* pParent);

	int MessageBox(LPCTSTR lpszText,LPCTSTR lpTitle, int count, bool bclose = false, UINT uType = MB_ICONINFORMATION );	

protected:
	int m_count;
	bool m_autoclose;	
	HWND  m_hWndParent;

	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSubclassedInit(WPARAM wParam, LPARAM lParam);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTimer(UINT nIDEvent);

	DECLARE_MESSAGE_MAP()
};
