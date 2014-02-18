
#pragma once
// XPGroupBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CXPGroupBox window

class CXPGroupBox : public CButton
{
	DECLARE_DYNAMIC(CXPGroupBox);

// Construction
public:
	CXPGroupBox();

	enum XPGroupBoxStyle
		{ XPGB_FRAME,  
		  XPGB_WINDOW};

// Attributes
public:

// Operations
public:
   virtual CXPGroupBox&	SetBorderColor(COLORREF clrBorder);
   virtual CXPGroupBox&	SetCatptionTextColor(COLORREF clrText);
   virtual CXPGroupBox& SetBackgroundColor(COLORREF clrBKClient);
   virtual CXPGroupBox&	SetBackgroundColor(COLORREF clrBKTilte,  COLORREF clrBKClient);
   virtual CXPGroupBox&	SetXPGroupStyle(XPGroupBoxStyle eStyle); 
    
   virtual CXPGroupBox& SetText(LPCTSTR lpszTitle);
   virtual CXPGroupBox& SetFontBold(BOOL bBold);
   virtual CXPGroupBox& SetFontName(const CString& strFont, BYTE byCharSet = ANSI_CHARSET);
   virtual CXPGroupBox& SetFontUnderline(BOOL bSet);
   virtual CXPGroupBox& SetFontItalic(BOOL bSet);
   virtual CXPGroupBox& SetFontSize(int nSize);
   virtual CXPGroupBox& SetFont(LOGFONT lf);

   virtual CXPGroupBox& SetAlignment(DWORD dwType);


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXPGroupBox)
	public:
	virtual BOOL Create(LPCTSTR lpszClassName, LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, CCreateContext* pContext = NULL);
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CXPGroupBox();

	// Generated message map functions
protected:
	void UpdateSurface();
	void ReconstructFont();
	//{{AFX_MSG(CXPGroupBox)
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

private:
	CString		m_strTitle;

	COLORREF	m_clrBorder;
	COLORREF	m_clrTitleBackground;
	COLORREF	m_clrClientBackground;
	COLORREF	m_clrTitleText;
	
	XPGroupBoxStyle		m_nType;
	DWORD       m_dwAlignment;  

	LOGFONT			m_lf;
	CFont			m_font;

};

