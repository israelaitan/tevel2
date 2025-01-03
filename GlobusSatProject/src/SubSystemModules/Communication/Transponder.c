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

#include <satellite-subsystems/isis_vu_e.h>
#include <satellite-subsystems/isis_ants.h>

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
#include "SubSystemModules/Maintenance/Log.h"

//global variables
static Boolean g_transp_mode=TURN_TRANSPONDER_OFF;
static time_unix g_transp_end_time=0;

#define TRXVU_NOMINAL_MODE 1
#define TRXVU_TRANSPONDER_MODE 2

void initTransponder()
{
	int err = 0;
	err = FRAM_read((unsigned char*) &g_transp_mode ,TRANSPONDER_STATE_ADDR, TRANSPONDER_STATE_SIZE);
	if(err !=0)
	{
		g_transp_mode = TURN_TRANSPONDER_OFF;
		g_transp_end_time=0;
	}
	else if (g_transp_mode == TURN_TRANSPONDER_ON)
	{
		time_unix duration;
		err = FRAM_read((unsigned char*) &g_transp_end_time ,TRANSPONDER_TURN_ON_END_TIME_ADRR, TRANSPONDER_TURN_ON_END_TIME_SIZE);

		if(err !=0)
		{
			g_transp_end_time=0;
			duration = DEFAULT_TRANSPONDER_DURATION;
		}
		else
		{
			time_unix curr = 0;
			Time_getUnixEpoch((unsigned int *)&curr);
			duration = curr - g_transp_end_time;
		}


		//turn on transponder
		sat_packet_t cmd;
		memcpy(cmd.data, &duration, sizeof(duration));
		CMD_turnOnTransponder(&cmd);
	}

}

int set_transonder_mode(uint8_t mode) {
	int err = isis_vu_e__set_tx_mode(0, mode);
	if (err) {
		logg(error, "E: set_transonder_mode to: %b  with err: %d\n", mode, err);
		return err;
	}

	g_transp_mode = TRUE;
	if (mode == TRXVU_NOMINAL_MODE)
		g_transp_mode = FALSE;

	FRAM_write((unsigned char*) &g_transp_mode ,TRANSPONDER_STATE_ADDR, TRANSPONDER_STATE_SIZE);
	return err;
}

int set_transonder_mode_i2c(Boolean mode)
{
	byte data[2];
	data[0] = 0x38;
	int err;

	if (mode) {
		logg(TRXInfo, "I:Transponder enabled\n");
		data[1] = 0x02;
	} else {
		logg(TRXInfo, "I: Transponder disabled\n");
		data[1] = 0x01;
	}

	err =  I2C_write(I2C_TRXVU_TC_ADDR, data, 2);

	if (err == 0) {
		g_transp_mode = mode;
		FRAM_write((unsigned char*) &g_transp_mode ,TRANSPONDER_STATE_ADDR, TRANSPONDER_STATE_SIZE);
	}
	else
		logg(error, "E: Failed setting transponder to: %b  with err: %d\n", mode, err);

	return err;
}
/*
RSSI threshold value. This value will be used as the new RSSI
threshold for the transponder chain activation. The value received
here will be the raw value, therefore between 0 and 4095. The most
significant byte is transmitted first (big endian).
*/
int set_transponder_RSSI(byte *param)
{
	byte data[3];
	data[0] = 0x52;
	data[1] = param[1];//big endian
	data[2] = param[0];

	int err = I2C_write(I2C_TRXVU_TC_ADDR, data, 2);
	return err;
}

int CMD_turnOnTransponder(sat_packet_t *cmd)
{
	int err = 0;
	byte rssiData[2];
	time_unix duration;

	if(cmd == NULL || cmd->data == NULL){
		logg(error, "E: Input is NULL");
		return E_INPUT_POINTER_NULL;
	}

	Boolean mute = GetMuteFlag();
	if(mute) {
		logg(event, "V: MUTED No Transponder\n");
		return err;
	}

	duration = getDuration(cmd);
	logg(event, "V: Inside CMD_turnOnTransponder duration=%lu\n", duration);
	CMD_SetIdleOff();

	//set RSSI
	unsigned short rssi;
	err = FRAM_read((unsigned char*) &rssi ,TRANSPONDER_RSSI_ADDR, TRANSPONDER_RSSI_SIZE);

	if(err) {
		rssi = DEFAULT_TRANS_RSSI;
		logg(error, "E: Transponder RSSI not found in FRAM - Using default");
	} else
		logg(event, "V: Transponder RSSI is: %d\n", rssi);

	memcpy(rssiData, &rssi, TRANSPONDER_RSSI_SIZE);
	err = set_transponder_RSSI(rssiData);
	if (err) {
		CMD_turnOffTransponder();
		return err;
	}

	uint32_t freq_in = TRXVU_RX_FREQ;//145970
	err = isis_vu_e__set_transponder_in_freq(0, freq_in);
	if (err)
		return err;
	vTaskDelay(100);
	isis_vu_e__get_transponder_in_freq__from_t response;
	err = isis_vu_e__get_transponder_in_freq(0, &response);
	if (!err) {
		logg(event, "V:isis_vu_e__get_transponder_in_freq=%d succeeded\n", response.fields.freq);
	} else
		logg(error, "E:isis_vu_e__get_transponder_in_freq=%d failed\n", err);

	//err = set_transonder_mode(TURN_TRANSPONDER_ON);
	err = set_transonder_mode(TRXVU_TRANSPONDER_MODE);
	if (err)
		return err;

	//Set transponder end time
	err = Time_getUnixEpoch(&g_transp_end_time);
	if (err)
		return err;
	g_transp_end_time += duration;
	FRAM_write((unsigned char*) &g_transp_end_time ,TRANSPONDER_TURN_ON_END_TIME_ADRR, TRANSPONDER_TURN_ON_END_TIME_SIZE);
	logg(event, "V: Turned On Transponder until: %lu\n", (long unsigned int)g_transp_end_time);

	return err;
}

//set RSSI Data from ground control
int CMD_set_transponder_RSSI(sat_packet_t *cmd)
{
	logg(TRXInfo, "I: Inside CMD_set_transponder_RSSI()\n");
	int err;

	if(cmd == NULL || cmd->data == NULL){
		logg(error, "E: Input is NULL");
		return E_INPUT_POINTER_NULL;
	}

	//set RSSI
	byte rssiData[2];
	memcpy(rssiData, cmd->data, TRANSPONDER_RSSI_SIZE);
	err = set_transponder_RSSI(rssiData);

	if (err == 0)
		FRAM_write((unsigned char*) &(cmd->data) ,TRANSPONDER_RSSI_ADDR, TRANSPONDER_RSSI_SIZE);
	else {
		CMD_turnOffTransponder();
		return err;
	}

	return err;
}

int CMD_turnOffTransponder()
{
	int err = err = set_transonder_mode(TRXVU_NOMINAL_MODE);
	logg(event, "V: Setting Transponder to OFF err=%d\n", err);
	return err;
}

Boolean getTransponderMode()
{
	return g_transp_mode;
}

Boolean checkEndTransponderMode()
{
	logg(TRXInfo, "I: Inside checkEndTransponderMode()\n");
	time_unix curr_tick_time = 0;
	Time_getUnixEpoch((unsigned int *)&curr_tick_time);
	if (curr_tick_time > g_transp_end_time) {
		logg(TRXInfo, "I: Transponder End time reached\n");
		return TRUE;
	}
	else {
		logg(TRXInfo, "I: Transponder End time NOT reached\n");
		return FALSE;
	}
}

int getDuration(sat_packet_t *cmd)
{
	time_unix duration;

	//getting the duration
	memcpy(&duration,cmd->data,sizeof(duration));

	if(duration > TRANSPONDER_MAX_DURATION) {
		duration = TRANSPONDER_MAX_DURATION;
		logg(TRXInfo, "I: Transponder duration exceeded Max duration. Setting Max duration");
	}

	logg(TRXInfo, "I: Transponder duration: %d \n", duration);

	return duration;
}



