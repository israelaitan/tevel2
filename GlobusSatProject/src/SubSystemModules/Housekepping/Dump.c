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

#define RESOLUTION 1
#define SIZE_DUMP_BUFFER (4096) ///< buffer size for dump operations

xQueueHandle xDumpQueue = NULL;
xSemaphoreHandle xDumpLock = NULL;
xTaskHandle xDumpHandle = NULL;			 //task handle for dump task

unsigned char dump_arr[SIZE_DUMP_BUFFER];


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
		logg(TLMInfo, "I:free args\n");
		free(task_args);
	}
	if (NULL != xDumpLock) {
		logg(TLMInfo, "I:release lock\n");
		xSemaphoreGive(xDumpLock);
	}
	if (xDumpHandle != NULL) {
		logg(TLMInfo, "I:delete handle\n");
		vTaskDelete(xDumpHandle);
		xDumpHandle = NULL;
	}

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
	}
}

Boolean CheckDumpAbort() {
	portBASE_TYPE err;
	Boolean queue_msg;
	err = xQueueReceive(xDumpQueue, &queue_msg, 0);
	if (err == pdPASS) {
		return queue_msg;
	}
	return FALSE;
}

int getTelemetryMetaData(tlm_type type, char* filename, int* size_of_element) {
	int err = 0;
	err = GetTelemetryFilenameByType(type, filename);
	if (0 != err) {
		return err;
	}
	if(c_fileGetSizeOfElement(filename,size_of_element) != FS_SUCCSESS) {
		return -1;
	}
	return err;
}

void DumpTask(void *args) {
	if (NULL == args) {
		FinishDump(NULL, NULL, ACK_DUMP_ABORT, NULL, 0);
		return;
	}
	dump_arguments_t *task_args = (dump_arguments_t *) args;
	sat_packet_t dump_tlm = { 0 };

	int err = 0;
	int ack_return_code = ACK_DUMP_FINISHED;
	Boolean is_last_read = FALSE;
	FileSystemResult result = 0;
	int availFrames = 1;
	int num_packets_read = 0; //number of packets read from buffer (single_time)
	int total_packets_read = 0; //total number of packets read from buffer
	unsigned int num_of_elements = 0;
	int size_of_element = 0;
	int size_of_element_with_timestamp;
	time_unix last_read_time; // this is the last time we have on the buffer
	time_unix last_sent_time = task_args->t_start; // this is the last we actually sent(where we want to search next)


	unsigned char *buffer = NULL;
	char filename[MAX_F_FILE_NAME_SIZE] = { 0 };

	buffer = dump_arr;

	err = getTelemetryMetaData(task_args->dump_type, filename, &size_of_element);
	if(0 != err) {
		// TODO: see if this can fit into our goto
		logg(error, "E:problem during dump init with err %d\n", err);
		FinishDump(task_args, buffer, ACK_DUMP_ABORT, (unsigned char*) &err,sizeof(err));
		return;
	}

	size_of_element_with_timestamp = size_of_element + sizeof(time_unix);
	f_managed_enterFS();
	logg(TLMInfo, "I:filename: %s, size of element: %d t_start: %d t_end: %d\n", filename, size_of_element, task_args->t_start, task_args->t_end);

	// TODO: consider if we actually want to know the number of packets that will be sent,
	// as it won't be exactly easy.
	SendAckPacket(ACK_DUMP_START, task_args->cmd,
			(unsigned char*) &num_of_elements, sizeof(num_of_elements));

	time_unix curr = 0;
	Time_getUnixEpoch(&curr);
	logg(TLMInfo, "I:starting dump loop at time: %u\n", curr);
	while(!is_last_read) {
		// read
		num_packets_read = 0;

		// TODO: consider different resolution
		result = c_fileRead(filename, buffer, SIZE_DUMP_BUFFER,
		last_sent_time, task_args->t_end, &num_packets_read, &last_read_time,1);
		if(result != FS_BUFFER_OVERFLOW) {
			logg(error, "E:c_fileRead returned not buffer overflow but: %d", result);
			is_last_read = TRUE;
		}
		logg(TLMInfo, "I:read from buffer, num_packets_read: %d", num_packets_read);
		last_sent_time = last_read_time;
		total_packets_read += num_packets_read;
		// send packets
		for(int i = 0; i < num_packets_read; ) {
			if (CheckDumpAbort() || !CheckTransmitionAllowed()) {
				logg(error, "E:got dump abort\n");
				ack_return_code = ACK_DUMP_ABORT;
				goto cleanup;
			}

			AssembleCommand((unsigned char*)buffer + size_of_element_with_timestamp * i,
				size_of_element_with_timestamp,
				(char) START_DUMP_SUBTYPE, (char) (task_args->dump_type),
				task_args->cmd->ID, i, T8GBS, &dump_tlm);
			err = TransmitSplPacket(&dump_tlm, &availFrames);
			if(err != 0) {
				logg(error, "E:transmitsplpacket error: %d", err);
			}
			if (availFrames != NO_TX_FRAME_AVAILABLE)
				i++;
			else {
				logg(info, "I:dump.no available frames\n");
				vTaskDelay(100);
			}
		}
	}
	logg(TLMInfo, "I:finish dump gracefully %d transmitted", total_packets_read);
cleanup:
	f_managed_releaseFS();
	FinishDump(task_args, buffer, ack_return_code, NULL, 0);
	while(1) {
		logg(TLMInfo, "I:at end of dump task");
		vTaskDelay(5000);
	};
}

int DumpTelemetry(sat_packet_t *cmd) {
	if (NULL == cmd) {
		return -1;
	}

	dump_arguments_t *dmp_pckt = malloc(sizeof(*dmp_pckt));
	int index = 0;
	memcpy(&(dmp_pckt->dump_type),&cmd->data[index],sizeof(dmp_pckt->dump_type));
	index+=sizeof(dmp_pckt->dump_type);
	memcpy(&(dmp_pckt->t_start),&cmd->data[index],sizeof(dmp_pckt->t_start));
	index+=sizeof(dmp_pckt->t_start);
	memcpy(&(dmp_pckt->t_end),&cmd->data[index],sizeof(dmp_pckt->t_end));
	index+=sizeof(dmp_pckt->t_end);
	memcpy(&(dmp_pckt->cmd),cmd,sizeof(*cmd));


	if (xSemaphoreTake(xDumpLock,SECONDS_TO_TICKS(1)) != pdTRUE) {
		return E_GET_SEMAPHORE_FAILED;
	}
	xTaskCreate(DumpTask, (const signed char* const )"DumpTask", 2000,
			(void* )dmp_pckt, configMAX_PRIORITIES - 2, &xDumpHandle);

	return 0;
}
