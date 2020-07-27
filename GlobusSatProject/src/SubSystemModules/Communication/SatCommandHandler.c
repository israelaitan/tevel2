#include <satellite-subsystems/IsisTRXVU.h>
#include <hal/Timing/Time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "GlobalStandards.h"
#include "SatCommandHandler.h"
#include "SubSystemModules/Maintenance/Log.h"


typedef struct __attribute__ ((__packed__)) delayed_cmd_t
{
	time_unix exec_time;	///< the execution time of the cmd in unix time
	sat_packet_t cmd;		///< command data
} delayed_cmd_t;

//parsing the packet to create a command
CommandHandlerErr ParseDataToCommand(unsigned char * data, sat_packet_t *cmd)
{
	if(NULL == data || NULL == cmd){
		return cmd_null_pointer_error;
	}
	void *err = NULL;

	unsigned int offset = 0;

	unsigned short id = 0;
	err = memcpy(&id,data,sizeof(id));
	if (NULL == err)
		return cmd_execution_error;
	offset += sizeof(id);

	char ord = 0;
	err = memcpy(&ord,data + offset,sizeof(ord));
	if (NULL == err)
		return cmd_execution_error;
	offset += sizeof(ord);

	char targetSat = 0;
	err = memcpy(&targetSat,data + offset,sizeof(targetSat));
	if (NULL == err)
	{
		return cmd_execution_error;
	}
	else if( targetSat != T8GBS && targetSat !=0 )
	{
		logg(TRXInfo, "The online command is for target satellite: %d\n" , targetSat);
		return cmd_no_command_found;
	}
	offset += sizeof(targetSat);

	char type;
	err = memcpy(&type,data+offset,sizeof(type));
	if (NULL == err)
		return cmd_execution_error;

	offset += sizeof(type);

	char subtype;
	err = memcpy(&subtype, data + offset,sizeof(subtype));
	if (NULL == err)
		return cmd_execution_error;
	offset += sizeof(subtype);

	unsigned short data_length = 0;
	err = memcpy(&data_length, data + offset,sizeof(data_length));
	if (NULL == err)
		return cmd_execution_error;
	offset += sizeof(data_length);

	return AssembleCommand(data+offset, data_length, type,subtype, id, ord, targetSat, cmd);
}

CommandHandlerErr AssembleCommand(unsigned char *data, unsigned int data_length, char type,
		char subtype, unsigned short id, char ord, char targetSat, sat_packet_t *cmd)
{
	if (NULL == cmd)
	{
		return cmd_null_pointer_error;
	}

	cmd->ID = id;
	cmd->targetSat = targetSat;
	cmd->cmd_type = type;
	cmd->cmd_subtype = subtype;
	cmd->length = 0;
	cmd->ordinal = ord;

	if (NULL != data)
	{
		if (data_length > MAX_COMMAND_DATA_LENGTH)
		{
			cmd->length =  MAX_COMMAND_DATA_LENGTH;
		}
		else
		{
			cmd->length = data_length;
		}

		void *err = memcpy(cmd->data, data, cmd->length);

		if (NULL == err)
		{
			return cmd_execution_error;
		}
	}
	logg(TRXInfo, "I:Command is: ID=%d, targetSat=%d, type=%d, subType=%d, length=%d, ordinal=%d\n", cmd->ID, cmd->targetSat, cmd->cmd_type, cmd->cmd_subtype, cmd->length, cmd->ordinal);
	return cmd_command_succsess;
}

// checks if a cmd time is valid for execution -> execution time has passed and command not expired
// @param[in] cmd_time command execution time to check
// @param[out] expired if command is expired the flag will be raised
// @return TRUE is success
//			FALSE if fail

CommandHandlerErr GetOnlineCommand(sat_packet_t *cmd)
{
	if (NULL == cmd) {
		return cmd_null_pointer_error;
	}
	int err = 0;

	unsigned short frame_count = 0;
	unsigned char received_frame_data[MAX_COMMAND_DATA_LENGTH];

	err = IsisTrxvu_rcGetFrameCount(0, &frame_count);
	if (0 != err) {
		return cmd_execution_error;
	}
	if (0 == frame_count) {
		return cmd_no_command_found;
	}
	ISIStrxvuRxFrame rxFrameCmd = { 0, 0, 0,
			(unsigned char*) received_frame_data }; // for getting raw data from Rx, nullify values

	err = IsisTrxvu_rcGetCommandFrame(0, &rxFrameCmd); //get the frame from the Rx buffer
	if (0 != err) {
		return cmd_execution_error;
	}

	err = ParseDataToCommand(received_frame_data,cmd);

	return err;
}
