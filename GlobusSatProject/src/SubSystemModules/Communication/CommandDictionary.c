#include <satellite-subsystems/IsisAntS.h>


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

	case ANT_CANCEL_DEPLOY:
		err = CMD_AntCancelDeployment(cmd);
		ackType=ACK_ANT_CANCEL_DEP;
		break;

	case TRANSPONDER_ON:
		err = CMD_turnOnTransponder(cmd);
		ackType=ACK_TRANSPONDER_ON;
		break;

	case TRANSPONDER_OFF:
		err = CMD_turnOffTransponder(cmd);
		ackType=ACK_TRANSPONDER_OFF;
		break;

	default:
		break;
	}

	//TODO: Not send ACK if no ack for command
	//Send Acknowledge to earth that command was executed
	SendErrorMSG(ACK_ERROR_MSG, ackType, cmd,err);
	return err;
}

int eps_command_router(sat_packet_t *cmd)
{
	//TODO: finish 'eps_command_router'
	int err = 0;

	switch (cmd->cmd_subtype)
	{
	case 0:
		err = UpdateAlpha(*(float*)cmd->data);
		SendErrorMSG(ACK_ERROR_MSG, ACK_UPDATE_EPS_ALPHA,cmd, err);
		break;

	default:
		SendAckPacket(ACK_UNKNOWN_SUBTYPE,cmd,NULL,0);
		break;
	}

	return err;
}

int telemetry_command_router(sat_packet_t *cmd)
{
	//TODO: finish 'telemetry_command_router'
	int err = 0;
	ack_subtype_t ackType=ACK_UNKNOWN_SUBTYPE;

	switch (cmd->cmd_subtype)
	{
	case 0:

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
	//TODO: finish 'managment_command_router'
	int err = 0;
	ack_subtype_t ackType=ACK_UNKNOWN_SUBTYPE;

	switch ((management_subtypes_t)cmd->cmd_subtype)
	{

	case SOFT_RESET_SUBTYPE:
		CMD_ResetComponent(reset_software);
		ackType=ACK_SOFT_RESET;
		break;

	case HARD_RESET_SUBTYPE:
		CMD_ResetComponent(reset_hardware);
		ackType=ACK_HARD_RESET;
		break;

	case EPS_RESET_SUBTYPE:
		CMD_ResetComponent(reset_eps);
		ackType=ACK_EPS_RESET;
		break;

	case TRXVU_HARD_RESET_SUBTYPE:
		CMD_ResetComponent(reset_trxvu_hard);
		ackType=ACK_TRXVU_HARD_RESET;
		break;

	case TRXVU_SOFT_RESET_SUBTYPE:
		CMD_ResetComponent(reset_trxvu_soft);
		ackType=ACK_TRXVU_SOFT_RESET;
		break;

	case FS_RESET_SUBTYPE:
		CMD_ResetComponent(reset_filesystem);
		ackType=ACK_FS_RESET;
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
	//TODO: finish 'filesystem_command_router'
	int err = 0;
	ack_subtype_t ackType=ACK_UNKNOWN_SUBTYPE;

	switch (cmd->cmd_subtype)
	{
	case 0:

		break;

	default:
		break;
	}

	//Send Acknowledge to earth that command was executed
	SendErrorMSG(ACK_ERROR_MSG, ackType, cmd,err);
	return err;
}
