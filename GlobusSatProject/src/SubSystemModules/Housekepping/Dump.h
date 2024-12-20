#ifndef DUMP_H_
#define DUMP_H_

#include "SubSystemModules/Communication/SatCommandHandler.h"


int InitDump();

/*!
 * @brief sends an abort message via a freeRTOS queue.
 */
void SendDumpAbortRequest();

/*!
 * @brief Closes a dump task if one is executing, using vTaskDelete.
 * @note Can be used to forcibly abort the task
 */
void AbortDump();

/*!
 * @brief dump telemetry to the ground station with telemetry in time range specified in 'cmd'
 * @param[in] cmd the dump command
 * @note this function starts a new dump task
 * @return	0 on success
 * 			-1 on failure
 */
int DumpTelemetry(sat_packet_t *cmd);
#endif
