#include <string.h>
#include <stdio.h>
#include <hal/errors.h>
#include "SPL.h"
#include "TRXVU.h"
#include "ActUponCommand.h"
#include "CommandDictionary.h"
#include "AckHandler.h"
#include "SubSystemModules/Maintenance/Log.h"

int ActUponCommand(sat_packet_t *cmd)
{
	int err = 0;
	if( cmd == NULL )
			err = -1;
	logg(TRXInfo, "I:Inside ActUponCommand() target=%d command type: %d, command sub-type: %d\n", cmd->ID, cmd->cmd_type, cmd->cmd_subtype);
	if (cmd->targetSat != T8GBS)
		err = 0;
	else if( cmd->cmd_type == trxvu_cmd_type )
		err = trxvu_command_router( cmd );
	else if( cmd->cmd_type == eps_cmd_type )
		err = eps_command_router( cmd );
	else if( cmd->cmd_type == telemetry_cmd_type )
		err = telemetry_command_router( cmd );
	else if( cmd->cmd_type == filesystem_cmd_type )
		 err = filesystem_command_router( cmd );
	else if( cmd->cmd_type == managment_cmd_type )
		 err = managment_command_router( cmd );
	else if( cmd->cmd_type == ack_type ) {
		unsigned char* data = NULL;
		unsigned int length = 0;
		SendAckPacket(ACK_PING, cmd->ID, cmd->ordinal, data, length);
	} else
		err = -2;

	if (err)
		logg(error, "E:finished command with error: %d\n" , err);
	return err;
}


