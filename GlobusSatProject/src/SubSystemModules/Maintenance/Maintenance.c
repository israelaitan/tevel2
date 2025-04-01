#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>

#include "GlobalStandards.h"


#include <hcc/api_fat.h>

#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "SubSystemModules/Communication/SubsystemCommands/Maintanence_Commands.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "TLM_management.h"
#include "Maintenance.h"
#include "Log.h"

Boolean CheckExecutionTime(time_unix prev_time, time_unix period)
{
	time_unix curr = 0;
	int err = Time_getUnixEpoch((unsigned int *)&curr);
	if(0 != err)
	{
		logg(error, "Time_getUnixEpoch failed\n");
		return FALSE;
	}

	if(curr - prev_time >= period)
	{
		logg(MTNInfo, "CheckExecutionTime return TRUE\n");
		return TRUE;
	}
	return FALSE;

}

Boolean CheckExecTimeFromFRAM(unsigned int fram_time_addr, time_unix period)
{
	int err = 0;
	time_unix prev_exec_time = 0;
	err = FRAM_read((unsigned char*)&prev_exec_time,fram_time_addr,sizeof(prev_exec_time));
	if(0 != err){
		return FALSE;
	}
	return CheckExecutionTime(prev_exec_time,period);
}

void SaveSatTimeInFRAM(unsigned int time_addr, unsigned int time_size)
{
	time_unix current_time = 0;
	Time_getUnixEpoch((unsigned int *)&current_time);

	FRAM_write((unsigned char*) &current_time, time_addr, time_size);
}

Boolean IsFS_Corrupted()
{
	FN_SPACE space;
	int drivenum = f_getdrive();

	f_getfreespace(drivenum, &space);

	if (space.bad > 0) {
		return TRUE;
	}
	return FALSE;
}

int WakeUpFromReset() {
	unsigned short num_of_resets = 0;
	FRAM_read((unsigned char*) &num_of_resets, NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);
	num_of_resets++;
	FRAM_write((unsigned char*) &num_of_resets, NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);

	unsigned int SATlastWakeUpTime = 0;
	FRAM_read((unsigned char*)&SATlastWakeUpTime, LAST_WAKEUP_TIME_ADDR, LAST_WAKEUP_TIME_SIZE);
	time_unix current_time = 0;
	Time_getUnixEpoch((unsigned int *)&current_time);
	Boolean turn_on_payload_in_init = FALSE;
	if(current_time - SATlastWakeUpTime < 60) {
		FRAM_write((unsigned char*) &turn_on_payload_in_init, TURN_ON_PAYLOAD_IN_INIT, TURN_ON_PAYLOAD_IN_INIT_SIZE);
		logg(event, "E:less than minute sequential resets 1=%d 2=%d dif=%d \n", SATlastWakeUpTime, current_time, current_time - SATlastWakeUpTime);
	}
	//set wakeup time in FRAM
	FRAM_write((unsigned char *)&current_time, LAST_WAKEUP_TIME_ADDR, LAST_WAKEUP_TIME_SIZE);
	return 0;
}

int WakeupFromResetCMD()
{
	int err = 0;
	unsigned char reset_flag = 0;
	unsigned short num_of_cmd_resets = 0;

	FRAM_read(&reset_flag, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE);
	if (reset_flag) {
		time_unix curr_time = 0;
		//set wakeup time in FRAM
		FRAM_read((unsigned char *)&curr_time, LAST_WAKEUP_TIME_ADDR, LAST_WAKEUP_TIME_SIZE);

		err = SendAckPacket(ACK_RESET_WAKEUP, 0xffff, 0xffff, (unsigned char*) &curr_time,
				sizeof(curr_time));

		reset_flag = FALSE_8BIT;
		FRAM_write(&reset_flag, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE);

		FRAM_read((unsigned char*) &num_of_cmd_resets, NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE);
		num_of_cmd_resets++;

		FRAM_write((unsigned char*) &num_of_cmd_resets, NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE);
	}

	return err;
}

void ResetGroundCommWDT()
{
	SaveSatTimeInFRAM(LAST_COMM_TIME_ADDR, LAST_COMM_TIME_SIZE);
}

// check if last communication with the ground station has passed WDT kick time
// and return a boolean describing it.
Boolean IsGroundCommunicationWDTKick()
{
	time_unix current_time = 0;
	Time_getUnixEpoch((unsigned int *)&current_time);

	//get last communication time and last wakeup time - take the latest of the two
	time_unix last_comm_time = 0, last_wake_time = 0;
	FRAM_read((unsigned char*) &last_comm_time, LAST_COMM_TIME_ADDR, LAST_COMM_TIME_SIZE);
	FRAM_read((unsigned char *)&last_wake_time, LAST_WAKEUP_TIME_ADDR, LAST_WAKEUP_TIME_SIZE);

	//if reset happened after last communication or time was moved back and last communication is in the future
	// - use wake up time as last communication time
	if ((last_wake_time > last_comm_time ) || (last_comm_time > current_time ))
	{
		last_comm_time = last_wake_time;
	}

	//get no communication max period
	time_unix wdt_kick_thresh = GetGsWdtKickTime();

	//if exceeded no communication period  - return true
	if (current_time - last_comm_time >= wdt_kick_thresh)
	{
		logg(event, "V: No communication with Earth was received for more then: %d \n", wdt_kick_thresh);
		return TRUE;
	}
	return FALSE;
}

int SetGsWdtKickTime(time_unix new_gs_wdt_kick_time)
{
	int err = FRAM_write((unsigned char*)&new_gs_wdt_kick_time, NO_COMM_WDT_KICK_TIME_ADDR,
		NO_COMM_WDT_KICK_TIME_SIZE);
	return err;
}

time_unix GetGsWdtKickTime()
{
	time_unix no_comm_thresh = 0;
	FRAM_read((unsigned char*)&no_comm_thresh, NO_COMM_WDT_KICK_TIME_ADDR, NO_COMM_WDT_KICK_TIME_SIZE);
	return no_comm_thresh;
}

void Maintenance()
{
	SaveSatTimeInFRAM(MOST_UPDATED_SAT_TIME_ADDR, MOST_UPDATED_SAT_TIME_SIZE);
	WakeupFromResetCMD();
	//initialize TLM periods
	InitTelemetryCollector();
	//reset if no communication for over a week
	if(IsGroundCommunicationWDTKick()) {
		logg(event, "Maintenance.Reseting ODBC");
		CMD_ResetComponent(reset_hardware);
	}


}
