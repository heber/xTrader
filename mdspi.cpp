
#pragma once

#include "stdafx.h"
#include "mdspi.h"
#include "xTraderDlg.h"
#include "xTrader.h"

#pragma warning(disable : 4996)

extern HANDLE g_hEvent;
extern CWnd* g_pCWnd;
//extern BOOL g_bReconnect;

//BOOL bMdSignal = FALSE;
void CtpMdSpi::OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast)
{
  IsErrorRspInfo(pRspInfo);
}

void CtpMdSpi::OnFrontDisconnected(int nReason)
{
	if (g_pCWnd)
	{
		//bMdSignal = FALSE;
		CXTraderDlg* pDlg = (CXTraderDlg*)g_pCWnd;
		//pDlg->SetStatusTxt(_T("MD×"),0);
		pDlg->SetTipTxt( _T("行情断开"),IDS_MD_TIPS);
		pDlg->SetPaneTxtColor(0,BLUE);
	}

}
		
void CtpMdSpi::OnHeartBeatWarning(int nTimeLapse)
{
}

void CtpMdSpi::OnFrontConnected()
{
	CXTraderApp* pApp = (CXTraderApp*)AfxGetApp();
	if (g_pCWnd)
	{
		pApp->m_cT->m_bReconnect = TRUE;
		CXTraderDlg* pDlg = (CXTraderDlg*)g_pCWnd;
		
		ReqUserLogin(pApp->m_sBROKER_ID);
		DWORD dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
		if (dwRet==WAIT_OBJECT_0)
		{
			pDlg->SetTipTxt(_T("行情在线"),IDS_MD_TIPS);
			pDlg->SetPaneTxtColor(0,RED);
			ResetEvent(g_hEvent);
		}
		else
		{
			return;
		}

		LPSTR* pInst = new LPSTR;
		pInst[0] = pApp->m_szInst;
		SubscribeMarketData(pInst,1);
		dwRet = WaitForSingleObject(g_hEvent,WAIT_MS);
		if (dwRet==WAIT_OBJECT_0)
		{
			ResetEvent(g_hEvent);
		}
	}
	
	else
	{
		ReqUserLogin(pApp->m_sBROKER_ID);
	}
}

void CtpMdSpi::ReqUserLogin(TThostFtdcBrokerIDType	appId)
{
	CThostFtdcReqUserLoginField req;
	ZeroMemory(&req, sizeof(req));
	strcpy(req.BrokerID, appId);
	strcpy(m_sBkrID,appId);

	pUserApi->ReqUserLogin(&req, ++m_iRequestID);
}

void CtpMdSpi::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
		CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pRspUserLogin)
	{
	}
  if(bIsLast) SetEvent(g_hEvent);
}


void CtpMdSpi::ReqUserLogout()
{
	CThostFtdcUserLogoutField req;
	ZeroMemory(&req, sizeof(req));
	strcpy(req.BrokerID, m_sBkrID);
	
	pUserApi->ReqUserLogout(&req, ++m_iRequestID);
}

///登出请求响应
void CtpMdSpi::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if( !IsErrorRspInfo(pRspInfo) && pUserLogout)
	{
		
	}
	if(bIsLast) SetEvent(g_hEvent);
}

void CtpMdSpi::SubscribeMarketData(char *pInstId[], int nCount)
{
  pUserApi->SubscribeMarketData(pInstId, nCount); 
}

void CtpMdSpi::OnRspSubMarketData(
         CThostFtdcSpecificInstrumentField *pSpecificInstrument, 
         CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pSpecificInstrument)
	{
		//bMdSignal = TRUE;
	
	}
	else
	{
		//bMdSignal = FALSE;
	}
  if(bIsLast)  SetEvent(g_hEvent);
}

void CtpMdSpi::UnSubscribeMarketData(char *pInstId[], int nCount)
{
	pUserApi->UnSubscribeMarketData(pInstId, nCount); 
}

void CtpMdSpi::OnRspUnSubMarketData(
             CThostFtdcSpecificInstrumentField *pSpecificInstrument,
             CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
	if (!IsErrorRspInfo(pRspInfo) && pSpecificInstrument)
	{
		//bMdSignal = FALSE;
	}
	else
	{
		//bMdSignal = TRUE;
	}
  if(bIsLast)  SetEvent(g_hEvent);
}

void CtpMdSpi::OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData)
{
	if (g_pCWnd!=NULL)
	{
		CThostFtdcDepthMarketDataField *pDepthMd = new CThostFtdcDepthMarketDataField;
		memcpy(pDepthMd,pDepthMarketData,sizeof(CThostFtdcDepthMarketDataField));
		::PostMessage(g_pCWnd->m_hWnd,WM_UPDATEMD_MSG,0,(LPARAM)pDepthMd);
	}
}

bool CtpMdSpi::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{	
  bool ret = ((pRspInfo) && (pRspInfo->ErrorID != 0));
  return ret;
}