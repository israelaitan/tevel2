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

int trxvu_command_router(sat_packet_t *cmd)
{
	int err = 0;

	switch (cmd->cmd_subtype)
	{
	case DUMP_SUBTYPE:
		err = CMD_StartDump(cmd);
		break;

	case ABORT_DUMP_SUBTYPE:
		err = CMD_SendDumpAbortRequest(cmd);
		break;

	case FORCE_ABORT_DUMP_SUBTYPE:
		err = CMD_ForceDumpAbort(cmd);
		break;

	case MUTE_TRXVU:
		err = CMD_MuteTRXVU(cmd);
		break;

	case UNMUTE_TRXVU:
		err = CMD_UnMuteTRXVU(cmd);
		break;

	case TRXVU_IDLE_ON:
		err = CMD_SetIdleOn(cmd);
		break;

	case TRXVU_IDLE_OFF:
		err = CMD_SetIdleOff();
		break;

	case GET_BEACON_INTERVAL:
		err = CMD_GetBeaconInterval(cmd);
		break;

	case SET_BEACON_INTERVAL:
		//err = CMD_SetBeaconInterval(cmd);
		err=UpdateBeaconInterval(cmd);
		break;
	case GET_TX_UPTIME:
		err = CMD_GetTxUptime(cmd);
		break;

	case GET_RX_UPTIME:
		err = CMD_GetRxUptime(cmd);
		break;

	case GET_NUM_OF_ONLINE_CMD:
		err = CMD_GetNumOfOnlineCommands(cmd);
		break;

	case ANT_SET_ARM_STATUS:
		err = CMD_AntSetArmStatus(cmd);
		break;

	case ANT_GET_ARM_STATUS:
		err = CMD_AntGetArmStatus(cmd);
		break;

	case ANT_GET_UPTIME:
		err = CMD_AntGetUptime(cmd);
		break;

	case ANT_CANCEL_DEPLOY:
		err = CMD_AntCancelDeployment(cmd);
		break;

	default:
		err = SendAckPacket(ACK_UNKNOWN_SUBTYPE,cmd,NULL,0);
		break;
	}

	SendErrorMSG_IfError(ACK_ERROR_MSG,cmd,err);
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
	switch (cmd->cmd_subtype)
	{
	case 0:

		break;

	default:
		SendAckPacket(ACK_UNKNOWN_SUBTYPE,cmd,NULL,0);
		break;
	}
	return err;
}

int managment_command_router(sat_packet_t *cmd)
{
	//TODO: finish 'managment_command_router'
	int err = 0;
	switch ((management_subtypes_t)cmd->cmd_subtype)
	{

	case SOFT_RESET_SUBTYPE:
		CMD_ResetComponent(reset_software);
		break;

	case HARD_RESET_SUBTYPE:
		CMD_ResetComponent(reset_hardware);
		break;

	case EPS_RESET_SUBTYPE:
		CMD_ResetComponent(reset_eps);
		break;

	case TRXVU_HARD_RESET_SUBTYPE:
		CMD_ResetComponent(reset_trxvu_hard);
		break;

	case TRXVU_SOFT_RESET_SUBTYPE:
		CMD_ResetComponent(reset_trxvu_soft);
		break;

	case FS_RESET_SUBTYPE:
		CMD_ResetComponent(reset_filesystem);
		break;

	default:
		SendAckPacket(ACK_UNKNOWN_SUBTYPE,cmd,NULL,0);
		break;
	}
	return err;
}

int filesystem_command_router(sat_packet_t *cmd)
{
	//TODO: finish 'filesystem_command_router'
	int err = 0;
	switch (cmd->cmd_subtype)
	{
	case 0:

		break;

	default:
		SendAckPacket(ACK_UNKNOWN_SUBTYPE,cmd,NULL,0);
		break;
	}
	return err;
}
