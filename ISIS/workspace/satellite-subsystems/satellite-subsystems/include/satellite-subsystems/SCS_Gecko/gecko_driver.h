/*
 * gecko_driver.h
 *
 *  Created on: 20 dec. 2017
 *      Author: pbot
 */

#ifndef SRC_SCS_GECKO_GECKO_DRIVER_H_
#define SRC_SCS_GECKO_GECKO_DRIVER_H_

#include <hal/Drivers/SPI.h>

#include <stdint.h>

typedef enum GECKO_Endiannes_t
{
	GECKO_Endianness_big,
	GECKO_Endianness_little,
}
GECKO_Endianess_t;

int GECKO_Init( SPIslaveParameters slaveParams );

int GECKO_GetReg( uint8_t addr, uint32_t *data, GECKO_Endianess_t endian );
int GECKO_SetReg( uint8_t addr, uint32_t data, GECKO_Endianess_t endian );

uint8_t GECKO_GetVersionSwBuild( void );
uint8_t GECKO_GetVersionSwMinor( void );
uint8_t GECKO_GetVersionSwMajor( void );
uint8_t GECKO_GetVersionHW( void );

int GECKO_StartReadout( void );
int GECKO_StartSample( void );
int GECKO_StartErase( void );
int GECKO_EnableTestPattern( void );

int GECKO_StopReadout( void );
int GECKO_StopSample( void );
int GECKO_StopErase( void );
int GECKO_DisableTestPattern( void );

int GECKO_GetFlashInitDone( void );
int GECKO_GetReadReady( void );
int GECKO_GetReadBusy( void );
int GECKO_GetSampleBusy( void );
int GECKO_GetEraseBusy( void );
int GECKO_GetReadDone( void );
int GECKO_GetSampleDone( void );
int GECKO_GetEraseDone( void );

int GECKO_ClearReadDone( void );
int GECKO_ClearSampleDone( void );
int GECKO_ClearEraseDone( void );

uint16_t GECKO_GetFlashCount( void );

uint32_t GECKO_GetImgData( void );

uint32_t GECKO_GetImageID( void );
int GECKO_SetImageID( uint32_t imageID );

uint32_t GECKO_GetFrameAmount( void );
int GECKO_SetFrameAmount( uint32_t frameAmount );

uint32_t GECKO_GetUpTime( void );

uint32_t GECKO_GetPageCount( void );

int GECKO_SensorOn( void );
int GECKO_SensorOff( void );

uint32_t GECKO_GetFrameRate( void );
int GECKO_SetFrameRate( uint32_t frameRate );

int GECKO_GetOnDone( void );
int GECKO_GetTrainingDone( void );
int GECKO_GetTrainingError( void );

int GECKO_SetAdcGain( uint8_t gain );
int GECKO_SetPgaGain( uint8_t gain );
int GECKO_SetOffset( uint16_t offset );

int GECKO_SetExposure( uint32_t exposure );

float GECKO_GetVoltageInput5V( void );
float GECKO_GetCurrentInput5V( void );

float GECKO_GetVoltageFPGA1V( void );
float GECKO_GetCurrentFPGA1V( void );

float GECKO_GetVoltageFPGA1V8( void );
float GECKO_GetCurrentFPGA1V8( void );

float GECKO_GetVoltageFPGA2V5( void );
float GECKO_GetCurrentFPGA2V5( void );

float GECKO_GetVoltageFPGA3V3( void );
float GECKO_GetCurrentFPGA3V3( void );

float GECKO_GetVoltageFlash1V8( void );
float GECKO_GetCurrentFlash1V8( void );

float GECKO_GetVoltageFlash3V3( void );
float GECKO_GetCurrentFlash3V3( void );

float GECKO_GetVoltageSNSR1V8( void );
float GECKO_GetCurrentSNSR1V8( void );

float GECKO_GetVoltageSNSRVDDPIX( void );
float GECKO_GetCurrentSNSRVDDPIX( void );

float GECKO_GetVoltageSNSR3V3( void );
float GECKO_GetCurrentSNSR3V3( void );

float GECKO_GetVoltageFlashVTT09( void );

float GECKO_GetTempSMU3AB( void );
float GECKO_GetTempSMU3BC( void );
float GECKO_GetTempREGU6( void );
float GECKO_GetTempREGU8( void );
float GECKO_GetTempFlash( void );

#endif /* SRC_SCS_GECKO_GECKO_DRIVER_H_ */
