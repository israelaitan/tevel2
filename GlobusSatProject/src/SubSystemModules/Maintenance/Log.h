/*
 * Log.h
 *
 *  Created on: 22 αιεμ 2020
 *      Author: USER
 */

#ifndef LOG_H_
#define LOG_H_

#define LOG_BUFFER_SIZE 4096
#define CURR_LOG_LEVEL 5

typedef enum _LogLevel {
	all,
	trace,
	debug,
	info,
	warn,
	error,
	fatal
} LogLevel;

//LogLevel currLogLevel = error;

void logg(LogLevel level, char* msg);


#endif /* LOG_H_ */
