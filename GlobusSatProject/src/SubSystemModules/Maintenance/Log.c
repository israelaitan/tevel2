/*
 * Log.c
 *
 *  Created on: 22 αιεμ 2020
 *      Author: USER
 */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "Log.h"
#include <hal/Timing/Time.h>
#include "SubSystemModules/Housekepping/TelemetryFiles.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "GlobalStandards.h"
#include "TLM_management.h"

int index_ = 0;
char logBuffer[LOG_BUFFER_SIZE];
char logMsg[LOG_MSG_SIZE];
char lastErrorMsg[LOG_MSG_SIZE];
int realErrorSize = 0;
Boolean hasError = FALSE;
LogLevel g_currLogLevel = CURR_LOG_LEVEL;

void __logg(char* msg) {
	//printf(msg);
	memset(logBuffer, 0, LOG_BUFFER_SIZE);
    int size =  strlen(msg);
    memcpy(logBuffer, msg, size);
    //FileSystemResult res = FS_SUCCSESS;
    //res = c_fileWrite(FILENAME_LOG_TLM, logBuffer);
    //if (res == FS_FAIL)//handle write fails due to concurrent dump
    	//c_fileWrite(FILENAME_LOG_BCKP_TLM, logBuffer);
}

void dump_now() {
	 if (index_  == 0 )
			 return;
	_c_fileWrite(FILENAME_LOG_TLM, logBuffer, LOG_BUFFER_SIZE, 0);
	index_ = 0;
	memset(logBuffer, 0, LOG_BUFFER_SIZE);
}

void _logg(char* msg) {
	printf(msg);//TODO:comment print
	int msgSize = strlen(msg);
	int msgSizeWithTime = sizeof(unsigned int) + msgSize;
	if ( msgSizeWithTime > (SIZE_TXFRAME - SIZE_SPL_HEADER))
		return;

	if (index_ == 0)
		memset(logBuffer, 0, LOG_BUFFER_SIZE);

    FileSystemResult res = FS_SUCCSESS;
    int dumpSize = (int)LOG_BUFFER_SIZE;
    int reminder = index_ % (SIZE_TXFRAME - SIZE_SPL_HEADER);
    int leftover = SIZE_TXFRAME - SIZE_SPL_HEADER - reminder;
    int considerTimeSize = 0;
    time_unix time;

    if ( !reminder || msgSize > leftover)
    	considerTimeSize = sizeof(time);

    if ( (index_ + msgSize + considerTimeSize) > dumpSize ) {
    	res = _c_fileWrite(FILENAME_LOG_TLM, logBuffer, LOG_BUFFER_SIZE, 0);
    	(void) res;
    	index_ = 0;
    	reminder = 0;
    	memset(logBuffer, 0, LOG_BUFFER_SIZE);
    }

    if (!reminder) {
    	Time_getUnixEpoch(&time);
    	memcpy(logBuffer + index_, &time, sizeof(time));
    	//printf("\r\n log index =%d\n\n\r", index_);
    	index_ += sizeof(time);
    	memcpy(logBuffer + index_, msg, msgSize);
    	index_ += msgSize;
    } else
    	if (msgSize <= leftover) {
    		memcpy(logBuffer + index_, msg, msgSize);
    		index_ += msgSize;
    	} else {
    		memcpy(logBuffer + index_, msg, leftover);
    		index_ += leftover;
    		Time_getUnixEpoch(&time);
    		memcpy(logBuffer + index_, &time, sizeof(time));
    		//printf("\r\n log index =%d\n\n\r", index_);
    		index_ += sizeof(time);
    		memcpy(logBuffer + index_, msg + leftover, msgSize - leftover);
    		index_ += (msgSize - leftover);
    	}
}

void logg(LogLevel level, char *fmt, ...) {
	if (g_currLogLevel > level)
			return;
	va_list argptr;
	va_start(argptr, fmt);
	vsprintf(logMsg, fmt, argptr);
	va_end(argptr);
	_logg(logMsg);
	if (level == error) {
		dump_now();
		time_unix time;
		Time_getUnixEpoch(&time);
		memcpy(lastErrorMsg, &time, sizeof(time));
		realErrorSize = strlen(logMsg);
		memcpy(lastErrorMsg + sizeof(time), logMsg, realErrorSize);
	}
}

void setLogLevel(LogLevel level)
{
	g_currLogLevel = level;
}

LogLevel getLogLevel()
{
	return g_currLogLevel;
}

void initLog()
{
	g_currLogLevel = CURR_LOG_LEVEL;
	realErrorSize = 0;
}

int getLastErrorMsgSize() {
	return realErrorSize;
}

void copyLastErrorMsg(unsigned char * buffer, unsigned char max){
	if (buffer == NULL || realErrorSize == 0)
		return;
	memcpy(buffer, lastErrorMsg, max);
}

void getLog_TLM(unsigned char * buffer, int size){
	if (buffer == NULL)
		return;
	if (size != LOG_TLM_SIZE_WITH_TIME)
		return;

	int delta = 0;
	if (index_ > LOG_TLM_SIZE)
		delta +=  index_ - LOG_TLM_SIZE;
	unsigned int curr_time;
	Time_getUnixEpoch(&curr_time);
	memcpy(buffer, &curr_time, sizeof(unsigned int));
	memcpy(buffer + sizeof(unsigned int), logBuffer + delta, LOG_TLM_SIZE);
}

