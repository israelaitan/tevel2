#include "FS_Commands.h"
#include "GlobalStandards.h"
#include "TLM_management.h"
#include "SubSystemModules/Housekepping/DUMP.h"
#include "SubSystemModules/Communication/SubsystemCommands/FS_Commands.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "Utils.h"
#include <hcc/api_fat.h>
#include <hal/Timing/Time.h>
#include <String.h>

int CMD_DeleteTLM(sat_packet_t *cmd)
{
	if (NULL == cmd) {
		return -1;
	}

	dump_arguments_t dmp_pckt;
	unsigned int offset = 0;

	AssembleCommand(cmd->data,cmd->length,cmd->cmd_type,cmd->cmd_subtype,cmd->ID,  1, 8, dmp_pckt.cmd);

	memcpy(&dmp_pckt.dump_type, cmd->data, sizeof(dmp_pckt.dump_type));
	offset += sizeof(dmp_pckt.dump_type);

	memcpy(&dmp_pckt.t_start, cmd->data + offset, sizeof(dmp_pckt.t_start));
	offset += sizeof(dmp_pckt.t_start);

	memcpy(&dmp_pckt.t_end, cmd->data + offset, sizeof(dmp_pckt.t_end));
	offset += sizeof(dmp_pckt.t_end);


	// calculate how many days we were asked to dump (every day has 86400 seconds)
	int numberOfDays = (dmp_pckt.t_end - dmp_pckt.t_start)/86400;
	Time date;
	timeU2time(dmp_pckt.t_start,&date);
	int numOfElementsSent = deleteTLMFiles(dmp_pckt.dump_type,date,numberOfDays);
	SendAckPacket(ACK_DELETE_TLM, cmd, cmd->data, sizeof(numOfElementsSent));
	return 0;
}


int CMD_DeleteFileByTime(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	return err;
}

int CMD_DeleteFilesOfType(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	return err;
}


int CMD_DeleteAllFiles(sat_packet_t *cmd)
{
	delete_allTMFilesFromSD();
	SendAckPacket(ACK_COMD_EXEC,cmd,NULL,0);
	return 0;
}

int CMD_DeleteFS(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	return err;
}

int CMD_GetNumOfFilesInTimeRange(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	return err;
}

int CMD_GetNumOfFilesByType(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	return err;
}

int CMD_GetLastFS_Error(sat_packet_t *cmd)
{
	int err = f_getlasterror();
	if (err == E_NO_SS_ERR)
	{
		TransmitDataAsSPL_Packet(cmd, (unsigned char*)&err, sizeof(int));
	}
	return err;
}

int CMD_FreeSpace(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	return err;
}

int CMD_GetFileLengthByTime(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	return err;
}

int CMD_GetTimeOfLastElementInFile(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	return err;
}

int CMD_GetTimeOfFirstElement(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	return err;
}
