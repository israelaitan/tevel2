#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>
#include <hal/errors.h>

#include <satellite-subsystems/IsisTRXVU.h>

#include <stdlib.h>
#include <string.h>

#include "GlobalStandards.h"
#include "TRXVU.h"
#include "AckHandler.h"
#include "ActUponCommand.h"
#include "SatCommandHandler.h"
#include "TLM_management.h"

#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#ifdef TESTING_TRXVU_FRAME_LENGTH
#include <hal/Utility/util.h>
#endif
#define SIZE_RXFRAME	200
#define SIZE_TXFRAME	235
#define QUEUE_LENGTH	16
#define QUEUE_ITEM_SIZE	4

Boolean 		g_mute_flag = MUTE_OFF;				// mute flag - is the mute enabled
time_unix 		g_mute_end_time = 0;				// time at which the mute will end
time_unix 		g_prev_beacon_time = 0;				// the time at which the previous beacon occured
time_unix 		g_beacon_interval_time = 0;			// seconds between each beacon
unsigned char	g_current_beacon_period = 0;		// marks the current beacon cycle(how many were transmitted before change in baud)
unsigned char 	g_beacon_change_baud_period = 0;	// every 'g_beacon_change_baud_period' beacon will be in 1200Bps and not 9600Bps

xQueueHandle xDumpQueue = NULL;
xSemaphoreHandle xDumpLock = NULL;
xTaskHandle xDumpHandle = NULL;			 //task handle for dump task
xSemaphoreHandle xIsTransmitting = NULL; // mutex on transmission.

void InitSemaphores()
{
	xDumpQueue = xQueueCreate(QUEUE_LENGTH, QUEUE_ITEM_SIZE);
	vSemaphoreCreateBinary(xDumpLock);
	vSemaphoreCreateBinary(xIsTransmitting);
}

int InitTrxvu() {
	//init ants
	ISIStrxvuBitrateStatus bps = trxvu_bitrate_9600;
	ISIStrxvuFrameLengths frameLengths;
	frameLengths.maxAX25frameLengthRX = SIZE_RXFRAME;
	frameLengths.maxAX25frameLengthTX = SIZE_TXFRAME;
	ISIStrxvuI2CAddress i2cAddr;
	i2cAddr.addressVu_rc = I2C_TRXVU_RC_ADDR;
	i2cAddr.addressVu_tc = I2C_TRXVU_TC_ADDR;
	int err = IsisTrxvu_initialize(&i2cAddr, &frameLengths, &bps, 1);
	//delay 100
	InitSemaphores();
	//read fram beacon bitrate
	return err;
}

int TRX_Logic() {
	int err = 0;
	sat_packet_t packet = { 0 };
	int nFrames = GetNumberOfFramesInBuffer();
	if ( nFrames > 0 ) {
		err = GetOnlineCommand( &packet );
		ResetGroundCommWDT();
		if (err == 0) {
			err = ActUponCommand(packet);
			SendAckPacket(ACK_RECEIVE_COMM, &packet, &err, sizeof(int));
		}

	} else {
		//TODO: get delayed command will decide if execution time is now and will response accordingly
		int nDelayed = GetDelayedCommandBufferCount();
		if ( nDelayed > 0 ) {
				err = GetDelayedCommand( &packet );
				if (err == command_found)
					err = ActUponCommand(packet);
		}
	}
    BeaconLogic();
	return err;
}

int GetNumberOfFramesInBuffer() {
	return 0;
}

Boolean CheckTransmitionAllowed() {
	return FALSE;
}


void FinishDump(dump_arguments_t *task_args,unsigned char *buffer, ack_subtype_t acktype,
		unsigned char *err, unsigned int size) {
}

void AbortDump()
{
}

void SendDumpAbortRequest() {
}

Boolean CheckDumpAbort() {
	return FALSE;
}

void DumpTask(void *args) {
}

int DumpTelemetry(sat_packet_t *cmd) {
	return 0;
}

//Sets the bitrate to 1200 every third beacon and to 9600 otherwise
int BeaconSetBitrate() {
	return 0;
}

void BeaconLogic() {
	unsigned int now = 0;
	int err = Time_getUnixEpoch( &now );
	if (err)
		return;
	if ( (now - g_prev_beacon_time) < g_beacon_interval_time * 1000)
		return;

	WOD_Telemetry_t wod = { 0 };
	GetCurrentWODTelemetry( &wod );
	BeaconSetBitrate();
	sat_packet_t packet;
	packet.cmd_type = telemetry_cmd_type;
	packet.cmd_subtype = BEACON_SUBTYPE;
	packet.data = wod;
	packet.length = sizeof(wod);
	err = TransmitSplPacket(packet, NULL);
	//TODO: log error
	//TODO: check not bigger than 200 chars
	g_prev_beacon_time = now;
	IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_bitrate_9600);
	//TODO: check if need to change bitrate
}

int muteTRXVU(time_unix duration) {
	unsigned int now = 0;
	Time_getUnixEpoch(now);
	g_mute_end_time = now + duration;
	SetMuteFlag(TRUE);
	return 0;
}

void UnMuteTRXVU() {
	g_mute_end_time = 0;
	g_mute_flag = FALSE;
}

Boolean GetMuteFlag() {
	Boolean ended = CheckForMuteEnd();
	if (ended)
		UnMuteTRXVU();
	return g_mute_flag;
}

Boolean CheckForMuteEnd() {
	unsigned int now = 0;
	Time_getUnixEpoch(now);
	if (now >= g_mute_end_time) {
		return TRUE;
	} else {
		return FALSE;
	}
}

int GetTrxvuBitrate(ISIStrxvuBitrateStatus *bitrate) {
	return 0;
}

int TransmitDataAsSPL_Packet(sat_packet_t *cmd, unsigned char *data,
		unsigned int length) {
	return 0;
}

int TransmitSplPacket(sat_packet_t *packet, int *avalFrames) {
	unsigned char avail = 0;//TODO:
	int realSize = sizeof(sat_packet_t) - MAX_COMMAND_DATA_LENGTH + packet->length;
	//TODO: semapfhors
	int err = IsisTrxvu_tcSendAX25DefClSign(ISIS_TRXVU_I2C_BUS_INDEX, packet, realSize, &avail);
	return err;
}

int UpdateBeaconBaudCycle(unsigned char cycle)
{
	return 0;
}

int UpdateBeaconInterval(time_unix intrvl) {
	return 0;
}
