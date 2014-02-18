// MyCombo.cpp : implementation file
//

#include "stdafx.h"
#include "MyCombo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MyCombo

MyCombo::MyCombo()
{
}

MyCombo::~MyCombo()
{
}


BEGIN_MESSAGE_MAP(MyCombo, CComboBox)
	//{{AFX_MSG_MAP(MyCombo)
	ON_WM_CTLCOLOR()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// MyCombo message handlers

HBRUSH MyCombo::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CComboBox::OnCtlColor(pDC, pWnd, nCtlColor);
	switch(nCtlColor)
	{
	case CTLCOLOR_EDIT:
		break;
	case CTLCOLOR_LISTBOX:
		int iItemNum=GetCount();
		int iWidth=0;
		CString strItem;
		CClientDC dc(this);
		int iSaveDC=dc.SaveDC();
		dc.SelectObject(GetFont());
		int iVSWidth=::GetSystemMetrics(SM_CXVSCROLL);
		for(int i=0;i<iItemNum;i++)
		{
			GetLBText(i,strItem);
			int iWholeWidth=dc.GetTextExtent(strItem).cx+iVSWidth;
			iWidth=max(iWidth,iWholeWidth);
		}
		iWidth+=dc.GetTextExtent(_T("a")).cx;
		dc.RestoreDC(iSaveDC);
		if(iWidth>0)
		{
			CRect rc;
			pWnd->GetWindowRect(&rc);
			if(rc.Width()!=iWidth)
			{
				rc.right=rc.left+iWidth;
				pWnd->MoveWindow(&rc);
			}
		}
		break;
	}
	return hbr;
}
