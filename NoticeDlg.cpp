// CNoticeDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "NoticeDlg.h"

#define IDC_NTMSG 1065
// CNoticeDlg 对话框

IMPLEMENT_DYNAMIC(CNoticeDlg, CDialog)
CNoticeDlg::CNoticeDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNoticeDlg::IDD, pParent)
{
	m_szTitle = _T("");
}

CNoticeDlg::~CNoticeDlg()
{
}

void CNoticeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NTMSG, m_SzMsg);
}


BEGIN_MESSAGE_MAP(CNoticeDlg, CDialog)
ON_WM_DESTROY()
END_MESSAGE_MAP()


BOOL CNoticeDlg::OnInitDialog()
{

	SetWindowText(m_szTitle);

	((CEdit*)GetDlgItem(IDC_NTMSG))->SetWindowText(m_SzMsg);

	return TRUE;
}

void CNoticeDlg::OnDestroy()
{
	CDialog::OnDestroy();
	
	delete this;
}

void CNoticeDlg::OnOK()
{
	CDialog::OnOK();
	DestroyWindow();
}

void CNoticeDlg::OnCancel()
{
	CDialog::OnCancel();
	DestroyWindow();
}