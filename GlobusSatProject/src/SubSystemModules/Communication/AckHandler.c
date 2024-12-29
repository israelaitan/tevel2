#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <string.h>
#include "AckHandler.h"


#include "GlobalStandards.h"
#include "SatDataTx.h"
#include "TRXVU.h"
#include <SubSystemModules/Maintenance/Log.h>

int SendAckPacket(ack_subtype_t acksubtype, unsigned short id, unsigned short ord,
		unsigned char *data, unsigned int length)
{
	int err = 0;
	sat_packet_t ack = { 0 };

	AssembleCommand(data, length, (char)ack_type, (char)acksubtype, id, ord, T8GBS, 1, &ack);

	uint8_t availableFrames = 0;
	err = TransmitSplPacket(&ack, &availableFrames);
	//vTaskDelay(10);
	return err;
}

void SendErrorMSG(ack_subtype_t fail_subt, ack_subtype_t succ_subt,
		sat_packet_t *cmd, int err)
{
	ack_subtype_t ack;

	if (err == 0) {
		ack = succ_subt;
		logg(event, "V:Command Ack: %d was successful\n", ack);
	}
	else {
		ack = fail_subt;
		logg(error, "V:Command ack: %d was Failed with error: %d\n", ack, err);
	}

	if (ack != ACK_NO_ACK)
	{
		SendAckPacket(ack, cmd->ID, cmd->ordinal, (unsigned char*) &err, sizeof(err));
	}
}

void SendErrorMSG_IfError(ack_subtype_t subtype, sat_packet_t *cmd, int err)
{
	if (err != 0) {
		SendAckPacket(subtype, cmd->ID, cmd->ordinal, (unsigned char*) &err, sizeof(err));
	}
}

