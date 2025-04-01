#ifndef TRXVU_H_
#define TRXVU_H_


#include <satellite-subsystems/isis_vu_e.h>
#include "GlobalStandards.h"
#include "AckHandler.h"
#include "SatCommandHandler.h"

#define SIZE_RXFRAME	200		///< max size of data field in uplink
#define SIZE_TXFRAME	235		///< max size of data field in downlink
#define SIZE_SPL_HEADER 8
#define NO_TX_FRAME_AVAILABLE 255 //indication that the send buffer is full



/*!
 * @brief initializes the TRXVU subsystem
 * @return	0 on successful init
 * 			errors according to <hal/errors.h>
 */
int InitTrxvu();

/*!
 * @brief The TRXVU logic according to the sub-system flowchart
 * @return	int indicating the error 
 */
CommandHandlerErr TRX_Logic();

/*!
 * @brief The TRXVU command to set TRXVU into idle
 * @return	int indicating the error
 */
int CMD_SetIdleOn();

/*!
 * @brief The TRXVU command to set TRXVU out of idle
 * @return	int indicating the error
 */
int CMD_SetIdleOff();

/*!
 * @brief Set the last Antenas deployment time
 * @params	time to set the global variable
 */
void setLastAntsAutoDeploymentTime(time_unix time);

/*!
 * @brief Check if Antennas are open
 * @return	true or false
 */
Boolean areAntennasOpen();

#endif
