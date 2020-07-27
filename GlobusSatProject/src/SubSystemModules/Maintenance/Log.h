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
#define LOG_BUFFER_SIZE (235 * 10)//SIZE_TXFRAME=235

#define CURR_LOG_LEVEL 5
#define TLMInfo (LogLevel)3
#define MTNInfo (LogLevel)3
#define TRXInfo (LogLevel)3
#define OBCInfo (LogLevel)3
#define EPSInfo (LogLevel)3

typedef enum _LogLevel {
	all = 0,
	trace = 1,
	debug = 2,
	info = 3,
	warn = 4,
	error = 5,
	fatal = 6
} LogLevel;

void logg(LogLevel level, char *fmt, ...);

#endif /* LOG_H_ */
