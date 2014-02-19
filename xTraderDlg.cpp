// xTraderDlg.cpp : implementation file
//

#include "stdafx.h"
#include "xTrader.h"
#include "xTraderDlg.h"
#include "NoticeDlg.h"
#include "NTP_helper.h"
#include "DlgModPass.h"
#include "GenMfDlg.h"
#include "DlgQryHiSet.h"
#include "BfTransfer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
	
extern CWnd* g_pCWnd;
extern HANDLE g_hEvent;

CString g_szInstOld=_T("IF1302");

BOOL g_bInitInstOnce = FALSE;
BOOL g_bInitOrdOnce = FALSE;
BOOL g_bInitTdOnce = FALSE;
BOOL g_bInitNtOnce = FALSE;
BOOL g_bInitPosOnce = FALSE;

bool cmpInst(const CThostFtdcInstrumentFieldEx* pfirst,const CThostFtdcInstrumentFieldEx* psecond) 
{    
	int iRes = stricmp(pfirst->iinf.InstrumentID,psecond->iinf.InstrumentID);
	return (iRes<=0);
}

//仅保留期货期权 1和2
void CXTraderDlg::FiltInsList()
{
	VIT_if vif;
	VIT_mr vmr;
	VIT_cf vcf;
	
	for(vif=m_InsinfVec.begin(); vif!=m_InsinfVec.end();)
	{
		if(((*vif)->iinf.ProductClass > '2') || ((*vif)->iinf.IsTrading==0))
		{vif = m_InsinfVec.erase(vif);}
		else
			++vif;
	}
	
	for(vif=m_InsinfVec.begin(); vif!=m_InsinfVec.end();vif++)
	{
		for(vmr=m_MargRateVec.begin(); vmr!=m_MargRateVec.end();vmr++)
		{
			if (!strcmp((*vmr)->InstrumentID,(*vif)->iinf.InstrumentID))
			{
				(*vif)->iinf.LongMarginRatio = (*vmr)->LongMarginRatioByMoney;
				
				(*vif)->iinf.ShortMarginRatio = (*vmr)->ShortMarginRatioByMoney;
			}
			
		}
		
	}
	
	for (vcf=m_FeeRateVec.begin();vcf!=m_FeeRateVec.end();vcf++)
	{	
		for(vif=m_InsinfVec.begin(); vif!=m_InsinfVec.end();vif++)
		{
			if (!strcmp((*vcf)->InstrumentID,(*vif)->iinf.ProductID))
			{
				(*vif)->OpenRatioByMoney = (*vcf)->OpenRatioByMoney;
				(*vif)->OpenRatioByVolume = (*vcf)->OpenRatioByVolume;
				(*vif)->CloseRatioByMoney = (*vcf)->CloseRatioByMoney;
				(*vif)->CloseRatioByVolume = (*vcf)->CloseRatioByVolume;
				(*vif)->CloseTodayRatioByMoney = (*vcf)->CloseTodayRatioByMoney;
				(*vif)->CloseTodayRatioByVolume = (*vcf)->CloseTodayRatioByVolume;
			}
		}
	}
	
	sort(m_InsinfVec.begin(),m_InsinfVec.end(),cmpInst);
	
}

CXTraderDlg::CXTraderDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CXTraderDlg::IDD, pParent)
{

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pSubMd = NULL;

	ansi2uni(CP_ACP,((CXTraderApp*)AfxGetApp())->m_szInst,m_szInst.GetBuffer(MAX_PATH));
	m_szInst.ReleaseBuffer();

	m_ixPos = ((CXTraderApp*)AfxGetApp())->m_ixPos;
	m_iyPos = ((CXTraderApp*)AfxGetApp())->m_iyPos;
	m_iWidth = ((CXTraderApp*)AfxGetApp())->m_iWidth;
	m_iHeight = ((CXTraderApp*)AfxGetApp())->m_iHeight;

	m_bTrans = false;
	m_iAlpha = 128;
	m_bTop = true;
	
	m_OldSize = CSize(-1,-1);

	m_bChkLastP = FALSE;
	m_bUpdateOp = TRUE;
	m_InstInf = new CThostFtdcInstrumentField();
	m_pDepthMd = new CThostFtdcDepthMarketDataField();
	m_pTdAcc = new CThostFtdcTradingAccountField();
	m_pInvInf = new CThostFtdcInvestorField();
	m_pNotifyBkYe = new CThostFtdcNotifyQueryAccountField();
	m_pCliInfo = new CLINFO();
	
	m_Timer =0;
	m_pSync = NULL;
	m_pQryAcc = NULL;
	m_pOrder = NULL;
	m_uSync=0;
	m_pInsInit = NULL;
	m_OrderPrice = 1.0;
	m_dOldPrice = 1.0;

	m_szExpDef = _T("");

	m_sDefServer = _T("cn.pool.ntp.org");
	InitMgrFee();

	InitAllVecs();
}

CXTraderDlg::~CXTraderDlg()
{
	ClearMemory();
}

void CXTraderDlg::InitAllVecs()
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	
	m_InsinfVec = pApp->m_cT->m_InsinfVec;
	FiltInsList();
	pApp->m_cT->m_InsinfVec.clear();
	pApp->m_cT->m_InsinfVec = m_InsinfVec;
	//////////////////////////
	m_orderVec = pApp->m_cT->m_orderVec;
	m_tradeVec = pApp->m_cT->m_tradeVec;
	m_AccRegVec = pApp->m_cT->m_AccRegVec;
	m_TdCodeVec = pApp->m_cT->m_TdCodeVec;
	m_InvPosVec = pApp->m_cT->m_InvPosVec;

	m_pCliInfo->FrtID = pApp->m_cT->m_ifrontId;
	m_pCliInfo->SesID = pApp->m_cT->m_isessionId;
	_tcscpy(m_pCliInfo->ProdInf,_T(PROD_INFO));

	memcpy(&m_TdAcc,&pApp->m_cT->m_TdAcc,sizeof(CThostFtdcTradingAccountField));
	for (int i=0;i<4;i++)
	{
		m_tsEXnLocal[i] = pApp->m_cT->m_tsEXnLocal[i];
	}
	m_onRoadVec = m_orderVec;
	///////////////////////////////////////////////////
	for(VOrd odIt=m_onRoadVec.begin(); odIt!=m_onRoadVec.end();)
	{
		if((*odIt)->OrderStatus !='1' && (*odIt)->OrderStatus !='3'  )
		{odIt = m_onRoadVec.erase(odIt);}
		else
			++odIt;
	}
	
	//////////////////////////////////////////////////
	for(VInvP vip=m_InvPosVec.begin(); vip!=m_InvPosVec.end();)
	{
		if((*vip)->YdPosition==0 && (*vip)->Position ==0)
		{vip = m_InvPosVec.erase(vip);}
		else
			++vip;
	}
	///////////////////////////////////////////////////////////

	UpdateMdList();
}

void CXTraderDlg::ClearMemory()
{
	DEL(m_InstInf);
	DEL(m_pDepthMd);
	DEL(m_pTdAcc);
	DEL(m_pInvInf);
	DEL(m_pNotifyBkYe);
	DEL(m_pCliInfo);
	
	ClearVector(m_SubList);
	ClearVector(m_MargRateVec);
	ClearVector(m_AccRegVec);
	ClearVector(m_TdCodeVec);
	ClearVector(m_InvPosVec);
	ClearVector(m_FeeRateVec);
	ClearVector(m_onRoadVec);
}

void CXTraderDlg::OnDestroy() 
{
	SaveConfig();

	if (m_pSubMd != NULL)
	{
		TerminateThread(m_pSubMd->m_hThread, 0);
		m_pSubMd= NULL;
	}
	
	if (m_pSync != NULL)
	{
		TerminateThread(m_pSync->m_hThread, 0);
		m_pSync = NULL;
	}
	
	KillTimer(REFRESH_TIMER);
	m_Timer = NULL;
	
	KillTimer(SYNC_TIMER);
	m_uSync= NULL;

	m_Notify.uFlags=NULL;
	Shell_NotifyIcon(NIM_DELETE,&m_Notify);

	//防止退出崩溃 用暴力手段!
	TerminateProcess(GetCurrentProcess(),0);
	//////////////////////////////////////////
	CDialog::OnDestroy();
}


void CXTraderDlg::SaveConfig()
{
	////////////////////////////////////////////////////
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	
	xml_document doc;
	xml_node proot;
	xml_parse_result result;
	
	result = doc.load_file(CFG_FILE,parse_full);
	if (result.status==status_ok)
	{
		proot = doc.child(ROOT);
		
		//保存修改过的交易密码
		proot.remove_child(INS_LST);
		proot.remove_child(WND_INF);
		
		xml_node nodeInst = proot.append_child(INS_LST);
		nodeInst.append_child(node_pcdata).set_value(pApp->m_szInst);
		//////////////////////////////////	
		CRect rect;
		GetWindowRect(rect);
		
		m_ixPos = rect.left;
		m_iyPos = rect.top;
		m_iWidth = rect.Width();
		m_iHeight = rect.Height();
		
		xml_node nodeWndInf = proot.append_child(WND_INF);
		
		nodeWndInf.append_attribute("xPos") = m_ixPos;
		nodeWndInf.append_attribute("yPos") = m_iyPos;
		nodeWndInf.append_attribute("width") = m_iWidth;
		nodeWndInf.append_attribute("height") = m_iHeight;
		/////////////////////////////////
		doc.save_file(CFG_FILE,PUGIXML_TEXT("\t"),format_default,encoding_utf8);
	}
	/*
	if (pApp->m_cT->m_isessionId != -1)
	{
		BOOL bRet = GenXmlHdr(SIDS);
		result = doc.load_file(SIDS,parse_full);
		if (result.status==status_ok)
		{
			if(bRet){ proot = doc.append_child(SESRT); }
			else
			{ proot = doc.child(SESRT);}
			xml_node nodeSids = proot.append_child(SESITEM);
			
			SYSTEMTIME curTime;
			::GetLocalTime(&curTime);
			
			char szEndt[MAX_PATH];
			sprintf(szEndt,"%02d:%02d:%02d.%03d",curTime.wHour,curTime.wMinute,curTime.wSecond,curTime.wMilliseconds);
			
			nodeSids.append_attribute("sid") = pApp->m_cT->m_isessionId;
			nodeSids.append_attribute("bkr") = pApp->m_sBROKER_ID;
			nodeSids.append_attribute("uid") = pApp->m_sINVESTOR_ID;
			nodeSids.append_attribute("begin") = pApp->m_cT->m_sTmBegin;
			nodeSids.append_attribute("end") = szEndt;
			doc.save_file(SIDS,PUGIXML_TEXT("\t"),format_default,encoding_utf8);
		}
	}
	*/
}


void CXTraderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CXTraderDlg)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_S1P, m_csS1P);
	DDX_Control(pDX, IDC_S1V, m_csS1V);
	DDX_Control(pDX, IDC_LASTP, m_csLastP);
	DDX_Control(pDX, IDC_LASTV, m_csLastV);
	DDX_Control(pDX, IDC_B1P, m_csB1P);
	DDX_Control(pDX, IDC_B1V, m_csB1V);
	DDX_Control(pDX, IDC_INST, m_CombInst);
	DDV_MaxChars(pDX, m_szInst, 6);
	DDX_Control(pDX, IDC_SUPDOWN, m_csSUpDown);
	DDX_Control(pDX, IDC_DUPDOWN, m_csDUpDown);
	DDX_Control(pDX, IDC_SHIGHEST, m_csSHighest);
	DDX_Control(pDX, IDC_DHIGHEST, m_csDHighest);
	DDX_Control(pDX, IDC_SOPT, m_csSOpt);
	DDX_Control(pDX, IDC_DOPT, m_csDOpt);
	DDX_Control(pDX, IDC_SLOWEST, m_csSLowest);
	DDX_Control(pDX, IDC_DLOWEST, m_csDLowest);
	DDX_Control(pDX, IDC_STOTAL, m_csSTotal);
	DDX_Control(pDX, IDC_VTOTAL, m_csVTotal);
	DDX_Control(pDX, IDC_SOPEN, m_csSHold);
	DDX_Control(pDX, IDC_VOPEN, m_csVHold);
	DDX_Control(pDX, IDC_SSMP, m_csSSmp);
	DDX_Control(pDX, IDC_DSMP, m_csDSmp);
	DDX_Control(pDX, IDC_UPTIME, m_csSUptime);
	DDX_Control(pDX, IDC_SS1, m_csSS1);
	DDX_Control(pDX, IDC_SLAST, m_csSLast);
	DDX_Control(pDX, IDC_SB1, m_csSB1);
	DDX_Control(pDX, IDC_MDPAN, m_GroupMd);
	DDX_Control(pDX, IDC_GRPACC, m_GroupAcc);
	//DDX_Control(pDX, IDC_GRPEXEC, m_GroupExec);
	DDX_Text(pDX, IDC_EDVOL, m_OrderVol);
	DDX_Text(pDX, IDC_EDPRICE, m_OrderPrice);
	DDX_Control(pDX, IDC_SPINPRICE, m_SpinPrice);
	DDX_Control(pDX, IDC_SPINVOL, m_SpinVol);
	DDX_Control(pDX, IDC_COMB_OC, m_CombOC);
	DDX_Control(pDX, IDC_COMB_BS, m_CombBS);
	DDX_Check(pDX, IDC_CHK_NEWP, m_bChkLastP);
	DDX_Control(pDX, IDC_LST_ONROAD, m_LstOnRoad);
	DDX_Control(pDX, IDC_LST_ODINF, m_LstOrdInf);
	DDX_Control(pDX, IDC_LST_TRADE, m_LstTdInf);
	DDX_Control(pDX, IDC_LST_INVPOS, m_LstInvPosInf);
	DDX_Control(pDX, IDC_LST_ALLINST, m_LstAllInsts);
	DDX_Control(pDX, IDC_TABPAGE, m_TabOption);
	DDX_Control(pDX, IDC_CPYK, m_csCpProf);
	DDX_Control(pDX, IDC_HPYK, m_csHpProf);
	DDX_Control(pDX, IDC_TDFEE, m_csTdFee);
	//DDX_Control(pDX,IDC_BTORDER,m_btnOrder);
	
}

BEGIN_MESSAGE_MAP(CXTraderDlg, CDialog)
	//{{AFX_MSG_MAP(CXTraderDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_INITMENUPOPUP()
	ON_WM_TIMER()
	//ON_WM_COPYDATA()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(ID_EXIT, OnExit)
	//ON_EN_CHANGE(IDC_INST, OnChangeInst)
	ON_EN_CHANGE(IDC_EDVOL, OnChangeEditVol)
	ON_EN_CHANGE(IDC_EDPRICE, OnChangeEditPrice)
	ON_COMMAND(ID_HELP_ABOUT, OnAbout)
	ON_COMMAND(ID_TOOL_TRANS, OnTrans)
	ON_COMMAND(ID_TIPS, OnTips)
	ON_COMMAND(ID_MODPASS, OnModifyPass)
	ON_COMMAND(ID_TRANSBF,OnTransBf)
	ON_UPDATE_COMMAND_UI(ID_TOOL_TRANS, OnUpdateTrans)
	ON_COMMAND(ID_VIEW_TOP, OnViewTop)
	ON_UPDATE_COMMAND_UI(ID_VIEW_TOP, OnUpdateViewTop)
	ON_BN_CLICKED(IDC_BTORDER, OnBtOrder)
	ON_BN_CLICKED(IDC_CHK_NEWP, OnBnClkChkLastP)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TABPAGE, OnTabSelchange)
	//ON_NOTIFY(TCN_SELCHANGING, IDC_TABPAGE, OnTabSelchanging)
	ON_NOTIFY(NM_DBLCLK, IDC_LST_ONROAD, OnNMDblclkOnroad)
	ON_NOTIFY(NM_CLICK, IDC_LST_ONROAD, OnNMClkLstOnroad)
	ON_NOTIFY(NM_RCLICK, IDC_LST_ONROAD, OnNMRClkLstOnroad)
	//ON_NOTIFY(LVN_BEGINLABELEDIT, IDC_LST_ONROAD, OnBeginlabeledit)
	//ON_NOTIFY(LVN_ENDLABELEDIT, IDC_LST_ONROAD, OnEndlabeledit)
	//ON_REGISTERED_MESSAGE(WM_XLISTCTRL_EDIT_END, OnEditEnd)
	ON_NOTIFY(NM_DBLCLK, IDC_LST_ONROAD, OnNMDblclkOnroad)
	ON_NOTIFY(NM_CLICK, IDC_LST_ONROAD, OnNMClkLstOnroad)
	ON_NOTIFY(NM_RCLICK, IDC_LST_ONROAD, OnNMRClkLstOnroad)
	ON_NOTIFY(NM_DBLCLK, IDC_LST_ODINF, OnNMDblclkOrdInf)
	ON_NOTIFY(NM_CLICK, IDC_LST_ODINF, OnNMClkLstOrdInf)
	ON_NOTIFY(NM_RCLICK, IDC_LST_ODINF, OnNMRClkLstOrdInf)
	ON_NOTIFY(NM_DBLCLK, IDC_LST_TRADE, OnNMDblclkTdInf)
	ON_NOTIFY(NM_CLICK, IDC_LST_TRADE, OnNMClkLstTdInf)
	ON_NOTIFY(NM_RCLICK, IDC_LST_TRADE, OnNMRClkLstTdInf)
	ON_NOTIFY(NM_DBLCLK, IDC_LST_INVPOS, OnNMDblclkInvPInf)
	ON_NOTIFY(NM_CLICK, IDC_LST_INVPOS, OnNMClkLstInvPInf)
	ON_NOTIFY(NM_RCLICK, IDC_LST_INVPOS, OnNMRClkLstInvPInf)
	ON_NOTIFY(NM_CLICK, IDC_LST_ALLINST, OnNMClkLstInsts)
	ON_NOTIFY(NM_RCLICK, IDC_LST_ALLINST, OnNMRClkLstInsts)
	ON_BN_CLICKED(IDC_BTQRYACC,OnClkQryAcc)
	ON_CBN_SELCHANGE(IDC_INST, OnSelchangeCombo)
	ON_CBN_EDITCHANGE(IDC_INST, OnCbnEditchangeInst)
	ON_COMMAND(ID_CANCEL_ORD, OnCancelOrd)
	ON_COMMAND(ID_CANCEL_ALL, OnCancelAll)
	ON_COMMAND(ID_CSV_EXPORT, OnCsvExport)
	ON_COMMAND(ID_MOD_DSJ, OnModDsj)
	ON_COMMAND(ID_USERINFO, OnUserInfo)
	ON_COMMAND(ID_CFMMC, OnCfmmc)
	ON_COMMAND(ID_GENMDFEE, OnGenMdFee)
	ON_COMMAND(ID_HISETTINF, OnHiSettInf)
	ON_COMMAND(ID_HKEY, OnHkeySet)
	ON_REGISTERED_MESSAGE(WM_SYNCTIME,SyncTimeMsg)
	ON_REGISTERED_MESSAGE(WM_QRYACC_MSG,QryAccMsg)
	ON_REGISTERED_MESSAGE(WM_QRYUSER_MSG,QryUserMsg)
	ON_REGISTERED_MESSAGE(WM_QRYBKYE_MSG,QryBkYe)
	ON_MESSAGE(WM_UPDATEMD_MSG,UpdateMdMsg)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CXTraderDlg message handlers

int CXTraderDlg::FindOrdInOnRoadVec(TThostFtdcSequenceNoType BkrOrdSeq)
{	
	bool founded=false;    UINT i=0;
	for(i=0; i<m_onRoadVec.size(); i++)
	{
		if(m_onRoadVec[i]->BrokerOrderSeq == BkrOrdSeq) 
		{ founded=true;    break;}
	}
	if (founded) { return i; }

	return (-1);
}

int CXTraderDlg::FindOrdInOnRoadLst(TThostFtdcOrderSysIDType OrdSysID)
{
	int nRow = -1;
	int nItem = m_LstOnRoad.GetItemCount();

	if (nItem<1) return (-1);
	//int nColumns = ONROAD_ITMES;//m_LstOnRoad.GetXHeaderCtrl()->GetItemCount();

	CString szID1,szID2;

	ansi2uni(CP_ACP,OrdSysID,szID1.GetBuffer(MAX_PATH));
	szID1.ReleaseBuffer();
	szID1.TrimLeft();

	for (nRow = 0; nRow < nItem; nRow++)
	{
		szID2 = m_LstOnRoad.GetItemText(nRow,0);

		if (szID1.Compare(szID2)==0) break;

	}

	return nRow;
}

void CXTraderDlg::AddCombInst()
{
	TCHAR szInst[MAX_PATH];
	BOOL bRes = FALSE;
	for (UINT i=0; i<m_InsinfVec.size();i++)
	{
		ansi2uni(CP_ACP,m_InsinfVec[i]->iinf.InstrumentID,szInst);
		m_CombInst.AddString(szInst);
	}
}

BOOL CXTraderDlg::IsValidInst(LPCTSTR lpszInst)
{
	TCHAR szInst[MAX_PATH];
	for (UINT i=0; i<m_InsinfVec.size();i++)
	{
		ansi2uni(CP_ACP,m_InsinfVec[i]->iinf.InstrumentID,szInst);
		if (!_tcscmp(lpszInst,szInst))
		{
			memcpy(m_InstInf,m_InsinfVec[i],sizeof(INSINFEX));
			return TRUE;
			break;
		}
	}
	return FALSE;
}
void CXTraderDlg::InitProfit()
{
	/////////////////显示盈亏情况//////////////////////////////////
	CString szCPft,szPPft,szTdFee;
	
	//szTdFee=_T("1568"); outStrAs4(szTdFee);
	//szCPft= _T("4890"); outStrAs4(szCPft);
	//szPPft=_T("0"); outStrAs4(szPPft);
	
	szTdFee.Format(_T("%d"),D2Int(m_TdAcc.Commission)); outStrAs4(szTdFee);
	szCPft.Format(_T("%d"),D2Int(m_TdAcc.CloseProfit)); outStrAs4(szCPft);
	szPPft.Format(_T("%d"),D2Int(m_TdAcc.PositionProfit)); outStrAs4(szPPft);
	
	m_csCpProf.SetFont(_T("Arial"),19,FW_BOLD);
	m_csCpProf.SetBkColor(ACC_BG);
	m_csCpProf.SetWindowText(szCPft,WHITE,DT_CENTER);
	
	m_csHpProf.SetFont(_T("Arial"),19,FW_BOLD);
	m_csHpProf.SetBkColor(ACC_BG);
	m_csHpProf.SetWindowText(szPPft,WHITE,DT_CENTER);
	
	m_csTdFee.SetFont(_T("Arial"),19,FW_BOLD);
	m_csTdFee.SetBkColor(ACC_BG);
	m_csTdFee.SetWindowText(szTdFee,WHITE,DT_CENTER);	
/////////////////////////////////////////////////////////////////
}

void CXTraderDlg::InitData()
{
	
	m_GroupMd.SetXPGroupStyle(CXPGroupBox::XPGB_WINDOW).SetBackgroundColor(BLACK, BLACK).SetBorderColor(BLACK);
	m_GroupAcc.SetXPGroupStyle(CXPGroupBox::XPGB_WINDOW).SetBackgroundColor(ACC_BG, ACC_BG).SetCatptionTextColor(WHITE).SetBorderColor(ACC_BG);

	SetWindowText(((CXTraderApp*)AfxGetApp())->m_szTitle);
	//CreateStatusBar();

	m_LstTdInf.ShowWindow( SW_HIDE );
	m_LstOrdInf.ShowWindow( SW_HIDE );
	m_LstInvPosInf.ShowWindow( SW_HIDE );
	m_LstAllInsts.ShowWindow( SW_HIDE );

	SYSTEMTIME curTime;	
	CString	szT;
	::GetLocalTime(&curTime);
	szT.Format(szT, _T("%02d:%02d:%02d"), curTime.wHour, curTime.wMinute, curTime.wSecond);

	m_csSS1.SetWindowText(_T("卖①"));
	m_csS1P.SetWindowText(_T("2279.2"),GREEN);
	m_csS1V.SetWindowText(_T("2"),YELLOW);
	m_csSLast.SetWindowText(_T("最新"));
	m_csLastV.SetWindowText(_T(" "),YELLOW);
	m_csLastP.SetWindowText(_T("2279.2"),RED);
	m_csSB1.SetWindowText(_T("买①"));
	m_csB1P.SetWindowText(_T("2279.0"),GREEN);
	m_csB1V.SetWindowText(_T("6"),YELLOW);
	m_csSUpDown.SetWindowText(_T("涨跌"));
	m_csDUpDown.SetWindowText(_T("-27.8"),GREEN);
	m_csSHighest.SetWindowText(_T("最高"));
	m_csDHighest.SetWindowText(_T("2328.6"),RED);
	m_csSLowest.SetWindowText(_T("最低"));
	m_csDLowest.SetWindowText(_T("2243.6"),GREEN);
	m_csSTotal.SetWindowText(_T("总手"));
	m_csVTotal.SetWindowText(_T("884883"),YELLOW);
	m_csSHold.SetWindowText(_T("持仓"));
	m_csVHold.SetWindowText(_T("88383"),YELLOW);
	m_csSSmp.SetWindowText(_T("昨结"));
	m_csDSmp.SetWindowText(_T("2307.0"),WHITE);
	m_csSOpt.SetWindowText(_T("今开"));
	m_csDOpt.SetWindowText(_T("2323.0"),RED);
	
	m_csSUptime.SetWindowText(_T("15:15:00.000"));
	m_csSUptime.SetFont(_T("Arial"),16,FW_NORMAL);
	//////////////////////////////////////////////////////////////////
	const TCHAR* strBS[2] ={_T("买入"),_T("卖出")};
	const TCHAR* strOC[3] ={ORD_O,ORD_C,ORD_CT};
	int i=0,nIndex = 0;
	for (i=0;i<2;i++)
	{
		nIndex = m_CombBS.AddString(strBS[i]);
		m_CombBS.SetItemData(nIndex,i);
	}
	m_CombBS.SetCurSel(0);
	for (i=0;i<3;i++)
	{
		nIndex = m_CombOC.AddString(strOC[i]);
		m_CombOC.SetItemData(nIndex,i);
	}
	m_CombOC.SetCurSel(0);

/////////////////////////////////////////////////////////////////////////////
	BOOL bRes = IsValidInst(m_szInst);
	if (!m_pSubMd && bRes)
	{
		m_pSubMd = AfxBeginThread((AFX_THREADPROC)SubscribeMD, this);
		//Sleep(100);
	}

	::GetLocalTime(&curTime);	
	szT.Format(_T("%02d:%02d:%02d CTP登录成功"), curTime.wHour, curTime.wMinute, curTime.wSecond);
	SetStatusTxt(szT,2);

	if (m_Timer == 0)
	{
		m_Timer = (UINT)SetTimer(REFRESH_TIMER, 1000, NULL);
	}

	int iMaxVol=1000000;
	double dPriceTick=1;
	int iDig = 1;
	if (bRes)
	{
		dPriceTick = m_InstInf->PriceTick;
		iMaxVol = m_InstInf->MaxLimitOrderVolume;
		iDig = JudgeDigit(dPriceTick);
	}

	m_SpinVol.SetBuddy((CEdit*)GetDlgItem(IDC_EDVOL));
	m_SpinVol.SetDecimalPlaces (0);
	m_SpinVol.SetTrimTrailingZeros (TRUE);
	m_SpinVol.SetRangeAndDelta (1, iMaxVol, 1);
	m_SpinVol.SetPos(10.0);
	
	m_SpinPrice.SetBuddy((CEdit*)GetDlgItem(IDC_EDPRICE));
	m_SpinPrice.SetDecimalPlaces (iDig);
	m_SpinPrice.SetTrimTrailingZeros (FALSE);

	InitTabs();

	InitAllHdrs();

	InitLstOnRoad();
	InitLstOrder();
	InitLstInvPos();
	InitLstTrade();

	AddCombInst();
	m_CombInst.SetWindowText(m_szInst);

	InitProfit();

	UpdateData(true);
}

void CXTraderDlg::InitAllHdrs()
{
	TCHAR* lpHdrs0[ONROAD_ITMES] = {_T("单号"),_T("合约"),_T("买卖"),_T("开平"),_T("未成"),_T("价格"),_T("时间"),_T("冻结金")};
	int iWidths0[ONROAD_ITMES] = {1,46,34,34,34,46,60,60};
	int i;
	int total_cx = 0;
	LVCOLUMN lvcolumn;
	memset(&lvcolumn, 0, sizeof(lvcolumn));
	
	for (i = 0;i<ONROAD_ITMES ; i++)
	{
		lvcolumn.mask     = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH|LVCFMT_IMAGE;
		lvcolumn.fmt      = LVCFMT_RIGHT;
		lvcolumn.iSubItem = i;
		lvcolumn.pszText  = lpHdrs0[i];
		lvcolumn.cx       = iWidths0[i];
		
		total_cx += lvcolumn.cx;
		m_LstOnRoad.InsertColumn(i, &lvcolumn);
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	TCHAR* lpHdrs1[ORDER_ITMES] = {_T("单号"),_T("合约"),_T("买卖"),_T("开平"),_T("状态"),_T("价格"),_T("报量"),_T("未成"),
		_T("已成"),_T("均价"),_T("时间"),_T("冻保证金"),_T("冻手续费"),_T("详细状态")};
	int iWidths1[ORDER_ITMES] = {46,46,34,34,60,46,34,34,34,46,60,60,60,180};
	total_cx = 0;
	memset(&lvcolumn, 0, sizeof(lvcolumn));
	
	for (i = 0;i<ORDER_ITMES ; i++)
	{
		lvcolumn.mask     = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH|LVCFMT_IMAGE;
		lvcolumn.fmt      = LVCFMT_RIGHT;
		lvcolumn.iSubItem = i;
		lvcolumn.pszText  = lpHdrs1[i];
		lvcolumn.cx       = iWidths1[i];
		
		total_cx += lvcolumn.cx;
		m_LstOrdInf.InsertColumn(i, &lvcolumn);
	}
	
	//////////////////////////////////////////////////////////////////////////////////////////////
	TCHAR* lpHdrs2[TRADE_ITMES] = {_T("合约"),_T("买卖"),_T("开平"),_T("价格"),_T("手数"),_T("时间"),_T("手续费"),
		_T("投保"),_T("成交类型"),_T("交易所"),_T("成交编号"),_T("报单编号")};
	int iWidths2[TRADE_ITMES] = {46,34,34,46,34,60,46,34,60,46,60,60};
	total_cx = 0;
	memset(&lvcolumn, 0, sizeof(lvcolumn));
	
	for (i = 0;i<TRADE_ITMES ; i++)
	{
		lvcolumn.mask     = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH|LVCFMT_IMAGE;
		lvcolumn.fmt      = LVCFMT_RIGHT;
		lvcolumn.iSubItem = i;
		lvcolumn.pszText  = lpHdrs2[i];
		lvcolumn.cx       = iWidths2[i];
		
		total_cx += lvcolumn.cx;
		m_LstTdInf.InsertColumn(i, &lvcolumn);
	}
	
	///////////////////////////////////////////////////////////////////////////////
	TCHAR* lpHdrs3[INVPOS_ITMES] = {_T("合约"),_T("买卖"),_T("总持仓"),_T("可平量"),_T("持仓均价"),_T("持仓盈亏"),_T("占保证金"),_T("总盈亏")};
	int iWidths3[INVPOS_ITMES] = {46,34,46,46,60,60,60,60};
	total_cx = 0;
	memset(&lvcolumn, 0, sizeof(lvcolumn));
	
	for (i = 0;i<INVPOS_ITMES ; i++)
	{
		lvcolumn.mask     = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH|LVCFMT_IMAGE;
		lvcolumn.fmt      = LVCFMT_RIGHT;
		lvcolumn.iSubItem = i;
		lvcolumn.pszText  = lpHdrs3[i];
		lvcolumn.cx       = iWidths3[i];
		
		total_cx += lvcolumn.cx;
		m_LstInvPosInf.InsertColumn(i, &lvcolumn);
	}
	///////////////////////////////////////////////////////////////////////////////////////
	
	TCHAR* lpHdrs4[ALLINST_ITMES] = {_T("代码"),_T("合约"),_T("合约名"),_T("交易所"),_T("乘数"),_T("点差"),
		_T("类型"),_T("最后日期"),_T("保证金率"),_T("手续费率")};
	int iWidths4[ALLINST_ITMES] = {26,46,80,46,34,34,34,60,60,120};
	total_cx = 0;
	memset(&lvcolumn, 0, sizeof(lvcolumn));
	
	for (i = 0;i<ALLINST_ITMES ; i++)
	{
		lvcolumn.mask     = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH|LVCFMT_IMAGE;
		lvcolumn.fmt      = LVCFMT_RIGHT;
		lvcolumn.iSubItem = i;
		lvcolumn.pszText  = lpHdrs4[i];
		lvcolumn.cx       = iWidths4[i];
		
		total_cx += lvcolumn.cx;
		m_LstAllInsts.InsertColumn(i, &lvcolumn);
	}
	///////////////////////////////////////////////////////////////////////////////////////	
}

void CXTraderDlg::InsertLstOrder(CThostFtdcOrderField *pOrder,int nIndex,LPCTSTR lpErrMsg)
{
	//nIndex为-1是新插入操作,其他为修改
	/*
		pOrder->OrderStatus==THOST_FTDC_OST_ErrOrd,lpErrMsg !=NULL
	*/
	int nItem, nSubItem;
	COLORREF rBg,rTx=BLACK;
	
	CString szItems[ORDER_ITMES];
	
	ansi2uni(CP_ACP,pOrder->OrderSysID,szItems[0].GetBuffer(MAX_PATH));
	szItems[0].ReleaseBuffer();
	szItems[0].TrimLeft();
	ansi2uni(CP_ACP,pOrder->InstrumentID,szItems[1].GetBuffer(MAX_PATH));
	szItems[1].ReleaseBuffer();

	szItems[2]=JgBsType(pOrder->Direction);
	szItems[3]=JgOcType(pOrder->CombOffsetFlag[0]);
	
	szItems[4]=JgOrdStatType(pOrder->OrderStatus);
	
	szItems[5].Format(_T("%f"),pOrder->LimitPrice);
	szItems[5].TrimRight('0');
	int iLen = szItems[5].GetLength();
	if (szItems[5].Mid(iLen-1,1)==_T(".")) {szItems[5].TrimRight(_T("."));}
	
	szItems[6].Format(_T("%d"),pOrder->VolumeTotalOriginal);
	szItems[7].Format(_T("%d"),pOrder->VolumeTotal);
	szItems[8].Format(_T("%d"),pOrder->VolumeTraded);
	if (pOrder->OrderStatus==THOST_FTDC_OST_Canceled && (pOrder->VolumeTraded>=1))
	{
		szItems[4]=_T("已撤余单");
	}
	szItems[9] = (pOrder->OrderStatus==THOST_FTDC_OST_ErrOrd)?_T("-"):szItems[5];
	ansi2uni(CP_ACP,pOrder->InsertTime,szItems[10].GetBuffer(MAX_PATH));
	szItems[10].ReleaseBuffer();	
	
	szItems[11] = (lpErrMsg==NULL)?UNCOMP:_T("0");
	szItems[12] = (lpErrMsg==NULL)?UNCOMP:_T("0");
	CString szStat = (lpErrMsg==NULL)?(JgOrdSubmitStat(pOrder->OrderSubmitStatus)):lpErrMsg;
	
	szItems[13] = szStat;
	if (pOrder->OrderStatus!=THOST_FTDC_OST_ErrOrd)
	{
		CString szInf=_T("");
		TCHAR szPinf[MAX_PATH];
		if ((m_pCliInfo->FrtID != pOrder->FrontID) || (m_pCliInfo->SesID != pOrder->SessionID))
		{
			ansi2uni(CP_ACP,pOrder->UserProductInfo,szPinf);
			szInf.Format(_T("重登录客户:%s"),szPinf);
		}
		else
		{
			szInf.Format(_T("当前客户:%s"),m_pCliInfo->ProdInf);
		}
		
		szItems[13].Format(_T("%s,%s"),(LPCTSTR)szStat,(LPCTSTR)szInf);
	}

	int iCount = m_LstOrdInf.GetItemCount();
	nItem = iCount-1-nIndex;

	int iPos = (nIndex==INSERT_OP)?nItem:nIndex;
	m_LstOrdInf.SetRedraw(FALSE);
	for (nSubItem = 0; nSubItem < ORDER_ITMES; nSubItem++)
	{		
		rBg = (iPos%2)?LITGRAY:WHITE;
		if ((nSubItem == 0)&&(nIndex==INSERT_OP)) {m_LstOrdInf.InsertItem(0, NULL);}
		
		if (nSubItem == 2)
		{
			if (!_tcscmp(szItems[nSubItem],DIR_BUY))
			{ rTx = LITRED;}
			else
			{rTx = LITGREEN;}
		}
		
		else if (nSubItem == 4)
		{
			if (pOrder->OrderStatus<THOST_FTDC_OST_NoTradeQueueing)
			{rTx = LITGREEN;}
			else if (pOrder->OrderStatus==THOST_FTDC_OST_Canceled)
			{rTx = BG_CANCEL;}
			else if (pOrder->OrderStatus==THOST_FTDC_OST_ErrOrd)
			{rTx=RED; }
			else
			{ rTx=BLACK;	}		
		}
		else
		{
			rTx=BLACK;
		}
		
		if (nIndex>=1)
		{
			m_LstOrdInf.SetItemText(nItem, nSubItem, szItems[nSubItem], rTx,rBg);
		}
		else
		{ m_LstOrdInf.SetItemText(0, nSubItem, szItems[nSubItem], rTx,rBg); }
		
		
	}
	m_LstOrdInf.SetRedraw(TRUE);


}

	//所有委托
void CXTraderDlg::InitLstOrder()
{
	int iOrderSize = m_orderVec.size();
	
	int nItem;
	
	for (nItem = 0; nItem <iOrderSize; nItem++)
	{
		InsertLstOrder(m_orderVec[nItem],INSERT_OP);
	}

	g_bInitOrdOnce = TRUE;
}

void CXTraderDlg::InsertLstTrade(CThostFtdcTradeField *pTrade,int nIndex)
{
	int nItem, nSubItem;
	COLORREF rBg,rTx=BLACK;
	
	CString szItems[TRADE_ITMES];
	
	ansi2uni(CP_ACP,pTrade->InstrumentID,szItems[0].GetBuffer(MAX_PATH));
	szItems[1]=JgBsType(pTrade->Direction);
	szItems[2]=JgOcType(pTrade->OffsetFlag);
	
	szItems[3].Format(_T("%f"),pTrade->Price);
	szItems[3].TrimRight('0');
	int iLen = szItems[3].GetLength();
	if (szItems[3].Mid(iLen-1,1)==_T(".")) {szItems[3].TrimRight(_T("."));}
	
	szItems[4].Format(_T("%d"),pTrade->Volume);
	ansi2uni(CP_ACP,pTrade->TradeTime,szItems[5].GetBuffer(MAX_PATH));
	//ansi2uni(CP_ACP,pTrade->InstrumentID,szItems[6].GetBuffer(MAX_PATH));
	szItems[6]= UNCOMP;
	
	szItems[7] = JgTbType(pTrade->HedgeFlag);
	szItems[8] = JgTdType(pTrade->TradeType);
	szItems[9] = JgExchage(pTrade->ExchangeID);
	
	ansi2uni(CP_ACP,pTrade->TradeID,szItems[10].GetBuffer(MAX_PATH));
	szItems[10].ReleaseBuffer();
	szItems[10].TrimLeft();
	
	ansi2uni(CP_ACP,pTrade->OrderSysID,szItems[11].GetBuffer(MAX_PATH));
	szItems[11].ReleaseBuffer();
	szItems[11].TrimLeft();
	
	int iCount = m_LstTdInf.GetItemCount();
	nItem = iCount-1-nIndex;

	int iPos = (nIndex==INSERT_OP)?nItem:nIndex;
	m_LstTdInf.SetRedraw(FALSE);
	for (nSubItem = 0; nSubItem < TRADE_ITMES; nSubItem++)
	{		
		rBg = (iPos%2)?LITGRAY:WHITE;
		if ((nSubItem == 0)&&(nIndex==INSERT_OP)) {m_LstTdInf.InsertItem(0, NULL);}
		
		if (nSubItem == 1)
		{
			if (!_tcscmp(szItems[nSubItem],DIR_BUY))
			{ rTx = LITRED;}
			else
			{rTx = LITGREEN;}
		}
		else
		{ rTx=BLACK;	}
		
		if (nIndex>=1)
		{
			m_LstTdInf.SetItemText(nItem, nSubItem, szItems[nSubItem], rTx,rBg);
		}
		else
		{ m_LstTdInf.SetItemText(0, nSubItem, szItems[nSubItem], rTx,rBg); }
		
	}
	m_LstTdInf.SetRedraw(TRUE);
}


void CXTraderDlg::InitLstTrade()
{
	int iTradeSize = m_tradeVec.size();
	
	int nItem;
	for (nItem = 0; nItem <iTradeSize; nItem++)
	{
		InsertLstTrade(m_tradeVec[nItem],INSERT_OP);
	}
	
	g_bInitTdOnce = TRUE;

}

int CXTraderDlg::FindInstMul(TThostFtdcInstrumentIDType InstID)
{
	bool founded=false;
	int iMul = 1;
	for (UINT i=0; i<m_InsinfVec.size();i++)
	{
		if (!strcmp(InstID,m_InsinfVec[i]->iinf.InstrumentID))
		{
		
			iMul=m_InsinfVec[i]->iinf.VolumeMultiple;
			founded=true;
			break;
		}
	}

	if (founded) {return iMul;}
	
	return (-1);
}

void CXTraderDlg::InsertLstInvP(CThostFtdcInvestorPositionField *pInv)
{

}

	//持仓
void CXTraderDlg::InitLstInvPos()
{
	VInvP vip;
	
	for(vip=m_InvPosVec.begin(); vip!=m_InvPosVec.end();)
	{
		if((*vip)->YdPosition==0 && (*vip)->Position ==0)
		{vip = m_InvPosVec.erase(vip);}
		else
			++vip;
	}

	int iInvPosSize = m_InvPosVec.size();

	int nItem, nSubItem;
	COLORREF rBg,rTx;
	
	CString szItems[INVPOS_ITMES];
	
	//m_LstInvPosInf.LockWindowUpdate();
	// insert the items and subitems into the list
	for (nItem = 0; nItem <iInvPosSize; nItem++)
	{	
		ansi2uni(CP_ACP,m_InvPosVec[nItem]->InstrumentID,szItems[0].GetBuffer(MAX_PATH));
		szItems[0].ReleaseBuffer();
		szItems[1]=JgBsType(m_InvPosVec[nItem]->PosiDirection-2);
		//szItems[3]=JgOcType(m_onRoadVec[nItem]->CombOffsetFlag[0]);
	
		szItems[2].Format(_T("%d"),m_InvPosVec[nItem]->YdPosition+m_InvPosVec[nItem]->Position);
		int iHoldPos = 0;
		if (m_InvPosVec[nItem]->PosiDirection == '2')
		{
			iHoldPos = m_InvPosVec[nItem]->YdPosition+m_InvPosVec[nItem]->Position-m_InvPosVec[nItem]->ShortFrozen;
			szItems[3].Format(_T("%d"),iHoldPos);
		}
		if (m_InvPosVec[nItem]->PosiDirection == '3')
		{
			iHoldPos = m_InvPosVec[nItem]->YdPosition+m_InvPosVec[nItem]->Position-m_InvPosVec[nItem]->LongFrozen;
			szItems[3].Format(_T("%d"),iHoldPos);
		}
		
		double dAvPrice = m_InvPosVec[nItem]->PositionCost/(m_InvPosVec[nItem]->YdPosition+m_InvPosVec[nItem]->Position)/(FindInstMul(m_InvPosVec[nItem]->InstrumentID));
		szItems[4].Format(_T("%.2f"),dAvPrice);
		
		szItems[5].Format(_T("%f"),m_InvPosVec[nItem]->PositionProfit);
		szItems[5].TrimRight('0');
		int iLen = szItems[5].GetLength();
		if (szItems[5].Mid(iLen-1,1)==_T(".")) {szItems[5].TrimRight(_T("."));}

		szItems[6].Format(_T("%f"),m_InvPosVec[nItem]->UseMargin);
		szItems[6].TrimRight('0');
		iLen = szItems[6].GetLength();
		if (szItems[6].Mid(iLen-1,1)==_T(".")) {szItems[6].TrimRight(_T("."));}

		szItems[7]= UNCOMP;
		
		for (nSubItem = 0; nSubItem < INVPOS_ITMES; nSubItem++)
		{
			rBg = (nItem%2)?LITGRAY:WHITE;
			if (nSubItem == 0) {m_LstInvPosInf.InsertItem(nItem, NULL);}
			if (nSubItem == 1)
			{
				if (!_tcscmp(szItems[1],DIR_BUY))
				{ rTx = LITRED;}
				else
				{rTx = LITGREEN;}
			}
			else if (nSubItem == 5)
			{
				if (szItems[5].Mid(0,1)==_T("-"))
				{ rTx = LITGREEN;}
				else
				{rTx = LITRED;}
			}
			else
			{ rTx=BLACK;	}
			m_LstInvPosInf.SetItemText(nItem, nSubItem, szItems[nSubItem], rTx,rBg);
			
		}
	}
	
	//m_LstInvPosInf.UnlockWindowUpdate();
	//m_LstInvPosInf.LoadColumns(_T("XListCtrl"), _T("Columns"));
	g_bInitPosOnce = TRUE;

}

	//合约

void CXTraderDlg::InitLstInsts()
{
	if (!m_pInsInit)
	{
		m_pInsInit = AfxBeginThread((AFX_THREADPROC)InsInfThread, this);
	}	

}

UINT CXTraderDlg::InsInfThread(LPVOID pParam)
{
	CXTraderDlg* pDlg = (CXTraderDlg*)pParam;
	int iInstSize = pDlg->m_InsinfVec.size();
	
	int nItem, nSubItem;
	COLORREF rBg,rTx=BLACK;
	
	CString szItems[ALLINST_ITMES];
	
	pDlg->m_LstAllInsts.SetRedraw(FALSE);
	for (nItem = 0; nItem <iInstSize; nItem++)
	{
		ansi2uni(CP_ACP,pDlg->m_InsinfVec[nItem]->iinf.ProductID,szItems[0].GetBuffer(MAX_PATH));
		szItems[0].ReleaseBuffer();
		ansi2uni(CP_ACP,pDlg->m_InsinfVec[nItem]->iinf.InstrumentID,szItems[1].GetBuffer(MAX_PATH));
		szItems[1].ReleaseBuffer();
		ansi2uni(CP_ACP,pDlg->m_InsinfVec[nItem]->iinf.InstrumentName,szItems[2].GetBuffer(MAX_PATH));
		szItems[2].ReleaseBuffer();
		szItems[3] = JgExchage(pDlg->m_InsinfVec[nItem]->iinf.ExchangeID);
		
		szItems[4].Format(_T("%d"),pDlg->m_InsinfVec[nItem]->iinf.VolumeMultiple);
		szItems[5].Format(_T("%f"),pDlg->m_InsinfVec[nItem]->iinf.PriceTick);
		szItems[5].TrimRight('0');
		int iLen = szItems[5].GetLength();
		if (szItems[5].Mid(iLen-1,1)==_T(".")) {szItems[5].TrimRight(_T("."));}
		
		szItems[6] = JgProType((BYTE)pDlg->m_InsinfVec[nItem]->iinf.ProductClass);
		ansi2uni(CP_ACP,pDlg->m_InsinfVec[nItem]->iinf.ExpireDate,szItems[7].GetBuffer(MAX_PATH));
		szItems[7].ReleaseBuffer();
		szItems[8].Format(_T("%d%%"),D2Int(pDlg->m_InsinfVec[nItem]->iinf.LongMarginRatio*100));
		
		for (UINT j=0;j<pDlg->m_FeeRateVec.size();j++)
		{
			if (!strcmp(pDlg->m_InsinfVec[nItem]->iinf.ProductID,pDlg->m_FeeRateVec[j]->InstrumentID))
			{
				Fee2String(szItems[9],pDlg->m_FeeRateVec[j]->OpenRatioByMoney,pDlg->m_FeeRateVec[j]->OpenRatioByVolume,pDlg->m_FeeRateVec[j]->CloseRatioByMoney,
					pDlg->m_FeeRateVec[j]->CloseRatioByVolume,pDlg->m_FeeRateVec[j]->CloseTodayRatioByMoney,pDlg->m_FeeRateVec[j]->CloseTodayRatioByVolume);
				
				break;
			}
		}

		for (nSubItem = 0; nSubItem < ALLINST_ITMES; nSubItem++)
		{
			rBg = (nItem%2)?LITGRAY:WHITE;
			if (nSubItem == 0) 
			{ 
				pDlg->m_LstAllInsts.InsertItem(nItem,NULL);
				
			}
			pDlg->m_LstAllInsts.SetItemText(nItem, nSubItem, szItems[nSubItem], rTx,rBg);
			
		}
		
	}
	pDlg->m_LstAllInsts.SetRedraw(TRUE);
	g_bInitInstOnce = TRUE;

	pDlg->m_pInsInit = NULL;

	return 0;
}

void CXTraderDlg::InsertLstOnRoad(CThostFtdcOrderField* pOrder,int nIndex)
{
	int nItem, nSubItem;
	COLORREF rBg,rTx;
	
	CString szItems[ONROAD_ITMES];

	ansi2uni(CP_ACP,pOrder->OrderSysID,szItems[0].GetBuffer(MAX_PATH));
	szItems[0].ReleaseBuffer();
	szItems[0].TrimLeft();
		
	ansi2uni(CP_ACP,pOrder->InstrumentID,szItems[1].GetBuffer(MAX_PATH));
	szItems[2]=JgBsType(pOrder->Direction);
	szItems[3]=JgOcType(pOrder->CombOffsetFlag[0]);
		
	szItems[4].Format(_T("%d"),pOrder->VolumeTotal);
	szItems[5].Format(_T("%f"),pOrder->LimitPrice);
	szItems[5].TrimRight('0');
	int iLen = szItems[5].GetLength();
	if (szItems[5].Mid(iLen-1,1)==_T(".")) {szItems[5].TrimRight(_T("."));}
			
	ansi2uni(CP_ACP,pOrder->InsertTime,szItems[6].GetBuffer(MAX_PATH));
	szItems[7]= UNCOMP;
	
	int iCount = m_LstOnRoad.GetItemCount();
	nItem = iCount-1-nIndex;

	int iPos = (nIndex==INSERT_OP)?nItem:nIndex;
	
	m_LstOnRoad.SetRedraw(FALSE);
	for (nSubItem = 0; nSubItem < ONROAD_ITMES; nSubItem++)
	{
		rBg = (iPos%2)?LITGRAY:WHITE;
		if ((nSubItem == 0)&&(nIndex==INSERT_OP)) {m_LstOnRoad.InsertItem(0, NULL);}
		if (nSubItem == 2)
		{
			if (!_tcscmp(szItems[nSubItem],DIR_BUY))
			{ rTx = LITRED;}
			else
			{rTx = LITGREEN;}
		}
		else
		{ rTx=BLACK;	}
		
		if (nIndex>=1)
		{
			m_LstOnRoad.SetItemText(nItem, nSubItem, szItems[nSubItem], rTx,rBg);
		}
		else
		{
			m_LstOnRoad.SetItemText(0, nSubItem, szItems[nSubItem], rTx,rBg);
		}
			
		//if (nSubItem == 4||nSubItem == 5)
		//{m_LstOnRoad.SetEdit(nItem, nSubItem);}
	}
	m_LstOnRoad.SetRedraw(TRUE);
}

void CXTraderDlg::InitLstOnRoad()
{
	VOrd odIt;

	for(odIt=m_onRoadVec.begin(); odIt!=m_onRoadVec.end();)
	{
		if((*odIt)->OrderStatus !='1' && (*odIt)->OrderStatus !='3'  )
		{odIt = m_onRoadVec.erase(odIt);}
		else
			++odIt;
	}
	
	TCHAR* lpHdrs[ONROAD_ITMES] = {_T("单号"),_T("合约"),_T("买卖"),_T("开平"),_T("未成"),_T("价格"),_T("时间"),_T("冻结金")};
	int iWidths[ONROAD_ITMES] = {46,46,34,34,34,46,60,60};
	int i;
	int total_cx = 0;
	LVCOLUMN lvcolumn;
	memset(&lvcolumn, 0, sizeof(lvcolumn));

	for (i = 0;i<ONROAD_ITMES ; i++)
	{
		lvcolumn.mask     = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH|LVCFMT_IMAGE;
		lvcolumn.fmt      = LVCFMT_RIGHT;
		lvcolumn.iSubItem = i;
		lvcolumn.pszText  = lpHdrs[i];
		lvcolumn.cx       = iWidths[i];
		
		total_cx += lvcolumn.cx;
		m_LstOnRoad.InsertColumn(i, &lvcolumn);
	}

	int iOnRoadSize = m_onRoadVec.size();
	// insert the items and subitems into the list
	for (int nItem = 0; nItem <iOnRoadSize; nItem++)
	{
		InsertLstOnRoad(m_onRoadVec[nItem],INSERT_OP);

	}
	g_bInitNtOnce = TRUE;

}
void CXTraderDlg::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu)
{
	
	ASSERT(pPopupMenu != NULL);

	CCmdUI state;
	state.m_pMenu = pPopupMenu;
	ASSERT(state.m_pOther == NULL);
	ASSERT(state.m_pParentMenu == NULL);
	HMENU hParentMenu;
	if (AfxGetThreadState()->m_hTrackingMenu == pPopupMenu->m_hMenu)
		state.m_pParentMenu = pPopupMenu; // Parent == child for tracking popup.
	else if ((hParentMenu = ::GetMenu(m_hWnd)) != NULL)
	{
		CWnd* pParent = this;
	
		if (pParent != NULL &&
			(hParentMenu = ::GetMenu(pParent->m_hWnd)) != NULL)
		{
			int nIndexMax = ::GetMenuItemCount(hParentMenu);
			for (int nIndex = 0; nIndex < nIndexMax; nIndex++)
			{
				if (::GetSubMenu(hParentMenu, nIndex) == pPopupMenu->m_hMenu)
				{
					state.m_pParentMenu = CMenu::FromHandle(hParentMenu);
					break;
				}
			}
		}
	}
	
	state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
	for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax;
	state.m_nIndex++)
	{
		state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
		if (state.m_nID == 0)
			continue; // Menu separator or invalid cmd - ignore it.
		
		ASSERT(state.m_pOther == NULL);
		ASSERT(state.m_pMenu != NULL);
		if (state.m_nID == (UINT)-1)
		{
			// Possibly a popup menu, route to first item of that popup.
			state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
			if (state.m_pSubMenu == NULL ||
				(state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
				state.m_nID == (UINT)-1)
			{
				continue;    // First item of popup can't be routed to.
			}
			state.DoUpdate(this, TRUE); // Popups are never auto disabled.
		}
		else
		{
			state.m_pSubMenu = NULL;
			state.DoUpdate(this, FALSE);
		}
		
		UINT nCount = pPopupMenu->GetMenuItemCount();
		if (nCount < state.m_nIndexMax)
		{
			state.m_nIndex -= (state.m_nIndexMax - nCount);
			while (state.m_nIndex < nCount &&
				pPopupMenu->GetMenuItemID(state.m_nIndex) == state.m_nID)
			{
				state.m_nIndex++;
			}
		}
		state.m_nIndexMax = nCount;
	}
	
}

BOOL CXTraderDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	g_pCWnd = AfxGetMainWnd();
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);
	
	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	MoveWindow(m_ixPos,m_iyPos,m_iWidth,m_iHeight,FALSE);
	
	CRect rect;
	GetClientRect(rect);
	m_OldSize = CSize(rect.Width(), rect.Height());

	InitData();

	if (m_bTop) 
	{	::SetWindowPos(m_hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE); }

	ShowNotifyIcon(this,WM_COMMAND,_T(""),_T(""),2,NIM_ADD);

	if (m_uSync == 0)
	{
		m_uSync = SetTimer(SYNC_TIMER,15*60000,NULL);
	}
	
	SendNotifyMessage(WM_SYNCTIME,0,0);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CXTraderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		OnAbout();
	}
	else
	{
		switch( nID )
		{
		case SC_MINIMIZE:
			OnMinimize();
			break;
			
		default:
			CDialog::OnSysCommand(nID, lParam);
		}
	}
			
}

void CXTraderDlg::OnMinimize()
{
	ShowWindow(SW_MINIMIZE);
	ShowWindow(SW_HIDE);
}

void CXTraderDlg::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent == REFRESH_TIMER)
	{
		SYSTEMTIME curTime;
		::GetLocalTime(&curTime);
		TCHAR	szT[4][256];
		
		CTime t[4];
		for(int i=0;i<4;i++)
		{
			t[i] = CTime(curTime);
			t[i] += m_tsEXnLocal[i];
			_stprintf(szT[i], _T("%02d:%02d:%02d"), t[i].GetHour(), t[i].GetMinute(), t[i].GetSecond());
		
			SetStatusTxt(szT[i],i+3);
		}

	}
	if (nIDEvent == SYNC_TIMER)
	{
		if (!m_pSync)
		{
			m_pSync = AfxBeginThread((AFX_THREADPROC)SyncThread, this);
			
		}
	}

	CDialog::OnTimer(nIDEvent);
}



void CXTraderDlg::OnAbout()
{
	CString str = _T("xTrader v2.0");
	
	CString sDevInfo = _T("");
	
	double ver = _MSC_VER/100 - 6.0;
	sDevInfo.Format(_T("VC%.1f-"),ver);
	
	#ifdef _UNICODE
		sDevInfo += _T("Uni-");
	#else
		sDevInfo += _T("Ansi-");
	#endif
		
	#ifdef _DEBUG
		sDevInfo += _T("Debug-");
	#else
		sDevInfo += _T("Rel-");
	#endif
	
	#ifdef _WIN64
	sDevInfo += _T("64bit@");
	#else
	sDevInfo += _T("32bit@");
	#endif
	
	sDevInfo += AUTHOR;
	sDevInfo += _T(":");
	sDevInfo += __DATE__;
	sDevInfo += _T(",");
	sDevInfo += __TIME__;
	
	ShellAbout(GetSafeHwnd(), str , sDevInfo ,AfxGetApp()->LoadIcon(IDR_MAINFRAME));

}

void CXTraderDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

HCURSOR CXTraderDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CXTraderDlg::OnExit()
{
	PostMessage(WM_CLOSE,0,0);	
}

void CXTraderDlg::OnRestore() 
{
	ShowWindow( SW_RESTORE );
	SetForegroundWindow();
}

BOOL CXTraderDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	switch ( wParam )
	{
	case IDSHELL_RESTORE:
		OnRestore();
		break;
	case IDSHELL_SYNC:
		SendNotifyMessage(WM_SYNCTIME,0,0);
		break;
	case IDSHELL_ABOUT:
		OnAbout();
		break;
	case IDSHELL_EXIT:
		OnExit();
		break;
	case 0:
		switch (lParam)
		{
		case WM_LBUTTONDBLCLK:
		case WM_LBUTTONUP:
			OnRestore();
			break;
		case WM_RBUTTONUP:
			CMenu PopMenu,*pSubMenu;
			CPoint m_point;
			PopMenu.LoadMenu(IDM_POPMENU);
			GetCursorPos(&m_point);
			pSubMenu = PopMenu.GetSubMenu(0);	
			pSubMenu->TrackPopupMenu(TPM_LEFTALIGN,m_point.x,m_point.y,this);
			break;	
		}
	}
	return CDialog::OnCommand(wParam, lParam);
}

BOOL CXTraderDlg::PreTranslateMessage(MSG* pMsg) 
{ 
	int nVirtKey = (int) pMsg->wParam;
	if(pMsg->message == WM_KEYDOWN ) 
	{
		if (GetFocus()->GetParent()->GetDlgCtrlID() == IDC_INST )
		{
			m_CombInst.m_bAutoComplete = TRUE;
			
			if (nVirtKey == VK_DELETE || nVirtKey == VK_BACK ||pMsg->wParam == VK_RETURN)
			{m_CombInst.m_bAutoComplete  = FALSE;}
				
		}

		NMHDR nmhdr; 
		nmhdr.code = TCN_SELCHANGE; 
		nmhdr.hwndFrom = m_TabOption.GetSafeHwnd();  
		nmhdr.idFrom= IDC_TABPAGE;  
		
		switch(nVirtKey)
		{
		case VK_F2:
			m_TabOption.SetCurSel(0);
			::SendMessage(m_TabOption.GetSafeHwnd(),WM_NOTIFY,MAKELONG(TCN_SELCHANGE,0),(LPARAM)(&nmhdr));
			break;
		case VK_F3:
			m_TabOption.SetCurSel(1);
			::SendMessage(m_TabOption.GetSafeHwnd(),WM_NOTIFY,MAKELONG(TCN_SELCHANGE,1),(LPARAM)(&nmhdr));
			break;
		case VK_F4:
			m_TabOption.SetCurSel(2);
			::SendMessage(m_TabOption.GetSafeHwnd(), WM_NOTIFY,MAKELONG(TCN_SELCHANGE,2), (LPARAM)(&nmhdr));
			break;
		case VK_F5:
			m_TabOption.SetCurSel(3);
			::SendMessage(m_TabOption.GetSafeHwnd(), WM_NOTIFY,MAKELONG(TCN_SELCHANGE,3), (LPARAM)(&nmhdr));
			break;
		case VK_F6:
			m_TabOption.SetCurSel(4);
			::SendMessage(m_TabOption.GetSafeHwnd(), WM_NOTIFY,MAKELONG(TCN_SELCHANGE,4), (LPARAM)(&nmhdr));
			break;
		default:
				break;
		}

	} 

	if(nVirtKey == VK_ESCAPE)
	{	return 0; }
	
	return CDialog::PreTranslateMessage(pMsg); 
}

LRESULT CXTraderDlg::DefWindowProc(UINT Msg,WPARAM wParam,LPARAM lParam)
{
	if (Msg == WM_TASKBARCREATED) 
	{
		ShowNotifyIcon(this,WM_COMMAND,_T(""),_T(""),2,NIM_ADD);
	}
	
	return CDialog::DefWindowProc(Msg, wParam, lParam);
}


void CXTraderDlg::OnChangeEditVol() 
{
	UpdateData(true);

}

void CXTraderDlg::OnChangeEditPrice() 
{
	UpdateData(true);
}

void CXTraderDlg::SetStatusTxt(UINT uID, int nPane)
{
	m_StatusBar.SetPaneText(nPane,LoadString(uID),TRUE);
}

void CXTraderDlg::SetStatusTxt(LPCTSTR pMsg, int nPane)
{
	m_StatusBar.SetPaneText(nPane, pMsg,TRUE);
}

void CXTraderDlg::SetTipTxt(LPCTSTR pMsg,int nTool)
{
	m_StatusBar.UpdateTipText(pMsg,nTool);
}

void CXTraderDlg::SetPaneTxtColor(int nPane,COLORREF cr)
{
	m_StatusBar.SetPaneTextColor(nPane,cr);
}

static UINT BASED_CODE indicators[] =
{
	IDS_MD_STAT,
	IDS_TRADE_STAT,
	IDS_RESPINFO,
	IDS_SFETIME,
	IDS_DCETIME,
	IDS_ZCETIME,
	IDS_CFXTIME 
};

static UINT tooltipIndicators[] =
{
	IDS_MD_TIPS,
	IDS_TRADE_TIPS,
	0,
	IDS_SFE_TIPS,
	IDS_DCE_TIPS,
	IDS_ZCE_TIPS,
	IDS_CFX_TIPS 
};

int CXTraderDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	CreateStatusBar();
	return 0;
}

void CXTraderDlg::CreateStatusBar()
{

	m_StatusBar.CreateEx(this);
	m_StatusBar.SetIndicators(indicators,7,tooltipIndicators,NULL);
	
	m_StatusBar.SetPaneInfo(0,IDS_MD_STAT,SBPS_NORMAL,30);
	m_StatusBar.SetPaneInfo(1,IDS_TRADE_STAT,SBPS_NORMAL,30);
	m_StatusBar.SetPaneInfo(2,IDS_RESPINFO,SBPS_NORMAL,160);
	m_StatusBar.SetPaneInfo(3,IDS_SFETIME,SBPS_NORMAL,50);
	m_StatusBar.SetPaneInfo(4,IDS_DCETIME,SBPS_NORMAL,50);
	m_StatusBar.SetPaneInfo(5,IDS_ZCETIME,SBPS_NORMAL,50);
	m_StatusBar.SetPaneInfo(6,IDS_CFXTIME,SBPS_NORMAL,50);

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST,AFX_IDW_CONTROLBAR_LAST,IDS_CFXTIME);
	m_StatusBar.SetDelayTime(300);

	SetPaneTxtColor(0,RED);
	SetPaneTxtColor(1,RED);
}

void CXTraderDlg::OnTrans()
{
	m_bTrans = !m_bTrans;
    
	if(m_bTrans)
	{ 
		SetWindowLong(m_hWnd,GWL_EXSTYLE,GetWindowLong(GetSafeHwnd(),GWL_EXSTYLE)|WS_EX_LAYERED);
		::SetLayeredWindowAttributes(m_hWnd,RGB(0,0,0),m_iAlpha,LWA_ALPHA);   
	}
	else
	{ 	SetWindowLong(m_hWnd,GWL_EXSTYLE,GetWindowLong(GetSafeHwnd(),GWL_EXSTYLE)|WS_EX_LAYERED);
		::SetLayeredWindowAttributes(m_hWnd,RGB(0,0,0),0xFF,LWA_ALPHA);  
	}
}

void CXTraderDlg::OnUpdateTrans(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bTrans);
}

void CXTraderDlg::OnViewTop()
{
	m_bTop = !m_bTop;
	if (m_bTop) 
	{	SetWindowPos(&wndTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE); }
	
	else 
	{	SetWindowPos(&wndNoTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE); }
}

void CXTraderDlg::OnUpdateViewTop(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bTop);
}

UINT CXTraderDlg::SyncThread(LPVOID pParam)
{
	CXTraderDlg *pDlg = (CXTraderDlg*) pParam;
	
	CSNTPClient sntp;
	NtpServerResponse response;
	CString szInfo,szTitle;
	
	if (sntp.GetServerTime(pDlg->m_sDefServer, response,123))
	{		
		CNtpTime newTime(CNtpTime::GetCurrentTime() + response.m_LocalClockOffset);
		if (sntp.SetClientTime(newTime))
		{
			SYSTEMTIME time;
			GetLocalTime(&time);
			
			szTitle.Format(_T("xTrader: %02d:%02d:%02d"), 
				time.wHour, time.wMinute, time.wSecond);
			
			szInfo.Format(_T("从服务器 %s 同步时间成功!"),(LPCTSTR)pDlg->m_sDefServer);
			pDlg->ShowNotifyIcon(g_pCWnd,WM_COMMAND,szTitle,szInfo,10,NIM_MODIFY);
			
		}
		else
		{
			SYSTEMTIME time;
			GetLocalTime(&time);
			
			szTitle.Format(_T("xTrader: %02d:%02d:%02d"), 
				time.wHour, time.wMinute, time.wSecond);
			
			szInfo.Format(_T("从服务器 %s 同步时间失败!"),(LPCTSTR)pDlg->m_sDefServer);
			pDlg->ShowNotifyIcon(g_pCWnd,WM_COMMAND,szTitle,szInfo,10,NIM_MODIFY);
		}
		
	}
	else
		pDlg->ShowNotifyIcon(g_pCWnd,WM_COMMAND,_T("提醒"),_T("从NTP服务器取得时间失败!"),10,NIM_MODIFY);
	
	pDlg->m_pSync = NULL;
	
	return 0;

}

UINT CXTraderDlg::SubscribeMD(LPVOID pParam)
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	CXTraderDlg* pDlg = (CXTraderDlg*) pParam;

	DWORD dwRet;
	char szOldInst[MAX_PATH];
	uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)g_szInstOld,szOldInst);

	if (_tcscmp(g_szInstOld,pDlg->m_szInst))
	{
		LPSTR* pInst = new LPSTR;
		pInst[0] = szOldInst;
		pApp->m_cQ->UnSubscribeMarketData(pInst,1);
		dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
		if (dwRet==WAIT_OBJECT_0)
		{ ResetEvent(g_hEvent); }

	}
	
	UINT uLen = pDlg->m_SubList.size();
	char** pInsLst = new char*[uLen];
	for(UINT i=0; i<uLen;i++)  {pInsLst[i]=pDlg->m_SubList[i];}

	pApp->m_cQ->SubscribeMarketData(pInsLst,uLen);

	dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if(dwRet==WAIT_OBJECT_0){ ResetEvent(g_hEvent); }
	
	g_szInstOld = pDlg->m_szInst;

	pDlg->m_pSubMd =NULL;

	return 0;
}

void CXTraderDlg::OnBnClkChkLastP()
{
	m_bChkLastP = !m_bChkLastP;
}

////////////////////////////////////////////

void CXTraderDlg::OnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
	}


	*pResult = 0;
}

void CXTraderDlg::OnDblClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	int nItem = -1;
	int nSubItem = -1;

	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
	}


	*pResult = 0;
}

void CXTraderDlg::OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	int nItem = -1;
	int nSubItem = -1;

	if (pNMLV)
	{
		nItem = pNMLV->iItem;
		nSubItem = pNMLV->iSubItem;
	}



	*pResult = 0;
}

void CXTraderDlg::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	if (pNMLV)
	{
		nItem = pNMLV->iItem;
		nSubItem = pNMLV->iSubItem;
	}
	//TRACE(_T("in CXListCtrlTestDlg::OnItemChanged:  %d, %d\n"), nItem, nSubItem);

	if (pNMLV && (pNMLV->uNewState == (UINT)(LVIS_FOCUSED|LVIS_SELECTED)))
	{

		CString strText = m_LstOnRoad.GetItemText(nItem, nSubItem);
		//Log(_T("Selection changed to item %d"), nItem);
	}

	*pResult = 0;
}

void CXTraderDlg::InitTabs()
{
	m_TabOption.InsertItem( 0, _T("挂单F2") );
	m_TabOption.InsertItem( 1, _T("委托F3") );
	m_TabOption.InsertItem( 2, _T("持仓F4") );
	m_TabOption.InsertItem( 3, _T("成交F5") );
	m_TabOption.InsertItem( 4, _T("合约F6") );
}

///////////////////////////////////////////////

void CXTraderDlg::OnTabSelchange(NMHDR *pNMHDR, LRESULT *pResult)
{
	switch ( m_TabOption.GetCurSel() )
	{
		case 0:
			m_LstTdInf.ShowWindow( SW_HIDE );
			m_LstOrdInf.ShowWindow( SW_HIDE );
			m_LstInvPosInf.ShowWindow( SW_HIDE );
			m_LstAllInsts.ShowWindow( SW_HIDE );
			m_LstOnRoad.ShowWindow( SW_SHOW );
			if (!g_bInitNtOnce)
			{
				InitLstOnRoad();
			}
			break;
		case 1:
			m_LstOnRoad.ShowWindow( SW_HIDE );
			m_LstTdInf.ShowWindow( SW_HIDE );
			m_LstInvPosInf.ShowWindow( SW_HIDE );
			m_LstAllInsts.ShowWindow( SW_HIDE );
			m_LstOrdInf.ShowWindow( SW_SHOW );
			if (!g_bInitOrdOnce)
			{
				InitLstOrder();
			}
			break;
		case 2:	
			m_LstOnRoad.ShowWindow( SW_HIDE );
			m_LstTdInf.ShowWindow( SW_HIDE );
			m_LstOrdInf.ShowWindow( SW_HIDE );
			m_LstAllInsts.ShowWindow( SW_HIDE );
			m_LstInvPosInf.ShowWindow( SW_SHOW );
			if (!g_bInitPosOnce)
			{
				InitLstInvPos();
			}
			break;
		case 3:
			m_LstOnRoad.ShowWindow( SW_HIDE );
			m_LstOrdInf.ShowWindow( SW_HIDE );
			m_LstInvPosInf.ShowWindow( SW_HIDE );
			m_LstAllInsts.ShowWindow( SW_HIDE );
			m_LstTdInf.ShowWindow( SW_SHOW );
			if (!g_bInitTdOnce)
			{
				InitLstTrade();
			}
			break;
		case 4:
			m_LstOnRoad.ShowWindow( SW_HIDE );
			m_LstTdInf.ShowWindow( SW_HIDE );
			m_LstOrdInf.ShowWindow( SW_HIDE );
			m_LstInvPosInf.ShowWindow( SW_HIDE );
			m_LstAllInsts.ShowWindow( SW_SHOW );
			if (!g_bInitInstOnce)
			{
				InitLstInsts();
			}
			
			break;
	}

	*pResult = 0;
}

void CXTraderDlg::ShowNotifyIcon(CWnd* pWnd,UINT uCallbackMessage,CString sInfoTitle,CString sInfo,int uTimeout,DWORD dwMsg)
{
	ZeroMemory( &m_Notify, sizeof( NOTIFYICONDATA ) );
	
	m_Notify.cbSize		      = sizeof( NOTIFYICONDATA );
	m_Notify.hWnd			  = pWnd->GetSafeHwnd();
	m_Notify.uID			  = 0;
	
	m_Notify.uFlags		      = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_INFO;
	m_Notify.dwInfoFlags      = NIIF_INFO; // add an icon to a balloon ToolTip
	
	m_Notify.uCallbackMessage = uCallbackMessage;  
	m_Notify.uTimeout         = uTimeout * 1000;
	m_Notify.hIcon		   	  = (HICON)LoadImage(AfxGetInstanceHandle(),
		MAKEINTRESOURCE(IDR_MAINFRAME),IMAGE_ICON,16,16,NULL);
	
	_tcscpy( m_Notify.szInfoTitle, sInfoTitle);
	_tcscpy( m_Notify.szInfo, sInfo);
	_tcscpy( m_Notify.szTip, _T("xTrader运行中>>>"));
	
	Shell_NotifyIcon(dwMsg, &m_Notify ); // add to the taskbar's status area
}

LRESULT CXTraderDlg::SyncTimeMsg(WPARAM wParam,LPARAM lParam)
{
	if (!m_pSync)
	{
		m_pSync = AfxBeginThread((AFX_THREADPROC)SyncThread, this);
		//m_pSync = NULL;
	}
	
	return TRUE;
}


LRESULT CXTraderDlg::QryUserMsg(WPARAM wParam,LPARAM lParam)
{
	memcpy(m_pInvInf,(CThostFtdcInvestorField*)lParam,sizeof(CThostFtdcInvestorField));
	
	PopupPrivInf();

	return TRUE;
}

LRESULT CXTraderDlg::QryAccMsg(WPARAM wParam,LPARAM lParam)
{
	memcpy(m_pTdAcc,(CThostFtdcTradingAccountField*)lParam,sizeof(CThostFtdcTradingAccountField));
	PopupAccDlg();

	return TRUE;
}

UINT CXTraderDlg::OrderThread(LPVOID pParam)
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	CXTraderDlg* pDlg = (CXTraderDlg*)pParam;

	pApp->m_cT->m_bReconnect  = FALSE;
	////////////////////////////////////////////////////////
	pDlg->UpdateData(TRUE);
	TThostFtdcInstrumentIDType instId;
	TThostFtdcDirectionType dir;
	TThostFtdcCombOffsetFlagType kpp;
	
	uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)pDlg->m_szInst,instId);
	
	//0买 1卖
	int iDir= pDlg->m_CombBS.GetCurSel();
	if (iDir==0) dir=THOST_FTDC_D_Buy;
	if (iDir==1) dir=THOST_FTDC_D_Sell;
	//0开 1平
	int iOC = pDlg->m_CombOC.GetCurSel();
	if (iOC==0) kpp[0]=THOST_FTDC_OF_Open;
	if (iOC==1) kpp[0]=THOST_FTDC_OF_Close;
	if (iOC==2) kpp[0]=THOST_FTDC_OF_CloseToday;	
	
	int iMaxVol = pDlg->m_InstInf->MaxLimitOrderVolume;
	int iTimes = pDlg->m_OrderVol/iMaxVol;
	int iLeft = pDlg->m_OrderVol%iMaxVol;

	if ((pDlg->m_OrderVol)<(pDlg->m_InstInf->MinLimitOrderVolume))
	{
		pDlg->m_OrderVol=pDlg->m_InstInf->MinLimitOrderVolume;
	}

	if ((pDlg->m_pDepthMd->UpperLimitPrice)>(1.03*(pDlg->m_pDepthMd->LowerLimitPrice)))
	{	
		if (pDlg->m_OrderPrice > pDlg->m_pDepthMd->UpperLimitPrice)
		{ pDlg->m_OrderPrice = pDlg->m_pDepthMd->UpperLimitPrice; }
		if ( pDlg->m_OrderPrice < pDlg->m_pDepthMd->LowerLimitPrice)
		{ pDlg->m_OrderPrice = pDlg->m_pDepthMd->LowerLimitPrice; }
	}

	if (iTimes>=1)
	{
		//OpenMP语法 支持多核并行
		//#pragma omp parallel for
		for (int i=0;i<iTimes;i++)
		{
			pApp->m_cT->ReqOrdLimit(instId,dir,kpp,pDlg->m_OrderPrice,iMaxVol);
		}

	}
	if (iLeft>0)
	{
		pApp->m_cT->ReqOrdLimit(instId,dir,kpp,pDlg->m_OrderPrice,iLeft);

	}
	
	///////////////////////////////////////////////////////

	pDlg->m_pOrder = NULL;

	return 0;
}

void CXTraderDlg::OnBtOrder()
{

	if (!m_pOrder)
	{
		m_pOrder = AfxBeginThread((AFX_THREADPROC)OrderThread,this);
	}
	
}

UINT CXTraderDlg::QryAccount(LPVOID pParam)
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	CXTraderDlg* pDlg = (CXTraderDlg*) pParam;

	pApp->m_cT->ReqQryTdAcc();
	DWORD dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		ResetEvent(g_hEvent);
	}	
	else
	{
		ShowErroTips(_T("查询账户超时!"),MY_TIPS);
		return 0;		
	}
	
	pDlg->m_pQryAcc=NULL;

	return 0;
}

void CXTraderDlg::PopupAccDlg()
{
	CNoticeDlg* tdlg = new CNoticeDlg;
	tdlg->m_szTitle = ACC_DETAILS;
	
	BOOL res=tdlg->Create(IDD_DLG_NOTICE,NULL);
	VERIFY( res==TRUE );
	tdlg->CenterWindow();
	tdlg->ShowWindow(SW_SHOW);

	CString szCPft,szPPft,szTdFee;
	szTdFee.Format(_T("%d"),D2Int(m_pTdAcc->Commission)); outStrAs4(szTdFee);
	szCPft.Format(_T("%d"),D2Int(m_pTdAcc->CloseProfit)); outStrAs4(szCPft);
	szPPft.Format(_T("%d"),D2Int(m_pTdAcc->PositionProfit)); outStrAs4(szPPft);
	
	m_csCpProf.SetWindowText(szCPft,WHITE,DT_CENTER);
	m_csHpProf.SetWindowText(szPPft,WHITE,DT_CENTER);
	m_csTdFee.SetWindowText(szTdFee,WHITE,DT_CENTER);	
	
	CString szLine = _T(""),szTemp = _T("");

	szLine += FormatLine(_T(""),_T(""),_T("="),42);

	szTemp.Format(_T("%.2f"),m_pTdAcc->PreBalance); outStrAs4(szTemp);
	szLine += FormatLine(_T("  上次结算准备金:"),szTemp,_T(" "),40);

	szTemp.Format(_T("%.2f"),m_pTdAcc->PreCredit); outStrAs4(szTemp);
	szLine += FormatLine(_T("- 上次信用额度:"),szTemp,_T(" "),40);

	szTemp.Format(_T("%.2f"),m_pTdAcc->PreMortgage); outStrAs4(szTemp);
	szLine += FormatLine(_T("- 上次质押金额:"),szTemp,_T(" "),40);

	szTemp.Format(_T("%.2f"),m_pTdAcc->Mortgage); outStrAs4(szTemp);
	szLine += FormatLine(_T("+ 质押金额:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),m_pTdAcc->Withdraw); outStrAs4(szTemp);
	szLine += FormatLine(_T("- 今日出金:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),m_pTdAcc->Deposit); outStrAs4(szTemp);
	szLine += FormatLine(_T("+ 今日入金:"),szTemp,_T(" "),41);

	szLine += FormatLine(_T(""),_T(""),_T("-"),42);

	double dStatProf = m_pTdAcc->PreBalance-m_pTdAcc->PreCredit-m_pTdAcc->PreMortgage+m_pTdAcc->Mortgage-m_pTdAcc->Withdraw+m_pTdAcc->Deposit;
	szTemp.Format(_T("%.2f"),dStatProf); outStrAs4(szTemp);
	szLine += FormatLine(_T("= 静态权益:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),m_pTdAcc->CloseProfit); outStrAs4(szTemp);
	szLine += FormatLine(_T("+ 平仓盈亏:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),m_pTdAcc->PositionProfit); outStrAs4(szTemp);
	szLine += FormatLine(_T("+ 持仓盈亏:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),m_pTdAcc->Commission); outStrAs4(szTemp);
	szLine += FormatLine(_T("- 手续费:"),szTemp,_T(" "),42);

	szLine += FormatLine(_T(""),_T(""),_T("-"),42);

	double dDymProf = dStatProf+m_pTdAcc->CloseProfit+m_pTdAcc->PositionProfit-m_pTdAcc->Commission;
	szTemp.Format(_T("%.2f"),dDymProf); outStrAs4(szTemp);
	szLine += FormatLine(_T("= 动态权益:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),m_pTdAcc->CurrMargin); outStrAs4(szTemp);
	szLine += FormatLine(_T("- 占用保证金:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),m_pTdAcc->FrozenMargin); outStrAs4(szTemp);
	szLine += FormatLine(_T("- 冻结保证金:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),m_pTdAcc->FrozenCommission); outStrAs4(szTemp);
	szLine += FormatLine(_T("- 冻结手续费:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),m_pTdAcc->DeliveryMargin); outStrAs4(szTemp);
	szLine += FormatLine(_T("- 交割保证金:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),m_pTdAcc->Credit); outStrAs4(szTemp);
	szLine += FormatLine(_T("+ 信用金额:"),szTemp,_T(" "),41);

	szLine += FormatLine(_T(""),_T(""),_T("-"),42);

	double dValidProf = dDymProf-m_pTdAcc->CurrMargin-m_pTdAcc->FrozenMargin-m_pTdAcc->FrozenCommission-m_pTdAcc->DeliveryMargin+m_pTdAcc->Credit;
	szTemp.Format(_T("%.2f"),dValidProf); outStrAs4(szTemp);
	szLine += FormatLine(_T("= 可用金额:"),szTemp,_T(" "),41);

	szLine += FormatLine(_T(""),_T(""),_T("="),42);
	szLine += FormatLine(_T(""),_T(""),_T("="),42);

	szTemp.Format(_T("%.2f"),m_pTdAcc->Reserve); outStrAs4(szTemp);
	szLine += FormatLine(_T("  保底资金:"),szTemp,_T(" "),41);

	szTemp.Format(_T("%.2f"),m_pTdAcc->WithdrawQuota); outStrAs4(szTemp);
	szLine += FormatLine(_T("  可取资金:"),szTemp,_T(" "),41);

	szLine += FormatLine(_T(""),_T(""),_T("="),42);

	//tdlg->m_SzMsg = szLine;

	HWND hEdit = ::GetDlgItem(tdlg->m_hWnd,IDC_NTMSG);
	::SendMessage(hEdit,WM_SETTEXT, 0, (LPARAM)(LPCTSTR)szLine);

}

void CXTraderDlg::OnClkQryAcc()
{
	if (!m_pQryAcc)
	{
		m_pQryAcc = AfxBeginThread((AFX_THREADPROC)QryAccount, this);
	
	}

}

LRESULT CXTraderDlg::UpdateMdMsg(WPARAM wParam,LPARAM lParam)
{
	memcpy(m_pDepthMd,(CThostFtdcDepthMarketDataField*)lParam,sizeof(CThostFtdcDepthMarketDataField));

	if (!strcmp(m_pDepthMd->InstrumentID,((CXTraderApp*)AfxGetApp())->m_szInst))
	{
		UpdateMdPane();
	}
	

	return 0;
}

void CXTraderDlg::UpdateInvPos()
{

}

void CXTraderDlg::UpdateMdList()
{
	vector<char*>::iterator vecf;
	
	m_SubList.clear();
	for (UINT i=0;i<m_InvPosVec.size();i++)
	{
		vecf = find(m_SubList.begin(), m_SubList.end(), m_InvPosVec[i]->InstrumentID );
		if(vecf == m_SubList.end()) { m_SubList.push_back(m_InvPosVec[i]->InstrumentID);}
	}
	
	uni2ansi(CP_ACP,(LPTSTR)(LPCTSTR)m_szInst,((CXTraderApp*)AfxGetApp())->m_szInst);
	vecf = find(m_SubList.begin(), m_SubList.end(), (char*)((CXTraderApp*)AfxGetApp())->m_szInst );
	if(vecf == m_SubList.end()) { m_SubList.push_back((char*)((CXTraderApp*)AfxGetApp())->m_szInst);}
}

void CXTraderDlg::UpdateMdPane()
{
	 TCHAR szUpTm[40],szuni[20];
 
	 ansi2uni(CP_ACP,m_pDepthMd->UpdateTime,szuni);
 
	 _stprintf(szUpTm,_T("%s.%03d"),szuni,m_pDepthMd->UpdateMillisec);
	 m_csSUptime.SetWindowText(szUpTm);
 
	 double dPresp=m_pDepthMd->PreSettlementPrice;
	 double dUpD = m_pDepthMd->LastPrice-dPresp;
 
	 m_csS1P.SetDouble(m_pDepthMd->AskPrice1,CmpPriceColor(m_pDepthMd->AskPrice1,dPresp));
	 m_csS1V.SetInt(m_pDepthMd->AskVolume1,YELLOW);
 
	 if (m_pDepthMd->LastPrice>m_dOldPrice)
	 {
 		m_csLastV.SetWindowText(_T("↑"),RED);
	 }
	 else if (m_pDepthMd->LastPrice<m_dOldPrice)
	 {
 		m_csLastV.SetWindowText(_T("↓"),GREEN);
	 }
	 else
	 {
 		m_csLastV.SetWindowText(_T("―"),WHITE);
	 }
 
	 m_csLastP.SetDouble(m_pDepthMd->LastPrice,CmpPriceColor(m_pDepthMd->LastPrice,m_dOldPrice));
	 m_csB1P.SetDouble(m_pDepthMd->BidPrice1,CmpPriceColor(m_pDepthMd->AskPrice1,dPresp));
	 m_csB1V.SetInt(m_pDepthMd->BidVolume1,YELLOW);
 
 
	 m_csDUpDown.SetDouble(dUpD,CmpPriceColor(m_pDepthMd->LastPrice,dPresp));
	 m_csDHighest.SetDouble(m_pDepthMd->HighestPrice,CmpPriceColor(m_pDepthMd->HighestPrice,dPresp));
 
	 m_csDLowest.SetDouble(m_pDepthMd->LowestPrice,CmpPriceColor(m_pDepthMd->LowestPrice,dPresp));
	 m_csVTotal.SetInt(m_pDepthMd->Volume,YELLOW);
	 m_csVHold.SetDouble(m_pDepthMd->OpenInterest,YELLOW);
	 m_csDSmp.SetDouble(m_pDepthMd->PreSettlementPrice,CmpPriceColor(m_pDepthMd->PreSettlementPrice,dPresp));
	
	 m_csDOpt.SetDouble(m_pDepthMd->OpenPrice,CmpPriceColor(m_pDepthMd->OpenPrice,dPresp));
 
	 m_dOldPrice = m_pDepthMd->LastPrice;
 
	 m_SpinPrice.SetRangeAndDelta(m_pDepthMd->LowerLimitPrice, m_pDepthMd->UpperLimitPrice, m_InstInf->PriceTick);
	 if(m_bUpdateOp) {m_SpinPrice.SetPos((m_pDepthMd->LastPrice<1e-10)?m_InstInf->PriceTick:m_pDepthMd->LastPrice);}

	 if (m_bChkLastP)
	 {
 		m_OrderPrice = (m_pDepthMd->LastPrice<1e-10)?m_InstInf->PriceTick:m_pDepthMd->LastPrice;
 		UpdateData(FALSE);
	 }

	 m_bUpdateOp = FALSE;

}

void CXTraderDlg::Go2InstMd()
{
	if (IsValidInst(m_szInst))
	{
		double dPriceTick = m_InstInf->PriceTick;
		int iMaxVol = m_InstInf->MaxLimitOrderVolume;
		
		m_SpinVol.SetRangeAndDelta(1, iMaxVol, 1);
		m_SpinPrice.SetDecimalPlaces(JudgeDigit(dPriceTick));
		
		if (!m_pSubMd)
		{
			m_pSubMd = AfxBeginThread((AFX_THREADPROC)SubscribeMD, this);
		}
	}
}

void CXTraderDlg::OnSelchangeCombo() 
{
	m_bUpdateOp = TRUE;

	int iIndex = m_CombInst.GetCurSel();
	if (iIndex>=0)
	{
		m_CombInst.GetLBText(iIndex,m_szInst); 

		UpdateMdList();
	}

	Go2InstMd();
			
}


void CXTraderDlg::OnCbnEditchangeInst()
{
	m_bUpdateOp = TRUE;
	m_CombInst.GetWindowText(m_szInst); 	
	UpdateMdList();

	Go2InstMd();
	
}

void CXTraderDlg::OnTips()
{
	CNoticeDlg* tdlg = new CNoticeDlg;
	
	tdlg->m_szTitle = _QNA;
	HRSRC hRSRC = FindResource(NULL, (LPCTSTR)IDR_TIPS, _T("STUFF"));
	if (hRSRC) 
	{
		HGLOBAL hGlobal;
		if (hGlobal = LoadResource(NULL, hRSRC)) 
		{
			LPVOID lpData;
			
			if (lpData = LockResource(hGlobal)) 
			{
				tdlg->m_SzMsg = (LPCTSTR)lpData;
				FreeResource(hGlobal);
			}
			FreeResource(hGlobal);
		}
	}
	
	BOOL res=tdlg->Create(IDD_DLG_NOTICE,NULL);
	VERIFY( res==TRUE );
	tdlg->CenterWindow();
	tdlg->ShowWindow(SW_SHOW);

}

////////////////////////////////////////////////////////////////////////
void CXTraderDlg::OnModifyPass()
{
	DlgModPass* dlgMop = new DlgModPass;
	
	BOOL res=dlgMop->Create(IDD_DLG_MODPASS,NULL);
	VERIFY( res==TRUE );
	dlgMop->CenterWindow();
	dlgMop->ShowWindow(SW_SHOW);

}
/////////////////////////////////////////////////////////////////////
void CXTraderDlg::OnTransBf()
{
	BfTransfer* dlgBft = new BfTransfer;

	BOOL res=dlgBft->Create(IDD_BFTRANS,NULL);
	VERIFY( res==TRUE );
	dlgBft->CenterWindow();
	dlgBft->ShowWindow(SW_SHOW);
	
}

void CXTraderDlg::PopupBkAccDlg()
{
	/////////////////////////////////////////////////
	CNoticeDlg* tdlg = new CNoticeDlg;
	tdlg->m_szTitle = BKACC_LEFT;
	
	BOOL res=tdlg->Create(IDD_DLG_NOTICE,this);
	VERIFY( res==TRUE );
	tdlg->CenterWindow();
	tdlg->ShowWindow(SW_SHOW);
	
	CString szLine = _T(""),szTemp = _T("");
	
	szLine += FormatLine(_T(""),_T(""),_T("="),42);
	
	ansi2uni(CP_ACP,m_pNotifyBkYe->BankAccount,szTemp.GetBuffer(MAX_PATH));
	szTemp.ReleaseBuffer();
	szLine += FormatLine(_T("  银行账号:"),szTemp,_T(" "),40);
	
	szTemp.Format(_T("%.2f"),m_pNotifyBkYe->BankUseAmount); outStrAs4(szTemp);
	szLine += FormatLine(_T("  银行可用金额:"),szTemp,_T(" "),39);
	
	szTemp.Format(_T("%.2f"),m_pNotifyBkYe->BankFetchAmount); outStrAs4(szTemp);
	szLine += FormatLine(_T("  银行可取金额:"),szTemp,_T(" "),39);
	
	ansi2uni(CP_ACP,m_pNotifyBkYe->CurrencyID,szTemp.GetBuffer(MAX_PATH));
	szTemp.ReleaseBuffer();
	szLine += FormatLine(_T("  币种:"),szTemp,_T(" "),41);
	
	szLine += FormatLine(_T(""),_T(""),_T("="),42);
	
	HWND hEdit = ::GetDlgItem(tdlg->m_hWnd,IDC_NTMSG);
	::SendMessage(hEdit,WM_SETTEXT, 0, (LPARAM)(LPCTSTR)szLine);
}

LRESULT CXTraderDlg::QryBkYe(WPARAM wParam,LPARAM lParam)
{
	memcpy(m_pNotifyBkYe,(CThostFtdcNotifyQueryAccountField*)lParam,sizeof(CThostFtdcNotifyQueryAccountField));
	PopupBkAccDlg();

	return 0;
}

/////////////////////////////////////////////////////////////////////
int g_iIdx=-1;
//TThostFtdcOrderSysIDType g_OrdSysID;
void CXTraderDlg::OnNMRClkLstOnroad(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	int nItem = -1;
	int nSubItem = -1;

	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstOnRoad.GetItemCount();

		m_LstOnRoad.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		
		if (iCount>0)
		{
			m_szExpDef = GenDef(_T("挂单列表"),_T("csv"));

			CMenu PopMenu,*pSubMenu;
			CPoint pt;
			PopMenu.LoadMenu(IDR_LSTRMENU1);
			GetCursorPos(&pt);
			pSubMenu = PopMenu.GetSubMenu(0);	
			pSubMenu->TrackPopupMenu(TPM_LEFTALIGN,pt.x,pt.y,this);
		
			g_iIdx = iCount-1-nItem;
			
		}

	}

	*pResult = 0;
}


void CXTraderDlg::OnCancelOrd()
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();

	pApp->m_cT->ReqOrderCancel(m_onRoadVec[g_iIdx]->OrderSysID);

}


void CXTraderDlg::OnCancelAll()
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();

	for (UINT i=0;i<m_onRoadVec.size();i++)
	{
		pApp->m_cT->ReqOrderCancel(m_onRoadVec[i]->OrderSysID);
	
	}

}


void CXTraderDlg::OnModDsj()
{
	//CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();

}

void CXTraderDlg::OnCsvExport()
{
	CString  strFilter = _T("文本文件(*.csv;*.txt)|(*.*;*.csv;*.txt;)|所有文件 |*.*||");

	CFileDialog* dlgSave = new CFileDialog(false, _T("*.csv"),  m_szExpDef, OFN_PATHMUSTEXIST | OFN_EXPLORER, strFilter, this);
	dlgSave->m_ofn.lStructSize=sizeof(OPENFILENAME);		//use the 2k+ open file dialog

	CString szFile;
	if (IDOK == dlgSave->DoModal())
	{
		szFile = dlgSave->GetPathName();

		if (m_LstOnRoad.IsWindowVisible()) List2Csv(&m_LstOnRoad,(LPCTSTR)szFile);
		if (m_LstOrdInf.IsWindowVisible()) List2Csv(&m_LstOrdInf,(LPCTSTR)szFile);
		if (m_LstTdInf.IsWindowVisible()) List2Csv(&m_LstTdInf,(LPCTSTR)szFile);
		if (m_LstInvPosInf.IsWindowVisible()) List2Csv(&m_LstInvPosInf,(LPCTSTR)szFile);
		if (m_LstAllInsts.IsWindowVisible()) List2Csv(&m_LstAllInsts,(LPCTSTR)szFile);

	}


	DELX(dlgSave);
}

void CXTraderDlg::PopupPrivInf()
{
	//登陆时候已经查询过,现在直接读取
	CNoticeDlg* tdlg = new CNoticeDlg;
	tdlg->m_szTitle = ACC_FBINFO;
	
	BOOL res=tdlg->Create(IDD_DLG_NOTICE,NULL);
	VERIFY( res==TRUE );
	tdlg->CenterWindow();
	tdlg->ShowWindow(SW_SHOW);

	if (m_AccRegVec.size()>0)
	{
		CString szLine = _T("");
		TCHAR szTemp[1000];
		
		szLine += FormatLine(_T(""),_T(""),_T("="),48);
		
		ansi2uni(CP_ACP,m_AccRegVec[0]->TradeDay,szTemp);
		szLine += FormatLine(_T(" 交易日期:"),szTemp,_T(" "),46);
		
		szLine += FormatLine(_T(" 银行名称:"),JgBkName(m_AccRegVec[0]->BankID[0]),_T(" "),42);
		
		//ansi2uni(CP_ACP,m_AccRegVec[0]->BankBranchID,szTemp);
		//szLine += FormatLine(_T(" 分行编码:"),szTemp,_T(" "),46);
		
		ansi2uni(CP_ACP,m_AccRegVec[0]->BankAccount,szTemp);
		szLine += FormatLine(_T(" 银行账号:"),szTemp,_T(" "),46);
		
		//ansi2uni(CP_ACP,m_AccRegVec[0]->BrokerID,szTemp);
		szLine += FormatLine(_T(" 期货经纪:"),((CXTraderApp*)AfxGetApp())->GetBkrById(m_AccRegVec[0]->BrokerID),_T(" "),42);
		
		//ansi2uni(CP_ACP,m_AccRegVec[0]->BrokerBranchID,szTemp);
		//szLine += FormatLine(_T(" 分支经纪:"),szTemp,_T(" "),46);
		
		ansi2uni(CP_ACP,m_AccRegVec[0]->AccountID,szTemp);
		szLine += FormatLine(_T(" 期货账户:"),szTemp,_T(" "),46);
		
		szLine += FormatLine(_T(""),_T(""),_T("="),48);
		ansi2uni(CP_ACP,m_AccRegVec[0]->CustomerName,szTemp);
		szLine += FormatLine(_T(" 客户姓名:"),szTemp,_T(" "),44);

		_tcscpy(szTemp, (LPCTSTR)JgCardType(m_AccRegVec[0]->IdCardType));
		szLine += FormatLine(_T(" 证件类型:"),szTemp,_T(" "),43);
		
		ansi2uni(CP_ACP,m_AccRegVec[0]->IdentifiedCardNo,szTemp);
		szLine += FormatLine(_T(" 证件号码:"),szTemp,_T(" "),46);

		if (m_pInvInf->IsActive)
		{	_tcscpy(szTemp,_T("是"));}
		else
		{	_tcscpy(szTemp,_T("否"));}
		szLine += FormatLine(_T(" 是否活跃:"),szTemp,_T(" "),45);

		ansi2uni(CP_ACP,m_pInvInf->Mobile,szTemp);
		szLine += FormatLine(_T(" 联系电话:"),szTemp,_T(" "),46);

		ansi2uni(CP_ACP,m_pInvInf->Address,szTemp);
		szLine += FormatLine(_T(" 通讯地址:"),szTemp,_T(" "),31);

		szLine += FormatLine(_T(""),_T(""),_T("="),48);
		ansi2uni(CP_ACP,m_AccRegVec[0]->CurrencyID,szTemp);
		szLine += FormatLine(_T(" 币种代码:"),szTemp,_T(" "),46);
		
		if (m_AccRegVec[0]->OpenOrDestroy == THOST_FTDC_OOD_Open) _tcscpy(szTemp,_T("开户"));
		if (m_AccRegVec[0]->OpenOrDestroy == THOST_FTDC_OOD_Destroy) _tcscpy(szTemp,_T("销户"));
		szLine += FormatLine(_T(" 开销账户:"),szTemp,_T(" "),44);
		
		ansi2uni(CP_ACP,m_AccRegVec[0]->RegDate,szTemp);
		szLine += FormatLine(_T(" 签约日期:"),szTemp,_T(" "),46);

		ansi2uni(CP_ACP,m_AccRegVec[0]->OutDate,szTemp);
		if (!_tcscmp(szTemp,_T("")))
		{
			_tcscpy(szTemp,_T("——"));
		}
		szLine += FormatLine(_T(" 解约日期:"),szTemp,_T(" "),44);
		
		_stprintf(szTemp,_T("%d"),m_AccRegVec[0]->TID);
		szLine += FormatLine(_T(" 交易ID:"),szTemp,_T(" "),48);
		
		if (m_AccRegVec[0]->CustType == THOST_FTDC_CUSTT_Person) _tcscpy(szTemp,_T("自然人"));
		if (m_AccRegVec[0]->CustType == THOST_FTDC_CUSTT_Institution) _tcscpy(szTemp, _T("机构户"));
		szLine += FormatLine(_T(" 客户类型:"),szTemp,_T(" "),43);
		
		if (m_AccRegVec[0]->BankAccType == 0) _tcscpy(szTemp, _T("储蓄卡"));
		/*
		memset(szTemp,0,sizeof(szTemp));
		if (AccRegVec[0]->BankAccType == THOST_FTDC_BAT_BankBook) _tcscpy(szTemp,_T("银行存折"));
		if (AccRegVec[0]->BankAccType == THOST_FTDC_BAT_SavingCard) _tcscpy(szTemp, _T("储蓄卡"));
		if (AccRegVec[0]->BankAccType == THOST_FTDC_BAT_CreditCard) _tcscpy(szTemp,_T("信用卡"));
		*/
		szLine += FormatLine(_T(" 银行类型:"),szTemp,_T(" "),43);

		ansi2uni(CP_ACP,m_pInvInf->CommModelID,szTemp);
		szLine += FormatLine(_T(" 手续费率模板:"),szTemp,_T(" "),45);

		ansi2uni(CP_ACP,m_pInvInf->MarginModelID,szTemp);
		szLine += FormatLine(_T(" 保证金率模板:"),szTemp,_T(" "),45);
		
		szLine += FormatLine(_T(""),_T(""),_T("="),48);
		for (UINT i=0;i<m_TdCodeVec.size();i++)
		{
			ansi2uni(CP_ACP,m_TdCodeVec[i]->ClientID,szTemp);
			if (!strcmp(m_TdCodeVec[i]->ExchangeID, "DCE"))
			{
				szLine += FormatLine(_T(" 大商所交易编码:"), szTemp, _T(" "), 44);
			}
			else if (!strcmp(m_TdCodeVec[i]->ExchangeID,"SHFE"))
			{
				szLine += FormatLine(_T(" 上期所交易编码:"),szTemp,_T(" "),44);
			}
			else if(!strcmp(m_TdCodeVec[i]->ExchangeID,"CZCE"))
			{		
				szLine += FormatLine(_T(" 郑商所交易编码:"),szTemp,_T(" "),44);
			}
			else if(!strcmp(m_TdCodeVec[i]->ExchangeID, "CFFEX"))
			{
				szLine += FormatLine(_T(" 中金所交易编码:"), szTemp, _T(" "), 44);
			}
			else
			{
				szLine += FormatLine(_T(" 未知交易编码:"),szTemp,_T(" "),44);
			}
		}
			//_CZCE;_DCE;_SHFE;_CFFEX;
		
		szLine += FormatLine(_T(""),_T(""),_T("="),48);
		
		HWND hEdit = ::GetDlgItem(tdlg->m_hWnd,IDC_NTMSG);
		::SendMessage(hEdit,WM_SETTEXT, 0, (LPARAM)(LPCTSTR)szLine);
	}
	
}
void CXTraderDlg::OnUserInfo()
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();	

	pApp->m_cT->ReqQryInvestor();
	DWORD dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
	if (dwRet==WAIT_OBJECT_0)
	{
		ResetEvent(g_hEvent);
	}	

}

void CXTraderDlg::OnNMDblclkOnroad(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	*pResult = 0;
}


void CXTraderDlg::OnNMClkLstOnroad(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstOnRoad.GetItemCount();
		if (iCount)
		{
			m_LstOnRoad.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			m_szInst=m_LstOnRoad.GetItemText(nItem,1); //合约
			
			UpdateMdList();

			int iIndex = m_CombInst.FindStringExact(-1,m_szInst);
			m_CombInst.SetCurSel(iIndex);

			m_bUpdateOp = TRUE;
			Go2InstMd();
		}

	}
	*pResult = 0;
}

void CXTraderDlg::OnNMDblclkOrdInf(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstOrdInf.GetItemCount();
		
		m_LstOrdInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	}
	*pResult = 0;
}

void CXTraderDlg::OnNMClkLstOrdInf(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstOrdInf.GetItemCount();
		if (iCount)
		{
			m_LstOrdInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			m_szInst=m_LstOrdInf.GetItemText(nItem,1); //合约
			
			UpdateMdList();

			int iIndex = m_CombInst.FindStringExact(-1,m_szInst);
			m_CombInst.SetCurSel(iIndex);

			m_bUpdateOp = TRUE;
			Go2InstMd();
		}

	}
	*pResult = 0;
}
void CXTraderDlg::OnNMRClkLstOrdInf(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstOrdInf.GetItemCount();
		
		m_LstOrdInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);

		
		iCount = m_LstOrdInf.GetItemCount();
		
		if (iCount>0)
		{
			m_szExpDef = GenDef(_T("委托列表"),_T("csv"));

			CPoint pt;
			GetCursorPos(&pt);
			
			CMenu menu;
			menu.CreatePopupMenu();
			//	DWORD flags = (iCount>0) ?  : MF_GRAYED;
			menu.InsertMenu(0, MF_POPUP|MF_BYPOSITION, ID_CSV_EXPORT,_T("导出列表"));
			
			menu.InsertMenu(1, MF_BYPOSITION | MF_SEPARATOR);
			
			menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON |TPM_RIGHTBUTTON, pt.x, pt.y, this);
		}

	}
	*pResult = 0;
}

void CXTraderDlg::OnNMDblclkTdInf(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstTdInf.GetItemCount();
		
		m_LstTdInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);


	}
	*pResult = 0;
}
void CXTraderDlg::OnNMClkLstTdInf(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstTdInf.GetItemCount();
		
		if (iCount)
		{
			m_LstTdInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			m_szInst=m_LstTdInf.GetItemText(nItem,0); //合约
			
			UpdateMdList();

			int iIndex = m_CombInst.FindStringExact(-1,m_szInst);
			m_CombInst.SetCurSel(iIndex);

			m_bUpdateOp = TRUE;
			Go2InstMd();
		}
	}
	*pResult = 0;
}
void CXTraderDlg::OnNMRClkLstTdInf(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstTdInf.GetItemCount();
		
		m_LstTdInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);

		iCount = m_LstTdInf.GetItemCount();

		if (iCount>0)
		{
			m_szExpDef = GenDef(_T("成交列表"),_T("csv"));

			CPoint pt;
			GetCursorPos(&pt);
			
			CMenu menu;
			menu.CreatePopupMenu();
			//	DWORD flags = (iCount>0) ?  : MF_GRAYED;
			menu.InsertMenu(0, MF_POPUP|MF_BYPOSITION, ID_CSV_EXPORT,_T("导出列表"));
			
			menu.InsertMenu(1, MF_BYPOSITION | MF_SEPARATOR);
			
			menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON |TPM_RIGHTBUTTON, pt.x, pt.y, this);
		}

	}
	*pResult = 0;
}

void CXTraderDlg::OnNMDblclkInvPInf(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstInvPosInf.GetItemCount();
		
		m_LstInvPosInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	}
	*pResult = 0;
}
void CXTraderDlg::OnNMClkLstInvPInf(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstInvPosInf.GetItemCount();
		
		m_LstInvPosInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		if (iCount)
		{
			m_LstInvPosInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			m_szInst=m_LstInvPosInf.GetItemText(nItem,0); //合约
			
			UpdateMdList();

			int iIndex = m_CombInst.FindStringExact(-1,m_szInst);
			m_CombInst.SetCurSel(iIndex);

			m_bUpdateOp = TRUE;
			Go2InstMd();
		}
	}
	*pResult = 0;
}
void CXTraderDlg::OnNMRClkLstInvPInf(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstInvPosInf.GetItemCount();
		
		m_LstInvPosInf.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);

		
		iCount =m_LstInvPosInf.GetItemCount();
		if (iCount>0)
		{
			m_szExpDef = GenDef(_T("持仓列表"),_T("csv"));

			CPoint pt;
			GetCursorPos(&pt);
			
			CMenu menu;
			menu.CreatePopupMenu();
			//	DWORD flags = (iCount>0) ?  : MF_GRAYED;
			menu.InsertMenu(0, MF_POPUP|MF_BYPOSITION, ID_CSV_EXPORT,_T("导出列表"));
			
			menu.InsertMenu(1, MF_BYPOSITION | MF_SEPARATOR);
			
			menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON |TPM_RIGHTBUTTON, pt.x, pt.y, this);
		}

	}
	*pResult = 0;
}

void CXTraderDlg::OnNMClkLstInsts(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstAllInsts.GetItemCount();
		
		m_LstAllInsts.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		if (iCount)
		{
			m_LstAllInsts.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
			m_szInst=m_LstAllInsts.GetItemText(nItem,1); //合约	
			
			UpdateMdList();

			int iIndex = m_CombInst.FindStringExact(-1,m_szInst);
			m_CombInst.SetCurSel(iIndex);

			m_bUpdateOp = TRUE;
			Go2InstMd();

		}
	}
	*pResult = 0;
}

void CXTraderDlg::OnNMRClkLstInsts(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMIA = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	int nItem = -1;
	int nSubItem = -1;
	
	int iCount = -1;
	if (pNMIA)
	{
		nItem = pNMIA->iItem;
		nSubItem = pNMIA->iSubItem;
		iCount = m_LstAllInsts.GetItemCount();
		
		m_LstAllInsts.SetItemState(nItem, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
		
		
		iCount = m_LstAllInsts.GetItemCount();
		
		if (iCount>0)
		{
			m_szExpDef = GenDef(_T("合约列表"),_T("csv"));

			CPoint pt;
			GetCursorPos(&pt);
			
			CMenu menu;
			menu.CreatePopupMenu();
			//	DWORD flags = (iCount>0) ?  : MF_GRAYED;
			menu.InsertMenu(0, MF_POPUP|MF_BYPOSITION, ID_CSV_EXPORT,_T("导出列表"));
			
			menu.InsertMenu(1, MF_BYPOSITION | MF_SEPARATOR);
			
			menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON |TPM_RIGHTBUTTON, pt.x, pt.y, this);
		}

	}
	*pResult = 0;
}

void CXTraderDlg::OnCfmmc()
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	
	pApp->m_cT->ReqQryCFMMCTdAccKey();

}

BOOL CXTraderDlg::InitMgrFee()
{
	m_FeeRateVec.clear();
	m_MargRateVec.clear();

	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	char szMgr[MAX_PATH],szFee[MAX_PATH];

	//mgr和fee文件
	sprintf(szMgr,MGR_XML,pApp->m_sINVESTOR_ID);
	sprintf(szFee,FEE_XML,pApp->m_sINVESTOR_ID);
	
	xml_document doc;
	xml_node proot;
	xml_node nodeFees,nodeMgrs;
	xml_parse_result result;

	result = doc.load_file(szFee,parse_full);
	if (result.status!=status_ok) 
	{
		ShowErroTips(_T("载入fee文件错误,请在帮助->工具里面生成!"),MY_TIPS);
		return FALSE;
	}

	proot = doc.child("Fees");

	char InvRange[4];
	//注意所有同品种合约手续费一样!
	for (nodeFees=proot.first_child();nodeFees;nodeFees=nodeFees.next_sibling())
	{
		PFEERATE pFeeRate = new FEERATE;
		strcpy(pFeeRate->BrokerID, proot.attribute("BkrID").value());
		strcpy(pFeeRate->InvestorID, proot.attribute("InvID").value());
		strcpy(InvRange, proot.attribute("InvRange").value());
		pFeeRate->InvestorRange = InvRange[0];

		strcpy(pFeeRate->InstrumentID, nodeFees.attribute("PID").value());
		pFeeRate->OpenRatioByMoney = nodeFees.attribute("OpenByM").as_double();
		pFeeRate->OpenRatioByVolume = nodeFees.attribute("OpenByV").as_double();
		pFeeRate->CloseRatioByMoney = nodeFees.attribute("CloseByM").as_double();
		pFeeRate->CloseRatioByVolume = nodeFees.attribute("CloseByV").as_double();
		pFeeRate->CloseTodayRatioByMoney = nodeFees.attribute("ClosetByM").as_double();
		pFeeRate->CloseTodayRatioByVolume = nodeFees.attribute("ClosetByV").as_double();

		m_FeeRateVec.push_back(pFeeRate);
	}

	////////////////////////////////////////////////////////////////
	result = doc.load_file(szMgr,parse_full);
	if (result.status!=status_ok) 
	{
		ShowErroTips(_T("载入mgr文件错误,请在帮助->工具里面生成!"),MY_TIPS);
		return FALSE;
	}
	
	proot = doc.child("Mgrs");
	//注意同品种合约保证金可能不一样!
	for (nodeMgrs=proot.first_child();nodeMgrs;nodeMgrs=nodeMgrs.next_sibling())
	{
		PMGRATE pMgRate = new MGRATE;
		strcpy(pMgRate->BrokerID, proot.attribute("BkrID").value());
		strcpy(pMgRate->InvestorID, proot.attribute("InvID").value());
		strcpy(InvRange, proot.attribute("InvRange").value());
		pMgRate->InvestorRange = InvRange[0];
		strcpy(InvRange, proot.attribute("Flag").value());
		pMgRate->HedgeFlag = InvRange[0];
		
		strcpy(pMgRate->InstrumentID, nodeMgrs.attribute("InstID").value());
		pMgRate->LongMarginRatioByMoney = nodeMgrs.attribute("LongByM").as_double();
		pMgRate->LongMarginRatioByVolume = nodeMgrs.attribute("LongByV").as_double();
		pMgRate->ShortMarginRatioByMoney = nodeMgrs.attribute("ShortByM").as_double();
		pMgRate->ShortMarginRatioByVolume = nodeMgrs.attribute("ShortByV").as_double();
		pMgRate->IsRelative = 0;
		
		m_MargRateVec.push_back(pMgRate);
	}


/////////////////////////////////////////////////////////////////


	return TRUE;
}

void CXTraderDlg::OnGenMdFee()
{
	GenMfDlg* tdlg = new GenMfDlg;

	BOOL res=tdlg->Create(IDD_DLG_GENMRFEE,NULL);
	VERIFY( res==TRUE );
	tdlg->CenterWindow();
	tdlg->ShowWindow(SW_SHOW);
}


void CXTraderDlg::OnHiSettInf()
{
	DlgQryHiSet* tdlg = new DlgQryHiSet;

	BOOL res=tdlg->Create(IDD_DLG_HISETM,NULL);
	VERIFY( res==TRUE );
	tdlg->CenterWindow();
	tdlg->ShowWindow(SW_SHOW);
}


void CXTraderDlg::OnHkeySet()
{
	
}

void CXTraderDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	
	if(cx<= 10 || cy <= 10)   return; 
	
	CSize Translate(cx - m_OldSize.cx, 0);
	
	::EnumChildWindows(AfxGetMainWnd()->GetSafeHwnd(), CXTraderDlg::EnumProc, (LPARAM)&Translate);
	
	m_OldSize = CSize(cx,cy); 
	
}

BOOL CALLBACK CXTraderDlg::EnumProc(HWND hwnd, LPARAM lParam)
{
	CWnd* pWnd = CWnd::FromHandle(hwnd);
	CSize* pTranslate = (CSize*) lParam;

	CXTraderDlg* pDlg = (CXTraderDlg*) pWnd->GetParent();
	if (!pDlg) return FALSE;

	CRect rect;
	pWnd->GetWindowRect(rect);
	pDlg->ScreenToClient(rect);

	///################### enumerate window&control ##################

	if (hwnd == pDlg->m_LstOnRoad.GetSafeHwnd())
	{

		pDlg->m_LstOnRoad.MoveWindow(rect.left, rect.top, rect.Width()+pTranslate->cx, 
			rect.Height(), FALSE);

	}
	if (hwnd == pDlg->m_LstOrdInf.GetSafeHwnd())
	{
		
		pDlg->m_LstOrdInf.MoveWindow(rect.left, rect.top, rect.Width()+pTranslate->cx, 
			rect.Height(), FALSE);
		
	}
	if (hwnd == pDlg->m_LstInvPosInf.GetSafeHwnd())
	{
		
		pDlg->m_LstInvPosInf.MoveWindow(rect.left, rect.top, rect.Width()+pTranslate->cx, 
			rect.Height(), FALSE);
		
	}
	if (hwnd == pDlg->m_LstTdInf.GetSafeHwnd())
	{
		
		pDlg->m_LstTdInf.MoveWindow(rect.left, rect.top, rect.Width()+pTranslate->cx, 
			rect.Height(), FALSE);
		
	}

	if (hwnd == pDlg->m_LstAllInsts.GetSafeHwnd())
	{
		
		pDlg->m_LstAllInsts.MoveWindow(rect.left, rect.top, rect.Width()+pTranslate->cx, 
			rect.Height(), FALSE);
		
	}

	if (hwnd == pDlg->m_TabOption.GetSafeHwnd())
	{	
		pDlg->m_TabOption.MoveWindow(rect.left, rect.top, rect.Width()+pTranslate->cx, 
			rect.Height(), FALSE);
		
	}

 
	if (hwnd == pDlg->m_GroupAcc.GetSafeHwnd())
	{
			pDlg->m_GroupAcc.MoveWindow(rect.left, rect.top, 
			rect.Width()+pTranslate->cx, rect.Height(), FALSE);
	}

	if (hwnd == pDlg->GetDlgItem(IDC_BTQRYACC)->GetSafeHwnd())
	{
		((CButton*)pDlg->GetDlgItem(IDC_BTQRYACC))->MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}

	/////////////////////////////////////////////////////////////////////////

   	if (hwnd == pDlg->m_GroupMd.GetSafeHwnd())
	{
		
		pDlg->m_GroupMd.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}

	if (hwnd == pDlg->m_csS1P.GetSafeHwnd())
	{
		
		pDlg->m_csS1P.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}

	if (hwnd == pDlg->m_csS1V.GetSafeHwnd())
	{
		
		pDlg->m_csS1V.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}

	if (hwnd == pDlg->m_csSS1.GetSafeHwnd())
	{
		
		pDlg->m_csSS1.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csSB1.GetSafeHwnd())
	{
		
		pDlg->m_csSB1.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csB1V.GetSafeHwnd())
	{
		
		pDlg->m_csB1V.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csB1P.GetSafeHwnd())
	{
		
		pDlg->m_csB1P.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}

	if (hwnd == pDlg->m_csSLast.GetSafeHwnd())
	{
		
		pDlg->m_csSLast.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csLastP.GetSafeHwnd())
	{
		
		pDlg->m_csLastP.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csLastV.GetSafeHwnd())
	{
		
		pDlg->m_csLastV.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csB1P.GetSafeHwnd())
	{
		
		pDlg->m_csB1P.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csB1V.GetSafeHwnd())
	{
		
		pDlg->m_csB1V.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csSUpDown.GetSafeHwnd())
	{
		
		pDlg->m_csSUpDown.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csDUpDown.GetSafeHwnd())
	{
		
		pDlg->m_csDUpDown.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csSHighest.GetSafeHwnd())
	{
		
		pDlg->m_csSHighest.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csDHighest.GetSafeHwnd())
	{
		
		pDlg->m_csDHighest.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csSOpt.GetSafeHwnd())
	{
		
		pDlg->m_csSOpt.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}

	if (hwnd == pDlg->m_csDOpt.GetSafeHwnd())
	{
		
		pDlg->m_csDOpt.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csSLowest.GetSafeHwnd())
	{
		
		pDlg->m_csSLowest.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csDLowest.GetSafeHwnd())
	{
		
		pDlg->m_csDLowest.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}

	if (hwnd == pDlg->m_csSTotal.GetSafeHwnd())
	{
		
		pDlg->m_csSTotal.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csVTotal.GetSafeHwnd())
	{
		
		pDlg->m_csVTotal.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csSHold.GetSafeHwnd())
	{
		
		pDlg->m_csSHold.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csVHold.GetSafeHwnd())
	{
		
		pDlg->m_csVHold.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csSSmp.GetSafeHwnd())
	{
		
		pDlg->m_csSSmp.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csDSmp.GetSafeHwnd())
	{
		
		pDlg->m_csDSmp.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csSUptime.GetSafeHwnd())
	{
		
		pDlg->m_csSUptime.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_csSOpt.GetSafeHwnd())
	{
		
		pDlg->m_csSOpt.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	////################## enumerate statusbar ##################
	if (hwnd == pDlg->m_StatusBar.GetSafeHwnd())
	{
			pDlg->m_StatusBar.MoveWindow(rect.left, rect.top, 
			rect.Width()+pTranslate->cx, rect.Height(), FALSE);

	}

	//////////////////////////////////////////////////////////////////
	if (hwnd == pDlg->m_CombBS.GetSafeHwnd())
	{		
		pDlg->m_CombBS.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_CombOC.GetSafeHwnd())
	{
		
		pDlg->m_CombOC.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->m_CombInst.GetSafeHwnd())
	{
		
		pDlg->m_CombInst.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}

	if (hwnd == pDlg->m_SpinPrice.GetSafeHwnd())
	{
		
		pDlg->m_SpinPrice.MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}

	if (hwnd == pDlg->m_SpinVol.GetSafeHwnd())
	{
		
		pDlg->m_SpinVol.MoveWindow(rect.left+pTranslate->cx, rect.top, rect.Width(), 
			rect.Height(), FALSE);
	}

	if (hwnd == pDlg->GetDlgItem(IDC_BTORDER)->GetSafeHwnd())
	{
		((CButton*)pDlg->GetDlgItem(IDC_BTORDER))->MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}

	if (hwnd == pDlg->GetDlgItem(IDC_EDVOL)->GetSafeHwnd())
	{
		((CEdit*)pDlg->GetDlgItem(IDC_EDVOL))->MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(),rect.Height(), FALSE);
	}
	if (hwnd == pDlg->GetDlgItem(IDC_EDPRICE)->GetSafeHwnd())
	{
		((CEdit*)pDlg->GetDlgItem(IDC_EDPRICE))->MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}

	if (hwnd == pDlg->GetDlgItem(IDC_EDVOL)->GetSafeHwnd())
	{
		((CEdit*)pDlg->GetDlgItem(IDC_EDVOL))->MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	if (hwnd == pDlg->GetDlgItem(IDC_STAVOL)->GetSafeHwnd())
	{
		((CStatic*)pDlg->GetDlgItem(IDC_STAVOL))->MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}

	if (hwnd == pDlg->GetDlgItem(IDC_CHK_NEWP)->GetSafeHwnd())
	{
		((CButton*)pDlg->GetDlgItem(IDC_CHK_NEWP))->MoveWindow(rect.left+pTranslate->cx, rect.top, 
			rect.Width(), rect.Height(), FALSE);
	}
	///////////////////////////////////////////////////////////////////////////////////
	::InvalidateRect(hwnd,NULL,TRUE);
	pDlg->Invalidate();
	return TRUE;
}

void CXTraderDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)   
{   
    lpMMI->ptMinTrackSize.y = 359;  
	lpMMI->ptMaxTrackSize.y= 359;

	lpMMI->ptMinTrackSize.x = 498 ;
	lpMMI->ptMaxTrackSize.x = 1200;

    CDialog::OnGetMinMaxInfo(lpMMI);  
}  
