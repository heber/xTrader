// LoginDlg.cpp : implementation file
//

#include "stdafx.h"
#include "xTrader.h"
#include "LoginDlg.h"
#include "NoticeDlg.h"
#include "DlgNetSel.h"
#include "DlgRecent.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern HANDLE g_hEvent;

/////////////////////////////////////////////////////////////////////////////
// LoginDlg dialog
LoginDlg::LoginDlg(CWnd* pParent /*=NULL*/)
	: CDialog(LoginDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_szUser = _T("00000028");
	m_szPass = _T("123456");
	m_bSavePwd = TRUE;
	m_bSaveHis = TRUE;
	m_pLogin = NULL;
	m_iSelBkr = 183;
	m_iSelSvr = 0;
	m_szArTs.RemoveAll();
	m_szArMd.RemoveAll();
	m_szBrkName = _T("");

}

LoginDlg::~LoginDlg()
{
	ClearVector(m_pInfVec);
}

void LoginDlg::LoadConfig()
{
	CXTraderApp* xTApp = (CXTraderApp*)AfxGetApp();

	xml_document doc;
	xml_node proot;
	xml_parse_result result = doc.load_file(CFG_FILE);
	if (result.status == status_ok)
	{
		proot = doc.child(ROOT);
		if (!proot) return;
		
		m_bSavePwd = atoi(proot.child_value(SV_PWD));
		
		strcpy(xTApp->m_szInst, proot.child_value(INS_LST));
		xml_node nodeWInf = proot.child(WND_INF);
		xTApp->m_ixPos = nodeWInf.attribute("xPos").as_int();
		xTApp->m_iyPos = nodeWInf.attribute("yPos").as_int();
		xTApp->m_iWidth = nodeWInf.attribute("width").as_int();
		xTApp->m_iHeight = nodeWInf.attribute("height").as_int();

		xml_node nodeRecent = proot.child(RECNT);
		m_bSaveHis = nodeRecent.attribute("bSave").as_bool();

		if (m_bSaveHis)
		{	//都是英文字符所以ansi和utf8编码一样，否则需要转码
			for (xml_node nodeHis = nodeRecent.first_child(); nodeHis; nodeHis = nodeHis.next_sibling())
			{
				PLOGINPARA pInf = new LOGINPARA;
				strcpy(pInf->szUid, nodeHis.attribute(USER_ID).value());
				strcpy(pInf->szPass, nodeHis.attribute(USER_PW).value());
				pInf->iBkrGroup = nodeHis.attribute(BKR_GRP).as_int();
				pInf->iSvrGroup = nodeHis.attribute(SV_GRP).as_int();

				m_pInfVec.push_back(pInf);
			}

			UINT iSize = m_pInfVec.size();
			if (iSize >= 1)
			{
				strcpy(xTApp->m_sINVESTOR_ID, m_pInfVec[iSize-1]->szUid);
				ansi2uni(CP_UTF8, xTApp->m_sINVESTOR_ID, m_szUser.GetBuffer(MAX_PATH));
				m_szUser.ReleaseBuffer();

				//char szEncPass[64];
				//strcpy(szEncPass, m_pInfVec[iSize-1]->szPass);
				Base64Decode(xTApp->m_sPASSWORD, (const char*)m_pInfVec[iSize-1]->szPass, 0);
				ansi2uni(CP_UTF8, xTApp->m_sPASSWORD, m_szPass.GetBuffer(MAX_PATH));
				m_szPass.ReleaseBuffer();

				m_iSelBkr = m_pInfVec[iSize-1]->iBkrGroup;
				m_iSelSvr = m_pInfVec[iSize-1]->iSvrGroup;
			}
		}
		
	}
	else
	{
		ProgressUpdate(_T("config.xml加载失败!"), 0);	
	}	

	int iBkrSize = xTApp->m_BkrParaVec.size();
	if (iBkrSize>0)
	{
		/////////////////////////////////////////
		int i=0,nIndex = 0;
		for (i=0;i<iBkrSize;i++)
		{
			nIndex = m_ComboBkr.AddString(xTApp->m_BkrParaVec[i]->BkrName);
			m_ComboBkr.SetItemData(nIndex,i);
		}
		
		m_ComboBkr.SetCurSel(m_iSelBkr);
		/////////////////////////////////////////////////////////////	
		result = doc.load_file(xTApp->m_BkrParaVec[m_iSelBkr]->XmlPath);
		if (result.status == status_ok) 
		{
			proot = doc.child(ROOT).child("broker");
			if (!proot) return;
			
			strcpy(xTApp->m_sBROKER_ID,proot.attribute("BrokerID").value());
			m_szBrkName = xTApp->m_BkrParaVec[m_iSelBkr]->BkrName;
			
			///////////读出服务器//////////////////
			LPCSTR szSvrRt="//broker/Servers/Server",sNmae="Name",sTrading="Trading",sMData="MarketData";
			char strName[64];
			CString tName,tTrading,tMData;
			CStringArray szAr;
			szAr.RemoveAll();
			
			xpath_node_set sVrs = doc.select_nodes(szSvrRt);
			if (sVrs.empty()) return;
			for (xpath_node_set::const_iterator it = sVrs.begin(); it !=  sVrs.end(); ++it)
			{
				xpath_node node = *it;
				strcpy(strName,node.node().child_value(sNmae));
				ansi2uni(CP_UTF8,strName,tName.GetBuffer(MAX_PATH));
				tName.ReleaseBuffer();
				szAr.Add(tName);
				//如果循环到指定组,读出信息,遍历服务器组名称
				if (szAr.GetSize()==(m_iSelSvr+1))
				{
					xml_node tool;
					
					for (tool = node.node().child(sTrading).first_child(); tool; tool = tool.next_sibling())
					{
						ansi2uni(CP_UTF8,(char*)tool.child_value(),tTrading.GetBuffer(MAX_PATH));
						tTrading.ReleaseBuffer();
						
						m_szArTs.Add(tTrading);
					}
					
					for (tool = node.node().child(sMData).first_child(); tool; tool = tool.next_sibling())
					{
						ansi2uni(CP_UTF8,(char*)tool.child_value(),tMData.GetBuffer(MAX_PATH));
						tMData.ReleaseBuffer();
						
						m_szArMd.Add(tMData);
					}
	
				}
			}
			////////////////////////////////////////////////////////////////////////////
			//int i=0,nIndex = 0;
			for (i=0;i<szAr.GetSize();i++)
			{
				nIndex = m_ComboIsp.AddString(szAr[i]);
				m_ComboIsp.SetItemData(nIndex,i);
			}
			
			m_ComboIsp.SetCurSel(m_iSelSvr);
			::SendMessage(m_hWnd, WM_COMMAND,MAKEWPARAM(IDC_ISPLIST,CBN_SELCHANGE),(LPARAM)m_ComboIsp.GetSafeHwnd());
		}
		
	}
	else
	{
		((CButton*)GetDlgItem(IDC_NETSET))->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDOK))->EnableWindow(FALSE);
		ProgressUpdate(_T("请检查brokers目录!"), 0);
	}
	
}

void LoginDlg::SaveConfig()
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();

	//if (m_bSavePwd)
	{
		UpdateData();
		
		xml_document doc;
		xml_node proot;
		xml_parse_result result = doc.load_file(CFG_FILE,parse_full);
		
		if (result.status==status_ok)
		{
			proot = doc.child(ROOT);
			
			proot.remove_child(SV_PWD);
			
			TThostFtdcInvestorIDType szUid;
			TThostFtdcPasswordType szPass;
			
			char szSave[4],szGrp1[4],szGrp2[4];
			sprintf(szSave,"%d",m_bSavePwd);
			sprintf(szGrp1,"%d",m_iSelBkr);
			sprintf(szGrp2,"%d",m_iSelSvr);
			uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)m_szUser,szUid);
			char szEncPass[64];
			//如果保存密码都要简单加密
			if (m_bSavePwd)
			{
				uni2ansi(CP_ACP, (LPTSTR)(LPCTSTR)m_szPass, szPass);
				Base64Encode(szEncPass, (const char*)szPass, 0);
			}
			else
			{
				ZeroMemory(szPass, sizeof(szPass));
			}
		
			//把登录参数记录成结构保存
			PLOGINPARA pInf = new LOGINPARA;
			strcpy(pInf->szUid, szUid);
			strcpy(pInf->szPass, szEncPass);
			pInf->iBkrGroup = m_iSelBkr;
			pInf->iSvrGroup = m_iSelSvr;

			xml_node nodeSi = proot.append_child(SV_PWD);
			nodeSi.append_child(node_pcdata).set_value(szSave);
			
			UINT i = 0;
			xml_node nodeRecent = proot.child(RECNT);
			if (nodeRecent)
			{
				nodeRecent.remove_attribute("bSave");
				if (m_bSaveHis)
				{
					UINT iSize = m_pInfVec.size();
					if (iSize>=1)
					{
						///////清除重复登陆的用户存档
						for (VIT_lp vlp = m_pInfVec.begin(); vlp != m_pInfVec.end();)
						{
							if ((!strcmp((*vlp)->szUid, szUid)) && ((*vlp)->iBkrGroup == m_iSelBkr))
							{
								vlp = m_pInfVec.erase(vlp);
							}
							else
								++vlp;
						}
					}
					m_pInfVec.push_back(pInf);
					///删掉超过MAX_HIS条记录的前几条
					iSize = m_pInfVec.size();
					if (iSize > MAX_HIS)
					{
						for (i = 0; i < (iSize - MAX_HIS); i++)
						{
							m_pInfVec.erase(m_pInfVec.begin());
						}
					}
					//////////新纪录保存
					proot.remove_child(RECNT);
					nodeRecent = proot.append_child(RECNT);
					nodeRecent.append_attribute("bSave") = "true";
					xml_node nodeHis;
					iSize = m_pInfVec.size();
					//char szEncPwd[64];
					for (i = 0; i < iSize; i++)
					{
						nodeHis = nodeRecent.append_child(ACINF);
						nodeHis.append_attribute(USER_ID) = m_pInfVec[i]->szUid;
						nodeHis.append_attribute(USER_PW) = m_pInfVec[i]->szPass;
						nodeHis.append_attribute(BKR_GRP) = m_pInfVec[i]->iBkrGroup;
						nodeHis.append_attribute(SV_GRP) = m_pInfVec[i]->iSvrGroup;
						doc.save_file(CFG_FILE, PUGIXML_TEXT("\t"), format_default, encoding_utf8);
					}
				}
				else
				{
					proot.remove_child(RECNT);
					nodeRecent = proot.append_child(RECNT);
					nodeRecent.append_attribute("bSave") = "false";
				}


			}
			
			doc.save_file(CFG_FILE,PUGIXML_TEXT("\t"),format_default,encoding_utf8);
		}
	}
}

void LoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(LoginDlg)
	DDX_Control(pDX, IDC_ISPLIST, m_ComboIsp);
	DDX_Control(pDX, IDC_BKR_LIST, m_ComboBkr);
	DDX_Text(pDX, IDC_USERNAME, m_szUser);
	DDV_MaxChars(pDX, m_szUser, 13);
	DDX_Text(pDX, IDC_PASSWORD, m_szPass);
	DDV_MaxChars(pDX, m_szPass, 41);
	DDX_Check(pDX, IDC_SAVE, m_bSavePwd);
	DDX_Control(pDX, IDC_PROGRESS, m_prgs);
	DDX_Control(pDX, IDC_LOGINFO, m_staInfo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(LoginDlg, CDialog)
	//{{AFX_MSG_MAP(LoginDlg)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, OnLogin)
	ON_BN_CLICKED(IDCANCEL, OnQuit)
	ON_BN_CLICKED(IDC_NETSET, OnNetset)
	ON_BN_CLICKED(IDC_SAVE, OnSave)
	ON_EN_CHANGE(IDC_USERNAME, OnChangeUser)
	ON_EN_CHANGE(IDC_PASSWORD, OnChangePass)
	ON_CBN_SELCHANGE(IDC_ISPLIST, OnSelISP)
	ON_CBN_SELCHANGE(IDC_BKR_LIST, OnSelBkr)
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_EXTRA, OnBnClkExtra)
END_MESSAGE_MAP()


BOOL LoginDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_prgs.SetRange(0, 100);
	ProgressUpdate(_T("未登陆!"), 0);

	LoadConfig();
	UpdateData(FALSE);

	return 0;
}
/////////////////////////////////////////////////////////////////////////////
// LoginDlg message handlers

void LoginDlg::OnLogin() 
{
	UpdateData(true);
	if (m_szUser.IsEmpty() || m_szPass.IsEmpty())
	{
		ShowErroTips(_T("用户名或者帐号为空!"),MY_TIPS);
	}
	else
	{
		((CButton*)GetDlgItem(IDC_NETSET))->EnableWindow(FALSE);
		((CButton*)GetDlgItem(IDOK))->EnableWindow(FALSE);	
		if (!m_pLogin)
		{
			AfxBeginThread((AFX_THREADPROC)LoginThread, this);
			m_pLogin=NULL;
		}
	}
	
}

void LoginDlg::OnQuit() 
{
	PostQuitMessage(0);		
}

void LoginDlg::OnDestroy()
{
	SaveConfig();
	CDialog::OnDestroy();
}

void LoginDlg::OnNetset() 
{
	//CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	CString str;
	m_ComboIsp.GetWindowText(str);

	DlgNetSel* tdlg = new DlgNetSel;
	
	tdlg->m_szGrpName = str;
	tdlg->m_iIdx = m_iSelBkr;

	BOOL res=tdlg->Create(IDD_DLG_NETSET,NULL);
	VERIFY( res==TRUE );
	tdlg->CenterWindow();
	tdlg->ShowWindow(SW_SHOW);

	tdlg->GetDelay();

}

void LoginDlg::OnSave() 
{
	m_bSavePwd = !m_bSavePwd;	
}

////////选定一组服务器响应事件///////
void LoginDlg::OnSelISP() 
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	if(m_ComboIsp.GetCurSel() >= 0)
	{
		m_iSelSvr = m_ComboIsp.GetCurSel();

		CString str;
		m_ComboIsp.GetWindowText(str);

		//把指定组的服务器信息读取填充
		pApp->AddSvr2Ar(m_szArTs,m_szArMd,str,m_iSelBkr);
	}
}

////////选定一家期货公司响应事件///////
void LoginDlg::OnSelBkr() 
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	if(m_ComboBkr.GetCurSel() >= 0)
	{
		m_iSelBkr = m_ComboBkr.GetCurSel();
		
		xml_document doc;
		xml_node proot;
		//CString szBrkID;
		xml_parse_result result = doc.load_file(pApp->m_BkrParaVec[m_iSelBkr]->XmlPath);
		if (result.status == status_ok) 
		{
			proot = doc.child(ROOT).child("broker");
			if (!proot) return;
			
			strcpy(pApp->m_sBROKER_ID,proot.attribute("BrokerID").value());
			m_szBrkName = pApp->m_BkrParaVec[m_iSelBkr]->BkrName;
			
			///////////读出服务器分组列表//////////////////
			LPCSTR szSvrRt="//broker/Servers/Server",sNmae="Name",sTrading="Trading",sMData="MarketData";
			char strName[64];
			CString tName,tTrading,tMData;
			CStringArray szAr;
			szAr.RemoveAll();
			
			xpath_node_set sVrs = doc.select_nodes(szSvrRt);
			if (sVrs.empty()) return;
			for (xpath_node_set::const_iterator it = sVrs.begin(); it !=  sVrs.end(); ++it)
			{
				xpath_node node = *it;
				strcpy(strName,node.node().child_value(sNmae));
				ansi2uni(CP_UTF8,strName,tName.GetBuffer(MAX_PATH));
				tName.ReleaseBuffer();
				//ansi2uni(CP_UTF8,strTrading,tTrading.GetBuffer(MAX_PATH));
				szAr.Add(tName);
				
			}
			/////开始清空服务组名并填充///
			m_ComboIsp.ResetContent();
			////////////////////////////////////////////////////////////////////////////
			int i=0,nIndex = 0;
			for (i=0;i<szAr.GetSize();i++)
			{
				nIndex = m_ComboIsp.AddString(szAr[i]);
				m_ComboIsp.SetItemData(nIndex,i);
			}
			/////////选中首组并激发响应/////////////
			m_ComboIsp.SetCurSel(0);

			::SendMessage(m_hWnd, WM_COMMAND,MAKEWPARAM(IDC_ISPLIST,CBN_SELCHANGE),(LPARAM)m_ComboIsp.GetSafeHwnd());
			///////////////////////////////////////////
			
		}
	}
}

void LoginDlg::OnChangeUser() 
{
	CXTraderApp* xTApp = (CXTraderApp*)AfxGetApp();
	
	UpdateData(true);
	uni2ansi(CP_ACP,m_szUser.GetBuffer(MAX_PATH),xTApp->m_sINVESTOR_ID);
	m_szUser.ReleaseBuffer();
}


void LoginDlg::OnChangePass() 
{
	CXTraderApp* xTApp = (CXTraderApp*)AfxGetApp();
	
	UpdateData(true);
	uni2ansi(CP_ACP,m_szPass.GetBuffer(MAX_PATH),xTApp->m_sPASSWORD);
	m_szPass.ReleaseBuffer();
	
}

void LoginDlg::ProgressUpdate(LPCTSTR szMsg, const int nPercentDone)
{
    ASSERT (AfxIsValidString(szMsg));
    ASSERT ( nPercentDone >= 0  &&  nPercentDone <= 100 );
	
    m_staInfo.SetWindowText(szMsg);
    m_prgs.SetPos(nPercentDone);
}


UINT LoginDlg::LoginThread(LPVOID pParam)
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	LoginDlg* pDlg = (LoginDlg*) pParam;

	pApp->m_szTitle.Format(_T("%s@%s-%s"),(LPCTSTR)pDlg->m_szUser,(LPCTSTR)pDlg->m_szBrkName,(LPCTSTR)LoadString(IDS_TITLE));

	//初始化行情和交易API,注册多个前置备用
	pApp->ReleaseApi();
	pApp->CreateApi();
	pApp->m_cT->ClrAllVecs();

	int iTdSvr = pDlg->m_szArTs.GetSize();
	int i =0,iLen;
	char szTd[MAX_PATH],szMd[MAX_PATH];
	for (i=0;i<iTdSvr;i++)
	{
		iLen = pDlg->m_szArTs[i].GetLength();
		uni2ansi(CP_ACP,pDlg->m_szArTs[i].GetBuffer(iLen),szTd);
		pDlg->m_szArTs[i].ReleaseBuffer();

		pApp->m_TApi->RegisterFront(szTd);
	}
	pApp->m_TApi->Init();	
	DWORD dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("登陆交易成功!"), 10);
		ResetEvent(g_hEvent);
	}
	else
	{
		::EnableWindow(::GetDlgItem(pDlg->GetSafeHwnd(),IDOK),TRUE);
		::EnableWindow(::GetDlgItem(pDlg->GetSafeHwnd(),IDC_NETSET),TRUE);
		pDlg->m_pLogin = NULL;
		pDlg->ProgressUpdate(_T("网络故障或CTP已离线!"),0);

		return 0;
	}
	
	if (pApp->m_cT->IsErrorRspInfo(&pApp->m_cT->m_RspMsg))
	{
		::EnableWindow(::GetDlgItem(pDlg->GetSafeHwnd(),IDOK),TRUE);
		::EnableWindow(::GetDlgItem(pDlg->GetSafeHwnd(),IDC_NETSET),TRUE);

		TCHAR szMsg[MAX_PATH];
		ansi2uni(CP_ACP,pApp->m_cT->m_RspMsg.ErrorMsg,szMsg);
		pDlg->ProgressUpdate(szMsg,0);
		pDlg->m_pLogin = NULL;
	
		return 0;
	}

	/////////////登录行情过程//////////////////////////
	int iMdSvr = pDlg->m_szArMd.GetSize();
	for (i=0;i<iTdSvr;i++)
	{
		iLen = pDlg->m_szArMd[i].GetLength();
		uni2ansi(CP_ACP,pDlg->m_szArMd[i].GetBuffer(iLen),szMd);
		pDlg->m_szArMd[i].ReleaseBuffer();

		pApp->m_MApi->RegisterFront(szMd);
	}
	pApp->m_MApi->Init();
	
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("登陆行情成功!"), 20);
		ResetEvent(g_hEvent);
	}

	///////////////////////////////////////////////////////////
	pApp->m_cT->ReqSettlementInfoConfirm();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("确认结算单!"), 40);
		ResetEvent(g_hEvent);
	}
	else
	{
		pDlg->m_pLogin = NULL;
		::EnableWindow(::GetDlgItem(pDlg->GetSafeHwnd(),IDOK),TRUE);

		pDlg->ProgressUpdate(_T("确认结算超时!"),0);
		return 0;
	}
	
	pApp->m_cT->ReqQryInst(NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("查合约列表!"), 60);
		ResetEvent(g_hEvent);
	}
	else
	{
		pDlg->m_pLogin = NULL;
		::EnableWindow(::GetDlgItem(pDlg->GetSafeHwnd(),IDOK),TRUE);

		pDlg->ProgressUpdate(_T("查合约超时!"),0);
		return 0;
	}


	Sleep(1000);
	pApp->m_cT->ReqQryInvPos(NULL);
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("查持仓信息!"), 70);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->m_pLogin = NULL;
		::EnableWindow(::GetDlgItem(pDlg->GetSafeHwnd(),IDOK),TRUE);

		pDlg->ProgressUpdate(_T("查持仓信息超时!"),0);
		return 0;
	}
	
	Sleep(1000);
	pApp->m_cT->ReqQryTdAcc();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("查资金账户!"), 80);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->m_pLogin = NULL;
		::EnableWindow(::GetDlgItem(pDlg->GetSafeHwnd(),IDOK),TRUE);

		pDlg->ProgressUpdate(_T("查账户超时!"),0);
		return 0;
	}

#ifdef _REAL_CTP_
	Sleep(1000);
	pApp->m_cT->ReqQryAccreg();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("查银期信息!"), 90);
		ResetEvent(g_hEvent);
		
	}
	else
	{
		pDlg->m_pLogin = NULL;
		::EnableWindow(::GetDlgItem(pDlg->GetSafeHwnd(),IDOK),TRUE);

		pDlg->ProgressUpdate(_T("查银期信息超时!"),0);
		return 0;
	}
	
	Sleep(1000);
	pApp->m_cT->ReqQryTradingCode();
	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		pDlg->ProgressUpdate(_T("查交易编码!"), 99);
		ResetEvent(g_hEvent);
	}	
	else
	{
		pDlg->m_pLogin = NULL;
		::EnableWindow(::GetDlgItem(pDlg->GetSafeHwnd(),IDOK),TRUE);

		pDlg->ProgressUpdate(_T("查交易编码超时!"),0);
		return 0;
	}
	
#endif

	pDlg->m_pLogin = NULL;
	pDlg->EndDialog(IDOK);
	return 0;

}


void LoginDlg::OnBnClkExtra()
{
	((CButton*)GetDlgItem(IDC_EXTRA))->EnableWindow(FALSE);

	DlgRecent* Dlg = new DlgRecent;

	BOOL res = Dlg->Create(IDD_DLG_RECENT, NULL);
	VERIFY(res == TRUE);

	CRect rect;
	GetWindowRect(rect);

	Dlg->MoveWindow(rect.right,rect.top,rect.Width(),rect.Height(),FALSE);

	Dlg->ShowWindow(SW_SHOW);
}
