/*
 * filesystem.c
 *
 *  Created on: 20 Ã¡Ã®Ã¸Ãµ 2019
 *      Author: Idan
 */


#include <satellite-subsystems/GomEPS.h>
#include <hal/Timing/Time.h>
#include <hcc/api_fat.h>
#include <hal/errors.h>
#include <hcc/api_hcc_mem.h>
#include <string.h>
#include <hcc/api_mdriver_atmel_mcipdc.h>
#include <hal/Storage/FRAM.h>
#include <at91/utility/trace.h>
#include "TLM_management.h"
#include <stdlib.h>
#include <GlobalStandards.h>

#define SKIP_FILE_TIME_SEC 1000000
#define _SD_CARD 0
#define FIRST_TIME -1
#define FILE_NAME_WITH_INDEX_SIZE MAX_F_FILE_NAME_SIZE+sizeof(int)*2

//struct for filesystem info
typedef struct
{
	int num_of_files;
} FS;
//TODO remove all 'PLZNORESTART' from code!!
#define PLZNORESTART() gom_eps_hk_basic_t myEpsTelemetry_hk_basic;	(GomEpsGetHkData_basic(0, &myEpsTelemetry_hk_basic)); //todo delete

//struct for chain file info
typedef struct
{
	int size_of_element;
	char name[FILE_NAME_WITH_INDEX_SIZE];
	unsigned int creation_time;
	unsigned int last_time_modified;
	int num_of_files;

} C_FILE;
#define C_FILES_BASE_ADDR (FSFRAM+sizeof(FS))


void delete_allTMFilesFromSD()
{
}
// return -1 for FRAM fail
static int getNumOfFilesInFS()
{
	FS fs;
	int err = FRAM_read( (unsigned char*)&fs, FSFRAM, sizeof(fs));
	if (err)
		return err;
	return fs.num_of_files;
}
//return -1 on fail
static int setNumOfFilesInFS(int new_num_of_files)
{
	return 0;
}
FileSystemResult InitializeFS(Boolean first_time)
{
	return FS_SUCCSESS;
}

//only register the chain, files will create dynamically
FileSystemResult c_fileCreate(char* c_file_name,
		int size_of_element)
{
	C_FILE c_file;
	int currTime = 0;
	unsigned int err = Time_getUnixEpoch(&currTime);
	if (err)
		return FS_FAIL;
	c_file.creation_time = currTime;
	c_file.last_time_modified = -1;
	c_file.name = c_file_name;
	c_file.num_of_files = 0;
	c_file.size_of_element = size_of_element;

	int n = getNumOfFilesInFS();
	if ( n < 0 )
		return FS_FAIL;
	unsigned int top = C_FILES_BASE_ADDR + n * sizeof(C_FILE);
	err = FRAM_write( (unsigned char *)&c_file, top, sizeof(C_FILE));
	if (err)
		return FS_FAIL;
	err = setNumOfFilesInFS(n + 1);
	if (err)
		return FS_FAIL;
	return FS_SUCCSESS;
}
//write element with timestamp to file
static void writewithEpochtime(F_FILE* file, byte* data, int size,unsigned int time)
{
}

// get C_FILE struct from FRAM by name
static Boolean get_C_FILE_struct(char* name, C_FILE* c_file, unsigned int *address)
{
	int nFiles = getNumOfFilesInFS();
	if (nFiles)
		return FALSE;
	unsigned int currAddr = C_FILES_BASE_ADDR;
	for (int i = 0; i < nFiles; i++) {
		int err = FRAM_read( c_file, currAddr, sizeof(C_FILE));
		if (err)
			return FALSE;
		if (strcmp( c_file->name, name ) == 0) {
			if (address)
				(*address) = currAddr;
			return TRUE;
		}
		currAddr += sizeof(C_FILE);
	}
	return FALSE;
}

//calculate index of file in chain file by time
static int getFileIndex(unsigned int creation_time, unsigned int current_time)
{
	return (current_time - creation_time) / SKIP_FILE_TIME_SEC;
}

//write to curr_file_name
void get_file_name_by_index(char* c_file_name, int index, char* curr_file_name)
{
	sprintf(curr_file_name, "%s%d.%s", c_file_name, index, FS_FILE_ENDING);
}

FileSystemResult c_fileReset(char* c_file_name)
{
	return FS_SUCCSESS;
}

FileSystemResult c_fileWrite(char* c_file_name, void* element)
{
	C_FILE c_file;
	Boolean  res = get_C_FILE_struct(c_file_name, &c_file, NULL);
	if (res == FALSE)
		return FS_FAIL;
	unsigned int currTime = 0;
	unsigned int err = Time_getUnixEpoch( &currTime );
	if (err)
		return FS_FAIL;
	unsigned int index = getFileIndex(c_file.creation_time, currTime);
	char curr_file_name[FILE_NAME_WITH_INDEX_SIZE];
	get_file_name_by_index(c_file_name, index, curr_file_name);
	//TODO: update num of files in the cfile struct and curr time
	FileSystemResult fsr = fileWrite(curr_file_name, element, c_file.size_of_element);
	return fsr;
}

FileSystemResult fileWrite(char* file_name, void* element,int size)
{
	int timeStamp = 0;
	unsigned int err = Time_getUnixEpoch(&timeStamp);
	if (err)
		return FS_FAIL;
	F_FILE *file;
	file = f_open(file_name, ”a”);
	if (!file)
		return FS_FAIL;
	if( f_write(timeStamp, 1, size_of(unsigned int), file) != 1 ) {
		f_close(file);
		//we can probe for error type using getlasterror
		//might be wrong if we manage to write time stamp and fail here
		return FS_FAIL;
	}
	if( f_write(element, 1, size) != 1 ) {
			f_close(file);
			return FS_FAIL;
	}
	f_close(file);
	return FS_SUCCSESS;
}
static FileSystemResult deleteElementsFromFile(char* file_name,unsigned long from_time,
		unsigned long to_time,int full_element_size)
{
	return FS_SUCCSESS;
}

FileSystemResult c_fileDeleteElements(char* c_file_name, time_unix from_time,
		time_unix to_time)
{
	return FS_SUCCSESS;
}

FileSystemResult fileRead(char* file_name, byte* buffer, int size_of_buffer,
		time_unix from_time, time_unix to_time, int* read, int element_size)
{
	//TODO: not completed
	F_FILE *file;
	file = f_open(file_name, ”r”);
	if (!file)
		return FS_FAIL;
	unsigned int currTimeStamp = 0;

	while ( from_time > currTimeStamp ) {
		if ( f_read(&currTimeStamp, 1, size_of(unsigned int), file) != 1 ) {
			f_close(file);
			//we can probe for error type using getlasterror
			//might be wrong if we manage to write time stamp and fail here
			return FS_FAIL;
		}
		long err = f_seek(file, element_size, SEEK_CUR);
		if (err)
			return FS_FAIL;
	}
	if (from_time == currTimeStamp) {
		long err = f_seek(file, -(element_size + sizeof(unsigned int)), SEEK_CUR);
		if (err)
			return FS_FAIL;
	}

	return FS_SUCCSESS;
}
FileSystemResult c_fileRead(char* c_file_name, byte* buffer, int size_of_buffer,
		time_unix from_time, time_unix to_time, int* read, time_unix* last_read_time)
{
	return FS_SUCCSESS;
}
void print_file(char* c_file_name)
{
}

void DeInitializeFS( void )
{
}
typedef struct{
	int a;
	int b;
}TestStruct ;
