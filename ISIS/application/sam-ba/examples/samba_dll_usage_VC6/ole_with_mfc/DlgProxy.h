// DlgProxy.h : header file
//

#if !defined(AFX_DLGPROXY_H__63409168_658E_4F8C_98CB_643F8AF1DF79__INCLUDED_)
#define AFX_DLGPROXY_H__63409168_658E_4F8C_98CB_643F8AF1DF79__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class COLE_MFCDlg;

/////////////////////////////////////////////////////////////////////////////
// COLE_MFCDlgAutoProxy command target

class COLE_MFCDlgAutoProxy : public CCmdTarget
{
	DECLARE_DYNCREATE(COLE_MFCDlgAutoProxy)

	COLE_MFCDlgAutoProxy();           // protected constructor used by dynamic creation

// Attributes
public:
	COLE_MFCDlg* m_pDialog;

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COLE_MFCDlgAutoProxy)
	public:
	virtual void OnFinalRelease();
	//}}AFX_VIRTUAL

// Implementation
protected:
	virtual ~COLE_MFCDlgAutoProxy();

	// Generated message map functions
	//{{AFX_MSG(COLE_MFCDlgAutoProxy)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	DECLARE_OLECREATE(COLE_MFCDlgAutoProxy)

	// Generated OLE dispatch map functions
	//{{AFX_DISPATCH(COLE_MFCDlgAutoProxy)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_DISPATCH
	DECLARE_DISPATCH_MAP()
	DECLARE_INTERFACE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DLGPROXY_H__63409168_658E_4F8C_98CB_643F8AF1DF79__INCLUDED_)
