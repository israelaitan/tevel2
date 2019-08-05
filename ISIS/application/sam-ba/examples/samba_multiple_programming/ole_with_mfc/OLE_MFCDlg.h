// OLE_MFCDlg.h : header file
//

#if !defined(AFX_OLE_MFCDLG_H__E686D73D_1227_470D_ABAF_A92CEEC8D462__INCLUDED_)
#define AFX_OLE_MFCDLG_H__E686D73D_1227_470D_ABAF_A92CEEC8D462__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "samba_thread.h"

class COLE_MFCDlgAutoProxy;
class IAT91BootDLL;

/////////////////////////////////////////////////////////////////////////////
// COLE_MFCDlg dialog

class COLE_MFCDlg : public CDialog
{
    DECLARE_DYNAMIC(COLE_MFCDlg);
    friend class COLE_MFCDlgAutoProxy;

// Construction
public:
    COLE_MFCDlg(CWnd* pParent = NULL);    // standard constructor
    virtual ~COLE_MFCDlg();

    IAT91BootDLL *m_pAT91BootDLL;

// Dialog Data
    //{{AFX_DATA(COLE_MFCDlg)
    enum { IDD = IDD_OLE_MFC_DIALOG };
    CProgressCtrl*   m_ProgressCtrl;
    CProgressCtrl    m_ProgressCtrl_1;
    CProgressCtrl    m_ProgressCtrl_2;
    CProgressCtrl    m_ProgressCtrl_3;
    CProgressCtrl    m_ProgressCtrl_4;
    CProgressCtrl    m_ProgressCtrl_5;
    CProgressCtrl    m_ProgressCtrl_6;
    CProgressCtrl    m_ProgressCtrl_7;
    CProgressCtrl    m_ProgressCtrl_8;
    CListBox     m_CtrlListBox;
    SAMBA_THREAD_PARA        m_ThreadPara[8];
    static UINT __cdecl Thread_Program_Device( LPVOID pParam );
    static void CALLBACK EXPORT TimerProc(
       HWND hWnd,      // handle of CWnd that called SetTimer
       UINT nMsg,      // WM_TIMER
       UINT nIDEvent,  // timer identification
       DWORD dwTime    // system time
    );
    static void CALLBACK EXPORT TimerScan(
       HWND hWnd,      // handle of CWnd that called SetTimer
       UINT nMsg,      // WM_TIMER
       UINT nIDEvent,  // timer identification
       DWORD dwTime    // system time
    );
    long UpdateProgressBar(unsigned int eventID, long RecentlyLogFileSize);
    long ReadFileSize(CString cFileName);
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(COLE_MFCDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL
// Implementation
protected:
    
    COLE_MFCDlgAutoProxy* m_pAutoProxy;
    HICON m_hIcon;
    BOOL CanExit();
    // Generated message map functions
    //{{AFX_MSG(COLE_MFCDlg)
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnClose();
    virtual void OnOK();
    virtual void OnCancel();
//    afx_msg void OnProg1();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
public:
    long m_logfileSize;
    unsigned int iNumThread;
    unsigned int iPortList[8];
    unsigned int iNumConnection;
    bool autoMode;
    void ScanDevices();
    void AutoScan();
    void checkPlugIn();
    void checkPlugOut();
    void ProgThread(unsigned int flag, unsigned int port, unsigned int threadId);
    void On_Program();
    void On_Auto();
    void On_Manu();
    void On_Scan();
    void On_ProgramAll();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_OLE_MFCDLG_H__E686D73D_1227_470D_ABAF_A92CEEC8D462__INCLUDED_)
