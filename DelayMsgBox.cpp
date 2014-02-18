#include "stdafx.h"
#include "DelayMsgBox.h"


IMPLEMENT_DYNAMIC(CDelayMsgBox, CWnd)
CDelayMsgBox::CDelayMsgBox(CWnd* pParent)
{
	m_hWndParent = pParent->GetSafeHwnd();    // can be NULL
	m_autoclose = NULL;
	m_count = 0;
}

BEGIN_MESSAGE_MAP(CDelayMsgBox, CWnd)
	ON_WM_TIMER()
	ON_WM_CREATE()
	ON_MESSAGE(WM_INITDIALOG, OnSubclassedInit)
END_MESSAGE_MAP()


// Purpose: Unhook window creation
int CDelayMsgBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	AfxUnhookWindowCreate();
	return CWnd::OnCreate(lpCreateStruct);
}

// Purpose: Disable OK button, start timer
LRESULT CDelayMsgBox::OnSubclassedInit(WPARAM wParam, LPARAM lParam)
{
	LRESULT lRet = Default();
	CWnd* pOk = GetDlgItem(IDCANCEL);
	if ( NULL != pOk )
		pOk->EnableWindow(FALSE);
	SetTimer(100,1000,NULL);			
	return lRet;
}

// Purpose: display running countdown, close when finished.
void CDelayMsgBox::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == 100)
	{		
		if (m_count>0)
			m_count--;

		if (m_count == 0)
		{
			CWnd* pOk = GetDlgItem(IDCANCEL);
			if ( NULL != pOk )
			{
				pOk->EnableWindow(TRUE);
				pOk->SetFocus();
			}
			KillTimer(100);
			if (m_autoclose)
				PostMessage(WM_CLOSE,0,0);
		}
	}
}

// Purpose: Display a message box, hooking it to do stuff
int CDelayMsgBox::MessageBox(LPCTSTR lpszText,LPCTSTR lpTitle, int count, bool bclose,UINT uType)
{	
	m_autoclose = bclose;
	m_count = count;
	AfxHookWindowCreate(this);
	return ::MessageBox(m_hWndParent, lpszText,lpTitle, uType);
}

// Purpose: prevent dialog from closing before it has timed out
LRESULT CDelayMsgBox::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_COMMAND && m_count > 0)
	{
		if(HIWORD(wParam) == BN_CLICKED )			
				return 0;
	}
	return CWnd::DefWindowProc(message, wParam, lParam);
}
