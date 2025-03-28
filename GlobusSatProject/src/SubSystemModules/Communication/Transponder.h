/*
 * Transponder.h
 *
 *  Created on: May 6, 2020
 *      Author: User
 */

#ifndef TRANSPONDER_H_
#define TRANSPONDER_H_

#include <satellite-subsystems/isis_vu_e.h>
#include "GlobalStandards.h"
#include "AckHandler.h"
#include "SatCommandHandler.h"

#define TURN_TRANSPONDER_OFF FALSE
#define TURN_TRANSPONDER_ON TRUE
#define DEFAULT_TRANS_RSSI 2500
#define DEFAULT_TRANSPONDER_DURATION 600

/**
 * @brief		Init transponder model
 */
void initTransponder();

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
int CMD_turnOnTransponder(sat_packet_t *cmd);

/**
 *@brief		Turn transponder mode OFF
 *@return		int indicating the error. 0 for success
 */
int CMD_turnOffTransponder();

/**
 *@brief		set RSSI Data from ground control
 *@return		int indicating the error. 0 for success
 */
int CMD_set_transponder_RSSI(sat_packet_t *cmd);

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

/**
 *@brief		Get Duration of Transponder
 *@return		return the period
 */
int getDuration(sat_packet_t *cmd);

#endif /* TRANSPONDER_H_ */
