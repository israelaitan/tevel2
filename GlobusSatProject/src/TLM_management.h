/*
 * TM_managment.h
 *
 *  Created on: Apr 8, 2019
 *      Author: Hoopoe3n
 */

#ifndef TM_MANAGMENT_H_
#define TM_MANAGMENT_H_

#include <hcc/api_fat.h>
#include <GlobalStandards.h>

#define MAX_F_FILE_NAME_SIZE 9
#define FIRST_ELEMENT_IN_C_FILE 0
#define LAST_ELEMENT_IN_C_FILE 0
#define DEFAULT_NUM_OF_FILES 0
#define DEFAULT_SD 0
#define FS_FILE_ENDING	"TLM"
#define FS_FILE_ENDING_SIZE	3

#ifndef FSFRAM
#define FSFRAM 0x10000
#endif
#define FS_MAX_OPENFILES	F_MAXFILES - 2
#define FS_MAX_TASK_TETER	FAT_MAXTASK - 1

#define COULD_NOT_TAKE_SEMAPHORE_ERROR 	-1
#define COULD_NOT_GIVE_SEMAPHORE_ERROR 	-2
#define FILE_NULL_ERROR					-3

typedef enum
{
	FS_SUCCSESS,
	FS_DUPLICATED,
	FS_LOCKED,
	FS_TOO_LONG_NAME,
	FS_BUFFER_OVERFLOW,
	FS_NOT_EXIST,
	FS_ALLOCATION_ERROR,
	FS_FRAM_FAIL,
	FS_FAT_API_FAIL,
	FS_FAIL,
	FS_COULD_NOT_CREATE_SEMAPHORE,
	FS_COULD_NOT_TAKE_SEMAPHORE,
	FS_COULD_NOT_GIVE_SEMAPHORE,
	FS_ABORT
} FileSystemResult;

#define NUMBER_OF_WRITES 1
#define SKIP_FILE_TIME_SEC ((60*60*24*0.5)/NUMBER_OF_WRITES)
#define _SD_CARD (0)
#define FIRST_TIME (-1)
#define FILE_NAME_WITH_INDEX_SIZE (MAX_F_FILE_NAME_SIZE+sizeof(int)*2)
#define ELEMENTS_PER_READ (1200)
#define MAX_ELEMENT_SIZE (225)
#define FS_TAKE_SEMPH_DELAY	(1000 * 30)

//struct for filesystem info
typedef struct
{
	int num_of_files;
	int sd_index;
} FS;

//struct for chain file info
typedef struct
{
	int size_of_element;
	char name[FILE_NAME_WITH_INDEX_SIZE];
	unsigned int creation_time;
	unsigned int last_time_modified;

} C_FILE;
#define C_FILES_BASE_ADDR (FSFRAM+sizeof(FS))

int f_managed_enterFS();

int f_managed_releaseFS();

int f_managed_open(char* file_name, char* config, F_FILE** fileHandler);

int f_managed_close(F_FILE** fileHandler);

FileSystemResult reset_FRAM_FS();

void sd_format(int index);

/*
 *
 */
void deleteDir(char* name, Boolean delete_folder);

/*!
 * Initializes the file system.
 * @note call once for boot and after DeInitializeFS.
 * @return FS_FAIL if Initializing the FS failed,
 * FS_ALLOCATION_ERROR on malloc error,
 * FS_SUCCSESS on success.
 */
FileSystemResult InitializeFS();

/*!
 * DeInitializes the file system.
 */
void DeInitializeFS();

/*!
 * Create new c_file.
 * @param c_file_name the name of the c_file.
 * @param size_of_element size of the structure.
 * DEFAULT_NUM_OF_FILES for default(recommended).
 * @return FS_TOO_LONG_NAME if c_file_name size is bigger then MAX_F_FILE_NAME_SIZE,
 * FS_FRAM_FAIL,
 * FS_SUCCSESS on success.
 */
FileSystemResult c_fileCreate(char* c_file_name, int size_of_element);
/*!
 * Write element to c_file.
 * @param c_file_name the name of the c_file.
 * @param element the structure of the telemetry/data.
 * @return FS_NOT_EXIST if c_file not exist,
 * FS_LOCKED if c_file used by other thread,
 * FS_SUCCSESS on success.
 */
FileSystemResult c_fileWrite(char* c_file_name, void* element);

FileSystemResult _c_fileWrite(char* c_file_name, void* element, int size, unsigned char withTime);

//Perform the format of filesystem and creation of the files
FileSystemResult formatAndCreateFiles();

/*!
 * Delete elements from c_file from "from_time" to "to_time".
 * @param c_file_name the name of the c_file.
 * @param from_time time of first element, FIRST_ELEMENT_IN_C_FILE to first element.
 * @param to_time time of last element, LAST_ELEMENT_IN_C_FILE to last element.
 * @return FS_NOT_EXIST if c_file not exist,
 * FS_LOCKED if c_file used by other thread,
 * FS_SUCCSESS on success.
 */
FileSystemResult c_fileDeleteElements(char* c_file_name, time_unix from_time, time_unix to_time);
/*!
 * Find number of elements from "from_time" to "to_time"
 * @param c_file_name the name of the c_file.
 * @param from_time time of first element, FIRST_ELEMENT_IN_C_FILE to first element.
 * @param to_time time of last element, LAST_ELEMENT_IN_C_FILE to last element.
 * @return num of elements.
 */
int c_fileGetNumOfElements(char* c_file_name, time_unix from_time, time_unix to_time);

/*!
 * Find size of a single element in the c_file
 * @param c_file_name the name of the c_file.
 * @return size of element.
 */
FileSystemResult c_fileGetSizeOfElement(char* c_file_name,int* size_of_element);

/*!
 * Read elements from c_file to buffer
 * @param c_file_name the name of the c_file.
 * @param buffer.
 * @param size_of_buffer.
 * @param read[out] number of elements read.
 * @param from_time time of first element, FIRST_ELEMENT_IN_C_FILE to first element.
 * @param to_time time of last element, LAST_ELEMENT_IN_C_FILE to last element.
 * @return FS_BUFFER_OVERFLOW if size_of_buffer too small,
 * @return FS_NOT_EXIST if c_file not exist,
 * FS_SUCCSESS on success.
 */
FileSystemResult c_fileRead(char* c_file_name, byte* buffer, int size_of_buffer,
		time_unix from_time, time_unix to_time, int* read,time_unix* last_read_time, unsigned int resolution);

FileSystemResult c_fileGetSizeOfElement(char* c_file_name,int* element_size);

//print c_file for testing
void print_files(char* c_file_name);

int print_file(char* c_file_name);

FileSystemResult c_fileReset(char* c_file_name);

int FS_test();

void test_i();

void get_file_name_by_index(char* c_file_name,int index,char* curr_file_name);

int getFileIndex(unsigned int creation_time, unsigned int current_time);

Boolean get_C_FILE_struct(char* name,C_FILE* c_file,unsigned int *address);

#endif /* TM_MANAGMENT_H_ */
