/*
 * Log.h
 *
 *  Created on: 22 ���� 2020
 *      Author: USER
 */

#ifndef LOG_H_
#define LOG_H_

#define LOG_MSG_SIZE 80
#define SIZE_SPL_HEADER 16
#define LOG_TLM_SIZE_WITH_TIME (235 - SIZE_SPL_HEADER)
#define LOG_BUFFER_SIZE (LOG_TLM_SIZE_WITH_TIME * 1)//SIZE_TXFRAME=235 - spl header
#define LOG_TLM_SIZE (LOG_TLM_SIZE_WITH_TIME - 4)//SIZE_TXFRAME - sizeof(unsigned int) - spl header

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

LogLevel getLogLevel();

void setLogLevel(LogLevel level);

void initLog();

int getLastErrorMsgSize();

void copyLastErrorMsg(unsigned char * buffer, unsigned char max);

void getLog_TLM(unsigned char * buffer, int size);

#endif /* LOG_H_ */
