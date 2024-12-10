// FCError.h: interface for the CFCError class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FCERROR_H__65CD09C4_FE42_49B0_86E4_BF60F68C5ED8__INCLUDED_)
#define AFX_FCERROR_H__65CD09C4_FE42_49B0_86E4_BF60F68C5ED8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////
// AT91BOOT_DLL ERRORS				   //
/////////////////////////////////////////
#define AT91C_BOOT_DLL_OK			  (int)(0x0000)	

// AT91Boot_DLL Error Codes returned
#define AT91C_BAD_HANDLE			  (int)(0xF001)	// Bad Handle
#define AT91C_BAD_ADDRESS			  (int)(0xF002)	// Bad Address argument
#define AT91C_BAD_SIZE				  (int)(0xF003)	// Bad Size
#define AT91C_COMM_NOT_OPENED		  (int)(0xF004)	// Communication Link not opened
#define AT91C_TARGET_NOT_RESPONDING   (int)(0xF005)	// Target not responding (Communication link broken)

#endif // !defined(AFX_FCERROR_H__65CD09C4_FE42_49B0_86E4_BF60F68C5ED8__INCLUDED_)
