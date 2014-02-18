// BfTransfer.cpp : implementation file
//

#include "stdafx.h"
#include "xtrader.h"
#include "BfTransfer.h"
#include "xTraderDlg.h"
#include "DlgBfLog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CWnd* g_pCWnd;
extern HANDLE g_hEvent;

////////////////////////////////////////////////////////////////////////////
// BfTransfer dialog

BfTransfer::BfTransfer(CWnd* pParent /*=NULL*/)
	: CDialog(BfTransfer::IDD, pParent)
{
	m_szAccpwd = _T("");
	m_szBkpwd = _T("");
	m_pQryBf=NULL;
	m_dTrsAmt = 0.00;
}

BfTransfer::~BfTransfer()
{
}

void BfTransfer::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(BfTransfer)
	DDX_Control(pDX, IDC_CBEXHTYPE, m_cbExhType);
	DDX_Control(pDX, IDC_CBBKLST, m_cbBkLst);
	DDX_Text(pDX, IDC_ACCPASSWD, m_szAccpwd);
	DDX_Text(pDX, IDC_BKPASSWD, m_szBkpwd);
	DDX_Text(pDX, IDC_TRANSAMT, m_dTrsAmt);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(BfTransfer, CDialog)
	//{{AFX_MSG_MAP(BfTransfer)
	ON_BN_CLICKED(IDC_BTNQRYBK, OnBtnQryBk)
	ON_BN_CLICKED(IDC_BTNQRYFT, OnBtnQryFt)
	ON_BN_CLICKED(IDC_BTNFT2BK, OnBtnFt2Bk)
	ON_BN_CLICKED(IDC_BTNBK2FT, OnBtnBk2Ft)
	ON_BN_CLICKED(IDC_QRYDETAIL, OnQryDetail)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


BOOL BfTransfer::OnInitDialog()
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	TCHAR szUser[MAX_PATH/2],szTitle[MAX_PATH];
	ansi2uni(CP_ACP,pApp->m_sINVESTOR_ID,szUser);

	_stprintf(szTitle,BFTRANS_TITLE,szUser);
	SetWindowText(szTitle);

	CString str;
	str.Format(_T("%.2f"),m_dTrsAmt);
	((CEdit*)GetDlgItem(IDC_TRANSAMT))->SetWindowText(str);

	((CEdit*)GetDlgItem(IDC_BKPASSWD))->SetWindowText(_T("1"));

	InitCombo();
	
	return TRUE;
}

void BfTransfer::InitCombo()
{
	CXTraderDlg* pDlg = (CXTraderDlg*)g_pCWnd;
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();

	m_cbBkLst.SubclassDlgItem(IDC_CBBKLST,this);
	m_cbExhType.SubclassDlgItem(IDC_CBEXHTYPE,this);

	TCHAR strBkAcc[MAX_PATH];
	CString szBkInfo,szBkName;
	int nIndex = 0;
	UINT i=0;
	UINT uSize = pApp->m_cT->m_AccRegVec.size();
	for (i=0;i<uSize;i++)
	{
		ansi2uni(CP_ACP,pApp->m_cT->m_AccRegVec[i]->BankAccount,strBkAcc);
		szBkName = JgBkName(pApp->m_cT->m_AccRegVec[i]->BankID[0]);

		szBkInfo.Format(_T("%s[%s]"),(LPCTSTR)szBkName,strBkAcc);
		nIndex = m_cbBkLst.AddString(szBkInfo);
		m_cbBkLst.SetItemData(nIndex,i);
	}

	if (uSize>=1)
	{
		m_cbBkLst.SetCurSel(0);
	}
	
	LPCTSTR strExhType[3] = {_T("RMB"),_T("USD"),_T("HKD")};
	
	for (i=0;i<3;i++)
	{
		nIndex = m_cbExhType.AddString(strExhType[i]);
		m_cbExhType.SetItemData(nIndex,i);
	}
	m_cbExhType.SetCurSel(0);

}

void BfTransfer::OnBtnQryFt() 
{
	CXTraderDlg* pDlg = (CXTraderDlg*)g_pCWnd;

	if (!pDlg->m_pQryAcc)
	{
		pDlg->m_pQryAcc = AfxBeginThread((AFX_THREADPROC)CXTraderDlg::QryAccount, pDlg);

	}
	
}
void BfTransfer::Transfunc(BFTRANSTYPE tp)
{
	
	UpdateData(TRUE);
	
	if (m_szAccpwd.IsEmpty()||m_szBkpwd.IsEmpty())
	{
		ShowErroTips(IDS_STREMPTY,IDS_STRTIPS);
		return;
	}
	
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	
	char szAccPass[MAX_PATH],szBkPass[MAX_PATH];
	
	uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)m_szAccpwd,szAccPass);
	uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)m_szBkpwd,szBkPass);
	
	int nIndex = m_cbBkLst.GetCurSel();
	if (nIndex<0) 
	{
		ShowErroTips(IDS_QRYBKERR,IDS_STRTIPS);
		return;
	}
	
	pApp->m_cT->m_bReconnect  = FALSE;

	if (tp==TRANS_F2B)
	{
		pApp->m_cT->ReqF2BkByF(pApp->m_cT->m_AccRegVec[nIndex]->BankID,szBkPass,szAccPass,m_dTrsAmt);
	}
	if (tp==TRANS_B2F)
	{
		pApp->m_cT->ReqBk2FByF(pApp->m_cT->m_AccRegVec[nIndex]->BankID,szBkPass,szAccPass,m_dTrsAmt);
	}
	if (tp==QRY_BKACC)
	{
		pApp->m_cT->ReqQryBkAccMoneyByF(pApp->m_cT->m_AccRegVec[nIndex]->BankID,szBkPass,szAccPass);
	}

}

void BfTransfer::OnBtnQryBk() 
{
	Transfunc(QRY_BKACC);
		
}

void BfTransfer::OnBtnFt2Bk() 
{
	Transfunc(TRANS_F2B);
}

void BfTransfer::OnBtnBk2Ft() 
{

	Transfunc(TRANS_B2F);
}

void BfTransfer::OnQryDetail() 
{
	DlgBfLog* tdlg = new DlgBfLog;
	
	BOOL res=tdlg->Create(IDD_DLG_BFDETAILS,NULL);
	VERIFY( res==TRUE );
	tdlg->CenterWindow();
	tdlg->ShowWindow(SW_SHOW);
	
	if (m_pQryBf==NULL)
	{
		m_pQryBf = AfxBeginThread((AFX_THREADPROC)BfTransfer::QryBfS,this);
	}
}


UINT BfTransfer::QryBfS(LPVOID pParam)
{
	BfTransfer* pDlg = (BfTransfer*)pParam;
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();

	int nIndex = pDlg->m_cbBkLst.GetCurSel();
	if (nIndex<0) 
	{
		ShowErroTips(IDS_QRYBKERR,IDS_STRTIPS);
		return 0;
	}

	ClearVector(pApp->m_cT->m_BfTransVec);
	pApp->m_cT->ReqQryTfSerial(pApp->m_cT->m_AccRegVec[nIndex]->BankID);
	
	DWORD dwRet = WaitForSingleObject(g_hEvent,INFINITE);
	if (dwRet==WAIT_OBJECT_0)
	{ 
		ResetEvent(g_hEvent); 
	}
	else
	{
		pDlg->m_pQryBf = NULL;
		return 0;
	}
		
	pDlg->m_pQryBf = NULL;
	return 0;
}
	  


void BfTransfer::OnOK() 
{
	CDialog::OnOK();
	DestroyWindow();
}

void BfTransfer::OnCancel() 
{
	CDialog::OnCancel();
	DestroyWindow();
}

void BfTransfer::OnDestroy()
{
	CDialog::OnDestroy();
	
	delete this;
}
