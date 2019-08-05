/*
 * ScsGeckoDemo.c
 *
 *  Created on: 29 jan. 2018
 *      Author: pbot
 */

#include <satellite-subsystems/SCS_Gecko/gecko_driver.h>
#include <satellite-subsystems/SCS_Gecko/gecko_use_cases.h>

#include <hal/Timing/WatchDogTimer.h>
#include <hal/Drivers/SPI.h>
#include <hal/Utility/util.h>
#include <hal/boolean.h>
#include <hal/errors.h>

#include <hcc/api_fat.h>
#include <hcc/api_hcc_mem.h>
#include <hcc/api_fat_test.h>
#include <hcc/api_mdriver_atmel_mcipdc.h>

#include <stdint.h>

#define _IMG_NUM 1
#define _ADC_GAIN 53
#define _PGA_GAIN 3
#define _EXPOSURE 2048
#define _FRAME_AMOUNT 1
#define _FRAME_COUNT 1
#define _SD_CARD 0
#define _IMAGE_BYTES (2048*1088)
#define _SPI_GECKO_BUS_SPEED MHZ(5)

static uint8_t _imageBuffer[8*1024*1024];



Boolean Gecko_GetRegister( void )
{
	unsigned int reg = 0;
	printf("Register to read (0-14)\n\r");
	while(UTIL_DbguGetIntegerMinMax(&reg, 1, 14) == 0);

	unsigned int regVal;
	GECKO_GetReg((uint8_t)reg,(uint32_t*)&regVal,GECKO_Endianness_big);

	printf("Register 0x%02X = 0x%08X\n\r",reg,regVal);

	return TRUE;
}

Boolean Gecko_SetRegister( void )
{
	unsigned int reg = 0;
	printf("Register to set (0-14)\n\r");
	while(UTIL_DbguGetIntegerMinMax(&reg, 0, 14) == 0);

	unsigned int regVal;
	printf("Register value to set\n\r0x");fflush(stdout);
	while(UTIL_DbguGetHexa32(&regVal) == 0);

	GECKO_SetReg((uint8_t)reg,(uint32_t)regVal,GECKO_Endianness_big);
	printf("\n\rSet register 0x%02X = 0x%08X\n\r",reg,regVal);

	GECKO_GetReg((uint8_t)reg,(uint32_t*)&regVal,GECKO_Endianness_big);
	printf("Register 0x%02X = 0x%08X\n\r",reg,regVal);

	return TRUE;
}

Boolean Gecko_DisplayVersion( void )
{
	uint8_t versionHW = GECKO_GetVersionHW();
	uint8_t versionSWMajor = GECKO_GetVersionSwMajor();
	uint8_t versionSWMinor = GECKO_GetVersionSwMinor();
	uint8_t versionSWBuild = GECKO_GetVersionSwBuild();

	printf("\n\r Version: %u.%u.%u.%u\n\r", versionHW, versionSWMajor, versionSWMinor, versionSWBuild);

	return TRUE;
}

Boolean Gecko_DisplayUptime( void )
{
	uint32_t uptime = GECKO_GetUpTime();

	printf("\n\r %lu\n\r", uptime);

	return TRUE;
}

Boolean Gecko_TakeImage( void )
{
	int err = GECKO_UC_TakeImage(53,3,2048,1,1,_IMG_NUM);
	if( err )
	{
		TRACE_ERROR("Error (%d) taking image!\n\r",err);
	}

	return TRUE;
}

Boolean Gecko_ReadImage( void )
{
	int err = GECKO_UC_ReadImage(_IMG_NUM,(uint32_t*)_imageBuffer, TRUE);
	if( err )
	{
		TRACE_ERROR("Error (%d) reading image!\n\r",err);
	}

	F_FILE *file = f_open( "gecko.raw", "w" ); /* open file for writing in safe mode */
	if( !file )
	{
		TRACE_ERROR("f_open pb: %d\n\r", f_getlasterror() ); /* if file pointer is NULL, get the error */
	}

	int bw = f_write( _imageBuffer, 1, _IMAGE_BYTES, file );
	if( _IMAGE_BYTES != bw )
	{
		TRACE_ERROR("f_write pb: %d\n\r", f_getlasterror() ); /* if bytes to write doesn't equal bytes written, get the error */
	}

	f_flush( file ); /* only after flushing can data be considered safe */

	err = f_close( file ); /* data is also considered safe when file is closed */
	if( err )
	{
		TRACE_ERROR("f_close pb: %d\n\r", err);
	}

	return TRUE;
}

Boolean Gecko_EraseImage( void )
{
	int err = GECKO_UC_EraseBlock(_IMG_NUM);
	if( err )
	{
		TRACE_ERROR("Error (%d) erasing image!\n\r",err);
	}

	return TRUE;
}

Boolean Gecko_PrintAllRegisters( void )
{
	unsigned int reg;
	for(reg=1;reg<=0x0E;reg++)
	{
		unsigned int regVal;
		GECKO_GetReg((uint8_t)reg,(uint32_t*)&regVal,GECKO_Endianness_big);
		printf("Register 0x%02X = 0x%08X\n\r",reg,regVal);
	}

	for(reg=0x20;reg<=0x30;reg++)
	{
		unsigned int regVal;
		GECKO_GetReg((uint8_t)reg,(uint32_t*)&regVal,GECKO_Endianness_big);
		printf("Register 0x%02X = 0x%08X\n\r",reg,regVal);
	}

	return TRUE;
}

Boolean Gecko_PrintTelemetry( void )
{
	printf("\nCURRENTS & VOLTAGES\n\r");
	printf("Input 5V:    %.3fA @ %.3fV\n\r", GECKO_GetCurrentInput5V(), GECKO_GetVoltageInput5V() );
	printf("FPGA 1V0:    %.3fA @ %.3fV\n\r", GECKO_GetCurrentFPGA1V(), GECKO_GetVoltageFPGA1V() );
	printf("FPGA 1V8:    %.3fA @ %.3fV\n\r", GECKO_GetCurrentFPGA1V8(), GECKO_GetVoltageFPGA1V8() );
	printf("FPGA 2V5:    %.3fA @ %.3fV\n\r", GECKO_GetCurrentFPGA2V5(), GECKO_GetVoltageFPGA2V5() );
	printf("FPGA 3V3:    %.3fA @ %.3fV\n\r", GECKO_GetCurrentFPGA3V3(), GECKO_GetVoltageFPGA3V3() );
	printf("Flash 1V8:   %.3fA @ %.3fV\n\r", GECKO_GetCurrentFlash1V8(), GECKO_GetVoltageFlash1V8() );
	printf("Flash 3V3:   %.3fA @ %.3fV\n\r", GECKO_GetCurrentFlash3V3(), GECKO_GetVoltageFlash3V3() );
	printf("Sensor 1V8:  %.3fA @ %.3fV\n\r", GECKO_GetCurrentSNSR1V8(), GECKO_GetVoltageSNSR1V8() );
	printf("Sensor Pix:  %.3fA @ %.3fV\n\r", GECKO_GetCurrentSNSRVDDPIX(), GECKO_GetVoltageSNSRVDDPIX() );
	printf("Sensor 3V3:  %.3fA @ %.3fV\n\r", GECKO_GetCurrentSNSR3V3(), GECKO_GetVoltageSNSR3V3() );
	printf("Flash VTT09: %.3fV\n\r", GECKO_GetVoltageFlashVTT09() );

	printf("\nTEMPERATURES:\n\r");
	printf("SM U3 AB: %.3f\n\r", GECKO_GetTempSMU3AB());
	printf("SM U3 BC: %.3f\n\r", GECKO_GetTempSMU3BC());
	printf("REG U6:   %.3f\n\r", GECKO_GetTempREGU6());
	printf("REG U8:   %.3f\n\r", GECKO_GetTempREGU8());
	printf("FLASH:    %.3f\n\r", GECKO_GetTempFlash());

	return TRUE;
}

Boolean selectAndExecuteGeckoDemoTest( void )
{
	unsigned int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf("\n\r Select a test to perform: \n\r");

	printf("\t 1) Display Version \n\r");
	printf("\t 2) Display Uptime \n\r");
	printf("\t 3) Print telemetry \n\r");
	printf("\t 4) Erase image \n\r");
	printf("\t 5) Take image \n\r");
	printf("\t 6) Read image \n\r");
	printf("\t 7) Get Register \n\r");
	printf("\t 8) Set Register \n\r");
	printf("\t 9) Print all registers \n\r");
	printf("\t 10) Return to main menu \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 10) == 0);

	switch (selection)
	{
	case 1:
		offerMoreTests = Gecko_DisplayVersion();
		break;
	case 2:
		offerMoreTests = Gecko_DisplayUptime();
		break;
	case 3:
		offerMoreTests = Gecko_PrintTelemetry();
		break;
	case 4:
		offerMoreTests = Gecko_EraseImage();
		break;
	case 5:
		offerMoreTests = Gecko_TakeImage();
		break;
	case 6:
		offerMoreTests = Gecko_ReadImage();
		break;
	case 7:
		offerMoreTests = Gecko_GetRegister();
		break;
	case 8:
		offerMoreTests = Gecko_SetRegister();
		break;
	case 9:
		offerMoreTests = Gecko_PrintAllRegisters();
		break;
	case 10:
		offerMoreTests = FALSE;
		break;

	default:
		break;
	}

	return offerMoreTests;
}

int _InitFileSystem( void )
{
	int ret;

	hcc_mem_init(); /* Initialize the memory to be used by filesystem */

	ret = fs_init(); /* Initialize the filesystem */
	if(ret != F_NO_ERROR )
	{
		TRACE_ERROR("fs_init pb: %d\n\r", ret);
		return -1;
	}

	ret = f_enterFS(); /* Register this task with filesystem */
	if(ret != F_NO_ERROR )
	{
		TRACE_ERROR("f_enterFS pb: %d\n\r", ret);
		return -1;
	}

	ret = f_initvolume( 0, atmel_mcipdc_initfunc, _SD_CARD ); /* Initialize volID as safe */

	if( F_ERR_NOTFORMATTED == ret )
	{
		TRACE_ERROR("Filesystem not formated!\n\r");
		return -1;
	}
	else if( F_NO_ERROR != ret)
	{
		TRACE_ERROR("f_initvolume pb: %d\n\r", ret);
		return -1;
	}

	return 0;
}

void _DeinitFileSystem( void )
{
	f_delvolume( _SD_CARD ); /* delete the volID */
	f_releaseFS(); /* release this task from the filesystem */

	fs_delete(); /* delete the filesystem */

	hcc_mem_delete(); /* free the memory used by the filesystem */
}

Boolean ScsGeckoDemoInit(void)
{
	if( _InitFileSystem() )
	{
		TRACE_ERROR("Error initializing the filesystem!\n\r");
		return FALSE;
	}

	if( GECKO_Init( (SPIslaveParameters){ bus1_spi, mode0_spi, slave1_spi, 100, 1, _SPI_GECKO_BUS_SPEED, 0 } ) )
	{
		TRACE_ERROR("Error initializing the gecko camera!\n\r");
		return FALSE;
	}

	return TRUE;
}

void ScsGeckoDemoLoop(void)
{
	Boolean offerMoreTests = FALSE;

	while (1)
	{
		offerMoreTests = selectAndExecuteGeckoDemoTest();

		if (offerMoreTests == FALSE)
		{
			break;
		}
	}
}

Boolean ScsGeckoDemoMain(void)
{
	if(ScsGeckoDemoInit())
	{
		ScsGeckoDemoLoop();

		_DeinitFileSystem();

		return TRUE;
	}
	else
	{
		return FALSE;
	}

	return TRUE;
}

Boolean GeckoTest( void )
{
	ScsGeckoDemoMain();
	return TRUE;
}


