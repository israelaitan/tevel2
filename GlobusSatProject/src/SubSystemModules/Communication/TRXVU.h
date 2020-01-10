#ifndef TRXVU_H_
#define TRXVU_H_


#include <satellite-subsystems/IsisTRXVU.h>
#include "GlobalStandards.h"
#include "AckHandler.h"
#include "SatCommandHandler.h"

#define SIZE_RXFRAME	200		///< max size of data field in uplink
#define SIZE_TXFRAME	235		///< max size of data field in downlink


typedef struct __attribute__ ((__packed__))
{
	sat_packet_t *cmd;
	unsigned char dump_type;
	time_unix t_start;
	time_unix t_end;
} dump_arguments_t;


/*!
 * @brief initializes the TRXVU subsystem
 * @return	0 on successful init
 * 			errors according to <hal/errors.h>
 */
int InitTrxvu();

/*!
 * @brief The TRXVU logic according to the sub-system flowchart
 * @return	command_succsess on success
 * 			errors according to CommandHandlerErr enumeration
 * @see "SatCommandHandler.h"
 */
CommandHandlerErr TRX_Logic();


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
