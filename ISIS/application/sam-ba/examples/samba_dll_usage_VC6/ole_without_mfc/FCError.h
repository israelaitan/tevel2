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
