
#pragma once

struct CNtpTimePacket
{
  DWORD m_dwInteger;
	DWORD m_dwFractional;
};
struct NtpBasicInfo
{
	BYTE m_LiVnMode;
	BYTE m_Stratum;
	char m_Poll;
	char m_Precision;
	long m_RootDelay;
	long m_RootDispersion;
	char m_ReferenceID[4];
	CNtpTimePacket m_ReferenceTimestamp;
	CNtpTimePacket m_OriginateTimestamp;
	CNtpTimePacket m_ReceiveTimestamp;
	CNtpTimePacket m_TransmitTimestamp;
};

//The optional part of an NTP packet
struct NtpAuthenticationInfo
{
	unsigned long m_KeyID;
	BYTE m_MessageDigest[16];
};

//The Full NTP packet
struct NtpFullPacket
{
	NtpBasicInfo          m_Basic;
	NtpAuthenticationInfo m_Auth;
};

//Helper class to encapulate NTP time stamps
class CNtpTime
{
public:
	//Constructors / Destructors
	CNtpTime();
	CNtpTime(const CNtpTime& time);
	CNtpTime(CNtpTimePacket& packet);
	CNtpTime(const SYSTEMTIME& st);
	
	//General functions
	CNtpTime& operator=(const CNtpTime& time);
	double operator-(const CNtpTime& time) const;
	CNtpTime operator+(const double& timespan) const;
	operator SYSTEMTIME() const;
	operator CNtpTimePacket() const;
	operator unsigned __int64() const { return m_Time; };
	DWORD Seconds() const;
	DWORD Fraction() const;
	
	//Static functions
	static CNtpTime GetCurrentTime();
	static DWORD MsToNtpFraction(WORD wMilliSeconds);
	static WORD NtpFractionToMs(DWORD dwFraction);
	static double NtpFractionToSecond(DWORD dwFraction);
	
protected:
	//Internal static functions and data
	static long GetJulianDay(WORD Year, WORD Month, WORD Day);
	static void GetGregorianDate(long JD, WORD& Year, WORD& Month, WORD& Day);
	static DWORD m_MsToNTP[1000];
	
	//The actual data
	unsigned __int64 m_Time;
};

struct NtpServerResponse
{
	int m_nLeapIndicator; //0: no warning
	//1: last minute in day has 61 seconds
	//2: last minute has 59 seconds
	//3: clock not synchronized
	
	int m_nStratum; //0: unspecified or unavailable
	//1: primary reference (e.g., radio clock)
	//2-15: secondary reference (via NTP or SNTP)
	//16-255: reserved
	
	CNtpTime     m_OriginateTime;    //Time when the request was sent from the client to the SNTP server
	CNtpTime     m_ReceiveTime;      //Time when the request was received by the server
	CNtpTime     m_TransmitTime;     //Time when the server sent the request back to the client
	CNtpTime     m_DestinationTime;  //Time when the reply was received by the client
	double       m_RoundTripDelay;   //Round trip time in seconds
	double       m_LocalClockOffset; //Local clock offset relative to the server
};

//The actual SNTP class
class CSNTPClient : public CObject
{
public:
	//Constructors / Destructors
	CSNTPClient();
	
	//General functions
	BOOL  GetServerTime(LPCTSTR pszHostName, NtpServerResponse& response, int nPort = 123);
	DWORD GetTimeout() const { return m_dwTimeout; };
	void  SetTimeout(DWORD dwTimeout) { m_dwTimeout = dwTimeout; };
	BOOL  SetClientTime(const CNtpTime& NewTime);
	
protected:
	BOOL EnableSetTimePriviledge();
	void RevertSetTimePriviledge();
	
	DWORD            m_dwTimeout;
	HANDLE           m_hToken;
	TOKEN_PRIVILEGES m_TokenPriv;
	BOOL             m_bTakenPriviledge;
};

class CNtpSocket
{
public:
	//Constructors / Destructors 
	CNtpSocket();
	~CNtpSocket();
	
	//General functions
	BOOL Create();
	BOOL Connect(LPCTSTR pszHostAddress, int nPort);
	BOOL Send(LPCSTR pszBuf, int nBuf);
	int Receive(LPSTR pszBuf, int nBuf);
	void Close();
	BOOL IsReadible(BOOL& bReadible, DWORD dwTimeout);
	
protected:
	BOOL Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen);
	SOCKET m_hSocket;
};
