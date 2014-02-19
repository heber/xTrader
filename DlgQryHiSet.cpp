// DlgQryHiSet.cpp : 实现文件
//

#include "stdafx.h"
#include "xTrader.h"
#include "DlgQryHiSet.h"

extern HANDLE g_hEvent;

IMPLEMENT_DYNAMIC(DlgQryHiSet, CDialog)

DlgQryHiSet::DlgQryHiSet(CWnd* pParent /*=NULL*/)
	: CDialog(DlgQryHiSet::IDD, pParent)
{
	m_szDate = _T("");
	m_szHiSet =_T("");
	m_pQry = NULL;
	m_pQpra = new QRYPARAM;
}

DlgQryHiSet::~DlgQryHiSet()
{
	DEL(m_pQpra);
}

void DlgQryHiSet::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_DTPICK, m_Date);
	DDX_Text(pDX, IDC_EDIT_HISOD, m_szHiSet);
}

BEGIN_MESSAGE_MAP(DlgQryHiSet, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BT_QRYDAY, OnClkQryDay)
	ON_BN_CLICKED(IDC_BT_QRYMONTH, OnClkQryMonth)
	ON_BN_CLICKED(IDC_BT_SAVEFILE, OnClkSave2file)
	ON_BN_CLICKED(IDC_BTCLOSE, OnClkClose)
	ON_REGISTERED_MESSAGE(WM_QRYSMI_MSG,QrySmiMsg)
END_MESSAGE_MAP()

// DlgQryHiSet 消息处理程序
void DlgQryHiSet::OnDestroy()
{
	CDialog::OnDestroy();

	delete this;
}


BOOL DlgQryHiSet::OnInitDialog()
{
	CDialog::OnInitDialog();

	return TRUE;  
}

LRESULT DlgQryHiSet::QrySmiMsg(WPARAM wParam,LPARAM lParam)
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	
	UINT uBufSize = pApp->m_cT->m_StmiVec.size()*sizeof(TThostFtdcContentType);
	char* szMsg = new char[uBufSize];
	ZeroMemory(szMsg,sizeof(szMsg));
	
	for (UINT i=0;i<pApp->m_cT->m_StmiVec.size();i++)
	{
		strcat(szMsg,(const char*)pApp->m_cT->m_StmiVec[i]->Content);
	}
	
	ansi2uni(CP_ACP,szMsg,m_szHiSet.GetBuffer(4*uBufSize));
	m_szHiSet.ReleaseBuffer();
	
	UpdateData(FALSE);
	
	DELX(szMsg);
	return 0;
}

UINT DlgQryHiSet::QrySmi(LPVOID pParam)
{
	QRYPARAM*  pQryPara = static_cast<QRYPARAM*>(pParam);
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();

	ClearVector(pApp->m_cT->m_StmiVec);
	((DlgQryHiSet*)pQryPara->pDlg)->m_szHiSet.Empty();

	pApp->m_cT->ReqQrySettlementInfo(pQryPara->TdDay);

	DWORD dwRet = WaitForSingleObject(g_hEvent,INFINITE);
	if (dwRet==WAIT_OBJECT_0)
	{ 
		ResetEvent(g_hEvent); 
	}
	else
	{
		((DlgQryHiSet*)pQryPara->pDlg)->m_pQry = NULL;
		return 0;
	}

	((DlgQryHiSet*)pQryPara->pDlg)->m_pQry = NULL;
	return 0;
}

void DlgQryHiSet::QryDate(QRYSMTYPE qType)
{
	CTime tm;
	DWORD dwResult = m_Date.GetTime(tm);
	if (dwResult == GDT_VALID)
	{
		switch (qType)
		{
		case Q_MONTH:	//查询月
			m_szDate = tm.Format(_T("%Y%m"));
			break;
		case Q_DAY:	//查询日
			m_szDate = tm.Format(_T("%Y%m%d"));
			break;
		default:
			m_szDate = _T("");
			break;
		}
		
		m_pQpra->pDlg = this;
		uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)m_szDate,m_pQpra->TdDay);
		
		if (m_pQry==NULL)
		{
			m_pQry = AfxBeginThread((AFX_THREADPROC)QrySmi,m_pQpra);
		}
	}
	
}

void DlgQryHiSet::OnClkQryDay()
{
	QryDate(Q_DAY);
}


void DlgQryHiSet::OnClkQryMonth()
{
	QryDate(Q_MONTH);
}

void DlgQryHiSet::OnClkSave2file()
{
	CString  strFilter = _T("文本文件(*.log;*.txt)|(*.*;*.log;*.txt;)|所有文件 |*.*||");
	
	CFileDialog* dlgSave = new CFileDialog(false, _T("*.txt"),  GenDef(_T("结算单"),_T("txt")), OFN_PATHMUSTEXIST | OFN_EXPLORER, strFilter, this);
	dlgSave->m_ofn.lStructSize=sizeof(OPENFILENAME);		//use the 2k+ open file dialog
	
	CString szFile;
	if (IDOK == dlgSave->DoModal())
	{
		szFile = dlgSave->GetPathName();
		
		CFile fLog(szFile, CFile::modeReadWrite | CFile::modeCreate | CFile::typeText);

		UpdateData(TRUE);

		int iLen = m_szHiSet.GetLength();
		char* szLog = new char[4*iLen];

		uni2ansi(CP_UTF8,(LPTSTR)(LPCTSTR)m_szHiSet,szLog);

		BYTE bBom[3]={0xEF,0xBB,0xBF};
		fLog.Write(&bBom,3);

		fLog.Write(szLog,strlen(szLog));
		fLog.Close();

		DELX(szLog);
		
	}
	
	DELX(dlgSave);	
}

void DlgQryHiSet::OnOK()
{
	CDialog::OnOK();
	DestroyWindow();
}

void DlgQryHiSet::OnCancel()
{
	CDialog::OnCancel();
	DestroyWindow();
}

void DlgQryHiSet::OnClkClose()
{
	OnOK();
}
