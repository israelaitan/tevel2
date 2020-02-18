#include <string.h>
#include <hal/errors.h>
#include "SPL.h"
#include "TRXVU.h"
#include "ActUponCommand.h"
#include "CommandDictionary.h"
#include "AckHandler.h"

int ActUponCommand(sat_packet_t *cmd)
{
	int err = 0;

	//TODO: check if cmd is NULL

	if(cmd->cmd_type==trxvu_cmd_type)
	{
		err=trxvu_command_router(cmd);
	}
	else if(cmd->cmd_type==eps_cmd_type)
	{
		err=eps_command_router(cmd);
	}
	else if(cmd->cmd_type==telemetry_cmd_type)
	{
		err=telemetry_command_router(cmd);
	}
	else if(cmd->cmd_type==filesystem_cmd_type)
	{
		 err=filesystem_command_router(cmd);
	}
	else if(cmd->cmd_type==managment_cmd_type)
	{
		 err=managment_command_router(cmd);
	}
	else if(cmd->cmd_type==ack_type)
	{
		//TODO: SendAckPacket with ACK_PING
	}

	return err;
}


