/*
 * Log.h
 *
 *  Created on: 22 αιεμ 2020
 *      Author: USER
 */

#ifndef LOG_H_
#define LOG_H_

//TODO: adjust
#define LOG_BUFFER_SIZE 4096
#define CURR_LOG_LEVEL 3
#define TLMInfo (LogLevel)3
#define TRXInfo (LogLevel)3
#define OBCInfo (LogLevel)3
#define EPSInfo (LogLevel)3

typedef enum _LogLevel {
	all,
	trace,
	debug,
	info,
	warn,
	error,
	fatal
} LogLevel;

void logg(LogLevel level, char *fmt, ...);

#endif /* LOG_H_ */
