#include <satellite-subsystems/IsisTRXVU.h>
#include <hal/Timing/Time.h>
#include <string.h>
#include <stdlib.h>

#include "GlobalStandards.h"
#include "SatCommandHandler.h"
#include "SPL.h"


typedef struct __attribute__ ((__packed__)) delayed_cmd_t
{
	time_unix exec_time;	///< the execution time of the cmd in unix time
	sat_packet_t cmd;		///< command data
} delayed_cmd_t;

int ClearDelayedCMD_FromBuffer(unsigned int start_addr, unsigned int end_addr)
{
	return 0;
}

int ParseDataToCommand(unsigned char * data, unsigned int length, sat_packet_t *cmd)
{
	if (data == NULL || cmd == NULL)
		return null_pointer_error;
	if (length > MAX_COMMAND_DATA_LENGTH)
		return index_out_of_bound;

	int id = 0;
	spl_command_type_t type;
	trxvu_subtypes_t subType;
	int metaSize = sizeof(int) + sizeof(spl_command_type_t) + sizeof(trxvu_subtypes_t);
	char dataCopy[length - metaSize];

	int offset = 0;
	memcpy(&id, data + offset, sizeof(int));
	offset += sizeof(int);
	memcpy(&type, data + offset, sizeof(spl_command_type_t));
	offset += sizeof(spl_command_type_t);
	memcpy(&subType, data + offset, sizeof(trxvu_subtypes_t));
	offset += sizeof(trxvu_subtypes_t);
	memcpy(dataCopy, data + offset, length - metaSize);

	//TODO: check for errors
	cmd->ID = id;
	cmd->cmd_type = type;
	cmd->cmd_subtype = subType;
	memcpy(cmd->data, dataCopy,  length - metaSize);
	return command_succsess;//we can use assemblecommand instead
}

int AssmbleCommand(unsigned char *data, unsigned int data_length, char type,
		char subtype, unsigned int id, sat_packet_t *cmd)
{
	return 0;
}

// checks if a cmd time is valid for execution -> execution time has passed and command not expired
// @param[in] cmd_time command execution time to check
// @param[out] expired if command is expired the flag will be raised
Boolean isDelayedCommandDue(time_unix cmd_time, Boolean *expired)
{
	return FALSE;
}

//TOOD: move delayed cmd logic to the SD and write 'checked/uncheked' bits in the FRAM
int GetDelayedCommand(sat_packet_t *cmd)
{
	return 0;
}

int AddDelayedCommand(sat_packet_t *cmd)
{
	return 0;
}

int GetDelayedCommandBufferCount()
{
	return 0;

}

int GetOnlineCommand(sat_packet_t *cmd)
{
	return 0;
}

int GetDelayedCommandByIndex(unsigned int index, sat_packet_t *cmd)
{
	return 0;
}

int DeleteDelayedCommandByIndex(unsigned int index)
{
	return 0;
}

int DeleteDelayedBuffer()
{
	return 0;
}
