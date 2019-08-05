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

// OLE_without_MFC.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <stdio.h>
using namespace SAMBADLL_Lib;

#include "FCError.h"

typedef int AT91BOOT_HANDLE_TYPE;

#define	DBGU_CIDR_MASK ~0x000F001F	// Remove version and SRAMSIZ (bug DBGU_CIDR S128 et S256 rev C)
#define PROG_FMR	0x00480100		// EFC FMR configuration for 48MHz 



// SAM-BA 2.10 applet contants
#include "applet.h"

#define FLASH_APPLET_PATH "..\\..\\..\\tcl_lib\\at91sam7s256-ek\\applet-flash-at91sam7s256.bin"
//#define FLASH_APPLET_PATH "isp-flash-at91sam7s256.bin"


// values 
#define APPLET_ADDR          0x202000
#define MAILBOX_ADDR         0x202004


#define APPLET_CMD_ADDR      (MAILBOX_ADDR)
#define APPLET_STATUS_ADDR   (MAILBOX_ADDR + 4)

#define APPLET_ARGV0_ADDR     (MAILBOX_ADDR + 0x08)
#define APPLET_ARGV1_ADDR     (MAILBOX_ADDR + 0x0c)
#define APPLET_ARGV2_ADDR     (MAILBOX_ADDR + 0x10)
#define APPLET_ARGV3_ADDR     (MAILBOX_ADDR + 0x14)

#define APPLET_BUFFER_ADDR   (APPLET_ARGV1_ADDR)
#define APPLET_BUFFER_SIZE   (APPLET_ARGV2_ADDR)


#define COM_TYPE_USB  0
#define COM_TYPE_DBGU 1
#define COM_TYPE_JTAG 2


// Number of Lock region in sam7s256
#define S256_LOCK_NBR   16
#define FILE_TO_WRITE_SIZE      (256 * 1024)



int main(int argc, char* argv[])

{
	if (FAILED(CoInitialize(NULL)))
		return 0;

	try
	{
		int i, uValue, error_code;
	    int appletBuffAddr;
	    int appletBuffSize;
		int comType;

		unsigned char *pTest[10];
			
		// COM Object Creation
		ISAMBADLLPtr pAT91BootDLL(__uuidof(SAMBADLL));

		AT91BOOT_HANDLE_TYPE h_handle;

		// MANDATORY: Allocate memory for each string in the table
		for (i=0; i<10; i++)
			pTest[i] = (unsigned char *)malloc(100);

		// Scan all devices connected
		pAT91BootDLL->AT91Boot_Scan((unsigned char *)pTest);

		// Print all devices connected
		i= 0;
		while (pTest[i] != 0)
		{
			printf("Device %d = %s\n\r", i, pTest[i]);
			i++;
		}
		
		// Open first Device of the list
		pAT91BootDLL->AT91Boot_Open(pTest[0], &h_handle);

        comType = COM_TYPE_DBGU;


		if (h_handle != 0)
		{
			// Read DBGU Chip ID
			pAT91BootDLL->AT91Boot_Read_Int(h_handle, &uValue, 0xFFFFF240, NULL);
		
			// Assume Target is a SAM7S256 (SAM7S-EK)
			// Otherwise, modify that part by yourself, it will be a good training! ;-)
			if ((uValue & DBGU_CIDR_MASK) == (0x270B0940 & DBGU_CIDR_MASK))
			{
				HANDLE hFile = {NULL};
				DWORD dwSize, bytes_read;
				unsigned char *buffer;
				unsigned char *progBuffer;
				int sizeToWrite = 0;
				int sizeWritten = 0;
				int bufferSize = 0;
				int offset = 0;
				
				
				hFile = CreateFile(FLASH_APPLET_PATH, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL, OPEN_EXISTING,
					FILE_FLAG_SEQUENTIAL_SCAN, NULL);
				if(hFile == INVALID_HANDLE_VALUE)
				{
					printf("Failed to open the file.\n\n");
					return -1;
				}
				// Get the file size so we know how long to make our buffer.
				dwSize = GetFileSize(hFile, NULL);
				// Create a CString buffer of the proper length
				// and fill it with spaces.
				buffer = (unsigned char *)malloc(dwSize);
				// Read the file into the buffer
				ReadFile(hFile, buffer, dwSize, &bytes_read, NULL);
				// We're done with the file handle so close it.
				CloseHandle(hFile);


				// MANDATORY: Program EFC_FMR register
				// Assume Target is at 48MHz
				pAT91BootDLL->AT91Boot_Write_Int(h_handle, PROG_FMR, 0xFFFFFF60, &error_code);
				if (error_code != AT91C_BOOT_DLL_OK)
					goto exit;

				
				pAT91BootDLL->AT91Boot_Write_Data(h_handle, APPLET_ADDR, buffer, dwSize, &error_code);
				if (error_code != AT91C_BOOT_DLL_OK)
					goto exit;
				
				// Set Communication type
				pAT91BootDLL->AT91Boot_Write_Int(h_handle, comType, APPLET_ARGV0_ADDR, &error_code);
				if (error_code != AT91C_BOOT_DLL_OK)
					goto exit;

				
				pAT91BootDLL->AT91Boot_Write_Int(h_handle, APPLET_CMD_INIT, MAILBOX_ADDR, &error_code);
				if (error_code != AT91C_BOOT_DLL_OK)
					goto exit;
				
				
				pAT91BootDLL->AT91Boot_Go(h_handle, APPLET_ADDR, &error_code);
				if (error_code != AT91C_BOOT_DLL_OK)
					goto exit;
				
				
				// Wait for the end of applet execution
				pAT91BootDLL->AT91Boot_Read_Int(h_handle, &uValue, MAILBOX_ADDR, &error_code);
				if (error_code != AT91C_BOOT_DLL_OK)
					goto exit;


				// Get the status returned by the applet
				pAT91BootDLL->AT91Boot_Read_Int(h_handle, &uValue, APPLET_STATUS_ADDR, &error_code);
				if (error_code != AT91C_BOOT_DLL_OK)
					goto exit;
				
				if (uValue != APPLET_SUCCESS) {
					printf("Flash applet returned error : 0x%x", uValue);
					goto exit;
				}

				// Get the buffer address
				pAT91BootDLL->AT91Boot_Read_Int(h_handle, &appletBuffAddr, APPLET_BUFFER_ADDR, &error_code);
				if (error_code != AT91C_BOOT_DLL_OK)
					goto exit;
				
				// Get the buffer size
				pAT91BootDLL->AT91Boot_Read_Int(h_handle, &appletBuffSize, APPLET_BUFFER_SIZE, &error_code);
				if (error_code != AT91C_BOOT_DLL_OK)
					goto exit;



				// Unprotect the whole flash
				for (i = 0; i < S256_LOCK_NBR; i++) {

					// Launch the applet with Write command as argument
					pAT91BootDLL->AT91Boot_Write_Int(h_handle, APPLET_CMD_UNLOCK, MAILBOX_ADDR, &error_code);
					if (error_code != AT91C_BOOT_DLL_OK)
						goto exit;

					// set buffer address
					pAT91BootDLL->AT91Boot_Write_Int(h_handle, i, APPLET_ARGV0_ADDR, &error_code);
					if (error_code != AT91C_BOOT_DLL_OK)
						goto exit;

					// perform GO command
					pAT91BootDLL->AT91Boot_Go(h_handle, APPLET_ADDR, &error_code);
					if (error_code != AT91C_BOOT_DLL_OK)
						goto exit;
					
					// Wait for the end of applet execution
					pAT91BootDLL->AT91Boot_Read_Int(h_handle, &uValue, MAILBOX_ADDR, &error_code);
					if (error_code != AT91C_BOOT_DLL_OK)
						goto exit;


					// Get the status returned by the applet
					pAT91BootDLL->AT91Boot_Read_Int(h_handle, &uValue, APPLET_STATUS_ADDR, &error_code);
					if (error_code != AT91C_BOOT_DLL_OK)
						goto exit;
					
					if (uValue != APPLET_SUCCESS) {
						printf("Flash applet returned error : 0x%x\n\r", uValue);
						goto exit;
					}
				}



				// Fill a buffer to be sent to the flash

				// Create a CString buffer of the proper length
				// and fill it some data.
				progBuffer = (unsigned char *)malloc(FILE_TO_WRITE_SIZE);
				for (i = 0; i < FILE_TO_WRITE_SIZE; i++) {
					progBuffer[i] = i;
				}



				sizeToWrite = FILE_TO_WRITE_SIZE;
				// Write data
				while (sizeToWrite > 0) {

					// Adjust the packet size to be sent
					if (sizeToWrite < appletBuffSize) {
						bufferSize = sizeToWrite;
					} else {
						bufferSize = appletBuffSize;
					}

					// Send data to be progreammed into the applet buffer
					pAT91BootDLL->AT91Boot_Write_Data(h_handle, appletBuffAddr, progBuffer, bufferSize, &error_code);
					if (error_code != AT91C_BOOT_DLL_OK)
						goto exit;
					

					// Launch the applet with Write command as argument
					pAT91BootDLL->AT91Boot_Write_Int(h_handle, APPLET_CMD_WRITE, MAILBOX_ADDR, &error_code);
					if (error_code != AT91C_BOOT_DLL_OK)
						goto exit;

					// set buffer address
					pAT91BootDLL->AT91Boot_Write_Int(h_handle, appletBuffAddr, APPLET_ARGV0_ADDR, &error_code);
					if (error_code != AT91C_BOOT_DLL_OK)
						goto exit;

					// set buffer size
					pAT91BootDLL->AT91Boot_Write_Int(h_handle, bufferSize, APPLET_ARGV1_ADDR, &error_code);
					if (error_code != AT91C_BOOT_DLL_OK)
						goto exit;

					// set address wher to program the data in flash
					pAT91BootDLL->AT91Boot_Write_Int(h_handle, offset, APPLET_ARGV2_ADDR, &error_code);
					if (error_code != AT91C_BOOT_DLL_OK)
						goto exit;

					pAT91BootDLL->AT91Boot_Go(h_handle, APPLET_ADDR, &error_code);
					if (error_code != AT91C_BOOT_DLL_OK)
						goto exit;
					
					// Wait for the end of applet execution
					pAT91BootDLL->AT91Boot_Read_Int(h_handle, &uValue, MAILBOX_ADDR, &error_code);
					if (error_code != AT91C_BOOT_DLL_OK)
						goto exit;


					// Get the status returned by the applet
					pAT91BootDLL->AT91Boot_Read_Int(h_handle, &uValue, APPLET_STATUS_ADDR, &error_code);
					if (error_code != AT91C_BOOT_DLL_OK)
						goto exit;

					// Get the number of bytes written by the applet
					pAT91BootDLL->AT91Boot_Read_Int(h_handle, &sizeWritten, APPLET_ARGV0_ADDR, &error_code);
					if (error_code != AT91C_BOOT_DLL_OK)
						goto exit;

					printf("Flash applet wrote : 0x%x\n\r", sizeWritten);

					if (uValue != APPLET_SUCCESS) {
						printf("Flash applet returned error : 0x%x", uValue);
						goto exit;
					} 
						
				    sizeToWrite -= sizeWritten;
					progBuffer += sizeWritten;
					offset += sizeWritten;

				}				

	
				// Send File into SAM7S256 Flash
//				pAT91BootDLL->AT91Boot_SAM7S256_Send_Flash(h_handle, 0, buffer, dwSize, &error_code);
//				if (error_code != AT91C_BOOT_DLL_OK)
//					goto exit;

exit:
				// Release buffer
				free(buffer);
			}
			

			// Close Device
			pAT91BootDLL->AT91Boot_Close(h_handle);
		}

		// Free Allocated memory
		for (i=0; i<10; i++)
			free(pTest[i]);
	}

	catch (const _com_error& Err)
	{
		printf("Error: %d\n\r", Err.ErrorMessage());
	}

	CoUninitialize();
	return 0;
}
