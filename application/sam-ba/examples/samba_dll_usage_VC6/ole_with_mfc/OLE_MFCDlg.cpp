/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2012, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */
 
// OLE_MFCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "at91boot_dll.h"
#include "OLE_MFC.h"
#include "OLE_MFCDlg.h"
#include "DlgProxy.h"

#include "FCError.h"

#define	DBGU_CIDR_MASK ~0x000F001F	// Remove version and SRAMSIZ (bug DBGU_CIDR S128 et S256 rev C)


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// SAM-BA 2.10 applet contants
#include "applet.h"

// values 
// Address in SRAM
#define APPLET_ADDR               0x300000
#define MAILBOX_ADDR              0x300004
      
#define APPLET_CMD_ADDR           (MAILBOX_ADDR)
#define APPLET_STATUS_ADDR        (MAILBOX_ADDR + 4)
#define APPLET_ARGV0_ADDR         (MAILBOX_ADDR + 0x08)
#define APPLET_ARGV1_ADDR         (MAILBOX_ADDR + 0x0c)
#define APPLET_ARGV2_ADDR         (MAILBOX_ADDR + 0x10)
#define APPLET_ARGV3_ADDR         (MAILBOX_ADDR + 0x14)
#define APPLET_ARGV4_ADDR         (MAILBOX_ADDR + 0x18)
#define APPLET_ARGV5_ADDR         (MAILBOX_ADDR + 0x1c)

// Address in SDRAM/DDRAM     
#define APPLET_ADDR_DRAM          0x20000000
#define MAILBOX_ADDR_DRAM         0x20000004
      
#define APPLET_CMD_ADDR_DRAM      (MAILBOX_ADDR_DRAM)
#define APPLET_STATUS_ADDR_DRAM   (MAILBOX_ADDR_DRAM + 4)
#define APPLET_ARGV0_ADDR_DRAM    (MAILBOX_ADDR_DRAM + 0x08)
#define APPLET_ARGV1_ADDR_DRAM    (MAILBOX_ADDR_DRAM + 0x0c)
#define APPLET_ARGV2_ADDR_DRAM    (MAILBOX_ADDR_DRAM + 0x10)
#define APPLET_ARGV3_ADDR_DRAM    (MAILBOX_ADDR_DRAM + 0x14)
#define APPLET_ARGV4_ADDR_DRAM    (MAILBOX_ADDR_DRAM + 0x18)
#define APPLET_ARGV5_ADDR_DRAM    (MAILBOX_ADDR_DRAM + 0x1c)

#define APPLET_MEM_SIZE           (APPLET_ARGV0_ADDR_DRAM)
#define APPLET_BUFFER_ADDR        (APPLET_ARGV1_ADDR_DRAM)
#define APPLET_BUFFER_SIZE        (APPLET_ARGV2_ADDR_DRAM)

#define COM_TYPE_USB  0
#define COM_TYPE_DBGU 1
#define COM_TYPE_JTAG 2

#define LOWLEVELINIT_APPLET_PATH  "..\\..\\..\\tcl_lib\\at91sam9g15-ek\\applet-lowlevelinit-sam9g15.bin"
#define EXTRAM_APPLET_PATH        "..\\..\\..\\tcl_lib\\at91sam9g15-ek\\applet-extram-sam9g15.bin"
#define SERIALFLASH_APPLET_PATH   "..\\..\\..\\tcl_lib\\at91sam9g15-ek\\applet-serialflash-sam9g15.bin"

#define EK_MODE        0

// Vdd Memory 1.8V = 0 / Vdd Memory 3.3V = 1
#define EXTRAM_VDD 0
// External SDRAM = 0 / External DDR = 1
#define EXTRAM_TYPE 1
// Set bus width (16 or 32)
#define EXTRAM_DATA_BUSWIDTH 16
// DDRAM Model (not used)
#define EXTRAM_MODEL 0

#define SERIALFLASH_CS0  0


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
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COLE_MFCDlg message handlers

BOOL COLE_MFCDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

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
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

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

// Automation servers should not exit when a user closes the UI
//  if a controller still holds on to one of its objects.  These
//  message handlers make sure that if the proxy is still in use,
//  then the UI is hidden but the dialog remains around if it
//  is dismissed.

void COLE_MFCDlg::OnClose() 
{
	if (CanExit())
		CDialog::OnClose();
}

void COLE_MFCDlg::OnOK() 
{
	AT91BOOT_HANDLE_TYPE h_handle;
	CWnd* pWnd = GetDlgItem(IDC_EDIT_CHIP_ID);
    HANDLE hFile = {NULL};
    char messageBuf[64];
	//CHAR  pBuffer[20];
	char *pTest[10];
	long uValue, error_code;
    int comType;
    long appletBuffAddr, appletMemSize, appletBuffSize;
    DWORD dwSize, bytes_read;
    char *buffer;
    int sizeToWrite;
    int bufferSize, offset = 0;
    int pfileToLoad = 0;
    long sizeWritten;

	HRSRC hRsrc;
	HGLOBAL hglb;
	CHAR *lpData;
	UINT BinarySize;

	// MANDATORY: Allocate memory for each string in the table
	for (UINT i=0; i<10; i++)
		pTest[i] = (char *)malloc(100);

	// Scan all devices connected
	m_pAT91BootDLL->AT91Boot_Scan((char *)pTest);

	// Print all devices connected
	OutputDebugString(pTest[0]);
    
    m_pAT91BootDLL->AT91Boot_Open(pTest[0], &h_handle);
    comType = COM_TYPE_DBGU;

	if (h_handle != 0)
	{
		// Read DBGU Chip ID
		m_pAT91BootDLL->AT91Boot_Read_Int(h_handle, &uValue, 0xFFFFF240, &error_code);
        sprintf(messageBuf, "Chip ID: 0x%08x", uValue);
        OutputDebugString(messageBuf);

		// Assume Target is a SAM9G15 (SAM9261-EK)
		// Otherwise, modify that part by yourself, it will be a good training! ;-)
		if ((uValue & DBGU_CIDR_MASK) == (0x819A05A0 & DBGU_CIDR_MASK))
        {
            OutputDebugString("Chip ID match!");

		// Load the lowlevelinit applet
            hFile = CreateFile(LOWLEVELINIT_APPLET_PATH, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, OPEN_EXISTING,
				FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				OutputDebugString("Failed to open the file");
				return;
			}
			// Get the file size so we know how long to make our buffer.
			dwSize = GetFileSize(hFile, NULL);
			// Create a CString buffer of the proper length
			// and fill it with spaces.
			buffer = (char *)malloc(dwSize);
			// Read the file into the buffer
			ReadFile(hFile, buffer, dwSize, &bytes_read, NULL);
			// We're done with the file handle so close it.
			CloseHandle(hFile);
            
            m_pAT91BootDLL->AT91Boot_Write_Data(h_handle, APPLET_ADDR, buffer, dwSize, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Load Applet failed");
                goto exit;
             }
            OutputDebugString("Load Applet OK");
			
			// Set Communication type
			m_pAT91BootDLL->AT91Boot_Write_Int(h_handle, comType, APPLET_ARGV0_ADDR, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Write Applet argument_0 failed");
                goto exit;
             }
            OutputDebugString("Write Applet argument_0 OK");

			m_pAT91BootDLL->AT91Boot_Write_Int(h_handle, EK_MODE, APPLET_ARGV2_ADDR, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Write Applet argument_2 failed");
                goto exit;
             }
            OutputDebugString("Write Applet argument_2 OK");

			
			m_pAT91BootDLL->AT91Boot_Write_Int(h_handle, APPLET_CMD_INIT, MAILBOX_ADDR, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Send Applet mailbox failed");
                goto exit;
             }
            OutputDebugString("Send Applet mailbox OK");
			
			
			m_pAT91BootDLL->AT91Boot_Go(h_handle, APPLET_ADDR, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Applet Go failed");
                goto exit;
             }
            OutputDebugString("Applet Go");
						
			// Wait for the end of applet execution
			m_pAT91BootDLL->AT91Boot_Read_Int(h_handle, &uValue, MAILBOX_ADDR, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Applet Return failed");
                goto exit;
             }
            OutputDebugString("Applet Return");


			// Get the status returned by the applet
			m_pAT91BootDLL->AT91Boot_Read_Int(h_handle, &uValue, APPLET_STATUS_ADDR, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK)    goto exit;
            OutputDebugString("Get applet status");
			
			if (uValue != APPLET_SUCCESS) {
				OutputDebugString("Lowlevelinit applet returned error");
				goto exit;
			} else {
                OutputDebugString("Lowlevelinit applet return!");
            }
        
        // Load the extram applet
            hFile = CreateFile(EXTRAM_APPLET_PATH, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, OPEN_EXISTING,
				FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				OutputDebugString("Failed to open the file");
				return;
			}
			// Get the file size so we know how long to make our buffer.
			dwSize = GetFileSize(hFile, NULL);
			// Create a CString buffer of the proper length
			// and fill it with spaces.
			buffer = (char *)malloc(dwSize);
			// Read the file into the buffer
			ReadFile(hFile, buffer, dwSize, &bytes_read, NULL);
			// We're done with the file handle so close it.
			CloseHandle(hFile);
            
            m_pAT91BootDLL->AT91Boot_Write_Data(h_handle, APPLET_ADDR, buffer, dwSize, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Load Applet failed");
                goto exit;
             }
            OutputDebugString("Load Applet OK");
			
			// Set Communication type
			m_pAT91BootDLL->AT91Boot_Write_Int(h_handle, comType, APPLET_ARGV0_ADDR, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Write Applet argument_0 failed");
                goto exit;
             }
            OutputDebugString("Write Applet argument_0 OK");

			m_pAT91BootDLL->AT91Boot_Write_Int(h_handle, EXTRAM_VDD, APPLET_ARGV2_ADDR, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Write Applet argument_2 failed");
                goto exit;
             }
            OutputDebugString("Write Applet argument_2 OK");

			m_pAT91BootDLL->AT91Boot_Write_Int(h_handle, EXTRAM_TYPE, APPLET_ARGV3_ADDR, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Write Applet argument_3 failed");
                goto exit;
             }
            OutputDebugString("Write Applet argument_3 OK");
            
            m_pAT91BootDLL->AT91Boot_Write_Int(h_handle, EXTRAM_DATA_BUSWIDTH, APPLET_ARGV4_ADDR, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Write Applet argument_4 failed");
                goto exit;
             }
            OutputDebugString("Write Applet argument_4 OK");
            
            m_pAT91BootDLL->AT91Boot_Write_Int(h_handle, EXTRAM_MODEL, APPLET_ARGV5_ADDR, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Write Applet argument_5 failed");
                goto exit;
             }
            OutputDebugString("Write Applet argument_5 OK");
            
			m_pAT91BootDLL->AT91Boot_Write_Int(h_handle, APPLET_CMD_INIT, MAILBOX_ADDR, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Send Applet mailbox failed");
                goto exit;
             }
            OutputDebugString("Send Applet mailbox OK");
			
			m_pAT91BootDLL->AT91Boot_Go(h_handle, APPLET_ADDR, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Applet Go failed");
                goto exit;
             }
            OutputDebugString("Applet Go");
						
			// Wait for the end of applet execution
			m_pAT91BootDLL->AT91Boot_Read_Int(h_handle, &uValue, MAILBOX_ADDR, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Applet Return failed");
                goto exit;
             }
            OutputDebugString("Applet Return");

			// Get the status returned by the applet
			m_pAT91BootDLL->AT91Boot_Read_Int(h_handle, &uValue, APPLET_STATUS_ADDR, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK)    goto exit;
            OutputDebugString("Get applet status");
			
			if (uValue != APPLET_SUCCESS) {
				OutputDebugString("Extram applet returned error");
				goto exit;
			} else {
                OutputDebugString("Extram applet return!");
            }
            
        // Load the dataflash applet
            hFile = CreateFile(SERIALFLASH_APPLET_PATH, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL, OPEN_EXISTING,
				FILE_FLAG_SEQUENTIAL_SCAN, NULL);
			if(hFile == INVALID_HANDLE_VALUE)
			{
				OutputDebugString("Failed to open the file");
				return;
			}
			// Get the file size so we know how long to make our buffer.
			dwSize = GetFileSize(hFile, NULL);
			// Create a CString buffer of the proper length
			// and fill it with spaces.
			buffer = (char *)malloc(dwSize);
			// Read the file into the buffer
			ReadFile(hFile, buffer, dwSize, &bytes_read, NULL);
			// We're done with the file handle so close it.
			CloseHandle(hFile);
            
			m_pAT91BootDLL->AT91Boot_Write_Data(h_handle, APPLET_ADDR_DRAM, buffer, dwSize, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Load Applet failed");
                goto exit;
             }
            OutputDebugString("Load Applet OK");
			
			// Set Communication type
			m_pAT91BootDLL->AT91Boot_Write_Int(h_handle, comType, APPLET_ARGV0_ADDR_DRAM, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Write Applet argument_0 failed");
                goto exit;
             }
            OutputDebugString("Write Applet argument_0 OK");

			m_pAT91BootDLL->AT91Boot_Write_Int(h_handle, SERIALFLASH_CS0, APPLET_ARGV2_ADDR_DRAM, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Write Applet argument_2 failed");
                goto exit;
             }
            OutputDebugString("Write Applet argument_2 OK");

			
			m_pAT91BootDLL->AT91Boot_Write_Int(h_handle, APPLET_CMD_INIT, MAILBOX_ADDR_DRAM, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Send Applet mailbox failed");
                goto exit;
             }
            OutputDebugString("Send Applet mailbox OK");
			
			
			m_pAT91BootDLL->AT91Boot_Go(h_handle, APPLET_ADDR_DRAM, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Applet Go failed");
                goto exit;
             }
            OutputDebugString("Applet Go");
						
			// Wait for the end of applet execution
			m_pAT91BootDLL->AT91Boot_Read_Int(h_handle, &uValue, MAILBOX_ADDR_DRAM, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Applet Return failed");
                goto exit;
             }
            OutputDebugString("Applet Return");

			// Get the status returned by the applet
			m_pAT91BootDLL->AT91Boot_Read_Int(h_handle, &uValue, APPLET_STATUS_ADDR_DRAM, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK)    goto exit;
            OutputDebugString("Get applet status");
			
			if (uValue != APPLET_SUCCESS) {
				OutputDebugString("SerialFlash applet returned NG");
				goto exit;
			} else {
                OutputDebugString("SerialFlash applet return OK");
            }
            
            // Get the SerialFlash Size
            m_pAT91BootDLL->AT91Boot_Read_Int(h_handle, &appletMemSize, APPLET_MEM_SIZE, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Get memory size failed");
                goto exit;
            }
            sprintf(messageBuf, "Applet MEM Size: 0x%08x", appletMemSize);
            OutputDebugString(messageBuf);
			
			// Get the buffer address
			m_pAT91BootDLL->AT91Boot_Read_Int(h_handle, &appletBuffAddr, APPLET_BUFFER_ADDR, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Get applet address failed");
                goto exit;
            }
            sprintf(messageBuf, "Applet buf adr: 0x%08x", appletBuffAddr);
            OutputDebugString(messageBuf);
			
			// Get the buffer size
			m_pAT91BootDLL->AT91Boot_Read_Int(h_handle, &appletBuffSize, APPLET_BUFFER_SIZE, &error_code);
			if (error_code != AT91C_BOOT_DLL_OK) {
                OutputDebugString("Get Applet buf size failed");
                goto exit;
            }
            sprintf(messageBuf, "Applet buf size: 0x%08x", appletBuffSize);
            OutputDebugString(messageBuf);

            hRsrc = FindResource(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_EXAMPLE_IMAGE), "example");
			hglb = LoadResource(AfxGetResourceHandle(), hRsrc);
			lpData = (CHAR *)LockResource(hglb);
			BinarySize = (UINT)SizeofResource(AfxGetResourceHandle(), hRsrc);

            // Write data page after page...
            sizeToWrite = BinarySize;
            while (sizeToWrite > 0) {
                // Adjust the packet size to be sent
                if (sizeToWrite < appletBuffSize) {
                    bufferSize = sizeToWrite;
                } else {
                    bufferSize = appletBuffSize;
                }

                // Copy in RAM the content of the page to be written
                m_pAT91BootDLL->AT91Boot_Write_Data(h_handle, appletBuffAddr, lpData + offset, bufferSize, &error_code);
                if (error_code != AT91C_BOOT_DLL_OK) {
                    OutputDebugString("Send piece of data failed");
                    goto exit;
                }
                OutputDebugString("Send piece of data OK");
                
                // Write the Cmd op code in the argument area
                m_pAT91BootDLL->AT91Boot_Write_Int(h_handle, APPLET_CMD_WRITE, APPLET_CMD_ADDR_DRAM, &error_code);
                if (error_code != AT91C_BOOT_DLL_OK) {
                    OutputDebugString("Send Write CMD failed");
                    goto exit;
                }
                OutputDebugString("Send Write CMD OK");
                
                // Write the buffer address in the argument area
                m_pAT91BootDLL->AT91Boot_Write_Int(h_handle, appletBuffAddr, APPLET_ARGV0_ADDR_DRAM, &error_code);
                if (error_code != AT91C_BOOT_DLL_OK) {
                    OutputDebugString("Send buf addr failed");
                    goto exit;
                }
                OutputDebugString("Send buf addr OK");
                
                // Write the buffer size in the argument area
                m_pAT91BootDLL->AT91Boot_Write_Int(h_handle, bufferSize, APPLET_ARGV1_ADDR_DRAM, &error_code);
                if (error_code != AT91C_BOOT_DLL_OK) {
                    OutputDebugString("Send buf size failed");
                    goto exit;
                }
                OutputDebugString("Send buf size OK");
                
                // Write the memory offset in the argument area
                m_pAT91BootDLL->AT91Boot_Write_Int(h_handle, offset, APPLET_ARGV2_ADDR_DRAM, &error_code);
                if (error_code != AT91C_BOOT_DLL_OK) {
                    OutputDebugString("Send offset failed");
                    goto exit;
                }
                OutputDebugString("Send offset OK");
        
                // Launch the applet Jumping to the appletAddr
                m_pAT91BootDLL->AT91Boot_Go(h_handle, APPLET_ADDR_DRAM, &error_code);
                if (error_code != AT91C_BOOT_DLL_OK) {
                    OutputDebugString("Applet Go failed");
                    goto exit;
                }
                OutputDebugString("Applet Go");
                            
                // Wait for the end of applet execution
                m_pAT91BootDLL->AT91Boot_Read_Int(h_handle, &uValue, MAILBOX_ADDR_DRAM, &error_code);
                if (error_code != AT91C_BOOT_DLL_OK) {
                    OutputDebugString("Applet Return failed");
                    goto exit;
                }
                OutputDebugString("Applet Return");
    
                // Get the status returned by the applet
                m_pAT91BootDLL->AT91Boot_Read_Int(h_handle, &uValue, APPLET_STATUS_ADDR_DRAM, &error_code);
                if (error_code != AT91C_BOOT_DLL_OK)    goto exit;
                OutputDebugString("Get applet status");
                if (uValue != APPLET_SUCCESS) {
                    OutputDebugString("SerialFlash applet returned NG");
                    goto exit;
                } else {
                    OutputDebugString("SerialFlash applet return OK");
                }
                
                // Retrieve how many bytes have been written
                m_pAT91BootDLL->AT91Boot_Read_Int(h_handle, &sizeWritten, APPLET_ARGV0_ADDR_DRAM, &error_code);
                if (error_code != AT91C_BOOT_DLL_OK) {
                    OutputDebugString("Get written size failed");
                    goto exit;
                }
                sprintf(messageBuf, "0x%08x bytes written", sizeWritten);
                OutputDebugString(messageBuf);
                
                sizeToWrite -= sizeWritten;
                offset += sizeWritten;
            }

exit:
			// Free Resource
			FreeResource(hglb);
            OutputDebugString("Exit");
		}
        // Free Allocated memory
        for (i=0; i<10; i++)
            free(pTest[i]);
    }    
    
    m_pAT91BootDLL->AT91Boot_Close(h_handle);

//	if (CanExit())
//		CDialog::OnOK();
}

void COLE_MFCDlg::OnCancel() 
{
	if (CanExit())
		CDialog::OnCancel();
}

BOOL COLE_MFCDlg::CanExit()
{
	// If the proxy object is still around, then the automation
	//  controller is still holding on to this application.  Leave
	//  the dialog around, but hide its UI.
	if (m_pAutoProxy != NULL)
	{
		ShowWindow(SW_HIDE);
		return FALSE;
	}

	return TRUE;
}

