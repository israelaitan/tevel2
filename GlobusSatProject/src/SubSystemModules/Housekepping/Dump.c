#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>
#include <hal/errors.h>

#include <hcc/api_fat.h>
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
#include "SubSystemModules/Maintenance/Log.h"

typedef struct __attribute__ ((__packed__))
{
	sat_packet_t *cmd;
	unsigned short dump_type;
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
	logg(info, "I:finishDump started\n");
	SendAckPacket(acktype, task_args->cmd, err, size);
	logg(info, "I:finishDump:ack packet sent\n");
	if (NULL != task_args) {
		if (NULL != task_args->cmd)
			free(task_args->cmd);
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
	logg(info, "I:finish dump:ended successfully\n");
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
	logg(info, "I:Starting DumpTask\n");
	if (NULL == args) {
		FinishDump(NULL, NULL, ACK_DUMP_ABORT, NULL, 0);
		return;
	}
	f_enterFS();
	dump_arguments_t *task_args = (dump_arguments_t *) args;
	logg(info, "dump: type: %u \n", task_args->dump_type);
	logg(info, "dump: start time: %lu \n", (long unsigned int)task_args->t_start);
	logg(info, "dump: end time: %lu \n", (long unsigned int)task_args->t_end);
	sat_packet_t dump_tlm = { 0 };
	int err = 0;
	int availFrames = 0;
	unsigned int num_of_packets = 0;
	unsigned int size_of_element = 0;
	time_unix last_time_read = 0;
	unsigned char * buffer = NULL ;//add size
	int num_of_tlm_elements_read = 0;
	char filename[MAX_F_FILE_NAME_SIZE] = { 0 };

	err = GetTelemetryFilenameByType((tlm_type) task_args->dump_type,
				filename);
	if(err) {
		logg(error, "E:%d dump.GetTelemetryFilenameByType", err);
		FinishDump(task_args, buffer, ACK_DUMP_ABORT, (unsigned char*) &err,sizeof(err));
		f_releaseFS();
		return;
	}
	c_fileGetNumOfElements(filename, task_args->t_start, task_args->t_end, &num_of_tlm_elements_read, &last_time_read, &size_of_element);
	logg(info, "I:dump: number of elements: %d\n", num_of_tlm_elements_read);
	unsigned int buffer_size = num_of_tlm_elements_read * (sizeof(unsigned int) + size_of_element);
	buffer = malloc(buffer_size);
	FileSystemResult res = c_fileRead(filename, buffer, buffer_size, task_args->t_start, task_args->t_end, &num_of_tlm_elements_read, &last_time_read);
	if (res != FS_SUCCSESS) {
		logg(error, "E:%d dump.c_fileRead", err);
		FinishDump(task_args, buffer, ACK_DUMP_ABORT, (unsigned char*) &err,sizeof(err));
		f_releaseFS();
		return;
	}
	num_of_packets = buffer_size / MAX_COMMAND_DATA_LENGTH;
	if (buffer_size % MAX_COMMAND_DATA_LENGTH)
			num_of_packets++;
	SendAckPacket(ACK_DUMP_START, task_args->cmd,
			(unsigned char*) &num_of_packets, sizeof(num_of_packets));
	logg(info, "I:dump: ack packet sent with number of packets=: %d\n",num_of_packets);
	unsigned int currPacketSize;
	unsigned int totalDataLeft = buffer_size;
	unsigned short i = 0;
	while (i < num_of_packets)
	{
		if (CheckDumpAbort() || !CheckTransmitionAllowed()){
			logg(info, "I:dump.aborted or not allowed\n");
			f_releaseFS();
			return FinishDump(task_args, buffer, ACK_DUMP_ABORT, NULL, 0);
		}

		currPacketSize = totalDataLeft < MAX_COMMAND_DATA_LENGTH ? totalDataLeft : MAX_COMMAND_DATA_LENGTH;
		AssembleCommand(buffer, currPacketSize,(char) START_DUMP_SUBTYPE, (char) (task_args->dump_type), task_args->cmd->ID, i, 8, &dump_tlm);
		err = TransmitSplPacket(&dump_tlm, &availFrames);
		logg(info, "dump: packet sent id = %u  ord = %u availFrames = %d \n", task_args->cmd->ID, i, availFrames);
		if (err) {
			logg(error, "E:%d dump.transmit\n", err);
			f_releaseFS();
			return FinishDump(task_args, buffer, ACK_DUMP_ABORT, NULL, 0);
		}
		if (availFrames != NO_TX_FRAME_AVAILABLE) {
			i++;
			totalDataLeft -= currPacketSize;
			buffer += currPacketSize;
		}
		else {
			logg(info, "I:dump.no available frames\n");
			vTaskDelay(10);
		}
	}
	f_releaseFS();
	FinishDump(task_args, buffer, ACK_DUMP_FINISHED, NULL, 0);
	logg(info, "I:dump.finish successfully\n");
}

int DumpTelemetry(sat_packet_t *cmd)
{
	logg(info, "I:Starting DumpTelemetry\n");
	if (NULL == cmd) 
		return -1;

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
			(void* )dmp_pckt, configMAX_PRIORITIES - 2, &xDumpHandle);

	return 0;
}
