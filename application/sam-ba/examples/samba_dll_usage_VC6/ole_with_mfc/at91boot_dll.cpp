// Machine generated IDispatch wrapper class(es) created with ClassWizard

#include "stdafx.h"
#include "at91boot_dll.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



/////////////////////////////////////////////////////////////////////////////
// IAT91BootDLL properties

/////////////////////////////////////////////////////////////////////////////
// IAT91BootDLL operations

#define MY_VTS_UI1	"\x11"
// Do NOT use VTS_UI1. It has a value of "0x0F" which is incorrect.
// The correct value for VTS_UI1 is 17 or 0x11
#define MY_VTS_PUI1	"\x51"
// Do NOT use VTS_PUI1. It has a value of "0x4F" which is incorrect.
// The correct value for VTS_PUI1 is 81 or 0x51

/////////////////////////////////////////////////////////////////////////////
// IAT91BootDLL properties

/////////////////////////////////////////////////////////////////////////////
// IAT91BootDLL operations

void IAT91BootDLL::AT91Boot_Close(long h_handle)
{
	static BYTE parms[] =
		VTS_I4;
	InvokeHelper(0x1, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 h_handle);
}

void IAT91BootDLL::AT91Boot_Open(char *name, long *h_handle)
{
	static BYTE parms[] =
		MY_VTS_PUI1 VTS_PI4;
	InvokeHelper(0x2, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 name, h_handle);
}

void IAT91BootDLL::AT91Boot_Write_Int(long h_handle, long uValue, long uAddress, long *err_code)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4 VTS_I4 VTS_PI4;
	InvokeHelper(0x3, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 h_handle, uValue, uAddress, err_code);
}

void IAT91BootDLL::AT91Boot_Write_Short(long h_handle, short wValue, long uAddress, long *err_code)
{
	static BYTE parms[] =
		VTS_I4 VTS_I2 VTS_I4 VTS_PI4;
	InvokeHelper(0x4, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 h_handle, wValue, uAddress, err_code);
}

void IAT91BootDLL::AT91Boot_Write_Byte(long h_handle, char bValue, long uAddress, long *err_code)
{
	static BYTE parms[] =
		VTS_I4 MY_VTS_UI1 VTS_I4 VTS_PI4;
	InvokeHelper(0x5, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 h_handle, bValue, uAddress, err_code);
}

void IAT91BootDLL::AT91Boot_Write_Data(long h_handle, long uAddress, char *bValue, long uSize, long *err_code)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4 MY_VTS_PUI1 VTS_I4 VTS_PI4;
	InvokeHelper(0x6, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 h_handle, uAddress, bValue, uSize, err_code);
}

void IAT91BootDLL::AT91Boot_Read_Int(long h_handle, long* uValue, long uAddress, long *err_code)
{
	static BYTE parms[] =
		VTS_I4 VTS_PI4 VTS_I4 VTS_PI4;
	InvokeHelper(0x7, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 h_handle, uValue, uAddress, err_code);
}

void IAT91BootDLL::AT91Boot_Read_Short(long h_handle, short* wValue, long uAddress, long *err_code)
{
	static BYTE parms[] =
		VTS_I4 VTS_PI2 VTS_I4 VTS_PI4;
	InvokeHelper(0x8, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 h_handle, wValue, uAddress, err_code);
}

void IAT91BootDLL::AT91Boot_Read_Byte(long h_handle, char* bValue, long uAddress, long *err_code)
{
	static BYTE parms[] =
		VTS_I4 MY_VTS_PUI1 VTS_I4 VTS_PI4;
	InvokeHelper(0x9, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 h_handle, bValue, uAddress, err_code);
}

void IAT91BootDLL::AT91Boot_Read_Data(long h_handle, long uAddress, char *bValue, long uSize, long *err_code)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4 MY_VTS_PUI1 VTS_I4 VTS_PI4;
	InvokeHelper(0xa, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 h_handle, uAddress, bValue, uSize, err_code);
}

void IAT91BootDLL::AT91Boot_Go(long h_handle, long uAddress, long *err_code)
{
	static BYTE parms[] =
		VTS_I4 VTS_I4 VTS_PI4;
	InvokeHelper(0xb, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 h_handle, uAddress, err_code);
}

void IAT91BootDLL::AT91Boot_Scan(char* pDevList)
{
	static BYTE parms[] =
		MY_VTS_PUI1;
	InvokeHelper(0xc, DISPATCH_METHOD, VT_EMPTY, NULL, parms,
		 pDevList);
}
