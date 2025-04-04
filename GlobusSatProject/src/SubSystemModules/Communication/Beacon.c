#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <satellite-subsystems/isis_vu_e.h>

#include <hal/Timing/Time.h>
#include <hal/Storage/FRAM.h>

#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Maintenance/Log.h"
#include "TRXVU.h"
#include "GlobalStandards.h"
#include "FRAM_FlightParameters.h"
#include "SPL.h"
#include "Beacon.h"
#include "SatDataTx.h"


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

	logg(TRXInfo, "I: Setting Beacon Intervals to %d\n", g_beacon_interval_time);
}


//Beacon logic code
void BeaconLogic()
{
	logg(TRXInfo, "I:Inside BeaconLogic()\n");
	sat_packet_t packet;

	//check if not on mute and EPS has enough power
	if(CheckTransmitionAllowed())
	{
		//check if time to send beacon based on beacon interval
		if(CheckExecutionTime(g_prev_beacon_time, g_beacon_interval_time))
		{
			//get current Whole Orbit Data
			WOD_Telemetry_t *beacon = GetCurrentWODTelemetry();

			//create from WOD a packet to send to earth
			AssembleCommand(beacon->raw, sizeof(beacon->raw), trxvu_cmd_type, BEACON_SUBTYPE, 0, 0, 0, T3GBS, 1, &packet);

			// transmit packet to earth
			uint8_t availableFrames = 0;
			TransmitSplPacket(&packet, &availableFrames);

			memset(beacon, 0, sizeof(WOD_Telemetry_t));
			//set last beacon time
			Time_getUnixEpoch((unsigned int *)&g_prev_beacon_time);

			logg(event, "V: ### Beacon sent - id: %d data.length: %d\n",packet.ID_SAT, packet.length );
		}
		else
			logg(TRXInfo, "I: Beacon time did not arrive\n");
	}
	else
		logg(event, "V:---beacon NOT allowed\n");
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
		logg(error, "E: interval update to FRAM failed\n");
		return err;
	}
	else
		logg(TRXInfo, "I:interval was updated to FRAM successfully\n");

	//set new interval value
	g_beacon_interval_time = intrvl;
	return E_NO_SS_ERR;
}

