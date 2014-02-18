
class CTcpSocket
{
public:
	//Constructors / Destructors 
	CTcpSocket();
	~CTcpSocket();
	
	//host and port,"tcp://127.0.0.1:80"
	void GetDelayMs(LPCTSTR pszHnP,__int64& iMs);
	//General functions
	BOOL Create();
	BOOL Connect(LPCTSTR pszHostAddress, int nPort);
	int Send(LPCSTR pszBuf, int nBuf);
	int Receive(LPSTR pszBuf, int nBuf);
	void Close();
	BOOL IsReadible(BOOL& bReadible, DWORD dwTimeout);
	
protected:
	BOOL Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen);
	SOCKET m_hSocket;
};
