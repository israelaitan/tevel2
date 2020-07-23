#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <satellite-subsystems/IsisTRXVU.h>

#include <hal/Timing/Time.h>
#include <hal/Storage/FRAM.h>

#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Maintenance/Log.h"
#include "GlobalStandards.h"
#include "FRAM_FlightParameters.h"
#include "SPL.h"
#include "Beacon.h"
#include "SatDataTx.h"

#define BEACON_ID 0xFFFFFFFF

time_unix g_prev_beacon_time = 0;				// the time at which the previous beacon occured
time_unix g_beacon_interval_time = 0;

//Initialize beacon parameters
void InitBeaconParams()
{
	logg(TRXInfo, "I: Inside InitBeaconParams()\n");

	//get interval from RAM
	int status = FRAM_read((unsigned char*)&g_beacon_interval_time, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE );

	//if failed to read from FRAM
	if(status != 0)
	{
		g_beacon_interval_time = DEFAULT_BEACON_INTERVAL_TIME;
	}
}


//Beacon logic code
void BeaconLogic()
{

	logg(TRXInfo, "I:Inside BeaconLogic()\n");
	sat_packet_t packet;
	WOD_Telemetry_t wod = { 0 };
	unsigned int id=  BEACON_ID;

	//check if not on mute and EPS has enough power
	if(CheckTransmitionAllowed())
	{
		//check if time to send beacon based on beacon interval
		if(CheckExecutionTime(g_prev_beacon_time, g_beacon_interval_time))
		{
			//get current Whole Orbit Data
			GetCurrentWODTelemetry(&wod);

			//create from WOD a packet to send to earth
			AssembleCommand((unsigned char *)&wod, sizeof(wod), trxvu_cmd_type, BEACON_SUBTYPE, id, 0, 8, &packet);

			// transmit packet to earth
			int availableFrames = 0;
			TransmitSplPacket(&packet, &availableFrames);

			//set last beacon time
			Time_getUnixEpoch((unsigned int *)&g_prev_beacon_time);

			logg(TRXInfo, "I:++++++++++++++++++++++++++++++++beacon sent - id: %d data: %s\n",packet.ID, packet.data );
		}
		else
			logg(TRXInfo, "I:beacon time did not arrive\n");
	}
	else
		logg(TRXInfo, "I:____________________---__________-beacon not allowed\n");
}

//Update beacon intervals - allows changing this interval from earth
int UpdateBeaconInterval(sat_packet_t *cmd)
{

	time_unix intrvl = 0;
	memcpy(&intrvl,cmd->data,sizeof(intrvl));
	logg(TRXInfo, "I:Inside UpdateBeaconInterval() interval: %d, max: %d, min: %d\n" , intrvl, MAX_BEACON_INTERVAL, MIN_BEACON_INTERVAL);

	//check if interval is in allowed range
	if(intrvl > MAX_BEACON_INTERVAL|| intrvl < MIN_BEACON_INTERVAL)
		return E_PARAM_OUTOFBOUNDS;

	//update new interval in FRAM
	int err = FRAM_write((unsigned char *)&intrvl, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE );
	if (err!=0)
	{
		logg(error, "E:interval update to FRAM failed\n");
		return err;
	}
	else
		logg(TRXInfo, "I:interval was updated to FRAM successfully\n");

	//set new interval value
	g_beacon_interval_time = intrvl;
	return E_NO_SS_ERR;
}

