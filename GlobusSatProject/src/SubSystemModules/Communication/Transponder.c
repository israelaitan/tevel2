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

int set_transonder_mode(Boolean mode)
{
	byte data[2];
	data[0] = 0x38;
	int err;
	Boolean transponder_mode_flag;
	if (mode)
	{
		printf("Transponder enabled\n");
		data[1] = 0x02;
		err = I2C_write(I2C_TRXVU_TC_ADDR, data, 2);
		transponder_mode_flag = TRUE;
		FRAM_write((unsigned char*)&transponder_mode_flag ,TRANSPONDER_STATE_ADDR, TRANSPONDER_STATE_SIZE);
	}
	else
	{
		printf("Transponder disabled\n");
		data[1] = 0x01;
		err =  I2C_write(I2C_TRXVU_TC_ADDR, data, 2);
		transponder_mode_flag = TRUE;
		FRAM_write((unsigned char*) &transponder_mode_flag ,TRANSPONDER_STATE_ADDR, TRANSPONDER_STATE_SIZE);
	}
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

int transponder_logic(time_unix period)
{
	int err;
	time_unix turn_on_time;
	byte rssiData[2];
	err = Time_getUnixEpoch(&turn_on_time);
	if (0 != err)
	{
		return err;
	}
	FRAM_write((unsigned char*) &turn_on_time ,TRANSPONDER_TURN_ON_TIME_ADRR, TRANSPONDER_TURN_ON_TIME_SIZE);

	Boolean mute = GetMuteFlag();
	if(!mute)
	{
		err = set_transonder_mode(TURN_TRANSPONDER_ON);
		if (0 != err)
		{
			return err;
		}
		for(;;)
		{
			unsigned short temp = DEFAULT_TRANS_RSSI; // TODO: change to 200
			memcpy(rssiData, &temp, 2);
			set_transponder_RSSI(rssiData);
			Boolean stop_transponder = CheckExecutionTime(turn_on_time, period);
			if (stop_transponder)
			{
				break;
			}
		}
		err = set_transonder_mode(TURN_TRANSPONDER_OFF);
	}
	return err;
}

