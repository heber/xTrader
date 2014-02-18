#pragma once
/////////////////////////////////////////////////////////////////////////////
// MyCombo window

class MyCombo : public CComboBox
{
// Construction
public:
	MyCombo();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(MyCombo)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~MyCombo();

	// Generated message map functions
protected:
	//{{AFX_MSG(MyCombo)
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};
