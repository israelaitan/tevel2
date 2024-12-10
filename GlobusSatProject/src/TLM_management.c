#include <freertos/FreeRTOS.h>

#include <freertos/task.h>
#include <freertos/semphr.h>

#include <config/config_fat.h>

#include <hcc/api_mdriver_atmel_mcipdc.h>
#include <hcc/api_hcc_mem.h>

#include <at91/utility/trace.h>

#include <hal/Timing/Time.h>
#include <hal/errors.h>
#include <hal/Storage/FRAM.h>

#include <string.h>
#include <stdlib.h>

#include "TLM_management.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Maintenance/Log.h"

char allocked_write_element[MAX_ELEMENT_SIZE];
char allocked_read_element[(MAX_ELEMENT_SIZE)*(ELEMENTS_PER_READ)];
char allocked_delete_element[MAX_ELEMENT_SIZE];

xSemaphoreHandle xFileOpenHandler;


static int getSdIndex()
{
	FS fs;

	if(FRAM_read((unsigned char*)&fs,FSFRAM,sizeof(FS))!=0)
	{
		return -1;
	}
	return fs.sd_index;
}

static int setSdIndex(int new_sd_index)
{

	if(FRAM_write((unsigned char*)&new_sd_index,FSFRAM+sizeof(int),sizeof(int))!=0)
	{
		return -1;
	}
	return 0;
}

FileSystemResult reset_FRAM_FS()
{
	int num_of_files = 0;
	if(FRAM_write((unsigned char*)&num_of_files,FSFRAM,sizeof(int))!=0)
	{
		return FS_FAT_API_FAIL;
	}
	return FS_SUCCSESS;
}

Boolean TLMfile(char* filename)
{
	int len = strlen(filename);
	if(strcmp(".TLM",filename+len-4)==0)
	{
		return TRUE;
	}
	return FALSE;
}


void sd_format(int index)
{
	int format_err = f_format(index,F_FAT32_MEDIA);
	(void)format_err;
}
void deleteDir(char* name, Boolean delete_folder)
{
	F_FIND find;
	char temp_name[256]={0};
	if (!f_findfirst(name,&find))
	{
		do
		{

			if (find.attr&F_ATTR_DIR)
			{
				sprintf(temp_name,"%s/%s",name,find.filename);
				deleteDir(temp_name,TRUE);
				if(delete_folder)
				{
					f_rmdir(temp_name);
				}
			}
			else
			{
				sprintf(temp_name,"%s/%s",name,find.filename);
				if (TLMfile(temp_name))
				{
					f_delete(temp_name);
				}
			}
		} while (!f_findnext(&find));
	}
}

// return -1 for FRAM fail
static int getNumOfFilesInFS()
{
	FS fs;

	if(FRAM_read((unsigned char*)&fs,FSFRAM,sizeof(FS))!=0)
	{
		return -1;
	}
	return fs.num_of_files;
}
//return -1 on fail

static int setNumOfFilesInFS(int new_num_of_files)
{

	if(FRAM_write((unsigned char*)&new_num_of_files,FSFRAM,sizeof(int))!=0)
	{
		return -1;
	}
	return 0;
}

int f_managed_enterFS()
{
	return f_enterFS();
}
int f_managed_releaseFS()
{
	f_releaseFS();
	return 0;
}

int f_managed_open(char* file_name, char* config, F_FILE** fileHandler) {
	//printf("f_managed_open. before take\n");
	//if (xSemaphoreTake(xFileOpenHandler, FS_TAKE_SEMPH_DELAY) == pdTRUE) {
		*fileHandler = f_open(file_name, config);
		int lastError = f_getlasterror();
		if (*fileHandler != NULL)
				return 0;
//		xSemaphoreGive(xFileOpenHandler);
		return lastError;
//	} else
	//	return COULD_NOT_TAKE_SEMAPHORE_ERROR;
}

int f_managed_close(F_FILE** fileHandler) {
	//printf("f_managed_close. before give\n");
	if (fileHandler != NULL)
		f_close(*fileHandler);
	//xSemaphoreGive(xFileOpenHandler);
	return 0;
}

FileSystemResult createSemahores_FS()
{
	//xFileOpenHandler = xSemaphoreCreateCounting(FS_MAX_OPENFILES, FS_MAX_OPENFILES);
	vSemaphoreCreateBinary(xFileOpenHandler);
	if (xFileOpenHandler == NULL)
		return FS_COULD_NOT_CREATE_SEMAPHORE;
	return FS_SUCCSESS;
}

FileSystemResult formatAndCreateFiles()
{
	sd_format(DEFAULT_SD);
	FS fs = {0,DEFAULT_SD};
	if(FRAM_write((unsigned char*)&fs,FSFRAM,sizeof(FS))!=0)
	{
		return FS_FAT_API_FAIL;
	}
	Boolean8bit tlms_created[NUMBER_OF_TELEMETRIES];
	TelemetryCreateFiles(tlms_created);
	return FS_SUCCSESS;
}

FileSystemResult InitializeFS(Boolean first_time)
{
	FileSystemResult FS_result = createSemahores_FS();
	if (FS_result == FS_COULD_NOT_CREATE_SEMAPHORE)
		return FS_result;

	int ret;
	hcc_mem_init(); /* Initialize the memory to be used by filesystem */

	ret = fs_init(); /* Initialize the filesystem */
	if(ret != F_NO_ERROR )
	{
		return FS_FAT_API_FAIL;
	}

	ret = f_managed_enterFS(); /* Register this task with filesystem *///task 1 enter fs
	if (ret == COULD_NOT_TAKE_SEMAPHORE_ERROR)
		return FS_COULD_NOT_TAKE_SEMAPHORE;

	if( ret != F_NO_ERROR)
	{
		return FS_FAT_API_FAIL;
	}

	int sd_index = getSdIndex();
	if(first_time||(sd_index!=0&&sd_index!=1))
	{
		sd_index=DEFAULT_SD;
		setSdIndex(sd_index);
	}

	ret = f_initvolume( 0, atmel_mcipdc_initfunc, sd_index ); /* Initialize volID as safe */

	if( F_ERR_CARDREMOVED == ret )
	{
		DeInitializeFS();
		vTaskDelay(1000);
		fs_init();
		f_managed_enterFS();
		f_initvolume( 0, atmel_mcipdc_initfunc, sd_index );

		return FS_FAT_API_FAIL;
	}
	else if( F_NO_ERROR != ret)
	{
		return FS_FAT_API_FAIL;
	}

	if(first_time)
	{
		return formatAndCreateFiles();
	}


	return FS_SUCCSESS;
}

//only register the chain, files will create dynamically
FileSystemResult c_fileCreate(char* c_file_name,
		int size_of_element)
{
	if(strlen(c_file_name)>MAX_F_FILE_NAME_SIZE)//check len
	{
		return FS_TOO_LONG_NAME;
	}

	C_FILE c_file; //chain file descriptor
	strcpy(c_file.name,c_file_name);
	Time_getUnixEpoch(&c_file.creation_time);//get current time
	c_file.size_of_element =size_of_element;
	c_file.last_time_modified = FIRST_TIME;//no written yet
	int num_of_files_in_FS = getNumOfFilesInFS();
	if(num_of_files_in_FS == -1)
	{
		return FS_FRAM_FAIL;
	}
	int c_file_address =C_FILES_BASE_ADDR+num_of_files_in_FS*sizeof(C_FILE);
	if(FRAM_write((unsigned char*)&c_file,
			c_file_address,sizeof(C_FILE))!=0)//write c_file struct in FRAM
	{
			return FS_FRAM_FAIL;
	}
	if(setNumOfFilesInFS(num_of_files_in_FS+1)!=0)
	{
		return FS_FRAM_FAIL;
	}
	if(f_mkdir(c_file_name)!=0)
	{
		int err = fm_getlasterror();
		printf("Failed in f_mkdir(%s) with error: %d\n ", c_file_name, err);
		return FS_FAIL;
	}

	return FS_SUCCSESS;
}
//write element with timestamp to file
static void writewithEpochtime(F_FILE* file, byte* data, int size,unsigned int time)
{
	int i;
	for (i = 0; i < NUMBER_OF_WRITES; i++)
	{
		int number_of_writes;
		number_of_writes = f_write( &time,sizeof(unsigned int),1, file );
		number_of_writes += f_write( data, size,1, file );
		//printf("writing element, time is: %u\n",time);
	}
	f_flush( file ); /* only after flushing can data be considered safe */
}

static void write(F_FILE* file, byte* data, int size)
{
	f_write( data, 1, size, file );
	f_flush( file );
}
// get C_FILE struct from FRAM by name
Boolean get_C_FILE_struct(char* name,C_FILE* c_file,unsigned int *address)
{
	int i;
	unsigned int c_file_address = 0;
	int err_read=0;
	int num_of_files_in_FS = getNumOfFilesInFS();
	for(i =0; i < num_of_files_in_FS; i++)			//search correct c_file struct
	{
		c_file_address= C_FILES_BASE_ADDR+sizeof(C_FILE)*(i);
		err_read = FRAM_read((unsigned char*)c_file,c_file_address,sizeof(C_FILE));
		if(0 != err_read)
		{
			return FALSE;
		}

		if(!strcmp(c_file->name,name))
		{
			if(address != NULL)
			{
				*address = c_file_address;
			}
			return TRUE;//stop when found
		}
	}
	return FALSE;
}
//calculate index of file in chain file by time
int getFileIndex(unsigned int creation_time, unsigned int current_time)
{

	if(current_time<creation_time)
	{
		return 0;
	}
	return ((current_time-creation_time)/SKIP_FILE_TIME_SEC);
}

//write to curr_file_name
void get_file_name_by_index(char* c_file_name,int index,char* curr_file_name)
{
	sprintf(curr_file_name,"a:/%s/%d.%s", c_file_name, index, FS_FILE_ENDING);
}

FileSystemResult c_fileReset(char* c_file_name)
{
	C_FILE c_file;
	unsigned int addr;//FRAM ADDRESS
	//F_FILE *file;
	//char curr_file_name[MAX_F_FILE_NAME_SIZE+sizeof(int)*2];

	unsigned int curr_time;
	Time_getUnixEpoch(&curr_time);
	if(get_C_FILE_struct(c_file_name,&c_file,&addr)!=TRUE)//get c_file
	{
		return FS_NOT_EXIST;
	}
	F_FIND find;
	char temp_name[256];
	sprintf(temp_name,"A:/%s/*.*",c_file_name);
	if (!f_findfirst(temp_name,&find))
	{
		do
		{
			sprintf(temp_name,"A:/%s/%s", c_file_name, find.filename);
			f_delete(temp_name);
		} while (!f_findnext(&find));
	}
	return FS_SUCCSESS;
}

FileSystemResult c_fileWrite(char* c_file_name, void* element)
{
	return _c_fileWrite(c_file_name, element, 0, 1);
}

FileSystemResult _c_fileWrite(char* c_file_name, void* element, int size, unsigned char withTime)
{
	C_FILE c_file;
	unsigned int addr;//FRAM ADDRESS
	F_FILE *file = NULL;
	char curr_file_name[MAX_F_FILE_NAME_SIZE+sizeof(int)*2];

	unsigned int curr_time;
	Time_getUnixEpoch(&curr_time);
	if(get_C_FILE_struct(c_file_name,&c_file,&addr)!=TRUE)//get c_file
	{
		return FS_NOT_EXIST;
	}
	int index_current = getFileIndex(c_file.creation_time,curr_time);
	get_file_name_by_index(c_file_name,index_current,curr_file_name);
	int error = f_managed_open(curr_file_name, "a+", &file);
	if (error == COULD_NOT_TAKE_SEMAPHORE_ERROR)
		return FS_COULD_NOT_TAKE_SEMAPHORE;
	if (file == NULL || error != 0)
		return FS_FAIL;
	if(error!=0)
	{
		f_managed_close(&file);	/* data is also considered safe when file is closed */
	}
	if (size == 0)
		size = c_file.size_of_element;
	if (withTime)
		writewithEpochtime(file,element, size, curr_time);
	else
		write(file,element, size);
	f_managed_close(&file);	/* data is also considered safe when file is closed */
	c_file.last_time_modified= curr_time;
	if(FRAM_write((unsigned char *)&c_file,addr,sizeof(C_FILE))!=0)//update last written
	{
		return FS_FRAM_FAIL;
	}
	return FS_SUCCSESS;
}

FileSystemResult fileWrite(char* file_name, void* element,int size)
{
	F_FILE *file;
	unsigned int curr_time;
	Time_getUnixEpoch(&curr_time);
	file= f_open(file_name,"a+");
	writewithEpochtime(file,element,size,curr_time);
	f_flush(file);
	f_close(file);
	return FS_SUCCSESS;
}

static FileSystemResult deleteElementsFromFile(char* file_name,unsigned long from_time,
		unsigned long to_time,int full_element_size)
{
	F_FILE* file = NULL;
	int error = f_managed_open(file_name,"r", &file);
	if (error == COULD_NOT_TAKE_SEMAPHORE_ERROR)
		return FS_COULD_NOT_TAKE_SEMAPHORE;
	else if (error != 0 || file == NULL)
		return FS_FAIL;

	F_FILE* temp_file = NULL;
	error = f_managed_open("temp","a+", &temp_file);
	if (error == COULD_NOT_TAKE_SEMAPHORE_ERROR)
		return FS_COULD_NOT_TAKE_SEMAPHORE;
	else if (error != 0 || file == NULL)
		return FS_FAIL;
	char* buffer = allocked_delete_element;
	int i;
	for(i = 0; i<f_filelength(file_name); i+=full_element_size)
	{

		f_read(buffer,1,full_element_size,file);
		unsigned int element_time = *((unsigned int*)buffer);
		if(element_time<from_time||element_time>to_time)
		{
			f_write(buffer,1,full_element_size,temp_file);
		}
	}
	f_managed_close(&file);
	f_managed_close(&temp_file);
	//free(buffer);
	f_delete(file_name);
	f_rename("temp",file_name);
	return FS_SUCCSESS;

}

FileSystemResult c_fileGetSizeOfElement(char* c_file_name,int* element_size)
{
	C_FILE c_file;
	unsigned int addr;//FRAM ADDRESS
	if(get_C_FILE_struct(c_file_name,&c_file,&addr)!=TRUE)//get c_file
	{
		return FS_NOT_EXIST;
	}
	*element_size = c_file.size_of_element;
	return FS_SUCCSESS;
}

FileSystemResult c_fileDeleteElements(char* c_file_name, time_unix from_time,
		time_unix to_time)
{
	C_FILE c_file;
	unsigned int addr;//FRAM ADDRESS
	char curr_file_name[MAX_F_FILE_NAME_SIZE+sizeof(int)*2];

	unsigned int curr_time;
	Time_getUnixEpoch(&curr_time);
	if(get_C_FILE_struct(c_file_name,&c_file,&addr)!=TRUE)//get c_file
	{
		return FS_NOT_EXIST;
	}
	int first_file_index = getFileIndex(c_file.creation_time,from_time);
	int last_file_index = getFileIndex(c_file.creation_time,to_time);
	if(first_file_index+1<last_file_index)//delete all files between first to kast file
	{
		int i;
		for(i =first_file_index+1; i<last_file_index;i++)
		{
			get_file_name_by_index(c_file_name,i,curr_file_name);
			f_delete(curr_file_name);
		}
	}
	get_file_name_by_index(c_file_name,first_file_index,curr_file_name);
	deleteElementsFromFile(curr_file_name,from_time,to_time,c_file.size_of_element+sizeof(int));
	if(first_file_index!=last_file_index)
	{
		get_file_name_by_index(c_file_name,last_file_index,curr_file_name);
		deleteElementsFromFile(curr_file_name,from_time,to_time,c_file.size_of_element+sizeof(int));
	}
	return FS_SUCCSESS;
}

FileSystemResult fileRead(char* c_file_name,byte* buffer, int size_of_buffer,
		time_unix from_time, time_unix to_time, int* read, int element_size)
{
	*read=0;
	F_FILE* current_file= f_open(c_file_name,"r+");
	int buffer_index = 0;
	void* element;
	unsigned int size_elementWithTimeStamp = element_size+sizeof(unsigned int);
	element = malloc(size_elementWithTimeStamp);//store element and his timestamp
	unsigned int length =f_filelength(c_file_name)/(size_elementWithTimeStamp);//number of elements in currnet_file
	int err_fread=0;

	f_seek( current_file, 0L , SEEK_SET );
	unsigned int j;
	for(j=0;j < length;j++)
	{
		err_fread = f_read(element,(size_t)size_elementWithTimeStamp,(size_t)1,current_file);
		(void)err_fread;
		unsigned int element_time = *((unsigned int*)element);
		if(element_time > to_time)
		{
			break;
		}

		if(element_time >= from_time)
		{
			if((unsigned int)buffer_index>(unsigned int)size_of_buffer)
			{
				return FS_BUFFER_OVERFLOW;
			}
			(*read)++;
			memcpy(buffer + buffer_index,element,size_elementWithTimeStamp);
			buffer_index += size_elementWithTimeStamp;
		}
	}
	f_close(current_file);


	//free(element);

	return FS_SUCCSESS;
}

//c file read
FileSystemResult c_fileRead(char* c_file_name,byte* buffer, int size_of_buffer,
		time_unix from_time, time_unix to_time, int* read, time_unix* last_read_time, unsigned int resolution)
{
	time_unix lastCopy_time = 0;
	C_FILE c_file;
	unsigned int addr;//FRAM ADDRESS
	char curr_file_name[MAX_F_FILE_NAME_SIZE+sizeof(int)*2];


	int buffer_index = 0;
	void* element;
	int end_read = 0;
	if(get_C_FILE_struct(c_file_name,&c_file,&addr)!=TRUE)//get c_file
	{
		return FS_NOT_EXIST;
	}
	if(from_time < c_file.creation_time || from_time > to_time)
	{
		from_time=c_file.creation_time;
	}
	F_FILE* current_file = NULL;
	int index_current = getFileIndex(c_file.creation_time,from_time);
	get_file_name_by_index(c_file_name,index_current,curr_file_name);
	unsigned int size_elementWithTimeStamp = c_file.size_of_element+sizeof(unsigned int);
	do
	{
		get_file_name_by_index(c_file_name,index_current++,curr_file_name);
		int error = f_managed_open(curr_file_name, "r", &current_file);
		if (error != 0 || curr_file_name == NULL)
			continue;
		int file_length =f_filelength(curr_file_name);
		int length = file_length/(size_elementWithTimeStamp);//number of elements in currnet_file
		int left_to_read = length;
		int how_much_to_read=0;
		int err_fread=0;
		(void)err_fread;
		f_seek( current_file, 0L , SEEK_SET );
		int j;
		for(j=0;j < length;j+=how_much_to_read)
		{
			if(left_to_read<ELEMENTS_PER_READ) {
				how_much_to_read = left_to_read;
			}
			else {
				how_much_to_read=ELEMENTS_PER_READ;
			}
			element = allocked_read_element;
			err_fread = f_read(element, (size_t)size_elementWithTimeStamp, how_much_to_read, current_file);
			int k;
			for(k=0;k<how_much_to_read;k++){
				unsigned int element_time;
				memcpy(&element_time, element, sizeof(int));
				if(element_time > to_time) {
					end_read = 1;
					break;
				}
				if(element_time >= from_time) {
					*last_read_time = element_time;
					if( ((unsigned int)buffer_index + size_elementWithTimeStamp) > (unsigned int)size_of_buffer ) {
						error = f_managed_close(&current_file);
						if (error == COULD_NOT_GIVE_SEMAPHORE_ERROR)
							return FS_COULD_NOT_GIVE_SEMAPHORE;
						return FS_BUFFER_OVERFLOW;
					}

					if (element_time >= (time_unix)resolution + lastCopy_time) {
						(*read)++;
						memcpy(buffer + buffer_index, element, size_elementWithTimeStamp);
						buffer_index += size_elementWithTimeStamp;
						lastCopy_time = element_time;
					}
				}
				element += (c_file.size_of_element + sizeof(int));
			}
			if(end_read)
				break;
		}
		error = f_managed_close(&current_file);
		if (error == COULD_NOT_GIVE_SEMAPHORE_ERROR)
			return FS_COULD_NOT_GIVE_SEMAPHORE;
		if (error != F_NO_ERROR)
			return FS_FAIL;
	} while(getFileIndex(c_file.creation_time,c_file.last_time_modified)>=index_current&&
			getFileIndex(c_file.creation_time,to_time)>=index_current);

	return FS_SUCCSESS;
}

int print_file(char* c_file_name)
{
	C_FILE c_file;
	unsigned int c_file_addr;
	if (get_C_FILE_struct(c_file_name, &c_file, &c_file_addr) == FALSE)
	{
		return -1;
	}
	int read=0;
	char *buffer = malloc(c_file.size_of_element+sizeof(int)*2);
	time_unix last_read_time;
	c_fileRead(c_file_name,(byte*)buffer,c_file.size_of_element+sizeof(int)*2,
			c_file.last_time_modified,c_file.last_time_modified, &read, &last_read_time, 1);

	printf("%d\n",c_file.size_of_element);
	unsigned int i;
	for(i= 0;i<c_file.size_of_element+sizeof(int);i++)
	{
	    printf("%X",buffer[i]);
	}

	free(buffer);
	return 0;
}

void DeInitializeFS( void )
{
	int err = f_delvolume( _SD_CARD ); /* delete the volID */

	if(err!=0)
		 printf("Failed in f_devolume(SD_CARD)");

	err = fs_delete(); /* delete the filesystem */
	if(err!=0)
		printf("Failed in fs_delete()");

	err = hcc_mem_delete(); /* free the memory used by the filesystem */
	if(err!=0)
		printf("Failed in hcc_mem_delete()");
}
