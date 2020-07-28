/*
 * Log.h
 *
 *  Created on: 22 αιεμ 2020
 *      Author: USER
 */

#ifndef LOG_H_
#define LOG_H_

//TODO: adjust
#define LOG_MSG_SIZE 80
#define LOG_BUFFER_SIZE (235 * 3)//SIZE_TXFRAME=235
#define LOG_TLM_SIZE (235 - 4)//SIZE_TXFRAME - sizeof(unsigned int)

#define CURR_LOG_LEVEL 3
#define TLMInfo (LogLevel)2
#define MTNInfo (LogLevel)2
#define TRXInfo (LogLevel)2
#define OBCInfo (LogLevel)2
#define EPSInfo (LogLevel)2

typedef enum _LogLevel {
	all = 0,
	trace = 1,
	debug = 2,
	info = 3,
	event = 4,
	error = 5,
	fatal = 6
} LogLevel;

void logg(LogLevel level, char *fmt, ...);

#endif /* LOG_H_ */
