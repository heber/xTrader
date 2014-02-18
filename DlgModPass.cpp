// DlgModPass.cpp : implementation file
//

#include "stdafx.h"
#include "xtrader.h"
#include "DlgModPass.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern HANDLE g_hEvent;
/////////////////////////////////////////////////////////////////////////////
// DlgModPass dialog

DlgModPass::DlgModPass(CWnd* pParent /*=NULL*/)
	: CDialog(DlgModPass::IDD, pParent)
{

	m_pModPass = NULL;
	m_itype = 0;
	m_szNewPass=_T("");
	m_szOldPass=_T("");
	m_szNewCfm=_T("");
}


void DlgModPass::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_TRADEPASS,m_itype);
	DDX_Text(pDX, IDC_ORIPASS, m_szOldPass);
	DDX_Text(pDX, IDC_NEWPASS, m_szNewPass);
	DDX_Text(pDX, IDC_NEWPASSCFM, m_szNewCfm);
	//{{AFX_DATA_MAP(DlgModPass)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgModPass, CDialog)
	//{{AFX_MSG_MAP(DlgModPass)
	ON_BN_CLICKED(IDC_TRADEPASS, OnTradepass)
	ON_BN_CLICKED(IDC_ACCPASS, OnAccpass)
	ON_EN_CHANGE(IDC_ORIPASS, OnChangeOripass)
	ON_EN_CHANGE(IDC_NEWPASS, OnChangeNewpass)
	ON_EN_CHANGE(IDC_NEWPASSCFM, OnChangeNewpasscfm)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgModPass message handlers
/*
BOOL DlgModPass::OnInitDialog()
{

	//((CButton*)GetDlgItem(IDC_TRADEPASS))->SetCheck(m_itype);

	return TRUE;
}
*/

void DlgModPass::OnTradepass() 
{
	m_itype = 0;
	
}

void DlgModPass::OnAccpass() 
{
 	m_itype = 1;
	
}

void DlgModPass::OnChangeOripass() 
{

	UpdateData(TRUE);
	
}

void DlgModPass::OnChangeNewpass() 
{
	UpdateData(TRUE);
	
}

void DlgModPass::OnChangeNewpasscfm() 
{
	UpdateData(TRUE);	
}

void DlgModPass::OnDestroy()
{
	CDialog::OnDestroy();
	
	delete this;
}

void DlgModPass::OnCancel()
{
	CDialog::OnCancel();
	DestroyWindow();
}

UINT DlgModPass::ModPwdThread(LPVOID pParam)
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	DlgModPass* pDlg = (DlgModPass*) pParam;

	char szOriPass[MAX_PATH],szNewPass[MAX_PATH];
	
	if (pDlg->m_szNewPass.IsEmpty()||pDlg->m_szOldPass.IsEmpty()||pDlg->m_szNewCfm.IsEmpty())
	{
		ShowErroTips(IDS_STREMPTY,IDS_STRTIPS);
		return 0;
	}
	
	if (pDlg->m_szNewPass.Compare(pDlg->m_szNewCfm)!=0)
	{
		ShowErroTips(IDS_CFMPASSERR,IDS_STRTIPS);
		return 0;
	}
	
	uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)pDlg->m_szOldPass,szOriPass);
	uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)pDlg->m_szNewPass,szNewPass);
	if (pDlg->m_itype==0)
	{	
		pApp->m_cT->ReqUserPwdUpdate(szNewPass,szOriPass);
		
		DWORD dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
		if (dwRet==WAIT_OBJECT_0)
		{
			strcpy(pApp->m_sPASSWORD,szNewPass);
			ResetEvent(g_hEvent);
		}
		
	}
	if (pDlg->m_itype==1)
	{
		pApp->m_cT->ReqTdAccPwdUpdate(szNewPass,szOriPass);
		
		DWORD dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
		if (dwRet==WAIT_OBJECT_0)
		{
			ResetEvent(g_hEvent);
		}
		
	}

	pDlg->m_pModPass = NULL;
	return 0;
}

void DlgModPass::OnOK()
{
	if (!m_pModPass)
	{
		AfxBeginThread((AFX_THREADPROC)ModPwdThread, this);
	}
}
