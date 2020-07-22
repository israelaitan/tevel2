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
void loggError(char* msg, int error);
void loggInfo(char* msg);

#endif /* LOG_H_ */
