#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Storage/FRAM.h>
#include <hal/Timing/Time.h>
#include <hal/errors.h>

#include <satellite-subsystems/isis_vu_e.h>
#include "SubSystemModules/Maintenance/Log.h"
#include "Transponder.h"
#include "GlobalStandards.h"
#include "SatDataTx.h"

Boolean 		g_mute_flag = MUTE_OFF;				// mute flag - is the mute enabled
time_unix 		g_mute_end_time = 0;				// time at which the mute will end

xSemaphoreHandle xIsTransmitting = NULL; // mutex on transmission.

void InitTxModule()
{
	if(NULL == xIsTransmitting)
		vSemaphoreCreateBinary(xIsTransmitting);

	int err = 0;
	err = FRAM_read((unsigned char*) &g_mute_flag ,MUTE_FLAG_ADRR, MUTE_FLAG_SIZE);
	if (err != 0)
	{
		g_mute_flag = MUTE_OFF;				// mute flag - set to false
		g_mute_end_time = 0;
	}
	else //err == 0
	{
		err = FRAM_read((unsigned char*) &g_mute_end_time ,MUTE_ON_END_TIME_ADRR, MUTE_ON_END_TIME_SIZE);
		if (err != 0)
		{
			g_mute_end_time = 0;
		}
	}


}

int muteTRXVU(time_unix duration)
{
	if (duration > MAX_MUTE_TIME) {
		return -2;
	}

	time_unix curr_tick_time = 0;
	Time_getUnixEpoch((unsigned int *)&curr_tick_time);

	g_mute_end_time = curr_tick_time + duration;
	g_mute_flag = MUTE_ON;

	FRAM_write((unsigned char*) &g_mute_end_time ,MUTE_ON_END_TIME_ADRR, MUTE_ON_END_TIME_SIZE);
	FRAM_write((unsigned char*) &g_mute_flag ,MUTE_FLAG_ADRR, MUTE_FLAG_SIZE);

	logg(TRXInfo, "I: Setting Mute ON until: %lu\n", (long unsigned int)g_mute_end_time);

	//close transponder
	CMD_turnOffTransponder();
	return 0;
}


void UnMuteTRXVU()
{
	g_mute_end_time = 0;
	g_mute_flag = MUTE_OFF;

	FRAM_write((unsigned char*) &g_mute_end_time ,MUTE_ON_END_TIME_ADRR, MUTE_ON_END_TIME_SIZE);
	FRAM_write((unsigned char*) &g_mute_flag ,MUTE_FLAG_ADRR, MUTE_FLAG_SIZE);

	logg(TRXInfo, "I: Setting Mute to OFF\n");
}

Boolean GetMuteFlag() {
	return g_mute_flag;
}

Boolean CheckForMuteEnd() {
	time_unix curr_tick_time = 0;
	Time_getUnixEpoch((unsigned int *)&curr_tick_time);
	if (curr_tick_time > g_mute_end_time)
	{
		logg(TRXInfo, "I:Mute End time reached\n");
		return TRUE;
	}
	else
	{
		logg(TRXInfo, "I:Mute End time NOT reached\n");
		return FALSE;
	}

}

int GetNumberOfFramesInBuffer() {
	uint16_t frame_count_out = 0;
	int err = isis_vu_e__get_frame_count(ISIS_TRXVU_I2C_BUS_INDEX, &frame_count_out);
	if (0 != err)
		return -1;
	return frame_count_out;
}

Boolean IsTransmitting() {
	if(pdTRUE == xSemaphoreTake(xIsTransmitting,0)){
		xSemaphoreGive(xIsTransmitting);
		return FALSE;
	}
	return TRUE;
}

Boolean CheckTransmitionAllowed() {


	if (g_mute_flag == MUTE_OFF )
	{
		if(pdTRUE == xSemaphoreTake(xIsTransmitting,0))
		{
			xSemaphoreGive(xIsTransmitting);
			logg(TRXInfo, "I:TRASMITION ALLOWED\n");
			return TRUE;
		}
	}
	logg(TRXInfo, "I:TRASMITION NOT ALLOWED\n");
	return FALSE;
}

int GetTrxvuBitrate(isis_vu_e__bitrate_t *bitrate) {

	if (NULL == bitrate)
		return E_NOT_INITIALIZED;

	isis_vu_e__state__from_t trxvu_state;
	int err = isis_vu_e__state(ISIS_TRXVU_I2C_BUS_INDEX, &trxvu_state);

	if (E_NO_SS_ERR == err)
		*bitrate = trxvu_state.fields.bitrate;

	return err;

}

int TransmitDataAsSPL_Packet(sat_packet_t *cmd, unsigned char *data, unsigned int length) {
	int err = 0;
	sat_packet_t packet = { 0 };
	if (NULL != cmd) {
		err = AssembleCommand(data, length, cmd->cmd_type, cmd->cmd_subtype,
				cmd->ID, cmd->ordinal, cmd->targetSat, cmd->total, &packet);
	} else {
		err = AssembleCommand(data, length, 0xFF, 0xFF, 0xFFFF, 0, cmd->targetSat, 1, &packet);
	}
	if (err != 0) {
		return err;
	}
	err = TransmitSplPacket(&packet, NULL);
	return err;
}

int TransmitSplPacket(sat_packet_t *packet, int *avalFrames) {
	if (!CheckTransmitionAllowed())
		return -1;

	if (NULL == packet)
		return E_NOT_INITIALIZED;

	int err = 0;
	unsigned char data_length = packet->length + SAT_PACKET_HEADER_LENGTH;

	if (xSemaphoreTake(xIsTransmitting,SECONDS_TO_TICKS(1)) != pdTRUE)
		return E_GET_SEMAPHORE_FAILED;

	err = isis_vu_e__send_frame(ISIS_TRXVU_I2C_BUS_INDEX,
			(unsigned char*) packet, data_length, (unsigned char*) avalFrames);

	xSemaphoreGive(xIsTransmitting);

	return err;
}

