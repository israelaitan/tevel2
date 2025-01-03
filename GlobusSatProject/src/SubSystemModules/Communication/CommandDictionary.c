#include <satellite-subsystems/isis_ants.h>


#include "SubSystemModules/Communication/SubsystemCommands/TRXVU_Commands.h"
#include "SubSystemModules/Communication/SubsystemCommands/Maintanence_Commands.h"
#include "SubSystemModules/Communication/SubsystemCommands/FS_Commands.h"
#include "SubSystemModules/Communication/SubsystemCommands/EPS_Commands.h"

#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "TLM_management.h"
#include <stdio.h>
#include "CommandDictionary.h"
#include "Beacon.h"
#include "Transponder.h"

int trxvu_command_router(sat_packet_t *cmd)
{
	int err = 0;
	ack_subtype_t ackType=ACK_UNKNOWN_SUBTYPE;

	switch (cmd->cmd_subtype)
	{
	case START_DUMP_SUBTYPE:
		err = CMD_StartDump(cmd);
		ackType=ACK_DUMP_START;
		break;

	case STOP_DUMP_SUBTYPE:
		err = CMD_SendDumpAbortRequest(cmd);
		ackType=ACK_DUMP_ABORT;
		break;

	case GET_BY_INDEX_DUMP_SUBTYPE:
		err = CMD_GetDumpByIndex(cmd);
		ackType=ACK_DUMP_BY_INDEX;
		break;

	case MUTE_TRXVU:
		err = CMD_MuteTRXVU(cmd);
		ackType=ACK_MUTE;
		break;

	case UNMUTE_TRXVU:
		err = CMD_UnMuteTRXVU(cmd);
		ackType=ACK_UNMUTE;
		break;

	case TRXVU_IDLE_ON:
		err = CMD_SetIdleOn(cmd);
		ackType=ACK_IDLE_ON;
		break;

	case TRXVU_IDLE_OFF:
		err = CMD_SetIdleOff();
		ackType=ACK_IDLE_OFF;
		break;

	case GET_BEACON_INTERVAL:
		err = CMD_GetBeaconInterval(cmd);
		ackType=ACK_NO_ACK;
		break;

	case SET_BEACON_INTERVAL:
		err = UpdateBeaconInterval(cmd);
		ackType=ACK_UPDATE_BEACON_TIME_DELAY;
		break;
	case GET_TX_UPTIME:
		err = CMD_GetTxUptime(cmd);
		ackType=ACK_NO_ACK;
		break;

	case GET_RX_UPTIME:
		err = CMD_GetRxUptime(cmd);
		ackType=ACK_NO_ACK;
		break;

	case GET_NUM_OF_ONLINE_CMD:
		err = CMD_GetNumOfOnlineCommands(cmd);
		ackType=ACK_NO_ACK;
		break;

	case ANT_SET_ARM_STATUS:
		err = CMD_AntSetArmStatus(cmd);
		ackType=ACK_ARM_DISARM;
		break;

	case ANT_GET_ARM_STATUS:
		err = CMD_AntGetArmStatus(cmd);
		ackType=ACK_NO_ACK;
		break;

	case ANT_GET_UPTIME:
		err = CMD_AntGetUptime(cmd);
		ackType=ACK_NO_ACK;
		break;

	case TRANSPONDER_ON:
		err = CMD_turnOnTransponder(cmd);
		ackType=ACK_TRANSPONDER_ON;
		break;

	case TRANSPONDER_OFF:
		err = CMD_turnOffTransponder(cmd);
		ackType=ACK_TRANSPONDER_OFF;
		break;

	case SET_TRANSPONDER_RSSI:
		err = CMD_set_transponder_RSSI(cmd);
		ackType=ACK_TRANSPONDER_RSSI;
		break;

	default:
		break;
	}

	//Send Acknowledge to earth that command was executed
	SendErrorMSG(ACK_ERROR_MSG, ackType, cmd,err);
	return err;
}

int eps_command_router(sat_packet_t *cmd)
{
	int err = 0;

	switch (cmd->cmd_subtype)
	{
	case EPS_RESET_WD:
		err = CMD_EPS_ResetWDT(cmd);
		SendErrorMSG(ACK_ERROR_MSG, ACK_RESET_EPS_WD,cmd, err);
		break;
	case EPS_SET_CHANNEL_ON:
		err = CMD_EPS_SetChannelStateOn(cmd);
		SendErrorMSG(ACK_ERROR_MSG, ACK_RESET_EPS_WD,cmd, err);//TODO:add app ack
		break;
	case EPS_SET_CHANNEL_OFF:
			err = CMD_EPS_SetChannelStateOff(cmd);
			SendErrorMSG(ACK_ERROR_MSG, ACK_RESET_EPS_WD,cmd, err);//TODO:add app ack
			break;
	default:
		SendAckPacket(ACK_UNKNOWN_SUBTYPE,cmd->ID, cmd->ordinal,NULL,0);
		break;
	}

	return err;
}

int telemetry_command_router(sat_packet_t *cmd)
{
	int err = 0;
	ack_subtype_t ackType=ACK_UNKNOWN_SUBTYPE;

	switch (cmd->cmd_subtype)
	{

	case TLM_GET_EPS_RAW_SUBTYPE:
		err = CMD_getEPS_RAW_TLM(cmd);
		ackType=ACK_NO_ACK;
		break;

	case TLM_GET_EPS_ENG_SUBTYPE:
		err = CMD_getEPS_ENG_TLM(cmd);
		ackType=ACK_NO_ACK;
		break;

	case TLM_GET_EPS_AVG_SUBTYPE:
		err = CMD_getEPS_AVG_TLM(cmd);
		ackType=ACK_NO_ACK;
		break;

	case TLM_GET_TX_SUBTYPE:
		err = CMD_getTX_TLM(cmd);
		ackType=ACK_NO_ACK;
		break;
	case TLM_GET_RX_SUBTYPE:
		err = CMD_getRX_TLM(cmd);
		ackType=ACK_NO_ACK;
		break;

	case TLM_GET_ANTENNA_A_SUBTYPE:
		err = CMD_getAnts_A_TLM(cmd);
		ackType=ACK_NO_ACK;
		break;

	case TLM_GET_ANTENNA_B_SUBTYPE:
		err = CMD_getAnts_B_TLM(cmd);
		ackType=ACK_NO_ACK;
		break;
	case TLM_GET_LOG_SUBTYPE:
		err = CMD_getLOG_TLM(cmd);
		ackType=ACK_NO_ACK;
		break;
	case TLM_GET_WOD_SUBTYPE:
		err = CMD_getWOD_TLM(cmd);
		ackType=ACK_NO_ACK;
		break;

	case TLM_GET_PIC32_SUBTYPE:
		err = CMD_getPic32_TLM(cmd);
		ackType=ACK_NO_ACK;
		break;

	case TLM_GET_RADFET_SUBTYPE:
		err = CMD_getRadfet_TLM(cmd);
		ackType=ACK_NO_ACK;
		break;

	case TLM_GET_SOLAR_SUBTYPE:
		err = CMD_getSolar_TLM(cmd);
		ackType=ACK_NO_ACK;
		break;

	default:
		break;
	}

	//Send Acknowledge to earth that command was executed
	SendErrorMSG(ACK_ERROR_MSG, ackType, cmd,err);
	return err;
}

int managment_command_router(sat_packet_t *cmd)
{
	int err = 0;
	ack_subtype_t ackType=ACK_UNKNOWN_SUBTYPE;

	switch ((management_subtypes_t)cmd->cmd_subtype)
	{

	case SOFT_RESET_SUBTYPE:
		CMD_ResetComponent(reset_software);
		ackType=ACK_NO_ACK;
		break;

	case HARD_RESET_SUBTYPE:
		CMD_ResetComponent(reset_hardware);
		ackType=ACK_NO_ACK;
		break;

	case EPS_RESET_SUBTYPE:
		CMD_ResetComponent(reset_eps);
		ackType=ACK_NO_ACK;
		break;

	case TRXVU_HARD_RESET_SUBTYPE:
		CMD_ResetComponent(reset_trxvu_hard);
		ackType=ACK_NO_ACK;
		break;

	case TRXVU_SOFT_RESET_SUBTYPE:
		CMD_ResetComponent(reset_trxvu_soft);
		ackType=ACK_NO_ACK;
		break;

	case FS_RESET_SUBTYPE:
		CMD_ResetComponent(reset_filesystem);
		ackType=ACK_NO_ACK;
		break;

	case ANTS_SIDE_A_RESET_SUBTYPE:
		CMD_ResetComponent(reset_ant_SideA);
		ackType=ACK_NO_ACK;
		break;

	case ANTS_SIDE_B_RESET_SUBTYPE:
		CMD_ResetComponent(reset_ant_SideB);
		ackType=ACK_NO_ACK;
		break;

	case ANTS_TURN_OFF_AUTO_DEP_SUBTYPE:
		err = CMD_AntStopAutoDeployment(cmd);
		ackType=ACK_NO_ACK;
		break;

	case ANTS_CANCEL_DEPLOY_SUBTYPE:
		err = CMD_AntCancelDeployment(cmd);
		ackType=ACK_ANT_CANCEL_DEP;
		break;

	case ANTS_AUTO_DEPLOY_SUBTYPE:
		err = CMD_AntennaDeploy(cmd);
		ackType=ACK_REDEPLOY;
		break;

	case I2C_GEN_CMD_SUBTYPE:
		err = CMD_GenericI2C(cmd);
		ackType=ACK_GENERIC_I2C_CMD;
		break;

	case FRAM_READ_SUBTYPE:
		err = CMD_FRAM_ReadAndTransmitt(cmd);
		ackType=ACK_NO_ACK;
		break;

	case FRAM_WRITE_SUBTYPE:
		err = CMD_FRAM_WriteAndTransmitt(cmd);
		ackType=ACK_NO_ACK;
		break;

	case FRAM_RESTART_SUBTYPE:
		err = CMD_FRAM_Restart(cmd);
		ackType=ACK_FRAM_RESET;
		break;

	case UPDATE_SAT_TIME_SUBTYPE:
		err = CMD_UpdateSatTime(cmd);
		ackType=ACK_UPDATE_TIME;
		break;

	case GET_SAT_TIME_SUBTYPE:
		err = CMD_GetSatTime(cmd);
		ackType=ACK_NO_ACK ;
		break;

	case GET_SAT_UP_TIME_SUBTYPE:
		err = CMD_GetSatUptime(cmd);
		ackType=ACK_NO_ACK;
		break;

	case TLM_SET_COLL_CYCLE_SUBTYPE:
		err =CMD_SetTLM_CollectionCycle(cmd);
		ackType=ACK_TLM_SET_COLL_CYCLE;
		break;

	case TLM_GET_COLL_CYCLE_SUBTYPE:
		err =CMD_GetTLM_CollectionCycle(cmd);
		ackType=ACK_NO_ACK;
		break;

	case SET_LOG_SUBTYPE:
		err =CMD_setLogLevel(cmd);
		ackType=ACK_SET_LOG_LEVEL;
		break;

	case GET_LOG_SUBTYPE:
		err = CMD_getLogLevel(cmd);
		ackType=ACK_GET_LOG_LEVEL;
		break;

	case FRAM_INIT_SUBTYPE:
		err = CMD_FRAM_Init(cmd);
		ackType=ACK_GET_LOG_LEVEL;
		break;


	default:
		break;
	}

	//Send Acknowledge to earth that command was executed
	SendErrorMSG(ACK_ERROR_MSG, ackType, cmd,err);
	return err;
}

int filesystem_command_router(sat_packet_t *cmd)
{
	int err = 0;
	ack_subtype_t ackType=ACK_UNKNOWN_SUBTYPE;

	switch (cmd->cmd_subtype)
	{
	case FS_IS_CORRUPT_SUBTYPE:
		err = CMD_isFS_Corrupt(cmd);
		ackType=ACK_NO_ACK;
		break;
	case FS_GET_FREE_SPACE_SUBTYPE:
		err = CMD_FreeSpace(cmd);
		ackType=ACK_NO_ACK;
		break;
	case FS_GET_LAST_ERR_SUBTYPE:
		err = CMD_GetLastFS_Error(cmd);
		ackType=ACK_NO_ACK;
		break;
	case FS_DELETE_ALL_SUBTYPE:
		err = CMD_DeleteFS(cmd);
		ackType=ACK_FS_DELETE_ALL;
		break;
	case FS_DELETE_FILE_BY_TYPE_SUBTYPE:
		err = CMD_DeleteFilesOfType(cmd);
		ackType=ACK_FS_DELETE_FILE;
		break;
	case FS_DELETE_FILE_BY_TIME_SUBTYPE:
		err = CMD_DeleteFileByTime(cmd);
		ackType=ACK_FS_DELETE_FILE;
		break;

	default:
		break;
	}

	//Send Acknowledge to earth that command was executed
	SendErrorMSG(ACK_ERROR_MSG, ackType, cmd,err);
	return err;
}



