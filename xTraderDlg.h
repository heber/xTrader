// xTraderDlg.h : header file
//

#pragma once

#include "global.h"

#ifndef IDD_XTRADER_DIALOG
#define IDD_XTRADER_DIALOG		102
#endif
/////////////////////////////////////////////////////////////////////////////
// CXTraderDlg dialog

class CXTraderDlg : public CDialog
{
// Construction
public:
	CXTraderDlg(CWnd* pParent = NULL);	// standard constructor
	~CXTraderDlg();
	enum{ IDD = IDD_XTRADER_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA
	static UINT SubscribeMD(LPVOID pParam);
	static UINT SyncThread(LPVOID pParam);
	static UINT QryAccount(LPVOID pParam);
	static UINT OrderThread(LPVOID pParam);
	static UINT InsInfThread(LPVOID pParam);
	static BOOL CALLBACK EnumProc(HWND hwnd, LPARAM lParam);
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXTraderDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual LRESULT DefWindowProc(UINT Msg,WPARAM wParam,LPARAM lParam);
	//}}AFX_VIRTUAL
public:
	void ShowNotifyIcon(CWnd* pWnd,UINT uCallbackMessage,CString sInfoTitle,CString sInfo,int uTimeout,DWORD dwMsg);
	void SaveConfig();
	void InitData();
	void CreateStatusBar();
	void SetStatusTxt(UINT uID, int nPane);
	void SetStatusTxt(LPCTSTR pMsg, int nPane);
	void SetTipTxt(LPCTSTR pMsg,int nTool);
	void SetPaneTxtColor(int nPane,COLORREF cr);
	BOOL IsValidInst(LPCTSTR lpszInst);
	void InitLstOnRoad();	//挂单
	void InitLstOrder();	//所有委托
	void InitLstTrade();	//已成交
	void InitLstInvPos();	//持仓
	void InitLstInsts();	//合约

	void InitAllHdrs();
	void InitAllVecs();
	void InitTabs();
	void FiltInsList();
	void AddCombInst();
	void Go2InstMd();
	BOOL InitMgrFee();
	void PopupAccDlg();
	void PopupPrivInf();
	void PopupBkAccDlg();
	void UpdateMdPane();
	void UpdateInvPos();
	void InitProfit();
	void InsertLstOnRoad(CThostFtdcOrderField* pOrder,int nIndex);
	void InsertLstOrder(CThostFtdcOrderField *pOrder,int nIndex,LPCTSTR lpErrMsg=NULL);
	void InsertLstTrade(CThostFtdcTradeField *pTrade,int nIndex);
	void InsertLstInvP(CThostFtdcInvestorPositionField *pInv);
	int FindOrdInOnRoadVec(TThostFtdcSequenceNoType BkrOrdSeq);
	int FindOrdInOnRoadLst(TThostFtdcOrderSysIDType OrdSysID);
	int FindInstMul(TThostFtdcInstrumentIDType InstID);

	void UpdateMdList();
	void ClearMemory();
	
protected:
	HICON m_hIcon;
	NOTIFYICONDATA m_Notify;		//托盘图标
	UINT		m_Timer;     // timer id
	UINT		m_uSync;

	// Generated message map functions
	//{{AFX_MSG(CXTraderDlg)
protected:
	afx_msg LRESULT SyncTimeMsg(WPARAM wParam,LPARAM lParam);
	////////////////////////////////////////////////////////////
	afx_msg LRESULT QryAccMsg(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT QryUserMsg(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT QryBkYe(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT UpdateMdMsg(WPARAM wParam,LPARAM lParam);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	////////////////////////////////////////////////////////////////
	afx_msg void OnSelchangeCombo();
	afx_msg void OnRestore();
	virtual BOOL OnInitDialog();
	afx_msg void OnMinimize();
	afx_msg void OnCbnEditchangeInst();
	afx_msg void OnChangeEditVol();
	afx_msg void OnChangeEditPrice();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnAbout();
	afx_msg void OnExit();
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnTrans();
	afx_msg void OnTips();
	afx_msg void OnUpdateTrans(CCmdUI *pCmdUI);
	afx_msg void OnViewTop();
	afx_msg void OnUpdateViewTop(CCmdUI *pCmdUI);
	///////////////下单///////////////
	afx_msg void OnBtOrder();
	afx_msg void OnBnClkChkLastP();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDblClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnColumnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	//afx_msg void OnBeginlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	//afx_msg void OnEndlabeledit(NMHDR* pNMHDR, LRESULT* pResult);
	//afx_msg LRESULT OnEditEnd(WPARAM, LPARAM);
	afx_msg void OnTabSelchange(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnTabSelchanging(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkOnroad(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClkLstOnroad(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClkLstOnroad(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnNMDblclkOrdInf(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClkLstOrdInf(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClkLstOrdInf(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnNMDblclkTdInf(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClkLstTdInf(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClkLstTdInf(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnNMDblclkInvPInf(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClkLstInvPInf(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClkLstInvPInf(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnNMClkLstInsts(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClkLstInsts(NMHDR *pNMHDR, LRESULT *pResult);
	
	afx_msg void OnCancelOrd();
	afx_msg void OnCancelAll();
	afx_msg void OnCsvExport();
	afx_msg void OnModDsj();

	afx_msg void OnDestroy();
	afx_msg void OnClkQryAcc();
	afx_msg void OnModifyPass();
	afx_msg void OnTransBf();
	afx_msg void OnUserInfo();
	afx_msg void OnCfmmc();
	afx_msg void OnGenMdFee();
	afx_msg void OnHiSettInf();
	afx_msg void OnHkeySet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	CWinThread *m_pSubMd;
	CWinThread *m_pSync;
	CWinThread *m_pQryAcc;
	CWinThread *m_pOrder;
	CWinThread *m_pInsInit;
	MMStatusBar m_StatusBar;
	CNumSpinCtrl m_SpinVol;
	CNumSpinCtrl m_SpinPrice;
	CComboBox	m_CombBS;
	CComboEx	m_CombInst;
	CXPGroupBox m_GroupMd;
	CXPGroupBox m_GroupAcc;
	CXPGroupBox m_GroupExec;
	CColorStatic m_csCpProf;	//平仓盈亏
	CColorStatic m_csHpProf;	//持仓盈亏
	CColorStatic m_csTdFee;		//手续费
	CComboBox	m_CombOC;
	CColorStatic m_csS1P;
	CColorStatic m_csS1V;
	CColorStatic m_csLastP;
	CColorStatic m_csLastV;
	CColorStatic m_csB1P;
	CColorStatic m_csB1V;
	CString		m_szInst;
	CColorStatic m_csSUpDown;
	CColorStatic m_csDUpDown;
	CColorStatic m_csSHighest;
	CColorStatic m_csDHighest;
	CColorStatic m_csSOpt;
	CColorStatic m_csDOpt;
	CColorStatic m_csSLowest;
	CColorStatic m_csDLowest;
	CColorStatic m_csSTotal;
	CColorStatic m_csVTotal;
	CColorStatic m_csSHold;
	CColorStatic m_csVHold;
	CColorStatic m_csSSmp;
	CColorStatic m_csDSmp;
	CColorStatic m_csSUptime;
	CColorStatic m_csSS1;
	CColorStatic m_csSLast;
	CColorStatic m_csSB1;
	CXListCtrl	m_LstOnRoad;
	CXListCtrl	m_LstOrdInf;
	CXListCtrl	m_LstTdInf;
	CXListCtrl	m_LstInvPosInf;
	CXListCtrl	m_LstAllInsts;

public:
	bool		m_bTrans;
	BOOL		m_bUpdateOp;
	UINT		m_iAlpha;
	bool		m_bTop;
	int			m_OrderVol;
	double		m_OrderPrice;
	double		m_dOldPrice;
	BOOL		m_bChkLastP;
	CThostFtdcInstrumentField *m_InstInf;
	CThostFtdcDepthMarketDataField *m_pDepthMd;
	CThostFtdcTradingAccountField *m_pTdAcc;
	CThostFtdcNotifyQueryAccountField *m_pNotifyBkYe;
	CThostFtdcInvestorField *m_pInvInf;
	CLINFO	*m_pCliInfo;

	CTabCtrl m_TabOption;
	CString m_sDefServer;
	CString m_szExpDef;
	CSize	m_OldSize;
	int	m_ixPos,m_iyPos,m_iWidth,m_iHeight;
	////////////////////////////////////////////
	vector<char*> m_SubList;
	vector<CThostFtdcOrderField*> m_orderVec;
	vector<CThostFtdcTradeField*> m_tradeVec;
	vector<CThostFtdcInstrumentFieldEx*> m_InsinfVec;
	vector<CThostFtdcInstrumentMarginRateField*> m_MargRateVec;
	vector<CThostFtdcAccountregisterField*> m_AccRegVec;
	vector<CThostFtdcTradingCodeField*> m_TdCodeVec;
	vector<CThostFtdcInvestorPositionField*> m_InvPosVec;
	vector<CThostFtdcInstrumentCommissionRateField*> m_FeeRateVec;
	vector<CThostFtdcOrderField*> m_onRoadVec;
	////////////////////////////////////////////
	CThostFtdcTradingAccountField m_TdAcc;
	CTimeSpan m_tsEXnLocal[4];
};
