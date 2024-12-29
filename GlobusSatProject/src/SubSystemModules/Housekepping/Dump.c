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
	unsigned short dump_type;
	time_unix t_start;
	time_unix t_end;
	unsigned short id;
	unsigned short ord;

} dump_arguments_t;

#define RESOLUTION 1
#define SIZE_DUMP_BUFFER (4500 * LOG_TLM_SIZE) ///< buffer size for dump operations

xQueueHandle xDumpQueue = NULL;
xSemaphoreHandle xDumpLock = NULL;
xTaskHandle xDumpHandle = NULL;			 //task handle for dump task

char allocked_read_elements[(MAX_ELEMENT_SIZE)*(ELEMENTS_PER_READ)];
char data_to_send[(MAX_ELEMENT_SIZE)*(ELEMENTS_PER_READ)];


int InitDump()
{
	if(NULL == xDumpLock)
		vSemaphoreCreateBinary(xDumpLock);
	if(NULL == xDumpQueue)
		xDumpQueue = xQueueCreate(1, sizeof(Boolean));
	return 0;
}

void FinishDump(dump_arguments_t *task_args, ack_subtype_t acktype,
		unsigned char *err, unsigned int size) {

	SendAckPacket(acktype, task_args->id, task_args->ord, err, size);
	if (NULL != task_args) {
		logg(TLMInfo, "I:free args\n");
		//free(task_args);
	}

	if (xDumpHandle != NULL) {
		logg(TLMInfo, "I:delete handle\n");
		//vTaskDelete(xDumpHandle);
		//xDumpHandle = NULL;
	}
	logg(TLMInfo, "I:at end of dump task");
	//f_managed_releaseFS();
	vTaskSuspend(NULL);
}


void SendDumpAbortRequest() {
	if (eTaskGetState(xDumpHandle) == eDeleted)
		return;
	Boolean queue_msg = TRUE;
	int err = xQueueSend(xDumpQueue, &queue_msg, SECONDS_TO_TICKS(1));
	if (0 != err)
		if (NULL != xDumpLock)
			xSemaphoreGive(xDumpLock);
}

Boolean CheckDumpAbort() {
	portBASE_TYPE err;
	Boolean queue_msg;
	err = xQueueReceive(xDumpQueue, &queue_msg, 0);
	if (err == pdPASS)
		return queue_msg;
	return FALSE;
}

int getTelemetryMetaData(tlm_type type, char* filename, int* size_of_element) {
	int err = 0;
	err = GetTelemetryFilenameByType(type, filename);
	if (0 != err)
		return err;
	if(c_fileGetSizeOfElement(filename,size_of_element) != FS_SUCCSESS)
		return -1;
	return err;
}

int send(unsigned char * element, unsigned char size, unsigned char id, unsigned short ord, unsigned char type, unsigned short total, int * availFrames){
	sat_packet_t dump_tlm = { 0 };
	AssembleCommand( element, size, (unsigned char) telemetry_cmd_type, type, id, ord, (unsigned char)T8GBS, total, &dump_tlm);
	return TransmitSplPacket(&dump_tlm, availFrames);
}


FileSystemResult c_fileReadAndSend(char* c_file_name, time_unix from_time, time_unix to_time, int * sent, unsigned char dump_id, char dump_type)
{
	C_FILE c_file;
	unsigned int addr;//FRAM ADDRESS
	char curr_file_name[MAX_F_FILE_NAME_SIZE+sizeof(int)*2];

	void* element;
	int end_read = 0;
	if(get_C_FILE_struct(c_file_name,&c_file,&addr)!=TRUE)
		return FS_NOT_EXIST;
	if(from_time < c_file.creation_time || from_time > to_time)
		from_time = c_file.creation_time;
	F_FILE* current_file = NULL;
	int index_current = getFileIndex(c_file.creation_time, from_time);
	get_file_name_by_index(c_file_name,index_current,curr_file_name);
	unsigned short size_elementWithTimeStamp = c_file.size_of_element + sizeof(unsigned int);
	uint8_t availFrames = 0;
	*sent = 0;
	do {
		get_file_name_by_index(c_file_name, index_current++, curr_file_name);
		int error = f_managed_open(curr_file_name, "r", &current_file);
		if ( error != 0 || curr_file_name == NULL ){
			logg(error, "E:dump f_managed_open failed=%d curr_file_name=%s\n", error, curr_file_name);
			continue;
		}
		int file_length = f_filelength(curr_file_name);
		int length = file_length / (size_elementWithTimeStamp);//number of elements in currnet_file
		int left_to_read = length;
		int how_much_to_read = 0;
		long readen = 0;
		f_seek( current_file, 0L , SEEK_SET );
		unsigned short ord = 0;
		int j;
		for(j = 0; j < length; j += how_much_to_read) {
			how_much_to_read = ELEMENTS_PER_READ;
			if(left_to_read < ELEMENTS_PER_READ)
				how_much_to_read = left_to_read;

			element = allocked_read_elements;
			readen = f_read(element, (size_t)size_elementWithTimeStamp, how_much_to_read, current_file);
			left_to_read -= readen;
			int k;
			for( k = 0; k < readen;) {
				unsigned int element_time;
				memcpy( &element_time, element, sizeof(int) );
				if(element_time > to_time) {
					end_read = 1;
					break;
				}
				if(element_time >= from_time) {
					if (CheckDumpAbort())
							return FS_ABORT;

					int err = send(element, size_elementWithTimeStamp, dump_id, ord, dump_type, 1, &availFrames);//TODO:total irrelevant
					
					if(err != 0) {
						logg(error, "E:transmitsplpacket error: %d", err);
						if (err == -1)//transmition not allowed
							vTaskDelay(100);
						else {
							end_read = 1;
							break;
						}
					}
					else {
						if (availFrames != NO_TX_FRAME_AVAILABLE) {
							k++;
							element += size_elementWithTimeStamp;
							(*sent)++;
							ord++;
						} else {
							logg(event, "V:dump.no available frames\n");
							vTaskDelay(100);
						}
					}
				} else {
					k++;
					element += size_elementWithTimeStamp;
				}
			}
			if(end_read)
				break;
		}
		error = f_managed_close(&current_file);
		if (error == COULD_NOT_GIVE_SEMAPHORE_ERROR)
			return FS_COULD_NOT_GIVE_SEMAPHORE;
		if (error != F_NO_ERROR)
			return FS_FAIL;
	} while( getFileIndex(c_file.creation_time, c_file.last_time_modified) >= index_current &&
			getFileIndex(c_file.creation_time, to_time) >= index_current );

	return FS_SUCCSESS;
}

void DumpStart(dump_arguments_t *task_args){
	while(1) {
		if (NULL == task_args) {
			FinishDump(NULL, ACK_DUMP_ABORT, NULL, 0);
			continue;;
		}
		int err = 0;
		int ack_return_code = ACK_DUMP_FINISHED;
		FileSystemResult result = 0;
		int num_packets_read = 0; //number of packets read from buffer (single_time)
		int total_packets_read = 0; //total number of packets read from buffer
		unsigned int num_of_elements = 0;
		int size_of_element = 0;
		char filename[MAX_F_FILE_NAME_SIZE] = { 0 };
		err = getTelemetryMetaData(task_args->dump_type, filename, &size_of_element);
		if(0 != err) {
			logg(error, "E:problem during dump init with err %d\n", err);
			FinishDump(task_args, ACK_DUMP_ABORT, (unsigned char*) &err,sizeof(err));
			continue;
		}
		logg(DMPInfo, "I:filename: %s, size of element: %d t_start: %d t_end: %d\n", filename, size_of_element, task_args->t_start, task_args->t_end);

		SendAckPacket(ACK_DUMP_START, task_args->id, task_args->ord,
				(unsigned char*) &num_of_elements, sizeof(num_of_elements));

		time_unix curr = 0;
		Time_getUnixEpoch(&curr);
		logg(DMPInfo, "I:starting dump loop at time: %u\n", curr);
		result = c_fileReadAndSend(filename, task_args->t_start, task_args->t_end, &total_packets_read, task_args->id, (char)(task_args->dump_type));

		/*result = FirstScan((char*)filename,
				task_args->t_start,
				task_args->t_end,
				&total_packets_read,
				task_args->id,
				task_args->dump_type);*/
		if (result) {
			logg(error, "E:%d c_fileReadAndSend returned\n", result);
			if (result == FS_ABORT)
				ack_return_code = ACK_DUMP_ABORT;
		} else
			logg(DMPInfo, "I:finish dump gracefully %d transmitted", num_packets_read);

		FinishDump(task_args, ack_return_code, NULL, 0);
	}
}

void DumpTask(void *args) {
	logg(DMPInfo, "I:starting the  dump task\n");
	f_managed_enterFS();
	DumpStart((dump_arguments_t *) args);
}


dump_arguments_t dmp_pckt;
int DumpTelemetry(sat_packet_t *cmd)
{
	logg(DMPInfo, "I:Starting DumpTelemetry\n");
	if (NULL == cmd)
		return -1;

	eTaskState state;
	if (xDumpHandle != NULL) {
		state = eTaskGetState( xDumpHandle );
		logg(DMPInfo, "I:eTaskState=%d\n", state);
		if (state != eSuspended)
			return 0;
	}

	//dump_arguments_t *dmp_pckt = malloc(sizeof(*dmp_pckt));
	unsigned int offset = 0;
	dmp_pckt.id = cmd->ID;
	dmp_pckt.ord = cmd->ordinal;

	memcpy(&dmp_pckt.dump_type, cmd->data, sizeof(dmp_pckt.dump_type));
	offset += sizeof(dmp_pckt.dump_type);

	memcpy(&dmp_pckt.t_start, cmd->data + offset, sizeof(dmp_pckt.t_start));
	offset += sizeof(dmp_pckt.t_start);

	memcpy(&dmp_pckt.t_end, cmd->data + offset, sizeof(dmp_pckt.t_end));

	//if (xSemaphoreTake(xDumpLock,SECONDS_TO_TICKS(1)) != pdTRUE) {
	//	return E_GET_SEMAPHORE_FAILED;
	//}

	if (xDumpHandle != NULL) {
		if ( state == eSuspended )
			vTaskResume(xDumpHandle);
	}	else
			xTaskCreate(DumpTask, (const signed char* const )"DumpTask", 2000,
				(void* )&dmp_pckt, configMAX_PRIORITIES - 2, &xDumpHandle);

	return 0;
}

unsigned short CalcPacketSize(char dump_type)
 {
	switch (dump_type) {
		case tlm_eps_raw_mb:
			return 120;//TODO:update and compress
		case tlm_eps_eng_mb:
			return 120;//TODO:update and compress
		case tlm_tx:
			return 220;
		case tlm_rx:
			return 220;
		case tlm_antA:
			return 221;
		case tlm_antB:
			return 221;
		case tlm_log:
			return 225;
		case tlm_wod:
			return 92;//TODO:update and compress
		case tlm_pic32:
			return 216;
		case tlm_radfet:
			return 224;
		default:
			return 0;

	}
}

int SendAll(unsigned char dump_id, char dump_type, unsigned short total, int* sent)
{
	C_FILE c_file;
	int ord = 0;
	void* element;
	*sent = 0;
	char* tmp = data_to_send;
	unsigned short sz = CalcPacketSize(dump_type);
	unsigned short size_elementWithTimeStamp = c_file.size_of_element + sizeof(unsigned int);
	int availFrames = 0;
	while (ord < total) {
		memcpy(&element, tmp, sz);
		int err = send(element, size_elementWithTimeStamp, dump_id, ord, dump_type, total, &availFrames);
		if(err != 0) {
			logg(error, "E:transmitsplpacket error: %d", err);
			if (err == -1)//transmition not allowed
				vTaskDelay(100);
			else {
				return err;
			}
		}
		else {
			if (availFrames != NO_TX_FRAME_AVAILABLE) {
				tmp += sz;
				(*sent)++;
				ord++;
			} else {
				logg(DMPInfo, "I:dump.no available frames\n");
				vTaskDelay(100);
			}
		}
	}
	return 0;
}


int SendByIndex(unsigned char dump_id, char dump_type, unsigned short total, int* sent, sat_packet_t pack) {
	C_FILE c_file;
	*sent = 0;
	void* element;
	unsigned short sz = CalcPacketSize(dump_type);
	unsigned short size_elementWithTimeStamp = c_file.size_of_element + sizeof(unsigned int);
	unsigned char* data1 = (unsigned char*)pack.data;
	unsigned short length = pack.length;
	unsigned short ind1;
	int availFrames = 0;
	char* tmp = data_to_send;
	for (int i = 0; i < length; i++) {
		memcpy(&ind1, data1, 2);
		tmp = data_to_send + sz * ind1;
		memcpy(&element, tmp, sz);
		int err = send(element, size_elementWithTimeStamp, dump_id, ind1, dump_type, total, &availFrames);
		if(err != 0) {
			logg(error, "E:transmitsplpacket error: %d", err);
			if (err == -1)//transmition not allowed
				vTaskDelay(100);
			else {
				return err;
			}
		}
		else {
			if (availFrames != NO_TX_FRAME_AVAILABLE) {
				(*sent)++;
			} else {
				logg(DMPInfo, "I:dump.no available frames\n");
				vTaskDelay(100);
			}
		}
	}
	return 0;
}


FileSystemResult FirstScan(char* c_file_name,
		time_unix from_time,
		time_unix to_time,
		int* sent,
		unsigned short dump_id,
		unsigned short dump_type)
{
	C_FILE c_file;
	unsigned int addr;//FRAM ADDRESS
	char curr_file_name[MAX_F_FILE_NAME_SIZE+sizeof(int)*2];
	char* tmp = data_to_send;
	void* element;
	int end_read = 0;
	if(get_C_FILE_struct(c_file_name,&c_file,&addr)!=TRUE)
		return FS_NOT_EXIST;
	if(from_time < c_file.creation_time || from_time > to_time)
		from_time = c_file.creation_time;
	F_FILE* current_file = NULL;
	int index_current = getFileIndex(c_file.creation_time,from_time);
	get_file_name_by_index(c_file_name,index_current,curr_file_name);
	unsigned short size_elementWithTimeStamp = c_file.size_of_element + sizeof(unsigned int);
	*sent = 0;
	do {
		get_file_name_by_index(c_file_name, index_current++, curr_file_name);
		int error = f_managed_open(curr_file_name, "r", &current_file);
		if ( error != 0 || curr_file_name == NULL )
			continue;
		int file_length = f_filelength(curr_file_name);
		int length = file_length / (size_elementWithTimeStamp);//number of elements in currnet_file
		int left_to_read = length;
		int how_much_to_read = 0;
		long readen = 0;
		f_seek( current_file, 0L , SEEK_SET );
		int j;
		for(j = 0; j < length; j += how_much_to_read) {
			how_much_to_read = ELEMENTS_PER_READ;
			if(left_to_read < ELEMENTS_PER_READ)
				how_much_to_read = left_to_read;

			element = allocked_read_elements;
			readen = f_read(element, (size_t)size_elementWithTimeStamp, how_much_to_read, current_file);
			left_to_read -= readen;
			int k;
			for( k = 0 ; k < readen ; k++) {
				unsigned int element_time;
				memcpy( &element_time, element, sizeof(int) );
				if(element_time > to_time) {
					end_read = 1;
					break;
				}
				if(element_time >= from_time) {
					if (CheckDumpAbort())
							return FS_ABORT;
					//int err = send(element, size_elementWithTimeStamp, dump_id, ord++, dump_type,  &availFrames);
					memcpy(data_to_send, element, size_elementWithTimeStamp);
					tmp += size_elementWithTimeStamp;
				} else {
					element += size_elementWithTimeStamp;
				}
			}
			if(end_read)
				break;
		}
		error = f_managed_close(&current_file);
		if (error == COULD_NOT_GIVE_SEMAPHORE_ERROR)
			return FS_COULD_NOT_GIVE_SEMAPHORE;
		if (error != F_NO_ERROR)
			return FS_FAIL;
	} while( getFileIndex(c_file.creation_time, c_file.last_time_modified) >= index_current &&
			getFileIndex(c_file.creation_time, to_time) >= index_current );
	unsigned short sz = CalcPacketSize(dump_type);
	unsigned short total = (tmp - data_to_send) / sz;
	int err = SendAll(dump_id, dump_type, total, sent);
	if (err != 0) {
		return FS_FAIL;
	}
	return FS_SUCCSESS;
}

// first scan v
// send all v
// send by index v()
// lets lehorid v
// beacon
// file read and send
