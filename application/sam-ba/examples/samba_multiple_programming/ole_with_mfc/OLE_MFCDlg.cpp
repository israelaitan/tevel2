// OLE_MFCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "at91boot_dll.h"
#include "OLE_MFC.h"
#include "OLE_MFCDlg.h"
#include "DlgProxy.h"

#define MAX_NUM_DEVICES            8
#define PROGRAM_TIMEOUT            100

#define DEV_PLUG_IN                0
#define DEV_PLUG_OUT               1
#define DEV_EXIST                  2

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// SAM-BA 2.12 applet contants
#include "applet.h"

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
    CAboutDlg();

// Dialog Data
    //{{AFX_DATA(CAboutDlg)
    enum { IDD = IDD_ABOUTBOX };
    //}}AFX_DATA

    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CAboutDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

// Implementation
protected:
    //{{AFX_MSG(CAboutDlg)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
    //{{AFX_DATA_INIT(CAboutDlg)
    //}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CAboutDlg)
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
    //{{AFX_MSG_MAP(CAboutDlg)
        // No message handlers
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COLE_MFCDlg dialog

IMPLEMENT_DYNAMIC(COLE_MFCDlg, CDialog);

COLE_MFCDlg::COLE_MFCDlg(CWnd* pParent /*=NULL*/)
    : CDialog(COLE_MFCDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(COLE_MFCDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
    // Note that LoadIcon does not require a subsequent DestroyIcon in Win32
    m_hIcon = AfxGetApp()->LoadIcon(IDI_SAMBA);
    m_pAutoProxy = NULL;
    // Create a new AT91BootDLL Object to manage COM Object
    m_pAT91BootDLL = new IAT91BootDLL;
    m_pAT91BootDLL->CreateDispatch(_T("SAMBA_DLL.SAMBADLL.1"));
}

COLE_MFCDlg::~COLE_MFCDlg()
{
    // If there is an automation proxy for this dialog, set
    //  its back pointer to this dialog to NULL, so it knows
    //  the dialog has been deleted.
    if (m_pAutoProxy != NULL)
        m_pAutoProxy->m_pDialog = NULL;
    delete m_pAT91BootDLL;
}

void COLE_MFCDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_PROGRESS1, m_ProgressCtrl_1);
    DDX_Control(pDX, IDC_PROGRESS2, m_ProgressCtrl_2);
    DDX_Control(pDX, IDC_PROGRESS3, m_ProgressCtrl_3);
    DDX_Control(pDX, IDC_PROGRESS4, m_ProgressCtrl_4);
    DDX_Control(pDX, IDC_PROGRESS5, m_ProgressCtrl_5);
    DDX_Control(pDX, IDC_PROGRESS6, m_ProgressCtrl_6);
    DDX_Control(pDX, IDC_PROGRESS7, m_ProgressCtrl_7);
    DDX_Control(pDX, IDC_PROGRESS8, m_ProgressCtrl_8);
    DDX_Control(pDX, IDC_LIST1, m_CtrlListBox);
    //{{AFX_DATA_MAP(COLE_MFCDlg)
        // NOTE: the ClassWizard will add DDX and DDV calls here
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COLE_MFCDlg, CDialog)
    //{{AFX_MSG_MAP(COLE_MFCDlg)
    ON_WM_SYSCOMMAND()
    ON_WM_CLOSE()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_COMMAND(IDC_AUTO, On_Auto)
    ON_COMMAND(IDC_MANU, On_Manu)
    ON_LBN_DBLCLK(IDC_LIST1, On_Program)
    ON_COMMAND(IDC_SCAN, On_Scan)
    ON_COMMAND(IDC_PROGRAMALL, On_ProgramAll)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COLE_MFCDlg message handlers

BOOL COLE_MFCDlg::OnInitDialog()
{
    unsigned int uiStartID_ProgressBar;
    unsigned int uiStartID_Status;
    unsigned int i;
    long detla;

    CDialog::OnInitDialog();
    // IDM_ABOUTBOX must be in the system command range.
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        CString strAboutMenu;
        strAboutMenu.LoadString(IDS_ABOUTBOX);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }
    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);         // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    m_logfileSize = ReadFileSize("template");
    if (!m_logfileSize) {
        MessageBox("Can't open file template.log !\n", NULL, MB_ICONERROR);
        OnCancel();
        return FALSE;
    }
    detla = (long(m_logfileSize /100 ) > 200) ? 200: (m_logfileSize /100 );
    m_logfileSize = m_logfileSize - detla;
    m_ProgressCtrl_1.SetRange(0, 100);
    m_ProgressCtrl_2.SetRange(0, 100);
    m_ProgressCtrl_3.SetRange(0, 100);
    m_ProgressCtrl_4.SetRange(0, 100);
    m_ProgressCtrl_5.SetRange(0, 100);
    m_ProgressCtrl_6.SetRange(0, 100);
    m_ProgressCtrl_7.SetRange(0, 100);
    m_ProgressCtrl_8.SetRange(0, 100);

    uiStartID_ProgressBar = IDC_PROGRESS1;
    uiStartID_Status = IDC_STATIC1;
    for ( i = 0; i < MAX_NUM_DEVICES; i++) {
        GetDlgItem(uiStartID_ProgressBar++)->ShowWindow(SW_HIDE);
        GetDlgItem(uiStartID_Status++)->ShowWindow(SW_HIDE);
    }
    GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_SCAN)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_PROGRAMALL)->ShowWindow(SW_HIDE);
    iNumConnection = 0;
    iNumThread = 0;
    for (i = 0; i < MAX_NUM_DEVICES; i++) {
            m_ThreadPara[i].iPortNo = 0;
            m_ThreadPara[i].plug_in = false;
            m_ThreadPara[i].plug_out = true;
            m_ThreadPara[i].is_running = false;
            m_ThreadPara[i].iPortNo = 0;
    }
    return TRUE;  // return TRUE  unless you set the focus to a control
}

void COLE_MFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialog::OnSysCommand(nID, lParam);
    }
}

void COLE_MFCDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting
        SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);
        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;
        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialog::OnPaint();
    }
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR COLE_MFCDlg::OnQueryDragIcon()
{
    return (HCURSOR) m_hIcon;
}

void COLE_MFCDlg::OnClose() 
{
  
    if (CanExit())
        CDialog::OnClose();
}

void COLE_MFCDlg::On_Auto()
{
    GetDlgItem(IDOK)->ShowWindow(SW_SHOW);
    GetDlgItem(IDC_SCAN)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_STATIC_DEV)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_PROGRAMALL)->ShowWindow(SW_HIDE);
    autoMode = true;
}

void COLE_MFCDlg::On_Manu()
{
   GetDlgItem(IDOK)->ShowWindow(SW_HIDE);
   GetDlgItem(IDC_SCAN)->ShowWindow(SW_SHOW);
   GetDlgItem(IDC_STATIC_DEV)->ShowWindow(SW_SHOW);
   autoMode = false;
}

void COLE_MFCDlg::OnOK() 
{
    SetTimer(0xff, 250, TimerScan);
    GetDlgItem(IDOK)->EnableWindow(FALSE);
    GetDlgItem(IDC_AUTO)->EnableWindow(FALSE);
    GetDlgItem(IDC_MANU)->EnableWindow(FALSE);
    GetDlgItem(IDC_SCAN)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_PROGRAMALL)->ShowWindow(SW_HIDE);
}

void COLE_MFCDlg::On_Scan() 
{
    unsigned int i;
    CString cDevice;
    CString csComName, csPortName;
    ScanDevices();
    m_CtrlListBox.ResetContent();
    for(i = 0; i < iNumConnection;  i++) {
        cDevice.Format("\\USBserial\\COM%d",iPortList[i]);
        m_CtrlListBox.AddString(cDevice);
    }
    if (iNumConnection) {
        GetDlgItem(IDC_PROGRAMALL)->ShowWindow(SW_SHOW);
        GetDlgItem(IDC_PROGRAMALL)->EnableWindow(TRUE);
    }
}

void COLE_MFCDlg::OnCancel() 
{
    if (CanExit())
        CDialog::OnCancel();
}

BOOL COLE_MFCDlg::CanExit()
{
    if (m_pAutoProxy != NULL) {
        ShowWindow(SW_HIDE);
        return FALSE;
    }
    return TRUE;
}

void COLE_MFCDlg::On_Program()
{
    CString selText;
    CString csComName, csStatesText;
    bool bPortFound = false;
    unsigned int i;
    unsigned int iComPort;

    iComPort = iPortList[m_CtrlListBox.GetCurSel()];
    for (i = 0; i < MAX_NUM_DEVICES; i++) {
        if (m_ThreadPara[i].iPortNo == iComPort) {
            bPortFound = true; break;
        }
    }
    if(!bPortFound) {
        if (iNumThread < (MAX_NUM_DEVICES - 1) ) ProgThread(DEV_PLUG_IN, iComPort, 0);
    } else {
        ProgThread(DEV_EXIST,iComPort, i);
    }
}

void COLE_MFCDlg::On_ProgramAll()
{
    checkPlugOut();
    checkPlugIn();
}

void COLE_MFCDlg::ScanDevices() 
{
    char* pDeviceList[MAX_NUM_DEVICES];
    unsigned int i;
    int iComPosition;
    CString Device;
    CString csComName, csPortName;

    for(i = 0; i < MAX_NUM_DEVICES;  i++) {
        pDeviceList[i] = (char*)malloc(128);
        memset(&pDeviceList[i][0], 0x00, 128);
    }
    iNumConnection = 0;
    m_pAT91BootDLL->AT91Boot_Scan((char *)pDeviceList); // Scan all devices connected
    for ( i = 0; i < MAX_NUM_DEVICES; i++) {
        if(pDeviceList[i] != NULL) {
            Device = pDeviceList[i];
            if((Device.Find("\\USBserial\\", 0) == 0) && iNumConnection < MAX_NUM_DEVICES) {
               iComPosition = Device.Find("COM" );
               csComName = Device.Mid( iComPosition , (Device.GetLength() - iComPosition + 2) );
               csPortName = csComName.Mid( 3, (csComName.GetLength() - 3) );
               iPortList[iNumConnection] = atoi(csPortName);
               iNumConnection++;
            }
        }
    }
    for(i = 0; i < MAX_NUM_DEVICES;  i++) {
        if(pDeviceList[i] != NULL) {
            free(pDeviceList[i]);     // Free memory
        }
    }
}

void COLE_MFCDlg::AutoScan() 
{
    unsigned int i;
    CString cDevice;
    CString csComName, csPortName;
    ScanDevices();
    m_CtrlListBox.ResetContent();
    for(i = 0; i < iNumConnection;  i++) {
        cDevice.Format("\\USBserial\\COM%d",iPortList[i]);
        m_CtrlListBox.AddString(cDevice);
    }
    checkPlugOut();
    checkPlugIn();
}

void COLE_MFCDlg::checkPlugOut() 
{
    unsigned int i, j;
    unsigned int iComPort;
    bool bPortFound;
    for (i = 0; i < MAX_NUM_DEVICES; i++) {
        bPortFound = false;
        iComPort = m_ThreadPara[i].iPortNo;
        if (iComPort) {
            for (j = 0; j < iNumConnection; j++) {
                if (iPortList[j] == iComPort) {
                    bPortFound = true;
                    break;
                }
            }
            if(!bPortFound) {
                ProgThread(DEV_PLUG_OUT, iComPort, i);
            }
        }
    }
}

void COLE_MFCDlg::checkPlugIn() 
{
    unsigned int i, j;
    unsigned int iComPort;
    bool bPortFound;
    for (i = 0; i < iNumConnection; i++) {
        iComPort = iPortList[i];
        bPortFound = false;
        for (j = 0; j < MAX_NUM_DEVICES; j++) {
            if (m_ThreadPara[j].iPortNo == iComPort) {
                bPortFound = true;
                break;
            }
        }
        if(!bPortFound) {
            if (iNumThread < (MAX_NUM_DEVICES ) ) ProgThread(DEV_PLUG_IN, iComPort, 0);
        } else {
            ProgThread(DEV_EXIST,iComPort, j);
        }
    }
}

void COLE_MFCDlg::ProgThread(unsigned int flag, unsigned int port, unsigned int threadId)
{
    unsigned int i;
    CString csStatesText;
    switch (flag) {
        case DEV_PLUG_IN:
            for (i = 0; i < MAX_NUM_DEVICES; i++) {
                if (m_ThreadPara[i].iPortNo == 0){
                    threadId = i; break;
                }
            }
            m_ThreadPara[threadId].plug_in = true;
            m_ThreadPara[threadId].plug_out = false;
            m_ThreadPara[threadId].is_running = true;
            m_ThreadPara[threadId].iPortNo = port;
            m_ThreadPara[threadId].This = this;
            m_ThreadPara[threadId].dwLogFileSize = 0;
            m_ThreadPara[threadId].iTimeout = 0;
            iNumThread++;
            break;
        case DEV_PLUG_OUT:
            if (m_ThreadPara[threadId].is_running) {
                return;
            } else {
                m_ThreadPara[threadId].plug_in = false;
                m_ThreadPara[threadId].plug_out = true;
                m_ThreadPara[threadId].dwLogFileSize = 0;
                m_ThreadPara[threadId].iTimeout = 0;
                m_ThreadPara[threadId].iPortNo = 0;
                GetDlgItem(IDC_PROGRESS1 + threadId)->ShowWindow(SW_HIDE);
                GetDlgItem(IDC_STATIC1 + threadId)->ShowWindow(SW_HIDE);
                iNumThread--;
                return;
            }
            break;
        case DEV_EXIST:
            if (((!m_ThreadPara[threadId].is_running) && m_ThreadPara[threadId].plug_out && autoMode) ||\
                ((!m_ThreadPara[threadId].is_running) && !autoMode)) {
                m_ThreadPara[threadId].plug_in = true;
                m_ThreadPara[threadId].plug_out = false;
                m_ThreadPara[threadId].dwLogFileSize = 0;
                m_ThreadPara[threadId].iTimeout = 0;
            } else {
                return;
            }
            break;
    }
    ((CProgressCtrl*)GetDlgItem(IDC_PROGRESS1+threadId))->SetPos(0);
    GetDlgItem(IDC_PROGRESS1+threadId)->ShowWindow(SW_SHOW);
    csStatesText.Format("programming COM%d ...",port ); 
    SetDlgItemText(IDC_STATIC1 + threadId, csStatesText);
    GetDlgItem(IDC_STATIC1 + threadId)->ShowWindow(SW_SHOW);
    m_ThreadPara[threadId].Timer_ID = SetTimer(threadId, 250, TimerProc);
    m_ThreadPara[threadId].pWinThreadConnect = AfxBeginThread(
                                               Thread_Program_Device,
                                               &m_ThreadPara[threadId],
                                               THREAD_PRIORITY_NORMAL,
                                               0,
                                               0, 
                                               NULL);
}

long COLE_MFCDlg::ReadFileSize(CString cFileName)
{
    WIN32_FIND_DATA FindFileData;
    HANDLE hFile;
    CString csFilePath;
    csFilePath += "bootfiles\\";
    csFilePath += cFileName;
    csFilePath += ".log";
    hFile = FindFirstFile(csFilePath, &FindFileData);
    if (hFile == INVALID_HANDLE_VALUE) 
    {
         return 0;
    } 
    return FindFileData.nFileSizeLow;
}

long COLE_MFCDlg::UpdateProgressBar(unsigned int eventID,
                                    long RecentlyLogFileSize)
{
    int iPosition = 0;
    unsigned int ProgressBarPercentID = 0;
    switch(eventID){
        case 0: m_ProgressCtrl = &m_ProgressCtrl_1; break;
        case 1: m_ProgressCtrl = &m_ProgressCtrl_2; break;
        case 2: m_ProgressCtrl = &m_ProgressCtrl_3; break;
        case 3: m_ProgressCtrl = &m_ProgressCtrl_4; break;
        case 4: m_ProgressCtrl = &m_ProgressCtrl_5; break;
        case 5: m_ProgressCtrl = &m_ProgressCtrl_6; break;
        case 6: m_ProgressCtrl = &m_ProgressCtrl_7; break;
        case 7: m_ProgressCtrl = &m_ProgressCtrl_8; break;
    }
    iPosition = (int)(RecentlyLogFileSize * 100 / m_logfileSize);
    m_ProgressCtrl->SetPos(iPosition);
    return 0;
}

void CALLBACK EXPORT COLE_MFCDlg::TimerScan(   HWND hWnd,      // handle of CWnd that called SetTimer
                                               UINT nMsg,      // WM_TIMER
                                               UINT nIDEvent,  // timer identification
                                               DWORD dwTime    // system time
                                           )
{
    COLE_MFCDlg* pDialog = ((COLE_MFCDlg*)((COLE_MFCApp*)AfxGetApp())->m_pMainWnd);
    pDialog->AutoScan();
}

void CALLBACK EXPORT COLE_MFCDlg::TimerProc(   HWND hWnd,      // handle of CWnd that called SetTimer
                                               UINT nMsg,      // WM_TIMER
                                               UINT nIDEvent,  // timer identification
                                               DWORD dwTime    // system time
                                           )
{
    long RecentlyLogFileSize;
    unsigned int uiStartID_Status, i;
    CString csComName, csStatesText, csCommand;

    uiStartID_Status = IDC_STATIC1 + nIDEvent ;
    COLE_MFCDlg* pDialog = ((COLE_MFCDlg*)((COLE_MFCApp*)AfxGetApp())->m_pMainWnd);
    csComName.Format("COM%d",pDialog->m_ThreadPara[nIDEvent].iPortNo);
    RecentlyLogFileSize = pDialog->ReadFileSize(csComName);
    pDialog->GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
    if (RecentlyLogFileSize > pDialog->m_ThreadPara[nIDEvent].dwLogFileSize) {
        pDialog->m_ThreadPara[nIDEvent].iTimeout = 0;
        pDialog->UpdateProgressBar(nIDEvent, RecentlyLogFileSize);
        pDialog->m_ThreadPara[nIDEvent].dwLogFileSize = RecentlyLogFileSize;
        if (RecentlyLogFileSize > (pDialog->m_logfileSize - 1)) {
            pDialog->KillTimer(nIDEvent);
            csStatesText.Format("%s Pass! ", csComName ); 
            pDialog->SetDlgItemText(uiStartID_Status, csStatesText);
            pDialog->m_ThreadPara[nIDEvent].is_running = false;
            for (i = 0; i < pDialog->iNumThread; i++) {
                if (pDialog->m_ThreadPara[i].is_running) return;
            }
            pDialog->GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
        }
    } else {
        pDialog->m_ThreadPara[nIDEvent].iTimeout = pDialog->m_ThreadPara[nIDEvent].iTimeout + 1;
        if (pDialog->m_ThreadPara[nIDEvent].iTimeout > PROGRAM_TIMEOUT) {
            pDialog->KillTimer(nIDEvent);
            csCommand.Format("bootfiles\\killtask.bat \\USBserial\\COM%d ", pDialog->m_ThreadPara[nIDEvent].iPortNo);
            WinExec(csCommand, SW_HIDE);
            pDialog->m_ThreadPara[nIDEvent].is_running = false;
            csStatesText.Format("Failed to program %s, task killed",csComName ); 
            pDialog->SetDlgItemText(uiStartID_Status, csStatesText);
        }
        for (i = 0; i < pDialog->iNumThread; i++) {
            if (pDialog->m_ThreadPara[i].is_running) return;
        }
        pDialog->GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
    }

}

UINT __cdecl COLE_MFCDlg::Thread_Program_Device( LPVOID pParam )
{
    SAMBA_THREAD_PARA* pThreadPara = (SAMBA_THREAD_PARA*)pParam;
    CString csCommand;
    csCommand.Format("bootfiles\\sam9gx5.bat \\USBserial\\COM%d COM%d.log", pThreadPara->iPortNo, pThreadPara->iPortNo);
    //AfxMessageBox(csCommand);
    WinExec(csCommand, SW_HIDE);
    return 0;
}
