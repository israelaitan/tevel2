// Machine generated IDispatch wrapper class(es) created with ClassWizard
/////////////////////////////////////////////////////////////////////////////
// IAT91BootDLL wrapper class
#ifndef _at91boot_dll_h
#define _at91boot_dll_h

typedef long AT91BOOT_HANDLE_TYPE;

class IAT91BootDLL : public COleDispatchDriver
{
public:
	IAT91BootDLL() {}		// Calls COleDispatchDriver default constructor
	IAT91BootDLL(LPDISPATCH pDispatch) : COleDispatchDriver(pDispatch) {}
	IAT91BootDLL(const IAT91BootDLL& dispatchSrc) : COleDispatchDriver(dispatchSrc) {}

// Attributes
public:

// Operations
public:
	void AT91Boot_Close(long h_handle);
	void AT91Boot_Open(char *name, long *h_handle);
	void AT91Boot_Write_Int(long h_handle, long uValue, long uAddress, long *err_code);
	void AT91Boot_Write_Short(long h_handle, short wValue, long uAddress, long *err_code);
	void AT91Boot_Write_Byte(long h_handle, char bValue, long uAddress, long *err_code);
	void AT91Boot_Write_Data(long h_handle, long uAddress, char *bValue, long uSize, long *err_code);
	void AT91Boot_Read_Int(long h_handle, long* uValue, long uAddress, long *err_code);
	void AT91Boot_Read_Short(long h_handle, short* wValue, long uAddress, long *err_code);
	void AT91Boot_Read_Byte(long h_handle, char* bValue, long uAddress, long *err_code);
	void AT91Boot_Read_Data(long h_handle, long uAddress, char *bValue, long uSize, long *err_code);
	void AT91Boot_Go(long h_handle, long uAddress, long *err_code);
	void AT91Boot_Scan(char *pDevList);
};

#endif