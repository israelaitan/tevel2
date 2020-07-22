/*
 * Log.c
 *
 *  Created on: 22 αιεμ 2020
 *      Author: USER
 */

#include <string.h>
#include <stdio.h>
#include "Log.h"
#include <hal/Timing/Time.h>
#include "SubSystemModules/Housekepping/TelemetryFiles.h"
#include "GlobalStandards.h"
#include "TLM_management.h"

int index = 0;
char logBuffer[CURR_LOG_LEVEL];

//TODO: consider lowering log level automatically after error

void logg(LogLevel level, char* msg) {
#ifdef TESTING
	printf(msg);
#endif
	if (CURR_LOG_LEVEL > level)
		return;
	time_unix time;
	Time_getUnixEpoch(&time);
    int size = sizeof(time) + strlen(msg);
    if ((index + size) < LOG_BUFFER_SIZE ) {
    	c_fileWrite(FILENAME_LOG_TLM, logBuffer);
    	index = 0;
    }
    memcpy(logBuffer + index, &time, sizeof(time));
    index += sizeof(time);
    memcpy(logBuffer + index, msg, size);
    index += size;
}





