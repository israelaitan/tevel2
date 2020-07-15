/*
 * Transponder.h
 *
 *  Created on: May 6, 2020
 *      Author: User
 */

#ifndef TRANSPONDER_H_
#define TRANSPONDER_H_

#include <satellite-subsystems/IsisTRXVU.h>
#include "GlobalStandards.h"
#include "AckHandler.h"
#include "SatCommandHandler.h"


/**
 * @brief		Set transponder mode using generic I2C command
 * @param[in]	set mode to TRUE for turning on the transponder, set to FALSE to turn off the transponder
 * @return		int indicating the error. 0 for success
 */
int set_transonder_mode(Boolean mode);

/**
 *@brief		change the RSSI to active the Transmit when the transponder mode is active
 *@param[in]	the data to send throw I2C to the TRXVU
 *@return		int indicating the error. 0 for success
 */
int set_transponder_RSSI(byte* param);

/**
 *@brief		Turn transponder mode ON
 *@param[in]	the duration of ON state
 *@return		int indicating the error. 0 for success
 */
int CMD_turnOnTransponder(time_unix duration);

/**
 *@brief		Turn transponder mode OFF
 *@return		int indicating the error. 0 for success
 */
int CMD_turnOffTransponder();

/**
 *@brief		Get transponder state - True for on and 0 for off
 *@return		int indicating the error. 0 for success
 */
Boolean getTransponderMode();

/**
 *@brief		Check if transponder ON state ended
 *@return		return true if period ended
 */
Boolean checkEndTransponderMode();



#endif /* TRANSPONDER_H_ */
