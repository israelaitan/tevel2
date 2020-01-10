#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>
#include <hal/errors.h>

#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>

#include <stdlib.h>
#include <string.h>

#include "GlobalStandards.h"
#include "TRXVU.h"
#include "AckHandler.h"
#include "ActUponCommand.h"
#include "SatCommandHandler.h"
#include "TLM_management.h"

#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "SubSystemModules/Communication/Beacon.h"



xQueueHandle xDumpQueue = NULL;
xSemaphoreHandle xDumpLock = NULL;
xTaskHandle xDumpHandle = NULL;			 //task handle for dump task

void InitSemaphores()
{
	if(NULL == xDumpLock)
		vSemaphoreCreateBinary(xDumpLock);
	if(NULL == xDumpQueue)
		xDumpQueue = xQueueCreate(1, sizeof(Boolean));
}

int InitTrxvu() {

	ISIStrxvuI2CAddress myTRXVUAddress;
	ISIStrxvuFrameLengths myTRXVUBuffers;

	int retValInt = 0;

	//Buffer definition
	myTRXVUBuffers.maxAX25frameLengthTX = SIZE_TXFRAME;//SIZE_TXFRAME;
	myTRXVUBuffers.maxAX25frameLengthRX = SIZE_RXFRAME;

	//I2C addresses defined
	myTRXVUAddress.addressVu_rc = I2C_TRXVU_RC_ADDR;
	myTRXVUAddress.addressVu_tc = I2C_TRXVU_TC_ADDR;


	//Bitrate definition
	ISIStrxvuBitrate myTRXVUBitrates;
	myTRXVUBitrates = trxvu_bitrate_9600;
	retValInt = IsisTrxvu_initialize(&myTRXVUAddress, &myTRXVUBuffers,
			&myTRXVUBitrates, 1);
	if (retValInt != 0) {
		return retValInt;
	}
	vTaskDelay(100);

	IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX,myTRXVUBitrates);
	vTaskDelay(100);

	ISISantsI2Caddress myAntennaAddress;
	myAntennaAddress.addressSideA = ANTS_I2C_SIDE_A_ADDR;
	myAntennaAddress.addressSideB = ANTS_I2C_SIDE_B_ADDR;

	//Initialize the AntS system
	retValInt = IsisAntS_initialize(&myAntennaAddress, 1);
	if (retValInt != 0) {
		return retValInt;
	}

	InitTxModule();
	InitBeaconParams();
	InitSemaphores();

	return 0;
}

CommandHandlerErr TRX_Logic() {
	int err = 0;
	int frame_count = GetNumberOfFramesInBuffer();
	sat_packet_t cmd = { 0 };

	if (frame_count > 0) {
		err = GetOnlineCommand(&cmd);
		ResetGroundCommWDT();
		SendAckPacket(ACK_RECEIVE_COMM, &cmd, NULL, 0);

	} else if (GetDelayedCommandBufferCount() > 0) {
		err = GetDelayedCommand(&cmd);
	}
	if (cmd_command_found == err) {
		err = ActUponCommand(&cmd);
		//TODO: log error
		//TODO: send message to ground when a delayed command was not executed-> add to log
	}
	BeaconLogic();

	if (cmd_command_found != err)
		return err;

	return cmd_command_succsess;
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
	unsigned int num_of_elements = 0;
	unsigned int size_of_element = 0;

	unsigned char *buffer = NULL;
	char filename[MAX_F_FILE_NAME_SIZE] = { 0 };

	err = GetTelemetryFilenameByType((tlm_type) task_args->dump_type,
			filename);
	if (0 != err) {
		FinishDump(task_args, buffer, ACK_DUMP_ABORT, (unsigned char*) &err,sizeof(err));
		return;
	}

	SendAckPacket(ACK_DUMP_START, task_args->cmd,
			(unsigned char*) &num_of_elements, sizeof(num_of_elements));

	for (unsigned int i = 0; i < num_of_elements; i++) {

		if (CheckDumpAbort() || CheckTransmitionAllowed()) {
			FinishDump(task_args, buffer, ACK_DUMP_ABORT, NULL, 0);
		}
		if (0 == availFrames)
			vTaskDelay(10);

		AssembleCommand((unsigned char*)buffer, size_of_element,
				(char) DUMP_SUBTYPE, (char) (task_args->dump_type),
				task_args->cmd->ID, &dump_tlm);

		//TODO: finish dump

		TransmitSplPacket(&dump_tlm, &availFrames);
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


