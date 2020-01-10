#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <string.h>
#include "AckHandler.h"


#include "GlobalStandards.h"
#include "SatDataTx.h"
#include "TRXVU.h"

int SendAckPacket(ack_subtype_t acksubtype, sat_packet_t *cmd,
		unsigned char *data, unsigned int length)
{
	int err = 0;
	sat_packet_t ack = { 0 };
	unsigned int id = 0;
	id = 0xFFFFFFFF; //default ID. system ACK. not a response to any command

	if (NULL != cmd) {
		id = cmd->ID;
	}

	AssembleCommand(data, length, (char)ack_type, (char)acksubtype, id, &ack);

	err = TransmitSplPacket(&ack,NULL);
	vTaskDelay(10);
	return err;
}

void SendErrorMSG(ack_subtype_t fail_subt, ack_subtype_t succ_subt,
		sat_packet_t *cmd, int err)
{
	if (err == 0) {
		SendAckPacket(succ_subt, cmd, (unsigned char*) &err, sizeof(err));
	}
	else {
		SendAckPacket(fail_subt, cmd, (unsigned char*) &err, sizeof(err));
	}
}

void SendErrorMSG_IfError(ack_subtype_t subtype, sat_packet_t *cmd, int err)
{
	if (err != 0) {
		SendAckPacket(subtype, cmd, (unsigned char*) &err, sizeof(err));
	}
}

