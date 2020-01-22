#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>
#include <hal/errors.h>


#include <stdlib.h>
#include <string.h>

#include "GlobalStandards.h"
#include "DUMP.h"
#include "TLM_management.h"
#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/ActUponCommand.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"


typedef struct __attribute__ ((__packed__))
{
	sat_packet_t *cmd;
	unsigned char dump_type;
	time_unix t_start;
	time_unix t_end;
} dump_arguments_t;

xQueueHandle xDumpQueue = NULL;
xSemaphoreHandle xDumpLock = NULL;
xTaskHandle xDumpHandle = NULL;			 //task handle for dump task

int InitDump()
{
	if(NULL == xDumpLock)
		vSemaphoreCreateBinary(xDumpLock);
	if(NULL == xDumpQueue)
		xDumpQueue = xQueueCreate(1, sizeof(Boolean));
	return 0;
}


void FinishDump(dump_arguments_t *task_args,unsigned char *buffer, ack_subtype_t acktype,
		unsigned char *err, unsigned int size) {

	SendAckPacket(acktype, task_args->cmd, err, size);
	if (NULL != task_args) {
		free(task_args);
	}
	if (NULL != xDumpLock) {
		xSemaphoreGive(xDumpLock);
	}
	if (xDumpHandle != NULL) {
		vTaskDelete(xDumpHandle);
	}
	if(NULL != buffer){
		free(buffer);
	}
}

void AbortDump()
{
	FinishDump(NULL,NULL,ACK_DUMP_ABORT,NULL,0);
}

void SendDumpAbortRequest() {
	if (eTaskGetState(xDumpHandle) == eDeleted) {
		return;
	}
	Boolean queue_msg = TRUE;
	int err = xQueueSend(xDumpQueue, &queue_msg, SECONDS_TO_TICKS(1));
	if (0 != err) {
		if (NULL != xDumpLock) {
			xSemaphoreGive(xDumpLock);
		}
		if (xDumpHandle != NULL) {
			vTaskDelete(xDumpHandle);
		}
	}
}

Boolean CheckDumpAbort() {
	portBASE_TYPE err;
	Boolean queue_msg;
	err = xQueueReceive(xDumpQueue, &queue_msg, SECONDS_TO_TICKS(1));
	if (err == pdPASS) {
		return queue_msg;
	}
	return FALSE;
}

void DumpTask(void *args) {
	if (NULL == args) {
		FinishDump(NULL, NULL, ACK_DUMP_ABORT, NULL, 0);
		return;
	}
	dump_arguments_t *task_args = (dump_arguments_t *) args;
	sat_packet_t dump_tlm = { 0 };
	int err = 0;
	int availFrames = 0;
	unsigned int num_of_packets = 0;
	unsigned int size_of_element = 0;
	time_unix last_time_read = 0;
	unsigned char *buffer = NULL;
	int size_of_buffer = 0;
	int num_of_tlm_elements_read = 0;
	char filename[MAX_F_FILE_NAME_SIZE] = { 0 };

	err = GetTelemetryFilenameByType((tlm_type) task_args->dump_type,
				filename);
	if(err)
	{
		FinishDump(task_args, buffer, ACK_DUMP_ABORT, (unsigned char*) &err,sizeof(err));
		return;
	}
	c_fileGetNumOfElements(filename, task_args->t_start, task_args->t_end, &num_of_tlm_elements_read, &last_time_read, &size_of_element);
	unsigned int buffer_size = num_of_tlm_elements_read * (sizeof(unsigned int) + size_of_element);
	buffer = malloc(buffer_size);
	//TODO: length of index (100 or 99) not handled in file name 7+3+4
	c_fileRead(filename, buffer, size_of_buffer, task_args->t_start, task_args->t_end, &num_of_tlm_elements_read, &last_time_read);
	num_of_packets = buffer_size / MAX_COMMAND_DATA_LENGTH;
	if (!(buffer_size % MAX_COMMAND_DATA_LENGTH))
			num_of_packets++;
	SendAckPacket(ACK_DUMP_START, task_args->cmd,
			(unsigned char*) &num_of_packets, sizeof(num_of_packets));

	unsigned int currPacketSize;
	unsigned int totalDataLeft = buffer_size;
	unsigned int i = 0;
	while (i < num_of_packets) {

		if (CheckDumpAbort() || CheckTransmitionAllowed())
			return FinishDump(task_args, buffer, ACK_DUMP_ABORT, NULL, 0);

		currPacketSize = totalDataLeft < MAX_COMMAND_DATA_LENGTH ? totalDataLeft : MAX_COMMAND_DATA_LENGTH;

		AssembleCommand(buffer, currPacketSize,
				(char) DUMP_SUBTYPE, (char) (task_args->dump_type),
				task_args->cmd->ID, i,  &dump_tlm);

		err = TransmitSplPacket(&dump_tlm, &availFrames);
		if (err)
			return FinishDump(task_args, buffer, ACK_DUMP_ABORT, NULL, 0);
		if (availFrames != NO_TX_FRAME_AVAILABLE) {
			i++;
			totalDataLeft -= currPacketSize;
			buffer += currPacketSize;
		}
		if (availFrames == 0 || availFrames == NO_TX_FRAME_AVAILABLE)
			vTaskDelay(10);
	}
	FinishDump(task_args, buffer, ACK_DUMP_FINISHED, NULL, 0);
}

int DumpTelemetry(sat_packet_t *cmd) {
	if (NULL == cmd) {
		return -1;
	}

	dump_arguments_t *dmp_pckt = malloc(sizeof(*dmp_pckt));
	unsigned int offset = 0;

	dmp_pckt->cmd = cmd;

	memcpy(&dmp_pckt->dump_type, cmd->data, sizeof(dmp_pckt->dump_type));
	offset += sizeof(dmp_pckt->dump_type);

	memcpy(&dmp_pckt->t_start, cmd->data + offset, sizeof(dmp_pckt->t_start));
	offset += sizeof(dmp_pckt->t_start);

	memcpy(&dmp_pckt->t_end, cmd->data + offset, sizeof(dmp_pckt->t_end));

	if (xSemaphoreTake(xDumpLock,SECONDS_TO_TICKS(1)) != pdTRUE) {
		return E_GET_SEMAPHORE_FAILED;
	}
	xTaskCreate(DumpTask, (const signed char* const )"DumpTask", 2000,
			(void* )dmp_pckt, configMAX_PRIORITIES - 2, xDumpHandle);

	return 0;
}