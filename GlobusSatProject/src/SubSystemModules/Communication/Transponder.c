/*
 * Transponder.c
 *
 *  Created on: May 6, 2020
 *      Author: User
 */


#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>
#include <hal/errors.h>
#include <hal/Drivers/I2C.h>

#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>

#include <stdlib.h>
#include <string.h>

#include "GlobalStandards.h"
#include "TRXVU.h"
#include "Transponder.h"
#include "AckHandler.h"
#include "ActUponCommand.h"
#include "SatCommandHandler.h"
#include "TLM_management.h"

#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "SubSystemModules/Communication/Beacon.h"

#define TURN_TRANSPONDER_OFF FALSE
#define TURN_TRANSPONDER_ON TRUE
#define DEFAULT_TRANS_RSSI 200

//global variables
time_unix g_transp_end_time;
Boolean g_transp_mode;

int set_transonder_mode(Boolean mode)
{
	byte data[2];
	data[0] = 0x38;
	int err;

	if (mode)
	{
		printf("Transponder enabled\n");
		data[1] = 0x02;
	}
	else
	{
		printf("Transponder disabled\n");
		data[1] = 0x01;
	}

	g_transp_mode = mode;
	err =  I2C_write(I2C_TRXVU_TC_ADDR, data, 2);
	FRAM_write((unsigned char*) &g_transp_mode ,TRANSPONDER_STATE_ADDR, TRANSPONDER_STATE_SIZE);

	return err;
}

int set_transponder_RSSI(byte *param)
{
	byte data[3];
	data[0] = 0x52;
	data[1] = param[0];
	data[2] = param[1];

	int err = I2C_write(I2C_TRXVU_TC_ADDR, data, 2);
	return err;
}

int CMD_turnOnTransponder(time_unix duration)
{
	int err;
	byte rssiData[2];

	Boolean mute = GetMuteFlag();
	if(!mute)
	{
		//turn off idle
		CMD_SetIdleOff();

		//turn on tarnsponder mode
		err = set_transonder_mode(TURN_TRANSPONDER_ON);
		if (0 != err)
		{
			return err;
		}

		//set RSSI
		unsigned short temp = DEFAULT_TRANS_RSSI;
		memcpy(rssiData, &temp, 2);
		err = set_transponder_RSSI(rssiData);
		if (0 != err)
		{
			CMD_turnOffTransponder();
			return err;
		}

		//Set transponder end time
		err = Time_getUnixEpoch(&g_transp_end_time);
		if (0 != err)
		{
			return err;
		}

		//add duration to current time
		g_transp_end_time += duration;

		FRAM_write((unsigned char*) &g_transp_end_time ,TRANSPONDER_TURN_ON_TIME_ADRR, TRANSPONDER_TURN_ON_TIME_SIZE);
		printf("*Turned On Transponder until: %lu\n", (long unsigned int)g_transp_end_time);
	}

	return err;
}


int CMD_turnOffTransponder()
{
	g_transp_end_time = 0;
	int err = set_transonder_mode(TURN_TRANSPONDER_OFF);
	printf("*********************Setting Transponder to OFF\n");
	return err;
}

Boolean getTransponderMode()
{
	return g_transp_mode;
}

Boolean checkEndTransponderMode()
{
	time_unix curr_tick_time = 0;
	Time_getUnixEpoch((unsigned int *)&curr_tick_time);
	if (curr_tick_time > g_transp_end_time)
	{
		printf("Transponder End time reached\n");
		return TRUE;
	}
	else
	{
		printf("Transponder End time NOT reached\n");
		return FALSE;
	}
}



