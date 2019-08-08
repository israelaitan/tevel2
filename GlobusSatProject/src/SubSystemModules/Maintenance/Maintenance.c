#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>

#include "GlobalStandards.h"

#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>

#include <hcc/api_fat.h>

#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "TLM_management.h"
#include "Maintenance.h"

Boolean CheckExecutionTime(time_unix prev_time, time_unix period)
{
	return FALSE;
}

Boolean CheckExecTimeFromFRAM(unsigned int fram_time_addr, time_unix period)
{
	return FALSE;
}

void SaveSatTimeInFRAM(unsigned int time_addr, unsigned int time_size)
{
}

Boolean IsFS_Corrupted()
{
	return FALSE;
}

int WakeupFromResetCMD()
{
	return 0;
}

void ResetGroundCommWDT()
{
	unsigned int now = 0;
	int err = Time_getUnixEpoch(&now);

	err = FRAM_read((unsined char *)&now, LAST_COMM_TIME_ADDR, LAST_COMM_TIME_SIZE);
}

// check if last communication with the ground station has passed WDT kick time
// and return a boolean describing it.
Boolean IsGroundCommunicationWDTKick()
{
	unsigned int now = 0;
	int err = Time_getUnixEpoch(&now);

	unsigned char noCommPeriod[NO_COMM_WDT_KICK_TIME_SIZE];
	err = FRAM_read(noCommPeriod, NO_COMM_WDT_KICK_TIME_ADDR, NO_COMM_WDT_KICK_TIME_SIZE);

	unsigned char lastComm[LAST_COMM_TIME_SIZE];
	err = FRAM_read(lastComm, LAST_COMM_TIME_ADDR, LAST_COMM_TIME_SIZE);

	return (lastComm + noCommPeriod) <= now;
}

//TODO: add to command dictionary
int SetGsWdtKickTime(time_unix new_gs_wdt_kick_time)
{
	return 0;
}

time_unix GetGsWdtKickTime()
{
	time_unix no_comm_thresh = 0;
	return no_comm_thresh;

}

void Maintenance()
{
}
