#include "FS_Commands.h"
#include <hcc/api_fat.h>
#include <string.h>
#include "GlobalStandards.h"
#include "TLM_management.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include <hal/Timing/Time.h>

int CMD_DeleteFileByTime(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;

	if(cmd == NULL || cmd->data == NULL)
	{
		return E_INPUT_POINTER_NULL;
	}

	//get file type
	int fileType;
	memcpy(&fileType, cmd->data, sizeof(fileType));

	//get file name
	char filename[MAX_F_FILE_NAME_SIZE];
	err = GetTelemetryFilenameByType(fileType, filename);

	if(err == 0)
	{
		//get from time
		time_unix fromTime;
		memcpy(&fromTime, cmd->data, sizeof(fromTime));

		//get to time
		time_unix toTime;
		memcpy(&toTime, cmd->data+sizeof(fromTime), sizeof(toTime));


		//delete all file types
		err = c_fileDeleteElements(filename, fromTime, toTime);
	}

	return err;
}

int CMD_DeleteFilesOfType(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;

	if(cmd == NULL || cmd->data == NULL)
	{
		return E_INPUT_POINTER_NULL;
	}

	//get file type
	int fileType;
	memcpy(&fileType, cmd->data, sizeof(fileType));

	//get file name
	char filename[MAX_F_FILE_NAME_SIZE];
	err = GetTelemetryFilenameByType(fileType, filename);

	if(err == 0)
	{
		//get current time
		time_unix toTime;
		Time_getUnixEpoch((unsigned int *)&toTime);

		//delete all file types
		err = c_fileDeleteElements(filename, 0, toTime);
	}

	return err;
}

int CMD_DeleteFS(sat_packet_t *cmd)
{
	(void)cmd;
	delete_allTMFilesFromSD();
	return 0;
}

int CMD_GetLastFS_Error(sat_packet_t *cmd)
{
	(void)cmd;
	int err = f_getlasterror();

	//update data
	TransmitDataAsSPL_Packet(cmd, (unsigned char*) &err, sizeof(err));

	return 0;
}

int CMD_FreeSpace(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	FN_SPACE space;
	int drivenum = f_getdrive();
	f_getfreespace(drivenum, &space);

	//send data
	TransmitDataAsSPL_Packet(cmd, (unsigned char*) &(space.free),sizeof(space.free));
	return err;
}

int CMD_isFS_Corrupt(sat_packet_t *cmd)
{
	(void)cmd;
	Boolean err = IsFS_Corrupted();

	//update data
	TransmitDataAsSPL_Packet(cmd, (unsigned char*) &err,sizeof(err));

	return 0;
}
