/*
 * Log.c
 *
 *  Created on: 22 ���� 2020
 *      Author: USER
 */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include "Log.h"
#include <hal/Timing/Time.h>
#include "SubSystemModules/Housekepping/TelemetryFiles.h"
#include "GlobalStandards.h"
#include "TLM_management.h"

int index = 0;
char logBuffer[LOG_BUFFER_SIZE];
char logMsg[LOG_MSG_SIZE];

void _logg(char* msg) {
#ifdef TESTING
	printf(msg);
#endif
	memset(logBuffer, 0, LOG_BUFFER_SIZE);
    int size =  strlen(msg);
    memcpy(logBuffer, msg, size);
    FileSystemResult res = FS_SUCCSESS;
    res = c_fileWrite(FILENAME_LOG_TLM, logBuffer);
    if (res == FS_FAIL)//handle write fails due to concurrent dump
    	c_fileWrite(FILENAME_LOG_BCKP_TLM, logBuffer);
}

void __logg(char* msg) {
#ifdef TESTING
	printf(msg);
#endif
	if (index == 0)
		memset(logBuffer, 0, LOG_BUFFER_SIZE);
	time_unix time;
	Time_getUnixEpoch(&time);
    int size = sizeof(time) + strlen(msg);
    FileSystemResult res = FS_SUCCSESS;
    if ((index + size) > (int)LOG_BUFFER_SIZE ) {
    	res = c_fileWrite(FILENAME_LOG_TLM, logBuffer);
    	if (res == FS_FAIL)//handle write fails due to concurrent dump
    		c_fileWrite(FILENAME_LOG_BCKP_TLM, logBuffer);
    	index = 0;
    	memset(logBuffer, 0, LOG_BUFFER_SIZE);
    }
    memcpy(logBuffer + index, &time, sizeof(time));
    index += sizeof(time);
    memcpy(logBuffer + index, msg, size);
    index += size;
}

void logg(LogLevel level, char *fmt, ...) {
	if (CURR_LOG_LEVEL > level)
			return;
	va_list argptr;
	va_start(argptr, fmt);
	vsprintf(logMsg, fmt, argptr);
	va_end(argptr);
	_logg(logMsg);
}


