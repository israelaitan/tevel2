// OLE_MFC.h : main header file for the OLE_MFC application
//

#if !defined(AFX_OLE_MFC_H__A8E481A9_254D_472A_A699_6E33AFC76279__INCLUDED_)
#define AFX_OLE_MFC_H__A8E481A9_254D_472A_A699_6E33AFC76279__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// COLE_MFCApp:
// See OLE_MFC.cpp for the implementation of this class
//

class COLE_MFCApp : public CWinApp
{
public:
	COLE_MFCApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(COLE_MFCApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(COLE_MFCApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OLE_MFC_H__A8E481A9_254D_472A_A699_6E33AFC76279__INCLUDED_)
