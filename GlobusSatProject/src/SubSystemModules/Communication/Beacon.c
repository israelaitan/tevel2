#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <satellite-subsystems/IsisTRXVU.h>

#include <hal/Timing/Time.h>
#include <hal/Storage/FRAM.h>

#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Maintenance/Maintenance.h"

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
	#ifdef TESTING
		printf("Inside InitBeaconParams()\n");
	#endif

	//TODO: Fix code to initialize to default after FRAM read
	g_beacon_interval_time = DEFAULT_BEACON_INTERVAL_TIME;
	FRAM_read((unsigned char*)&g_beacon_interval_time, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE );
}


//Beacon logic code
void BeaconLogic()
{
	#ifdef TESTING
		printf("Inside BeaconLogic()\n");
	#endif
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
			AssembleCommand((unsigned char *)&wod, sizeof(wod), trxvu_cmd_type, BEACON_SUBTYPE, id, 0, &packet);

			// transmit packet to earth
			TransmitSplPacket(&packet, NULL);

			//set last beacon time
			Time_getUnixEpoch((unsigned int *)&g_prev_beacon_time);

			#ifdef TESTING
				printf("beacon sent - id: %d data: %s\n",packet.ID, packet.data );
			#endif
		}
		#ifdef TESTING
			else
				printf("beacon time did not arrive\n");
		#endif

	}
	#ifdef TESTING
		else
			printf("beacon not allowed\n");
	#endif

}

//Update beacon intervals - allows changing this interval from earth
int UpdateBeaconInterval(time_unix intrvl)
{
	//TODO: remove print after testing complete
	printf("Inside UpdateBeaconInterval() interval: %ld, max: %d, min: %d\n" , intrvl, MAX_BEACON_INTERVAL, MIN_BEACON_INTERVAL);

	//check if interval is in allowed range
	if(intrvl>MAX_BEACON_INTERVAL|| intrvl<MIN_BEACON_INTERVAL)
		return E_PARAM_OUTOFBOUNDS;

	//update new interval in FRAM
	int err = FRAM_write((unsigned char *)&intrvl, BEACON_INTERVAL_TIME_ADDR, BEACON_INTERVAL_TIME_SIZE );
	if (err!=0)
	{
		printf("interval update to FRAM failed\n");
		return err;
	}
	else
	{
		//TODO: remove print after testing complete
		printf("interval was updated to FRAM successfully\n");
	}

	//set new interval value
	g_beacon_interval_time = intrvl;
	return E_NO_SS_ERR;
}

