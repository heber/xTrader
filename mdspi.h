#ifndef MD_SPI_H_
#define MD_SPI_H_

#include "StdAfx.h"

class CtpMdSpi : public CThostFtdcMdSpi
{
public:
 	CtpMdSpi(CThostFtdcMdApi* api):pUserApi(api){ m_iRequestID = 0;}
	///错误应答
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo,
		int nRequestID, bool bIsLast);

	virtual void OnFrontDisconnected(int nReason);
		
	///心跳超时警告。当长时间未收到报文时，该方法被调用。
	///@param nTimeLapse 距离上次接收报文的时间
	virtual void OnHeartBeatWarning(int nTimeLapse);

	///当客户端与交易后台建立起通信连接时（还未登录前），该方法被调用。
	virtual void OnFrontConnected();
	
	///登录请求响应
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,	CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///登出请求响应
	virtual void OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	///订阅行情应答
	virtual void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///取消订阅行情应答
	virtual void OnRspUnSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);

	///深度行情通知
	virtual void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *pDepthMarketData);

public:
	void ReqUserLogin(TThostFtdcBrokerIDType	appId);
	void ReqUserLogout();
	void SubscribeMarketData(char *ppInstrumentID[], int nCount);
	void UnSubscribeMarketData(char *ppInstrumentID[], int nCount);
	bool IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo);
public:
	int m_iRequestID;
	TThostFtdcBrokerIDType	m_sBkrID;
private:
  CThostFtdcMdApi* pUserApi;
};

#endif