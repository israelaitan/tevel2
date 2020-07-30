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
#define LOG_BUFFER_SIZE ((235 - 8) * 3)//SIZE_TXFRAME=235 - spl header
#define LOG_TLM_SIZE (235 - 4 - 8)//SIZE_TXFRAME - sizeof(unsigned int) - spl header

#define CURR_LOG_LEVEL 4
#define TLMInfo (LogLevel)2
#define MTNInfo (LogLevel)2
#define TRXInfo (LogLevel)3
#define OBCInfo (LogLevel)2
#define EPSInfo (LogLevel)2
#define DMPInfo (LogLevel)3
#define FileInfo (LogLevel)3

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

void setLogLevel(LogLevel level);

void initLog();

#endif /* LOG_H_ */
