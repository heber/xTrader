// DlgRecent.cpp : 实现文件
//

#include "stdafx.h"
#include "xTrader.h"
#include "DlgRecent.h"
#include "LoginDlg.h"
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
	LoginDlg* pLogin = (LoginDlg*)GetParent();
	m_bSave = !m_bSave;
	pLogin->m_bSaveHis = m_bSave;
}


void DlgRecent::OnLbnDblclkLbRecent()
{
	LoginDlg* pLogin = (LoginDlg*)GetParent();
	
	HWND hOK = ::GetDlgItem(pLogin->m_hWnd,IDOK);
	::SendMessage(hOK,BM_CLICK,0,0);
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

void DlgRecent::InitLBox()
{
	LoginDlg* pLogin = (LoginDlg*)GetParent();
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();

	m_List.SubclassDlgItem(IDC_LBRECENT, this);

	UINT i = 0;
	UINT iSize = pLogin->m_pInfVec.size();
	CString szAccInf,szUid,szPass;

	int iBkrGroup, iSvrGroup;
	CString szSvrGN;
	for (i = 0; i < iSize; i++)
	{
		ansi2uni(CP_UTF8, pLogin->m_pInfVec[i]->szUid, szUid.GetBuffer(MAX_PATH));
		szUid.ReleaseBuffer();

		//Base64Decode(szDecPass, (const char*)pLogin->m_pInfVec[i]->szPass, 0);
		ansi2uni(CP_UTF8, pLogin->m_pInfVec[i]->szPass, szPass.GetBuffer(MAX_PATH));
		szPass.ReleaseBuffer();
		
		iBkrGroup = pLogin->m_pInfVec[i]->iBkrGroup;
		iSvrGroup = pLogin->m_pInfVec[i]->iSvrGroup;
		GetSvrGNByIdx(szSvrGN, iBkrGroup, iSvrGroup);

		szAccInf.Format(_T("%s,%s,账户:%s,密码:%s"), pApp->m_BkrParaVec[iBkrGroup]->BkrName,
			(LPCTSTR)szSvrGN, (LPCTSTR)szUid, (LPCTSTR)szPass);


		m_List.AddString(szAccInf);
		SetHScroll();
	}

}

void DlgRecent::GetSvrGNByIdx(CString &szOut, int iBkrG, int iSvrG)
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	xml_document doc;
	xml_parse_result result;

	szOut.Empty();
	result = doc.load_file(pApp->m_BkrParaVec[iBkrG]->XmlPath);
	if (result.status == status_ok)
	{
		///////////读出服务器//////////////////
		LPCSTR szSvrRt = "//broker/Servers/Server", sNmae = "Name";
		xpath_node_set sVrs = doc.select_nodes(szSvrRt);
		if (sVrs.empty()) return;
		int iCount = 0;
		char strName[64];
		for (xpath_node_set::const_iterator it = sVrs.begin(); it != sVrs.end(); ++it)
		{
			xpath_node node = *it;
			iCount++;
			//读出服务器组名称
			if (iCount == (iSvrG + 1))
			{
				strcpy(strName, node.node().child_value(sNmae));
				ansi2uni(CP_UTF8, strName, szOut.GetBuffer(MAX_PATH));
				szOut.ReleaseBuffer();

				break;
			}
		}
	}
}

BOOL DlgRecent::OnInitDialog()
{
	CDialog::OnInitDialog();

	
	InitLBox();
	return TRUE;
}


void DlgRecent::OnLbnSelchange()
{
	LoginDlg* pLogin = (LoginDlg*)GetParent();

	CString szUid,szPass;
	TThostFtdcPasswordType szDecPass;

	int iSel = m_List.GetCurSel();
	int iBkrGroup = pLogin->m_pInfVec[iSel]->iBkrGroup;
	int iSvrGroup = pLogin->m_pInfVec[iSel]->iSvrGroup;

	ansi2uni(CP_UTF8, pLogin->m_pInfVec[iSel]->szUid, szUid.GetBuffer(MAX_PATH));
	szUid.ReleaseBuffer();
	
	Base64Decode(szDecPass, (const char*)pLogin->m_pInfVec[iSel]->szPass, 0);
	ansi2uni(CP_UTF8, szDecPass, szPass.GetBuffer(MAX_PATH));
	szPass.ReleaseBuffer();

	pLogin->m_ComboBkr.SetCurSel(iBkrGroup);
	::SendMessage(pLogin->m_hWnd, WM_COMMAND,MAKEWPARAM(IDC_BKR_LIST,CBN_SELCHANGE),(LPARAM)(pLogin->m_ComboBkr.GetSafeHwnd()));

	pLogin->m_ComboIsp.SetCurSel(iSvrGroup);
	::SendMessage(pLogin->m_hWnd, WM_COMMAND,MAKEWPARAM(IDC_ISPLIST,CBN_SELCHANGE),(LPARAM)(pLogin->m_ComboIsp.GetSafeHwnd()));

	HWND hEdUser = ::GetDlgItem(pLogin->m_hWnd,IDC_USERNAME);
	::SendMessage(hEdUser,WM_SETTEXT, 0, (LPARAM)(LPCTSTR)szUid);

	HWND hEdPass = ::GetDlgItem(pLogin->m_hWnd,IDC_PASSWORD);
	::SendMessage(hEdPass,WM_SETTEXT, 0, (LPARAM)(LPCTSTR)szPass);
}


void DlgRecent::SetHScroll()
{
	CDC* dc = GetDC();
	SIZE s;
	int index;
	CString str;
	long temp;
	for(index= 0; index< m_List.GetCount(); index++)
	{
		m_List.GetText(index,str);
		s = dc->GetTextExtent(str,str.GetLength()+1);  
		temp = (long)SendDlgItemMessage(IDC_LBRECENT, LB_GETHORIZONTALEXTENT, 0, 0);
		if (s.cx > temp)  
		{
			SendDlgItemMessage(IDC_LBRECENT, LB_SETHORIZONTALEXTENT, (WPARAM)s.cx, 0);
		}
	}
	ReleaseDC(dc);
}
