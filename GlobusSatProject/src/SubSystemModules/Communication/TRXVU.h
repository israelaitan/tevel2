#ifndef TRXVU_H_
#define TRXVU_H_


#include <satellite-subsystems/IsisTRXVU.h>
#include "GlobalStandards.h"
#include "AckHandler.h"
#include "SatCommandHandler.h"

#define SIZE_RXFRAME	200		///< max size of data field in uplink
#define SIZE_TXFRAME	235		///< max size of data field in downlink

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


#endif
