
#include "stdafx.h"
#include "TcpSocket.h"
/////////////////////////

CTcpSocket::CTcpSocket()
{
	m_hSocket = INVALID_SOCKET; //default to an invalid scoket descriptor

}

CTcpSocket::~CTcpSocket()
{
	Close();
}

BOOL CTcpSocket::Create()
{
	// Uses TCP 
	m_hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_hSocket == INVALID_SOCKET) {return FALSE;}

	return TRUE;
}

void CTcpSocket::GetDelayMs(LPCTSTR pszHnP,__int64& iMs)
{
	CString szSvr;
	CString str(pszHnP);

	int iPos = str.ReverseFind(':');
	int iPos2 = str.Find(_T("//"));

	int iPort = _ttoi(str.Mid(iPos+1));
	szSvr = str.Mid(iPos2+2,iPos-iPos2-2);
	
	if (Create())
	{
		LARGE_INTEGER dwMs1,dwMs2;
		QueryPerformanceCounter(&dwMs1);

		BOOL bRes = Connect((LPCTSTR)szSvr, iPort);

		if (!bRes)
		{ 
			Close();
			iMs = -1;
		}
		else
		{
			QueryPerformanceCounter(&dwMs2);
			iMs = dwMs2.QuadPart-dwMs1.QuadPart;
			Close();
		}
	}

}

BOOL CTcpSocket::Connect(LPCTSTR pszHostAddress, int nPort)
{
	//For correct operation of the T2A macro, see MFC Tech Note 59
	USES_CONVERSION;
	
    //must have been created first
    ASSERT(m_hSocket != INVALID_SOCKET);
	
	LPSTR lpszAscii = T2A((LPTSTR)pszHostAddress);
	
	//Determine if the address is in dotted notation
	SOCKADDR_IN sockAddr;
	ZeroMemory(&sockAddr, sizeof(sockAddr));
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons((u_short)nPort);
	sockAddr.sin_addr.s_addr = inet_addr(lpszAscii);
	
	//If the address is not dotted notation, then do a DNS 
	//lookup of it.
	if (sockAddr.sin_addr.s_addr == INADDR_NONE)
	{
		LPHOSTENT lphost;
		lphost = gethostbyname(lpszAscii);
		if (lphost != NULL)
			sockAddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
		else
			return FALSE;
	}
	
	//Call the protected version which takes an address 
	//in the form of a standard C style struct.
	return Connect((SOCKADDR*)&sockAddr, sizeof(sockAddr));
}

BOOL CTcpSocket::Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen)
{
	int nConnect = connect(m_hSocket, lpSockAddr, nSockAddrLen);
    return (nConnect == 0);
}

int CTcpSocket::Send(LPCSTR pszBuf, int nBuf)
{
	//must have been created first
	ASSERT(m_hSocket != INVALID_SOCKET);
	
	int n=0,sendCount=0;
	int length = nBuf;
	if(pszBuf==NULL) return 0;
	while(length>0)
	{
     n = send(m_hSocket,pszBuf+sendCount,length,0);
     if(n == SOCKET_ERROR) { break; }
     
     length -= n;
     sendCount += n; 
	}

   return sendCount;
}

int CTcpSocket::Receive(LPSTR pszBuf, int nBuf)
{
	//must have been created first
	ASSERT(m_hSocket != INVALID_SOCKET);
	
	int nRev=0,recvCount=0;
	int length = nBuf;

	if(pszBuf==NULL) return 0;

	while(length>0)
	{
     nRev = recv(m_hSocket,pszBuf+recvCount,length,0);
     if(nRev==SOCKET_ERROR) { break; }
   
     length -= nRev;
     recvCount += nRev;
	}

  return recvCount;
}

void CTcpSocket::Close()
{
	if (m_hSocket != INVALID_SOCKET)
	{
		VERIFY(SOCKET_ERROR != closesocket(m_hSocket));
		m_hSocket = INVALID_SOCKET;
	}
}

BOOL CTcpSocket::IsReadible(BOOL& bReadible, DWORD dwTimeout)
{
	timeval timeout;
	timeout.tv_sec = dwTimeout / 1000;
	timeout.tv_usec = dwTimeout % 1000;
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(m_hSocket, &fds);
	int nStatus = select(0, &fds, NULL, NULL, &timeout);
	if (nStatus == SOCKET_ERROR)
		return FALSE;
	else
	{
		bReadible = !(nStatus == 0);
		return TRUE;
	}
}
