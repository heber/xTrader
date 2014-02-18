// DlgNetSel.cpp : 实现文件
//

#include "stdafx.h"
#include "xTrader.h"
#include "DlgNetSel.h"
#include "TcpSocket.h"


extern LARGE_INTEGER g_lFreq;
// DlgNetSel 对话框

IMPLEMENT_DYNAMIC(DlgNetSel, CDialog)

DlgNetSel::DlgNetSel(CWnd* pParent /*=NULL*/)
	: CDialog(DlgNetSel::IDD, pParent)
{
	m_bSockType=FALSE;
	m_szSkAddr=_T("");
	m_szSkPort=_T("");
	m_szSkUser=_T("");
	m_szSkPwd=_T("");
	m_szGrpName =_T("");
	//m_szDefTds=_T("");
	//m_szDefMds=_T("");
	m_pTdTest = NULL;
	//m_pMdTest = NULL;
}

DlgNetSel::~DlgNetSel()
{
}

void DlgNetSel::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHK_SOCK, m_bSockType);
	DDX_Control(pDX, IDC_COMBO_NETTYPE, m_CombSockLst);
	DDX_Text(pDX, IDC_SOCKSERV, m_szSkAddr);
	DDX_Text(pDX, IDC_SOCK_PORT, m_szSkPort);
	DDX_Text(pDX, IDC_SK_USER, m_szSkUser);
	DDX_Text(pDX, IDC_SKPWD, m_szSkPwd);
	DDX_Control(pDX, IDC_LST_TRADE, m_LstTds);
	DDX_Control(pDX, IDC_LST_MD, m_LstMds);
}


BEGIN_MESSAGE_MAP(DlgNetSel, CDialog)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_CHK_SOCK, OnBnClkChkSock)
	ON_BN_CLICKED(IDOK, OnBnClickedOk)
	ON_NOTIFY(NM_CLICK, IDC_LST_TRADE, OnNMClickLstTrade)
	ON_NOTIFY(NM_CLICK, IDC_LST_MD, OnNMClickLstMd)
END_MESSAGE_MAP()

BOOL DlgNetSel::OnInitDialog()
{
	InitCombo();
	InitLists();

	if (!m_bSockType)
	{
		m_CombSockLst.EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_SOCKSERV))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_SOCK_PORT))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_SK_USER))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_SKPWD))->EnableWindow(FALSE);
	}
	else
	{
		m_CombSockLst.EnableWindow(1);
		((CEdit*)GetDlgItem(IDC_SOCKSERV))->EnableWindow(1);
		((CEdit*)GetDlgItem(IDC_SOCK_PORT))->EnableWindow(1);
		((CEdit*)GetDlgItem(IDC_SK_USER))->EnableWindow(1);
		((CEdit*)GetDlgItem(IDC_SKPWD))->EnableWindow(1);

	}

	return TRUE;
}

void DlgNetSel::OnOK()
{
	CDialog::OnOK();
	DestroyWindow();
}

void DlgNetSel::OnCancel()
{
	CDialog::OnCancel();
	DestroyWindow();
}

void DlgNetSel::OnDestroy()
{
	CDialog::OnDestroy();
	
	delete this;
}

void DlgNetSel::InitCombo()
{
	m_CombSockLst.SubclassDlgItem(IDC_COMBO_NETTYPE,this);
	const TCHAR* strServs[2] ={_T("sock4"),_T("sock5")};
	int nIndex = 0;
	for (int i=0;i<2;i++)
	{
		nIndex = m_CombSockLst.AddString(strServs[i]);
		m_CombSockLst.SetItemData(nIndex,i);
	}
	m_CombSockLst.SetCurSel(0);
}

UINT DlgNetSel::TestSvr(LPVOID pParam)
{
	TESTSVRPARAM*  pTest = static_cast<TESTSVRPARAM*>(pParam);
	CTcpSocket* pSocket = new CTcpSocket();

	pSocket->GetDelayMs((LPCTSTR)pTest->szSvr,pTest->iDelayNs);

	DEL(pSocket);

	((DlgNetSel*)pTest->pDlg)->m_pTdTest = NULL;

	return 0;
}

void DlgNetSel::InitLists()
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	m_LstTds.SubclassDlgItem(IDC_LST_TRADE,this);
	m_LstMds.SubclassDlgItem(IDC_LST_MD,this);

	if (pApp->AddSvr2Ar(m_szArTs,m_szArMd,m_szGrpName,m_iIdx))
	{	
		TCHAR* lpHdrs[2] = {_T("交易服务器地址"),_T("网络延时")};
		TCHAR* lpHdrs2[2] = {_T("行情服务器地址"),_T("网络延时")};
		int iWidths[2] = {260,60};
		int i;
		int total_cx = 0;
		LVCOLUMN lvcolumn;
		memset(&lvcolumn, 0, sizeof(lvcolumn));
		
		for (i = 0;i<2 ; i++)
		{
			lvcolumn.mask     = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH|LVCFMT_IMAGE;
			lvcolumn.fmt      = LVCFMT_RIGHT;
			lvcolumn.iSubItem = i;
			lvcolumn.pszText  = lpHdrs[i];
			lvcolumn.cx       = iWidths[i];
			
			total_cx += lvcolumn.cx;
			m_LstTds.InsertColumn(i, &lvcolumn);
		}
		
		for (i = 0;i<2 ; i++)
		{
			lvcolumn.mask     = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH|LVCFMT_IMAGE;
			lvcolumn.fmt      = LVCFMT_RIGHT;
			lvcolumn.iSubItem = i;
			lvcolumn.pszText  = lpHdrs2[i];
			lvcolumn.cx       = iWidths[i];
			
			total_cx += lvcolumn.cx;
			m_LstMds.InsertColumn(i, &lvcolumn);
		}

		int iSizeTd = m_szArTs.GetSize();
		int iSubItem=0;
		
		CString szDelay = _T("");
		for (i = 0; i <iSizeTd; i++)
		{
			for (iSubItem = 0; iSubItem < 2; iSubItem++)
			{
				if (iSubItem == 0) 	
				{
					m_LstTds.InsertItem(i, NULL);
					m_LstMds.InsertItem(i, NULL);
				}
				m_LstTds.SetItemText(i, 0, m_szArTs[i], BLACK,WHITE);
				//m_LstTds.SetEdit(i, 0);
				m_LstMds.SetItemText(i, 0, m_szArMd[i], BLACK,WHITE);
				//m_LstMds.SetEdit(i, 0);	
			}
		}	
	}


}

void DlgNetSel::GetDelay()
{
	int iSizeTd = m_szArTs.GetSize();
	int iSubItem=0;
	
	CString szDelay = _T("");
	for (int i = 0; i <iSizeTd; i++)
	{
		//for (iSubItem = 0; iSubItem < 2; iSubItem++)
		{		
			TESTSVRPARAM* testpara1 = new TESTSVRPARAM;
			testpara1->pDlg = this;
			testpara1->iDelayNs = -1;
			testpara1->szSvr = m_szArTs[i];
			if (!m_pTdTest)
			{
				m_pTdTest=AfxBeginThread((AFX_THREADPROC)DlgNetSel::TestSvr, (LPVOID)testpara1);
				WaitForSingleObject(m_pTdTest->m_hThread,250); 
			}
			
			if (testpara1->iDelayNs <=0)
			{
				m_LstTds.SetItemText(i, 1, CONN_ERR, BLACK,WHITE);
			}
			else
			{
				szDelay.Format(_T("%ldms"),1000*testpara1->iDelayNs/g_lFreq.QuadPart);
				m_LstTds.SetItemText(i, 1, szDelay, BLACK,WHITE);
			}
			DEL(testpara1);
			
			
			TESTSVRPARAM* testpara2 = new TESTSVRPARAM;
			testpara2->pDlg = this;
			testpara2->iDelayNs = -1;
			testpara2->szSvr = m_szArMd[i];
			if (!m_pTdTest)
			{
				m_pTdTest=AfxBeginThread((AFX_THREADPROC)DlgNetSel::TestSvr, (LPVOID)testpara2);
				WaitForSingleObject(m_pTdTest->m_hThread,250); 
			}
			
			if (testpara2->iDelayNs <=0)
			{
				m_LstMds.SetItemText(i, 1, CONN_ERR, BLACK,WHITE);
			}
			else
			{
				szDelay.Format(_T("%ldms"),1000*testpara2->iDelayNs/g_lFreq.QuadPart);
				m_LstMds.SetItemText(i, 1, szDelay, BLACK,WHITE);
			}
			DEL(testpara2);	
		}
	}
}

void DlgNetSel::OnBnClkChkSock()
{
	m_bSockType = !m_bSockType;
	
	if (!m_bSockType)
	{
		m_CombSockLst.EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_SOCKSERV))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_SOCK_PORT))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_SK_USER))->EnableWindow(FALSE);
		((CEdit*)GetDlgItem(IDC_SKPWD))->EnableWindow(FALSE);
	}
	else
	{
		m_CombSockLst.EnableWindow(1);
		((CEdit*)GetDlgItem(IDC_SOCKSERV))->EnableWindow(1);
		((CEdit*)GetDlgItem(IDC_SOCK_PORT))->EnableWindow(1);
		((CEdit*)GetDlgItem(IDC_SK_USER))->EnableWindow(1);
		((CEdit*)GetDlgItem(IDC_SKPWD))->EnableWindow(1);
		
	}
}


void DlgNetSel::OnBnClickedOk()
{

	CDialog::OnOK();
}


void DlgNetSel::OnNMClickLstTrade(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	int nItem = -1;
	int nSubItem = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;

		m_LstTds.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);

	}
	
	
	*pResult = 0;
}


void DlgNetSel::OnNMClickLstMd(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	
	int nItem = -1;
	int nSubItem = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;

		m_LstMds.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	
	}
	
	
	*pResult = 0;
}
