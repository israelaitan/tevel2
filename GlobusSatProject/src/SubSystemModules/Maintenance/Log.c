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

int index = 0;
char logBuffer[LOG_BUFFER_SIZE];
char logMsg[LOG_MSG_SIZE];
LogLevel g_currLogLevel = CURR_LOG_LEVEL;

void __logg(char* msg) {
#ifdef TESTING
	printf(msg);
#endif
	Boolean TESTING =TRUE;//TODO:REMOVE *****
	if (TESTING)
		printf(msg);
	memset(logBuffer, 0, LOG_BUFFER_SIZE);
    int size =  strlen(msg);
    memcpy(logBuffer, msg, size);
    //FileSystemResult res = FS_SUCCSESS;
    //res = c_fileWrite(FILENAME_LOG_TLM, logBuffer);
    //if (res == FS_FAIL)//handle write fails due to concurrent dump
    	//c_fileWrite(FILENAME_LOG_BCKP_TLM, logBuffer);
}

void _logg(char* msg) {
#ifdef TESTING
	printf(msg);
#endif
	Boolean TESTING =TRUE;//TODO:REMOVE *****
		if (TESTING)
			printf(msg);
	int msgSize = strlen(msg);
	int msgSizeWithTime = sizeof(unsigned int) + msgSize;
		return;

	if (index == 0)
		memset(logBuffer, 0, LOG_BUFFER_SIZE);

    FileSystemResult res = FS_SUCCSESS;
    int dumpSize = (int)LOG_BUFFER_SIZE;
    int reminder = index % (SIZE_RXFRAME - SIZE_SPL_HEADER);
    int leftover = SIZE_RXFRAME - SIZE_SPL_HEADER - reminder;
    int considerTimeSize = 0;
    time_unix time;

    if ( !reminder || msgSize > leftover)
    	considerTimeSize = sizeof(time);

    if ( (index + msgSize + considerTimeSize) > dumpSize ) {
    	res = _c_fileWrite(FILENAME_LOG_TLM, logBuffer, LOG_BUFFER_SIZE, 0);
    	(void) res;
    	index = 0;
    	memset(logBuffer, 0, LOG_BUFFER_SIZE);
    }

    if (!reminder) {
    	Time_getUnixEpoch(&time);
    	memcpy(logBuffer + index, &time, sizeof(time));
    	index += sizeof(time);
    	memcpy(logBuffer + index, msg, msgSize);
    	index += msgSize;
    } else
    	if (msgSize <= leftover) {
    		memcpy(logBuffer + index, msg, msgSize);
    		index += msgSize;
    	} else {
    		memcpy(logBuffer + index, msg, leftover);
    		index += leftover;
    		Time_getUnixEpoch(&time);
    		memcpy(logBuffer + index, &time, sizeof(time));
    		index += sizeof(time);
    		memcpy(logBuffer + index, msg + leftover, msgSize - leftover);
    		index += (msgSize - leftover);
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
}

void setLogLevel(LogLevel level)
{
	g_currLogLevel = level;
}

void initLog()
{
	g_currLogLevel = CURR_LOG_LEVEL;
}


