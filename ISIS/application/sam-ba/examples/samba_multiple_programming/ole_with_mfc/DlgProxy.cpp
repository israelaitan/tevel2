// DlgProxy.cpp : implementation file
//

#include "stdafx.h"
#include "OLE_MFC.h"
#include "DlgProxy.h"
#include "OLE_MFCDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COLE_MFCDlgAutoProxy

IMPLEMENT_DYNCREATE(COLE_MFCDlgAutoProxy, CCmdTarget)

COLE_MFCDlgAutoProxy::COLE_MFCDlgAutoProxy()
{
	EnableAutomation();
	
	// To keep the application running as long as an automation 
	//	object is active, the constructor calls AfxOleLockApp.
	AfxOleLockApp();

	// Get access to the dialog through the application's
	//  main window pointer.  Set the proxy's internal pointer
	//  to point to the dialog, and set the dialog's back pointer to
	//  this proxy.
	ASSERT (AfxGetApp()->m_pMainWnd != NULL);
	ASSERT_VALID (AfxGetApp()->m_pMainWnd);
	ASSERT_KINDOF(COLE_MFCDlg, AfxGetApp()->m_pMainWnd);
	m_pDialog = (COLE_MFCDlg*) AfxGetApp()->m_pMainWnd;
	m_pDialog->m_pAutoProxy = this;
}

COLE_MFCDlgAutoProxy::~COLE_MFCDlgAutoProxy()
{
	// To terminate the application when all objects created with
	// 	with automation, the destructor calls AfxOleUnlockApp.
	//  Among other things, this will destroy the main dialog
	if (m_pDialog != NULL)
		m_pDialog->m_pAutoProxy = NULL;
	AfxOleUnlockApp();
}

void COLE_MFCDlgAutoProxy::OnFinalRelease()
{
	// When the last reference for an automation object is released
	// OnFinalRelease is called.  The base class will automatically
	// deletes the object.  Add additional cleanup required for your
	// object before calling the base class.

	CCmdTarget::OnFinalRelease();
}

BEGIN_MESSAGE_MAP(COLE_MFCDlgAutoProxy, CCmdTarget)
	//{{AFX_MSG_MAP(COLE_MFCDlgAutoProxy)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BEGIN_DISPATCH_MAP(COLE_MFCDlgAutoProxy, CCmdTarget)
	//{{AFX_DISPATCH_MAP(COLE_MFCDlgAutoProxy)
		// NOTE - the ClassWizard will add and remove mapping macros here.
	//}}AFX_DISPATCH_MAP
END_DISPATCH_MAP()

// Note: we add support for IID_IOLE_MFC to support typesafe binding
//  from VBA.  This IID must match the GUID that is attached to the 
//  dispinterface in the .ODL file.

// {52A1CB2F-AD32-4A75-8239-35E706ED16AB}
static const IID IID_IOLE_MFC =
{ 0x52a1cb2f, 0xad32, 0x4a75, { 0x82, 0x39, 0x35, 0xe7, 0x6, 0xed, 0x16, 0xab } };

BEGIN_INTERFACE_MAP(COLE_MFCDlgAutoProxy, CCmdTarget)
	INTERFACE_PART(COLE_MFCDlgAutoProxy, IID_IOLE_MFC, Dispatch)
END_INTERFACE_MAP()

// The IMPLEMENT_OLECREATE2 macro is defined in StdAfx.h of this project
// {59A1794A-9D7F-49DC-96AF-879BEC70992C}
IMPLEMENT_OLECREATE2(COLE_MFCDlgAutoProxy, "OLE_MFC.Application", 0x59a1794a, 0x9d7f, 0x49dc, 0x96, 0xaf, 0x87, 0x9b, 0xec, 0x70, 0x99, 0x2c)

/////////////////////////////////////////////////////////////////////////////
// COLE_MFCDlgAutoProxy message handlers
