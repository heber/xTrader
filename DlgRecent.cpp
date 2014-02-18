// DlgRecent.cpp : 实现文件
//

#include "stdafx.h"
#include "xTrader.h"
#include "DlgRecent.h"

// DlgRecent 对话框

IMPLEMENT_DYNAMIC(DlgRecent, CDialog)

DlgRecent::DlgRecent(CWnd* pParent /*=NULL*/)
	: CDialog(DlgRecent::IDD, pParent)
{
	m_bSave = TRUE;
}

DlgRecent::~DlgRecent()
{
}

void DlgRecent::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LBRECENT, m_List);
	DDX_Check(pDX, IDC_CHK_RECENT, m_bSave);
}


BEGIN_MESSAGE_MAP(DlgRecent, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHK_RECENT, OnBnClkChkRecent)
	ON_LBN_DBLCLK(IDC_LBRECENT, OnLbnDblclkLbRecent)
	ON_LBN_SELCHANGE(IDC_LBRECENT, OnLbnSelchange)
END_MESSAGE_MAP()


// DlgRecent 消息处理程序


void DlgRecent::OnBnClkChkRecent()
{
	m_bSave = !m_bSave;
}


void DlgRecent::OnLbnDblclkLbRecent()
{
	
}

void DlgRecent::OnDestroy()
{
	CDialog::OnDestroy();

	::EnableWindow(::GetDlgItem(GetParent()->GetSafeHwnd(),IDC_EXTRA),TRUE);

	delete this;
}

void DlgRecent::OnCancel()
{
	CDialog::OnCancel();
	DestroyWindow();
}

void DlgRecent::OnOK()
{
	CDialog::OnOK();
	DestroyWindow();
}

BOOL DlgRecent::OnInitDialog()
{
	CDialog::OnInitDialog();

	
	return TRUE;
}


void DlgRecent::OnLbnSelchange()
{
	// TODO:  在此添加控件通知处理程序代码
}
