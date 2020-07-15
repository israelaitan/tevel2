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
 * @brief		call transponder logic
 * @param[in]	the period of time that the transponder shall work, as was sent by the commaned
 */
int transponder_logic(time_unix period);

/**
 * @brief		Set transponder mode using generic I2C command
 * @param[in]	set mode to TRUE for turning on the transponder, set to FALSE to turn off the transponder
 */
int set_transonder_mode(Boolean mode);

/**
 *@brief		change the RSSI to active the Transmit when the transponder mode is active
 *@param[in]	the data to send throw I2C to the TRXVU
 */
int set_transponder_RSSI(byte* param);

#endif /* TRANSPONDER_H_ */
