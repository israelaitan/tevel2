#ifndef _SAMBA_THREAD_H_
#define _SAMBA_THREAD_H_
typedef struct _SAMBA_THREAD_PARA
{
    bool                    plug_in;           // check if the usb plug_in
    bool                    plug_out;          // check if the usb plug_out
    bool                    is_running;        // check if thread is running
    unsigned int            iPortNo;           // Port Number         
    void*                   This;              // pointer to class
    long                    dwLogFileSize;     // Recently log file size
    int                     iTimeout;          // Timeout 
    CWinThread*             pWinThreadConnect; // ThreadPointer
    UINT                    Timer_ID;

    _SAMBA_THREAD_PARA() : is_running(false),
                           This(NULL),
                           dwLogFileSize(0),
                           iTimeout(0),
                           pWinThreadConnect(NULL),
                           Timer_ID(0)
                           {}
} SAMBA_THREAD_PARA;

#endif // _SAMBA_THREAD_H_
