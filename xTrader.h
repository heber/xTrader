// xTrader.h : main header file for the XTRADER application
//


#pragma once

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
#include "mdspi.h"
#include "traderspi.h"

/////////////////////////////////////////////////////////////////////////////
// CXTraderApp:
// See xTrader.cpp for the implementation of this class
//

class CXTraderApp : public CWinApp
{
public:
	CXTraderApp();
	~CXTraderApp();

public:
	CThostFtdcMdApi* m_MApi;
	CThostFtdcTraderApi* m_TApi;
	CtpMdSpi* m_cQ;
	CtpTraderSpi* m_cT;
	//int m_nReq;
	//char* m_szErr; 
	CString m_szTitle;
public:
	TThostFtdcBrokerIDType	m_sBROKER_ID;
	TThostFtdcInvestorIDType m_sINVESTOR_ID;
	TThostFtdcPasswordType  m_sPASSWORD;
	char m_szInst[MAX_PATH];
	CString m_szNtpSvr;
	int	m_ixPos,m_iyPos,m_iWidth,m_iHeight;
	vector<BKRPARA*> m_BkrParaVec;
public:
	void CreateApi();
	void ReleaseApi();
	void LoadConfig();
	void ParseXml(CString strDir);
	CString GetBkrById(TThostFtdcBrokerIDType szBkrId);
	BOOL AddSvr2Ar(CStringArray& szArTs,CStringArray& szArMd,CString szGrpName,int iIndex);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXTraderApp)
	public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CXTraderApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


