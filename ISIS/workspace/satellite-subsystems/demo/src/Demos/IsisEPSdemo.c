/*
 * IsisEPSdemo.c
 *
 *  Created on: 22 sep. 2015
 *      Author: lrot
 */

#include "IsisEPSdemo.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <at91/utility/exithandler.h>
#include <at91/commons.h>
#include <at91/utility/trace.h>
#include <at91/peripherals/cp15/cp15.h>

#include <hal/Utility/util.h>
#include <hal/Timing/WatchDogTimer.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/LED.h>
#include <hal/boolean.h>
#include <hal/errors.h>

#include <satellite-subsystems/IsisEPS.h>

#include <Demos/common.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>



///***************************************************************************************************************************
///
/// Implementers notice:
/// This file contains example code that aids building understanding of how to interface with the subsystem
/// using the subsystems library. Validation code has been kept to a minimum in an effort not to obfuscate
/// the code that is needed for calling the subsystem. Be cautious and critical when using portions of this
/// code in your own projects. It is *not* recommended to copy/paste this code into your projects,
/// instead use the example as a guide while building your modules separately.
///
///***************************************************************************************************************************


///***************************************************************************************************************************
///
/// Notes on the parameter system:
/// the parameter system requires an unsigned short parameter_index
/// and a pointer to the location of the data to be get/set/reset
/// the void pointer can be pointing to an array or directly to a strongly typed variable
///
/// note that the parameter index consists of: param-type in the highest nibble (4 bits), and the ordinal index in the
/// lower 3 nibbles (12 bits). Optionally the fifth to highest bit of the 16 bits is used to indicate read/only
/// e.g.: 0xA802 = third read-only double, 0xA002 = third read/write double
///
/// example 1: getting param 0x1000 (which is the first int8):
///		unsigned short param_id = 0x1000;
///		char param_val;										// value that receives the gotten variable information
///
///		rv = IsisEPS_getParameter(0, param_id, &param_val, &rsp_stat);
///
/// example 2: setting param 0xA000 (which is the first 8 byte double):
///		unsigned short param_id = 0xA000;
///		double param_inp = 12.01;							// value that is set
///		double param_outp;									// value that is read back after update for verification purposes
///
///		rv = IsisEPS_setParameter(0, param_id, &param_inp, &param_outp &rsp_stat);
///
/// example 3: setting param 0x7003 (which is the fourth 4 byte float) using a (little endian) byte array as input:
///		unsigned short param_id = 0x7003;
///		unsigned char barr_inp[4] = {0x10,0x32,0x54,0x76};	// 4 byte values making up the 4 byte float variable
///		float param_outp;									// value that is read back after update for verification purposes
///
///		rv = IsisEPS_setParameter(0, param_id, barr_inp, &param_outp, &rsp_stat);
///
///	Generally usage within the I2C master falls in either of two categories: direct-use and pass-through
/// *direct-use 	This entails having the I2C master changing the configuration parameters autonomously.
///					Use of pointers to typed variables is most convenient in this case. The system will only see a void
///					pointer, hence the user must ensure a pointer to a variable with the correct parameter type (i.e. size!)
///					is supplied with any param-id.
/// *pass-through 	This entails having the ground station provide a byte stream to be passed to the subsystem.
///					Use of pointers to byte arrays is most convenient in this case. The system automatically derives the
///  				amount of bytes that needs to be read from the pointer location.
///***************************************************************************************************************************


///***************************************************************************************************************************
///
/// Demo helper functions:
/// below are helper functions specific to the demo application for interacting with the user and presenting results
///
/// The subsystem calls can be found below!
///
///***************************************************************************************************************************


static void _parse_resp(ieps_statcmd_t* p_rsp_code)
{
	// this function parses the response that is provided as the result of
	// issuing a command to the subsystem. It provides information on whether the response
	// was accepted for processing, not necessarily whether the command succeeded because
	// generally too much processing time is required for executing the command to allow
	// waiting for a response. Generally separate calls can be made to verify successful
	// command execution, usually that call includes any output data gather during command
	// execution.
	// In case of getting measurement results the inva status indicates whether issues
	// were encountered during measurement causing the axis value to become suspect
	if(p_rsp_code == NULL)
	{
		TRACE_ERROR(" internal error: p_rsp_code is NULL");
		return;
	}

	if(p_rsp_code->fields.new)																	// is the new flag set?
	{
		printf(" \t new = %d (new response/data)\r\n", p_rsp_code->fields.new);					// indicate its a never before retrieved response
	}
	else
	{
		printf(" \t new = %d (old response/data)\r\n", p_rsp_code->fields.new);					// indicate we've read this response before
	}

	switch(p_rsp_code->fields.cmd_error)
	{
	case ieps_cmd_accepted: 		///< Accepted
		printf(" \t cmd_err = %d (command accepted) \r\n", p_rsp_code->fields.cmd_error);
		break;
	case ieps_cmd_rejected: 		///< Rejected: no reason indicated
		printf(" \t cmd_err = %d (!REJECTED! command rejected) \r\n", p_rsp_code->fields.cmd_error);
		break;
	case ieps_cmd_rejected_invalid: ///< Rejected: invalid command code
		printf(" \t cmd_err = %d (!REJECTED! command code invalid) \r\n", p_rsp_code->fields.cmd_error);
		break;
	case ieps_cmd_rejected_parmiss: ///< Rejected: parameter missing
		printf(" \t cmd_err = %d (!REJECTED! command parameter missing) \r\n", p_rsp_code->fields.cmd_error);
		break;
	case ieps_cmd_rejected_parinv: ///< Rejected: parameter invalid
		printf(" \t cmd_err = %d (!REJECTED! command parameter invalid) \r\n", p_rsp_code->fields.cmd_error);
		break;
	case ieps_cmd_rejected_ccunav: ///< Rejected: CC unavailable in current mode
		printf(" \t cmd_err = %d (!REJECTED! command unavailable in current mode) \r\n", p_rsp_code->fields.cmd_error);
		break;
	case ieps_cmd_reserved: 	   ///< Reserved value
		printf(" \t cmd_err = %d (!REJECTED! command caused return of reserved response code) \r\n", p_rsp_code->fields.cmd_error);
		break;
	case ieps_cmd_internal_error: ///< Internal error occurred during processing
		printf(" \t cmd_err = %d (!REJECTED! command caused internal error) \r\n", p_rsp_code->fields.cmd_error);
		break;
	}
}

static Boolean _get_channel_flagfield(char *p_str, ieps_obus_channel_t* p_chan)
{
	// the channel flagfield indicates a channel with each bit.
	// the bits of a single byte represent up to 8 channels:
	// <b7><b6><b5><b4><b3><b2><b1><b0> = <CH7><CH6><CH5><CH4><CH3><CH2><CH1><CH0>
	// Examples:
	// 0x08 = 0b00001000 = CH3
	// 0x24 = 0b00100100 = CH5 and CH2

	double value;
	unsigned int rv;

	if(p_chan == NULL)
	{
		TRACE_ERROR(" internal error: p_chan is NULL");
		return FALSE;
	}

	memset(p_chan, 0, sizeof(ieps_obus_channel_t));

	while(1)
	{
		rv = ing(p_str, &value, 0, 255, 0);														// request info from the user
		if(rv == INGRV_esc) {printf("<ESC> \r\n"); return FALSE;}								// esc? exit function
		else if(rv == INGRV_val) {printf("\r\n"); break;}										// valid? continue
	}

	p_chan->raw = (unsigned char) value;

	return TRUE;
}

static Boolean _get_channel_index(char *p_str, unsigned char* p_chan_idx)
{
	// the channel index indicates a channel using a 0 based index.
	// valid indices run from 0 to 7 (inclusive) allowing 8 channels to be specified.
	// Examples:
	// 0 = CH0
	// 3 = CH3
	// 5 = CH5

	double value;
	unsigned int rv;

	if(p_chan_idx == NULL)
	{
		TRACE_ERROR(" internal error: p_chan_idx is NULL");
		return FALSE;
	}

	while(1)
	{
		rv = ing(p_str, &value, 0, 255, 0);														// request info from the user
		if(rv == INGRV_esc) {printf("<ESC> \r\n"); return FALSE;}								// esc? exit function
		else if(rv == INGRV_val) {printf("\r\n"); break;}										// valid? continue
	}

	*p_chan_idx = (unsigned char) value;

	return TRUE;
}

static Boolean _get_cdb_board_selection(char *p_str, ieps_board_t* p_board)
{
	// the IEPS consists of a mainboard and up to 7 companion/daughterboard (CDB)
	// this function allows the user to select a single CDB board
	// board to index mapping is provided by ieps_board_t

	double value;
	unsigned int rv;

	if(p_board == NULL)
	{
		TRACE_ERROR(" internal error: p_board is NULL");
		return FALSE;
	}

	while(1)
	{
		rv = ing(p_str, &value, ieps_board_cdb1, ieps_board_cdb7, ieps_board_cdb1);				// request info from the user
		if(rv == INGRV_esc) {printf("<ESC> \r\n"); return FALSE;}								// esc? exit function
		else if(rv == INGRV_val) {printf("\r\n"); break;}										// valid? continue
	}

	*p_board = (ieps_board_t) value;

	return TRUE;
}

///***************************************************************************************************************************
///
/// Demo helper functions end
///
///***************************************************************************************************************************

///***************************************************************************************************************************
///
/// IsisEPS command demo functions:
/// below are the IsisEPS interface commands making up the messaging interface.
/// these generally send the corresponding command to the EPS and present the results to the user
///
///***************************************************************************************************************************

static Boolean _hardReset(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information hardReset *** \r\n");
		printf(" Sends the hard-reset command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_RESET);
		printf(" Turns off all EPS output busses and resets the system \r\n");
		printf(" After command acceptance the reset cycle will start immediately. \r\n");
		printf(" The reset cycle takes up to 1 s to be completed. \r\n");

		return TRUE;
	}

	printf("\r\n Perform hardReset \r\n");

	rv = IsisEPS_hardReset(0, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	return TRUE;
}


static Boolean _noOperation(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information noOperation *** \r\n");
		printf(" Sends the no-operation command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_NOP);
		printf(" Does not affect the EPS other than providing a 'success' reply. \r\n");
		printf(" Can be used to verify availability of the IEPS in a non-intrusive manner. \r\n");

		return TRUE;
	}

	printf("\r\n Perform noOperation \r\n");

	rv = IsisEPS_noOperation(0, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	return TRUE;
}


static Boolean _cancelOperation(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information cancelOperation *** \r\n");
		printf(" Sends the cancel-operation command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_CANCELOP);
		printf(" Turns off all switchable output bus channels with a single command. \r\n");
		printf(" This includes the 3V3, 5V, battery (BV) and high-voltage (HV) bus channels. \r\n");

		return TRUE;
	}

	printf("\r\n Perform cancelOperation \r\n");

	rv = IsisEPS_cancelOperation(0, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	return TRUE;
}


static Boolean _resetWDT(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information resetWDT *** \r\n");
		printf(" Sends the reset watchdog command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_WDT);
		printf(" Explicit watchdog timer reset command keeping the EPS watchdog from resetting the system. \r\n");
		printf(" Any I2C interaction with the IEPS implicitly resets the watchdog, hence using this \r\n");
		printf(" command is not mandatory if other commands are issued on a regular basis. \r\n");
		printf(" Regardless issuing a single of these command from the main loop is recommended. \r\n");

		return TRUE;
	}

	printf("\r\n Perform resetWDT \r\n");

	rv = IsisEPS_resetWDT(0, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	return TRUE;
}


static Boolean _outputBusGroupOn(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	ieps_obus_channel_t chan3V3;
	ieps_obus_channel_t chan5V;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information outputBusGroupOn *** \r\n");
		printf(" Sends the output bus group-on command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_OUTBUSGR_ON);
		printf(" This allows switching on multiple 3V3 and 5V bus channels with a single command. \r\n");
		printf(" Channels that are already on are left unaffected. \r\n");
		printf(" The channels to turn-on are encoded in two bytes: <3V3 bitflag> <5V bitflag> \r\n");
		printf(" Each of these bitflag bytes is defined as follows: \r\n");
		printf(" \tbit7 bit6 bit5 bit4 bit3 bit2 bit1 bit0 = CH7 CH6 CH5 CH4 CH3 CH2 CH1 CH0 \r\n");
		printf(" both 3V3C0 and 5VC0 are 'permanent-on' channels and these bits are ignored. \r\n");
		printf(" Example: \r\n");
		printf(" \t turning on 3V3C1, 3V3C3 and 5VC2 requires the bitflag bytes\r\n");
		printf(" \t 0b00001010 = 0x0A and 0b00000100 = 0x04 for 3V3 and 5V respectively. \r\n");

		return TRUE;
	}

	printf("\r\n Perform outputBusGroupOn \r\n");

	if(!_get_channel_flagfield(" Provide bitmask 3V3 [0]: ", &chan3V3)) return TRUE;		// ask for the bus channel information; exit on failure

	if(!_get_channel_flagfield(" Provide bitmask 5V [0]: ", &chan5V)) return TRUE;			// ask for the bus channel information; exit on failure

	rv = IsisEPS_outputBusGroupOn(0, chan3V3, chan5V, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)			// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	return TRUE;
}


static Boolean _outputBusGroupOff(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	ieps_obus_channel_t chan3V3;
	ieps_obus_channel_t chan5V;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information outputBusGroupOff *** \r\n");
		printf(" Sends the output bus group-off command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_OUTBUSGR_OFF);
		printf(" This allows switching off multiple 3V3 and 5V bus channels with a single command. \r\n");
		printf(" Channels that are already off are left unaffected. \r\n");
		printf(" The channels to turn-off are encoded in two bytes: <3V3 bitflag> <5V bitflag> \r\n");
		printf(" Each of these bitflag bytes is defined as follows: \r\n");
		printf(" \tbit7 bit6 bit5 bit4 bit3 bit2 bit1 bit0 = CH7 CH6 CH5 CH4 CH3 CH2 CH1 CH0 \r\n");
		printf(" both 3V3C0 and 5VC0 are 'permanent-on' channels and these bits are ignored. \r\n");
		printf(" Example: \r\n");
		printf(" \t turning off 3V3C1, 3V3C3 and 5VC2 requires the bitflag bytes\r\n");
		printf(" \t 0b00001010 = 0x0A and 0b00000100 = 0x04 for 3V3 and 5V respectively. \r\n");

		return TRUE;
	}

	printf("\r\n Perform outputBusGroupOff \r\n");

	if(!_get_channel_flagfield(" Provide bitmask 3V3 [0]: ", &chan3V3)) return TRUE;		// ask for the bus channel information; exit on failure

	if(!_get_channel_flagfield(" Provide bitmask 5V [0]: ", &chan5V)) return TRUE;			// ask for the bus channel information; exit on failure

	rv = IsisEPS_outputBusGroupOff(0, chan3V3, chan5V, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	return TRUE;
}


static Boolean _outputBusGroupState(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	ieps_obus_channel_t chan3V3;
	ieps_obus_channel_t chan5V;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information outputBusGroupState *** \r\n");
		printf(" Sends the output bus group-state command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_OUTBUSGR_STATE);
		printf(" This allows switching on/off multiple 3V3 and 5V bus channels with a single command. \r\n");
		printf(" Channels that are already in the requested state are left unaffected. \r\n");
		printf(" The bits that are set indicate which channels need to be on. \r\n");
		printf(" The bits that are not set indicate which channels need to be off. \r\n");
		printf(" The desired output bus state is encoded in two bytes: <3V3 bitflag> <5V bitflag> \r\n");
		printf(" Each of these bitflag bytes is defined as follows: \r\n");
		printf(" \tbit7 bit6 bit5 bit4 bit3 bit2 bit1 bit0 = CH7 CH6 CH5 CH4 CH3 CH2 CH1 CH0 \r\n");
		printf(" both 3V3C0 and 5VC0 are 'permanent-on' channels and these bits are ignored. \r\n");
		printf(" Example: \r\n");
		printf(" \t Needing 3V3C1, 3V3C3 and 5VC2 on and the other 3V3/5V channels off requires the bitflag bytes\r\n");
		printf(" \t 0b00001010 = 0x0A and 0b00000100 = 0x04 for 3V3 and 5V respectively. \r\n");

		return TRUE;
	}

	printf("\r\n Perform outputBusGroupState \r\n");

	if(!_get_channel_flagfield(" Provide bitmask 3V3 [0]: ", &chan3V3)) return TRUE;		// ask for the bus channel information; exit on failure

	if(!_get_channel_flagfield(" Provide bitmask 5V [0]: ", &chan5V)) return TRUE;			// ask for the bus channel information; exit on failure

	rv = IsisEPS_outputBusGroupState(0, chan3V3, chan5V, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)			// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	return TRUE;
}


static Boolean _outputBus3v3On(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	unsigned char chan_idx;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information outputBus3v3On *** \r\n");
		printf(" Sends the output bus 3V3-on command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_OUTBUS_3V3_ON);
		printf(" This allows switching a single 3V3 bus channel on. \r\n");
		printf(" The index that indicates which channel to switch is defined as follows: \r\n");
		printf(" \t 7/6/5/4/3/2/1/0 = CH7/CH6/CH5/CH4/CH3/CH3/CH2/CH1/CH0 respectively.\r\n");
		printf(" If the channel is already on it is left unaffected. \r\n");
		printf(" 3V3C0 is a 'permanent-on' channel and trying to switch this channel is ignored. \r\n");
		printf(" Example: \r\n");
		printf(" \t For turning 3V3C1 on use index 1\r\n");

		return TRUE;
	}

	printf("\r\n Perform outputBus3v3On \r\n");

	if(!_get_channel_index(" Provide channel index [0]: ", &chan_idx)) return TRUE;		// ask for the bus channel index information; exit on failure

	rv = IsisEPS_outputBus3v3On(0, chan_idx, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)			// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	return TRUE;
}


static Boolean _outputBus3v3Off(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	unsigned char chan_idx;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information outputBus3v3Off *** \r\n");
		printf(" Sends the output bus 3V3-off command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_OUTBUS_3V3_OFF);
		printf(" This allows switching a single 3V3 bus channel off. \r\n");
		printf(" The index that indicates which channel to switch is defined as follows: \r\n");
		printf(" \t 7/6/5/4/3/2/1/0 = CH7/CH6/CH5/CH4/CH3/CH3/CH2/CH1/CH0 respectively.\r\n");
		printf(" If the channel is already off it is left unaffected. \r\n");
		printf(" 3V3C0 is a 'permanent-on' channel and trying to switch this channel results in an error. \r\n");
		printf(" Example: \r\n");
		printf(" \t For turning 3V3C1 off use index 1\r\n");

		return TRUE;
	}

	printf("\r\n Perform outputBus3v3Off \r\n");

	if(!_get_channel_index(" Provide channel index [0]: ", &chan_idx)) return TRUE;		// ask for the bus channel index information; exit on failure

	rv = IsisEPS_outputBus3v3Off(0, chan_idx, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	return TRUE;
}


static Boolean _outputBus5vOn(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	unsigned char chan_idx;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information outputBus5vOn *** \r\n");
		printf(" Sends the output bus 5V-on command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_OUTBUS_5V_ON);
		printf(" This allows switching a single 5V bus channel on. \r\n");
		printf("  The index that indicates which channel to switch is defined as follows: \r\n");
		printf(" \t 7/6/5/4/3/2/1/0 = CH7/CH6/CH5/CH4/CH3/CH3/CH2/CH1/CH0 respectively.\r\n");
		printf(" If the channel is already on it is left unaffected. \r\n");
		printf(" 5VC0 is a 'permanent-on' channel and trying to switch this channel is ignored. \r\n");
		printf(" Example: \r\n");
		printf(" \t For turning 5VC1 on use index 1\r\n");

		return TRUE;
	}

	printf("\r\n Perform outputBus5vOn \r\n");

	if(!_get_channel_index(" Provide channel index [0]: ", &chan_idx)) return TRUE;		// ask for the bus channel index information; exit on failure

	rv = IsisEPS_outputBus5vOn(0, chan_idx, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	return TRUE;
}


static Boolean _outputBus5vOff(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	unsigned char chan_idx;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information outputBus5vOff *** \r\n");
		printf(" Sends the output bus 5V-off command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_OUTBUS_5V_OFF);
		printf(" This allows switching a single 5V bus channel off. \r\n");
		printf(" The index that indicates which channel to switch is defined as follows: \r\n");
		printf(" \t 7/6/5/4/3/2/1/0 = CH7/CH6/CH5/CH4/CH3/CH3/CH2/CH1/CH0 respectively.\r\n");
		printf(" If the channel is already off it is left unaffected. \r\n");
		printf(" 5VC0 is a 'permanent-on' channel and trying to switch this channel results in an error. \r\n");
		printf(" Example: \r\n");
		printf(" \t For turning 5VC1 off use index 1\r\n");

		return TRUE;
	}

	printf("\r\n Perform outputBus5vOff \r\n");

	if(!_get_channel_index(" Provide channel index [0]: ", &chan_idx)) return TRUE;		// ask for the bus channel index information; exit on failure

	rv = IsisEPS_outputBus5vOff(0, chan_idx, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	return TRUE;
}


static Boolean _outputBusHvOn(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	unsigned char chan_idx;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information outputBusHvOn *** \r\n");
		printf(" Sends the output bus HV-on command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_OUTBUS_HV_ON);
		printf(" This allows switching a single HV bus channel on. \r\n");
		printf(" The index that indicates which channel to switch is defined as follows: \r\n");
		printf(" \t 7/6/5/4/3/2/1/0 = CH7/CH6/CH5/CH4/CH3/CH3/CH2/CH1/CH0 respectively.\r\n");
		printf(" If the channel is already on it is left unaffected. \r\n");
		printf(" Example: \r\n");
		printf(" \t For turning HVC1 on use index 1\r\n");

		return TRUE;
	}

	printf("\r\n Perform outputBusHvOn \r\n");

	if(!_get_channel_index(" Provide channel index [0]: ", &chan_idx)) return TRUE;		// ask for the bus channel index information; exit on failure

	rv = IsisEPS_outputBusHvOn(0, chan_idx, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	return TRUE;
}


static Boolean _outputBusHvOff(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	unsigned char chan_idx;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information outputBusHvOff *** \r\n");
		printf(" Sends the output bus HV-off command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_OUTBUS_HV_OFF);
		printf(" This allows switching a single HV bus channel off. \r\n");
		printf(" The index that indicates which channel to switch is defined as follows: \r\n");
		printf(" \t 7/6/5/4/3/2/1/0 = CH7/CH6/CH5/CH4/CH3/CH3/CH2/CH1/CH0 respectively.\r\n");
		printf(" If the channel is already off it is left unaffected. \r\n");
		printf(" Example: \r\n");
		printf(" \t For turning HVC1 off use index 1\r\n");

		return TRUE;
	}

	printf("\r\n *** Perform outputBusHvOff *** \r\n");

	if(!_get_channel_index(" Provide channel index [0]: ", &chan_idx)) return TRUE;		// ask for the bus channel index information; exit on failure

	rv = IsisEPS_outputBusHvOff(0, chan_idx, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)	// non-zero return value means error!
		return TRUE;								// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	return TRUE;
}


static Boolean _getSystemState(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	ieps_systemstate_t system_state;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getSystemState *** \r\n");
		printf(" Sends the getSystemState command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_GET_SYSTEMSTATE);
		printf(" Provides system state information on the IEPS. \r\n");
		printf(" The following information is returned: \r\n");
		printf(" mode = %s \r\n", "current mode of the IEPS");
		printf(" conf = %s \r\n", "Was the in-memory configuration altered by the user since last load/save.");
		printf(" uptime = %s \r\n", "uptime in seconds");
		printf(" error = %s \r\n", "first internal error encountered during last control iteration");
		printf(" reset_cause = %s \r\n", "cause of last reset");
		printf(" reset_cause_pwron_cnt = %s \r\n", "amount of power-on resets occurred since begin-of-life");
		printf(" reset_cause_wdg_cnt = %s \r\n", "amount of watchdog resets occurred");
		printf(" reset_cause_cmd_cnt = %s \r\n", "amount of commanded resets occurred");
		printf(" reset_cause_emlopo_cnt = %s \r\n", "amount of times emlopo was entered");
		printf(" reset_cause_mcu_subcnt = %s \r\n", "amount of times pwr-on reset while power available");
		printf(" reset_cause_fic_subcnt = %s \r\n", "amount of times cmd reset through FIC");

		return TRUE;
	}

	printf("\r\n Perform getSystemState \r\n");

	rv = IsisEPS_getSystemState(0, &system_state, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)	// non-zero return value means error!
		return TRUE;								// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	printf(" \t mode = %d ", system_state.fields.mode);
	switch(system_state.fields.mode)
	{
	case ieps_mode_startup: printf("( startup ) \r\n"); break;
	case ieps_mode_emlopo: printf("( emergency-low-power ) \r\n"); break;
	case ieps_mode_nominal: printf("( nominal ) \r\n"); break;
	default: printf("( !UNKNOWN! ) \r\n"); break;
	}
	printf(" \t conf = %d ", system_state.fields.conf);
	if(system_state.fields.conf) printf("( config params were changed ) \r\n"); else printf("( no config param changes ) \r\n");
	printf(" \t uptime = %d [s] \r\n", system_state.fields.uptime);
	printf(" \t error = %d \r\n", system_state.fields.error);
	printf(" \t reset_cause = %d \r\n", system_state.fields.reset_cause);
	printf(" \t reset_cause_pwron_cnt = %d \r\n", system_state.fields.reset_cause_pwron_cnt);
	printf(" \t reset_cause_wdg_cnt = %d \r\n", system_state.fields.reset_cause_wdg_cnt);
	printf(" \t reset_cause_cmd_cnt = %d \r\n", system_state.fields.reset_cause_cmd_cnt);
	printf(" \t reset_cause_emlopo_cnt = %d \r\n", system_state.fields.reset_cause_emlopo_cnt);
	printf(" \t reset_cause_mcu_subcnt = %d \r\n", system_state.fields.reset_cause_mcu_subcnt);
	printf(" \t reset_cause_fic_subcnt = %d \r\n", system_state.fields.reset_cause_fic_subcnt);

	return TRUE;
}


static Boolean _getRawHKDataMB(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	ieps_rawhk_data_mb_t hk;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getRawHKDataMB *** \r\n");
		printf(" Sends the getRawHKDataMB command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_GET_RAWHKDATA_MB);
		printf(" Retrieves mainboard house keeping data in RAW form (as-is provided by the hardware). \r\n");
		printf(" The IEPS system consists of a single mainboard which is combined with up to 7 CDBs. \r\n");
		printf(" Requesting housekeeping data has been split into separate calls for each board. \r\n");
		printf(" Availability of CDBs and their batteries/components depends on the IEPS package bought. \r\n");
		printf(" The following information is returned: \r\n");
		printf(" board_idx = %s; always %d \r\n", "mainboard board index", ieps_board_mb);
		printf(" comp_bf = %s \r\n", "board components successfully queried");
		printf(" swion_3V3_bf = %s \r\n", "bitflag with a 1 bit for each channel that is commanded to the 'on' state");
		printf(" swion_5V_bf = %s \r\n", "bitflag with a 1 bit for each channel that is commanded to the 'on' state");
		printf(" swion_BV_bf = %s \r\n", "bitflag with a 1 bit for each channel that is commanded to the 'on' state");
		printf(" swion_HV_bf = %s \r\n", "bitflag with a 1 bit for each channel that is commanded to the 'on' state");
		printf(" pgood_3V3_bf = %s \r\n", "bitflag with a 1 bit when channel voltage >90%% of 3.3V");
		printf(" pgood_5V_bf = %s \r\n", "bitflag with a 1 bit when channel voltage >90%% of 5V");
		printf(" pgood_BV_bf = %s \r\n", "bitflag with a 1 bit when battery channel voltage >90%%");
		printf(" pgood_HV_bf = %s \r\n", "bitflag with a 1 bit when high-voltage channel voltage >90%% of target voltage");
		printf(" bat_charging_bf = %s \r\n", "bitflag with a 1 bit when corresponding battery charging");
		printf(" bat_heating_bf = %s \r\n", "bitflag with a 1 bit when corresponding heater heating");

		printf(" pwr_generating_mW = %s \r\n", "power provided to the IEPS internal power bus by the solar panels/MPPTs");
		printf(" pwr_charging_mW = %s \r\n", "power exchanged between IEPS internal power bus and all batteries (positive = charging)");
		printf(" pwr_consuming_mW = %s \r\n", "power taken from the IEPS internal power bus (used by IEPS and satellite)");
		printf(" pwr_delivering_mW = %s \r\n", "power being delivered to the satellite bus");

		printf(" obus_3V3_curr_raw = %s \r\n", "3V3 bus current measurement in raw counts");
		printf(" obus_3V3_volt_raw = %s \r\n", "3V3 bus voltage measurement in raw counts");
		printf(" obus_5V_curr_raw = %s \r\n", "5V bus current measurement in raw counts");
		printf(" obus_5V_volt_raw = %s \r\n", "5V bus voltage measurement in raw counts");
		printf(" obus_BV_curr_raw = %s \r\n", "battery bus current measurement in raw counts");
		printf(" obus_BV_volt_raw = %s \r\n", "battery bus voltage measurement in raw counts");

		printf(" charge_bus_volt_raw = %s \r\n", "charge bus voltage measurement in raw counts");
		printf(" pcb_temp_raw = %s \r\n", "pcb temperature measurement in raw counts");
		printf(" mppt_temp_raw = %s \r\n", "mppt temperature measurement in raw counts");
		printf(" mcu_temp_raw = %s \r\n", "mcu temperature measurement in raw counts");

		printf(" mppt0_in_curr_raw = %s \r\n", "current measurement in raw counts");
		printf(" mppt0_in_volt_raw = %s \r\n", "voltage measurement in raw counts");
		printf(" mppt0_out_curr_raw = %s \r\n", "current measurement in raw counts");
		printf(" mppt0_out_volt_raw = %s \r\n", "voltage measurement in raw counts");

		printf(" mppt1_in_curr_raw = %s \r\n", "current measurement in raw counts");
		printf(" mppt1_in_volt_raw = %s \r\n", "voltage measurement in raw counts");
		printf(" mppt1_out_curr_raw = %s \r\n", "current measurement in raw counts");
		printf(" mppt1_out_volt_raw = %s \r\n", "voltage measurement in raw counts");

		printf(" mppt2_in_curr_raw = %s \r\n", "current measurement in raw counts");
		printf(" mppt2_in_volt_raw = %s \r\n", "voltage measurement in raw counts");
		printf(" mppt2_out_curr_raw = %s \r\n", "current measurement in raw counts");
		printf(" mppt2_out_volt_raw = %s \r\n", "voltage measurement in raw counts");
		return TRUE;
	}

	printf("\r\n *** Perform getRawHKDataMB *** \r\n");

	rv = IsisEPS_getRawHKDataMB(0, &hk, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)	// non-zero return value means error!
		return TRUE;								// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	printf(" \t board_idx = %d \r\n", hk.fields.board);
	printf(" \t comp_bf = %d \r\n", hk.fields.comp.raw);
	printf(" \t swion_3V3_bf = %d \r\n", hk.fields.swion_3V3.raw);
	printf(" \t swion_5V_bf = %d \r\n", hk.fields.swion_5V.raw);
	printf(" \t swion_BV_bf = %d \r\n", hk.fields.swion_BV.raw);
	printf(" \t swion_HV_bf = %d \r\n", hk.fields.swion_HV.raw);
	printf(" \t pgood_3V3_bf = %d \r\n", hk.fields.pgood_3V3.raw);
	printf(" \t pgood_5V_bf = %d \r\n", hk.fields.pgood_5V.raw);
	printf(" \t pgood_BV_bf = %d \r\n", hk.fields.pgood_BV.raw);
	printf(" \t pgood_HV_bf = %d \r\n", hk.fields.pgood_HV.raw);
	printf(" \t bat_charging_bf = %d \r\n", hk.fields.bat_charging.raw);
	printf(" \t bat_heating_bf = %d \r\n", hk.fields.bat_heating.raw);

	printf(" \t pwr_generating_mW = %d \r\n", hk.fields.pwr_generating);
	printf(" \t pwr_charging_mW = %d \r\n", hk.fields.pwr_charging);
	printf(" \t pwr_consuming_mW = %d \r\n", hk.fields.pwr_consuming);
	printf(" \t pwr_delivering_mW = %d \r\n", hk.fields.pwr_delivering);

	printf(" \t obus_3V3_curr_raw = %d \r\n", hk.fields.obus3V3_curr_raw);
	printf(" \t obus_3V3_volt_raw = %d \r\n", hk.fields.obus3V3_volt_raw);
	printf(" \t obus_5V_curr_raw = %d \r\n", hk.fields.obus5V_curr_raw);
	printf(" \t obus_5V_volt_raw = %d \r\n", hk.fields.obus5V_volt_raw);
	printf(" \t obus_BV_curr_raw = %d \r\n", hk.fields.obusBV_curr_raw);
	printf(" \t obus_BV_volt_raw = %d \r\n", hk.fields.obusBV_volt_raw);

	printf(" \t charge_bus_volt_raw = %d \r\n", hk.fields.chrg_bus_volt_raw);
	printf(" \t pcb_temp_raw = %d \r\n", hk.fields.pcb_temp_raw);
	printf(" \t mppt_temp_raw = %d \r\n", hk.fields.mppt_temp_raw);
	printf(" \t mcu_temp_raw = %d \r\n", hk.fields.mcu_temp_raw);

	printf(" \t mppt0_in_curr_raw = %d \r\n", hk.fields.mppt0_in_curr_raw);
	printf(" \t mppt0_in_volt_raw = %d \r\n", hk.fields.mppt0_in_volt_raw);
	printf(" \t mppt0_out_curr_raw = %d \r\n", hk.fields.mppt0_out_curr_raw);
	printf(" \t mppt0_out_volt_raw = %d \r\n", hk.fields.mppt0_out_volt_raw);

	printf(" \t mppt1_in_curr_raw = %d \r\n", hk.fields.mppt1_in_curr_raw);
	printf(" \t mppt1_in_volt_raw = %d \r\n", hk.fields.mppt1_in_volt_raw);
	printf(" \t mppt1_out_curr_raw = %d \r\n", hk.fields.mppt1_out_curr_raw);
	printf(" \t mppt1_out_volt_raw = %d \r\n", hk.fields.mppt1_out_volt_raw);

	printf(" \t mppt2_in_curr_raw = %d \r\n", hk.fields.mppt2_in_curr_raw);
	printf(" \t mppt2_in_volt_raw = %d \r\n", hk.fields.mppt2_in_volt_raw);
	printf(" \t mppt2_out_curr_raw = %d \r\n", hk.fields.mppt2_out_curr_raw);
	printf(" \t mppt2_out_volt_raw = %d \r\n", hk.fields.mppt2_out_volt_raw);

	return TRUE;
}


static Boolean _getEngHKDataMB(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	ieps_enghk_data_mb_t hk;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getEngHKDataMB *** \r\n");
		printf(" Sends the getEngHKDataMB command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_GET_ENGHKDATA_MB);
		printf(" Retrieves mainboard house keeping data in engineering form. \r\n");
		printf(" The IEPS system consists of a single mainboard which is combined with up to 7 CDBs. \r\n");
		printf(" Requesting housekeeping data has been split into separate calls for each board. \r\n");
		printf(" Availability of CDBs and their batteries/components depends on the IEPS package bought. \r\n");
		printf(" The following information is returned: \r\n");
		printf(" board_idx = %s; always %d \r\n", "mainboard board index", ieps_board_mb);
		printf(" comp_bf = %s \r\n", "board components successfully queried");
		printf(" swion_3V3_bf = %s \r\n", "bitflag with a 1 bit for each channel that is commanded to the 'on' state");
		printf(" swion_5V_bf = %s \r\n", "bitflag with a 1 bit for each channel that is commanded to the 'on' state");
		printf(" swion_BV_bf = %s \r\n", "bitflag with a 1 bit for each channel that is commanded to the 'on' state");
		printf(" swion_HV_bf = %s \r\n", "bitflag with a 1 bit for each channel that is commanded to the 'on' state");
		printf(" pgood_3V3_bf = %s \r\n", "bitflag with a 1 bit when channel voltage >90% of 3.3V");
		printf(" pgood_5V_bf = %s \r\n", "bitflag with a 1 bit when channel voltage >90% of 5V");
		printf(" pgood_BV_bf = %s \r\n", "bitflag with a 1 bit when battery channel voltage >90%");
		printf(" pgood_HV_bf = %s \r\n", "bitflag with a 1 bit when high-voltage channel voltage >90% of target voltage");
		printf(" bat_charging_bf = %s \r\n", "bitflag with a 1 bit when corresponding battery charging");
		printf(" bat_heating_bf = %s \r\n", "bitflag with a 1 bit when corresponding heater heating");

		printf(" pwr_generating_mW = %s \r\n", "power provided to the IEPS internal power bus by the solar panels/MPPTs");
		printf(" pwr_charging_mW = %s \r\n", "power exchanged between IEPS internal power bus and all batteries (positive = charging)");
		printf(" pwr_consuming_mW = %s \r\n", "power taken from the IEPS internal power bus (used by IEPS and satellite)");
		printf(" pwr_delivering_mW = %s \r\n", "power being delivered to the satellite bus");

		printf(" obus_3V3_curr_mA = %s \r\n", "3V3 bus current measurement in mA");
		printf(" obus_3V3_volt_mV = %s \r\n", "3V3 bus voltage measurement in mV");
		printf(" obus_5V_curr_mA = %s \r\n", "5V bus current measurement in mA");
		printf(" obus_5V_volt_mV = %s \r\n", "5V bus voltage measurement in mV");
		printf(" obus_BV_curr_mA = %s \r\n", "battery bus current measurement in mA");
		printf(" obus_BV_volt_mV = %s \r\n", "battery bus voltage measurement in mV");

		printf(" charge_bus_volt_mV = %s \r\n", "charge bus voltage measurement in mV");
		printf(" pcb_temp_degC = %s \r\n", "pcb temperature measurement in degrees C");
		printf(" mppt_temp_degC = %s \r\n", "mppt temperature measurement in degrees C");
		printf(" mcu_temp_degC = %s \r\n", "mcu temperature measurement in degrees C");

		printf(" mppt0_in_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt0_in_volt_mV = %s \r\n", "voltage measurement in mV");
		printf(" mppt0_out_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt0_out_volt_mV = %s \r\n", "voltage measurement in mV");

		printf(" mppt1_in_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt1_in_volt_mV = %s \r\n", "voltage measurement in mV");
		printf(" mppt1_out_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt1_out_volt_mV = %s \r\n", "voltage measurement in mV");

		printf(" mppt2_in_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt2_in_volt_mV = %s \r\n", "voltage measurement in mV");
		printf(" mppt2_out_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt2_out_volt_mV = %s \r\n", "voltage measurement in mV");

		return TRUE;
	}

	printf("\r\n Perform getEngHKDataMB \r\n");

	rv = IsisEPS_getEngHKDataMB(0, &hk, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)	// non-zero return value means error!
		return TRUE;								// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	printf(" \t board_idx = %d \r\n", hk.fields.board);
	printf(" \t comp_bf = %d \r\n", hk.fields.comp.raw);
	printf(" \t swion_3V3_bf = %d \r\n", hk.fields.swion_3V3.raw);
	printf(" \t swion_5V_bf = %d \r\n", hk.fields.swion_5V.raw);
	printf(" \t swion_BV_bf = %d \r\n", hk.fields.swion_BV.raw);
	printf(" \t swion_HV_bf = %d \r\n", hk.fields.swion_HV.raw);
	printf(" \t pgood_3V3_bf = %d \r\n", hk.fields.pgood_3V3.raw);
	printf(" \t pgood_5V_bf = %d \r\n", hk.fields.pgood_5V.raw);
	printf(" \t pgood_BV_bf = %d \r\n", hk.fields.pgood_BV.raw);
	printf(" \t pgood_HV_bf = %d \r\n", hk.fields.pgood_HV.raw);
	printf(" \t bat_charging_bf = %d \r\n", hk.fields.bat_charging.raw);
	printf(" \t bat_heating_bf = %d \r\n", hk.fields.bat_heating.raw);

	printf(" \t pwr_generating_mW = %d \r\n", hk.fields.pwr_generating);
	printf(" \t pwr_charging_mW = %d \r\n", hk.fields.pwr_charging);
	printf(" \t pwr_consuming_mW = %d \r\n", hk.fields.pwr_consuming);
	printf(" \t pwr_delivering_mW = %d \r\n", hk.fields.pwr_delivering);

	printf(" \t obus_3V3_curr_mA = %d \r\n", hk.fields.obus3V3_curr);
	printf(" \t obus_3V3_volt_mV = %d \r\n", hk.fields.obus3V3_volt);
	printf(" \t obus_5V_curr_mA = %d \r\n", hk.fields.obus5V_curr);
	printf(" \t obus_5V_volt_mV = %d \r\n", hk.fields.obus5V_volt);
	printf(" \t obus_BV_curr_mA = %d \r\n", hk.fields.obusBV_curr);
	printf(" \t obus_BV_volt_mV = %d \r\n", hk.fields.obusBV_volt);

	printf(" \t charge_bus_volt_mV = %d \r\n", hk.fields.chrg_bus_volt);
	printf(" \t pcb_temp_degC = %d \r\n", hk.fields.pcb_temp);
	printf(" \t mppt_temp_degC = %d \r\n", hk.fields.mppt_temp);
	printf(" \t mcu_temp_degC = %d \r\n", hk.fields.mcu_temp);

	printf(" \t mppt0_in_curr_mA = %d \r\n", hk.fields.mppt0_in_curr);
	printf(" \t mppt0_in_volt_mV = %d \r\n", hk.fields.mppt0_in_volt);
	printf(" \t mppt0_out_curr_mA = %d \r\n", hk.fields.mppt0_out_curr);
	printf(" \t mppt0_out_volt_mV = %d \r\n", hk.fields.mppt0_out_volt);

	printf(" \t mppt1_in_curr_mA = %d \r\n", hk.fields.mppt1_in_curr);
	printf(" \t mppt1_in_volt_mV = %d \r\n", hk.fields.mppt1_in_volt);
	printf(" \t mppt1_out_curr_mA = %d \r\n", hk.fields.mppt1_out_curr);
	printf(" \t mppt1_out_volt_mV = %d \r\n", hk.fields.mppt1_out_volt);

	printf(" \t mppt2_in_curr_mA = %d \r\n", hk.fields.mppt2_in_curr);
	printf(" \t mppt2_in_volt_mV = %d \r\n", hk.fields.mppt2_in_volt);
	printf(" \t mppt2_out_curr_mA = %d \r\n", hk.fields.mppt2_out_curr);
	printf(" \t mppt2_out_volt_mV = %d \r\n", hk.fields.mppt2_out_volt);

	return TRUE;
}


static Boolean _getRAEngHKDataMB(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	ieps_enghk_data_mb_t hk;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getRAEngHKDataMB *** \r\n");
		printf(" Sends the getRAEngHKDataMB command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_GET_AVGHKDATA_MB);
		printf(" Retrieves a running averaged version of getEngHKDataMB reducing measurement noise. \r\n");
		printf(" The following information is returned: \r\n");
		printf(" board_idx = %s; always %d \r\n", "mainboard board index", ieps_board_mb);
		printf(" comp_bf = %s \r\n", "board components successfully queried");
		printf(" pgood_3V3_bf = %s \r\n", "bitflag with a 1 bit when channel voltage >90% of 3.3V");
		printf(" pgood_5V_bf = %s \r\n", "bitflag with a 1 bit when channel voltage >90% of 5V");
		printf(" pgood_BV_bf = %s \r\n", "bitflag with a 1 bit when battery channel voltage >90%");
		printf(" pgood_HV_bf = %s \r\n", "bitflag with a 1 bit when high-voltage channel voltage >90% of target voltage");
		printf(" bat_charging_bf = %s \r\n", "bitflag with a 1 bit when corresponding battery charging");
		printf(" bat_heating_bf = %s \r\n", "bitflag with a 1 bit when corresponding heater heating");

		printf(" pwr_generating_mW = %s \r\n", "power provided to the IEPS internal power bus by the solar panels/MPPTs");
		printf(" pwr_charging_mW = %s \r\n", "power exchanged between IEPS internal power bus and all batteries (positive = charging)");
		printf(" pwr_consuming_mW = %s \r\n", "power taken from the IEPS internal power bus (used by IEPS and satellite)");
		printf(" pwr_delivering_mW = %s \r\n", "power being delivered to the satellite bus");

		printf(" obus_3V3_curr_mA = %s \r\n", "3V3 bus current measurement in mA");
		printf(" obus_3V3_volt_mV = %s \r\n", "3V3 bus voltage measurement in mV");
		printf(" obus_5V_curr_mA = %s \r\n", "5V bus current measurement in mA");
		printf(" obus_5V_volt_mV = %s \r\n", "5V bus voltage measurement in mV");
		printf(" obus_BV_curr_mA = %s \r\n", "battery bus current measurement in mA");
		printf(" obus_BV_volt_mV = %s \r\n", "battery bus voltage measurement in mV");

		printf(" charge_bus_volt_mV = %s \r\n", "charge bus voltage measurement in mV");
		printf(" pcb_temp_degC = %s \r\n", "pcb temperature measurement in degrees C");
		printf(" mppt_temp_degC = %s \r\n", "mppt temperature measurement in degrees C");
		printf(" mcu_temp_degC = %s \r\n", "mcu temperature measurement in degrees C");

		printf(" mppt0_in_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt0_in_volt_mV = %s \r\n", "voltage measurement in mV");
		printf(" mppt0_out_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt0_out_volt_mV = %s \r\n", "voltage measurement in mV");

		printf(" mppt1_in_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt1_in_volt_mV = %s \r\n", "voltage measurement in mV");
		printf(" mppt1_out_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt1_out_volt_mV = %s \r\n", "voltage measurement in mV");

		printf(" mppt2_in_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt2_in_volt_mV = %s \r\n", "voltage measurement in mV");
		printf(" mppt2_out_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt2_out_volt_mV = %s \r\n", "voltage measurement in mV");

		return TRUE;
	}

	printf("\r\n Perform getRAEngHKDataMB \r\n");

	rv = IsisEPS_getRAEngHKDataMB(0, &hk, &rsp_stat);	// here we're getting the *running average* version of the engineering hk values
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	printf(" \t board_idx = %d \r\n", hk.fields.board);
	printf(" \t comp_bf = %d \r\n", hk.fields.comp.raw);
	printf(" \t pgood_3V3_bf = %d \r\n", hk.fields.pgood_3V3.raw);
	printf(" \t pgood_5V_bf = %d \r\n", hk.fields.pgood_5V.raw);
	printf(" \t pgood_BV_bf = %d \r\n", hk.fields.pgood_BV.raw);
	printf(" \t pgood_HV_bf = %d \r\n", hk.fields.pgood_HV.raw);
	printf(" \t bat_charging_bf = %d \r\n", hk.fields.bat_charging.raw);
	printf(" \t bat_heating_bf = %d \r\n", hk.fields.bat_heating.raw);

	printf(" \t pwr_generating_mW = %d \r\n", hk.fields.pwr_generating);
	printf(" \t pwr_charging_mW = %d \r\n", hk.fields.pwr_charging);
	printf(" \t pwr_consuming_mW = %d \r\n", hk.fields.pwr_consuming);
	printf(" \t pwr_delivering_mW = %d \r\n", hk.fields.pwr_delivering);

	printf(" \t obus_3V3_curr_mA = %d \r\n", hk.fields.obus3V3_curr);
	printf(" \t obus_3V3_volt_mV = %d \r\n", hk.fields.obus3V3_volt);
	printf(" \t obus_5V_curr_mA = %d \r\n", hk.fields.obus5V_curr);
	printf(" \t obus_5V_volt_mV = %d \r\n", hk.fields.obus5V_volt);
	printf(" \t obus_BV_curr_mA = %d \r\n", hk.fields.obusBV_curr);
	printf(" \t obus_BV_volt_mV = %d \r\n", hk.fields.obusBV_volt);

	printf(" \t charge_bus_volt_mV = %d \r\n", hk.fields.chrg_bus_volt);
	printf(" \t pcb_temp_degC = %d \r\n", hk.fields.pcb_temp);
	printf(" \t mppt_temp_degC = %d \r\n", hk.fields.mppt_temp);
	printf(" \t mcu_temp_degC = %d \r\n", hk.fields.mcu_temp);

	printf(" \t mppt0_in_curr_mA = %d \r\n", hk.fields.mppt0_in_curr);
	printf(" \t mppt0_in_volt_mV = %d \r\n", hk.fields.mppt0_in_volt);
	printf(" \t mppt0_out_curr_mA = %d \r\n", hk.fields.mppt0_out_curr);
	printf(" \t mppt0_out_volt_mV = %d \r\n", hk.fields.mppt0_out_volt);

	printf(" \t mppt1_in_curr_mA = %d \r\n", hk.fields.mppt1_in_curr);
	printf(" \t mppt1_in_volt_mV = %d \r\n", hk.fields.mppt1_in_volt);
	printf(" \t mppt1_out_curr_mA = %d \r\n", hk.fields.mppt1_out_curr);
	printf(" \t mppt1_out_volt_mV = %d \r\n", hk.fields.mppt1_out_volt);

	printf(" \t mppt2_in_curr_mA = %d \r\n", hk.fields.mppt2_in_curr);
	printf(" \t mppt2_in_volt_mV = %d \r\n", hk.fields.mppt2_in_volt);
	printf(" \t mppt2_out_curr_mA = %d \r\n", hk.fields.mppt2_out_curr);
	printf(" \t mppt2_out_volt_mV = %d \r\n", hk.fields.mppt2_out_volt);

	return TRUE;
}


static Boolean _getRawHKDataCDB(Boolean info)
{
	ieps_board_t board;
	ieps_statcmd_t rsp_stat;
	ieps_rawhk_data_cdb_t hk;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getRawHKDataCDB *** \r\n");
		printf(" Sends the getRawHKDataCDB command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_GET_RAWHKDATA_CDB);
		printf(" Retrieves companion/daughterboard (CDB) house keeping data in RAW form (as-is provided by the hardware). \r\n");
		printf(" The IEPS system consists of a single mainboard which is combined with up to 7 CDBs. \r\n");
		printf(" Requesting housekeeping data has been split into separate calls for each board. \r\n");
		printf(" Availability of CDBs and their batteries/components depends on the IEPS package bought. \r\n");
		printf(" The following information is returned: \r\n");
		printf(" board_idx = %s \r\n", "companion/daughterboard board index");
		printf(" comp_bf = %s \r\n", "board components successfully queried");
		printf(" obusHV_curr_raw = %s \r\n", "current measurement in raw counts");
		printf(" obusHV_volt_raw = %s \r\n", "voltage measurement in raw counts");

		printf(" bat_charge_raw = %s \r\n", "battery charge in raw counts");
		printf(" bat_current_raw = %s \r\n", "current measurement in raw counts");
		printf(" bat_voltage_raw = %s \r\n", "voltage measurement in raw counts");
		printf(" bat_temp_raw = %s \r\n", "temperature measurement in raw counts");

		printf(" mppt0_in_curr_raw = %s \r\n", "current measurement in raw counts");
		printf(" mppt0_in_volt_raw = %s \r\n", "voltage measurement in raw counts");
		printf(" mppt0_out_curr_raw = %s \r\n", "current measurement in raw counts");
		printf(" mppt0_out_volt_raw = %s \r\n", "voltage measurement in raw counts");

		printf(" mppt1_in_curr_raw = %s \r\n", "current measurement in raw counts");
		printf(" mppt1_in_volt_raw = %s \r\n", "voltage measurement in raw counts");
		printf(" mppt1_out_curr_raw = %s \r\n", "current measurement in raw counts");
		printf(" mppt1_out_volt_raw = %s \r\n", "voltage measurement in raw counts");

		return TRUE;
	}

	printf("\r\n Perform getRawHKDataCDB \r\n");

	if(!_get_cdb_board_selection(" Select companion/daughterboard (1 through 7) [1]: ", &board)) return TRUE;		// ask for the cdb index; exit on failure

	rv = IsisEPS_getRawHKDataCDB(0, board, &hk, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)	// non-zero return value means error!
		return TRUE;								// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	printf(" \t board_idx = %d \r\n", hk.fields.board);
	printf(" \t comp_bf = %d \r\n", hk.fields.comp.raw);
	printf(" \t obusHV_curr_raw = %d \r\n", hk.fields.obusHV_curr_raw);
	printf(" \t obusHV_volt_raw = %d \r\n", hk.fields.obusHV_volt_raw);

	printf(" \t bat_charge_raw = %d \r\n", hk.fields.bat_charge_raw);
	printf(" \t bat_current_raw = %d \r\n", hk.fields.bat_current_raw);
	printf(" \t bat_voltage_raw = %d \r\n", hk.fields.bat_voltage_raw);
	printf(" \t bat_temp_raw = %d \r\n", hk.fields.bat_temp_raw);

	printf(" \t mppt0_in_curr_raw = %d \r\n", hk.fields.mppt0_in_curr_raw);
	printf(" \t mppt0_in_volt_raw = %d \r\n", hk.fields.mppt0_in_volt_raw);
	printf(" \t mppt0_out_curr_raw = %d \r\n", hk.fields.mppt0_out_curr_raw);
	printf(" \t mppt0_out_volt_raw = %d \r\n", hk.fields.mppt0_out_volt_raw);

	printf(" \t mppt1_in_curr_raw = %d \r\n", hk.fields.mppt1_in_curr_raw);
	printf(" \t mppt1_in_volt_raw = %d \r\n", hk.fields.mppt1_in_volt_raw);
	printf(" \t mppt1_out_curr_raw = %d \r\n", hk.fields.mppt1_out_curr_raw);
	printf(" \t mppt1_out_volt_raw = %d \r\n", hk.fields.mppt1_out_volt_raw);

	return TRUE;
}

static Boolean _getEngHKDataCDB(Boolean info)
{
	ieps_board_t board;
	ieps_statcmd_t rsp_stat;
	ieps_enghk_data_cdb_t hk;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getEngHKDataCDB *** \r\n");
		printf(" Sends the getEngHKDataCDB command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_GET_ENGHKDATA_CDB);
		printf(" Retrieves companion/daughterboard (CDB) house keeping data in engineering form. \r\n");
		printf(" The IEPS system consists of a single mainboard which is combined with up to 7 CDBs. \r\n");
		printf(" Requesting housekeeping data has been split into separate calls for each board. \r\n");
		printf(" Availability of CDBs and their batteries/components depends on the IEPS package bought. \r\n");
		printf(" The following information is returned: \r\n");
		printf(" board_idx = %s \r\n", "companion/daughterboard board index");
		printf(" comp_bf = %s \r\n", "board components successfully queried");
		printf(" obusHV_curr_mA = %s \r\n", "current measurement in mA");
		printf(" obusHV_volt_mV = %s \r\n", "voltage measurement in mV");

		printf(" bat_charge_mAh = %s \r\n", "battery charge in mAh; total charge per CDB ~6000mAh");
		printf(" bat_current_mA = %s \r\n", "signed current measurement in mA; positive when charging.");
		printf(" bat_voltage_mV = %s \r\n", "voltage measurement in mV");
		printf(" bat_temp_degC = %s \r\n", "temperature measurement in degC");

		printf(" mppt0_in_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt0_in_volt_mV = %s \r\n", "voltage measurement in mV");
		printf(" mppt0_out_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt0_out_volt_mV = %s \r\n", "voltage measurement in mV");

		printf(" mppt1_in_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt1_in_volt_mV = %s \r\n", "voltage measurement in mV");
		printf(" mppt1_out_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt1_out_volt_mV = %s \r\n", "voltage measurement in mV");

		return TRUE;
	}

	printf("\r\n Perform getEngHKDataCDB \r\n");

	if(!_get_cdb_board_selection(" Select companion/daughterboard (1 through 7) [1]: ", &board)) return TRUE;		// ask for the cdb index; exit on failure

	rv = IsisEPS_getEngHKDataCDB(0, board, &hk, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)	// non-zero return value means error!
		return TRUE;								// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	printf(" \t board_idx = %d \r\n", hk.fields.board);
	printf(" \t comp_bf = %d \r\n", hk.fields.comp.raw);
	printf(" \t obusHV_curr_mA = %d \r\n", hk.fields.obusHV_curr);
	printf(" \t obusHV_volt_mV = %d \r\n", hk.fields.obusHV_volt);

	printf(" \t bat_charge_mAh = %d \r\n", hk.fields.bat_charge);
	printf(" \t bat_current_mA = %d \r\n", hk.fields.bat_current);
	printf(" \t bat_voltage_mV = %d \r\n", hk.fields.bat_voltage);
	printf(" \t bat_temp_degC = %d \r\n", hk.fields.bat_temp);

	printf(" \t mppt0_in_curr_mA = %d \r\n", hk.fields.mppt0_in_curr);
	printf(" \t mppt0_in_volt_mV = %d \r\n", hk.fields.mppt0_in_volt);
	printf(" \t mppt0_out_curr_mA = %d \r\n", hk.fields.mppt0_out_curr);
	printf(" \t mppt0_out_volt_mV = %d \r\n", hk.fields.mppt0_out_volt);

	printf(" \t mppt1_in_curr_mA = %d \r\n", hk.fields.mppt1_in_curr);
	printf(" \t mppt1_in_volt_mV = %d \r\n", hk.fields.mppt1_in_volt);
	printf(" \t mppt1_out_curr_mA = %d \r\n", hk.fields.mppt1_out_curr);
	printf(" \t mppt1_out_volt_mV = %d \r\n", hk.fields.mppt1_out_volt);

	return TRUE;
}

static Boolean _getRAEngHKDataCDB(Boolean info)
{
	ieps_board_t board;
	ieps_statcmd_t rsp_stat;
	ieps_enghk_data_cdb_t hk;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getRAEngHKDataCDB *** \r\n");
		printf(" Sends the getRAEngHKDataCDB command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_GET_AVGHKDATA_CDB);
		printf(" Retrieves a running averaged version of getEngHKDataCDB reducing measurement noise. \r\n");
		printf(" The following information is returned: \r\n");
		printf(" board_idx = %s \r\n", "companion/daughterboard board index");
		printf(" comp_bf = %s \r\n", "board components successfully queried");
		printf(" obusHV_curr_mA = %s \r\n", "current measurement in mA");
		printf(" obusHV_volt_mV = %s \r\n", "voltage measurement in mV");

		printf(" bat_charge_mAh = %s \r\n", "battery charge in mAh; total charge per CDB ~6000mAh");
		printf(" bat_current_mA = %s \r\n", "signed current measurement in mA; positive when charging.");
		printf(" bat_voltage_mV = %s \r\n", "voltage measurement in mV");
		printf(" bat_temp_degC = %s \r\n", "temperature measurement in degC");

		printf(" mppt0_in_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt0_in_volt_mV = %s \r\n", "voltage measurement in mV");
		printf(" mppt0_out_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt0_out_volt_mV = %s \r\n", "voltage measurement in mV");

		printf(" mppt1_in_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt1_in_volt_mV = %s \r\n", "voltage measurement in mV");
		printf(" mppt1_out_curr_mA = %s \r\n", "current measurement in mA");
		printf(" mppt1_out_volt_mV = %s \r\n", "voltage measurement in mV");

		return TRUE;
	}

	printf("\r\n Perform getRAEngHKDataCDB \r\n");

	if(!_get_cdb_board_selection(" Select companion/daughterboard (1 through 7) [1]: ", &board)) return TRUE;		// ask for the cdb index; exit on failure

	rv = IsisEPS_getRAEngHKDataCDB(0, board, &hk, &rsp_stat);	// here we're getting the *running average* version of the engineering hk values
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)	// non-zero return value means error!
		return TRUE;								// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	printf(" \t board_idx = %d \r\n", hk.fields.board);
	printf(" \t comp_bf = %d \r\n", hk.fields.comp.raw);
	printf(" \t obusHV_curr_mA = %d \r\n", hk.fields.obusHV_curr);
	printf(" \t obusHV_volt_mV = %d \r\n", hk.fields.obusHV_volt);

	printf(" \t bat_charge_mAh = %d \r\n", hk.fields.bat_charge);
	printf(" \t bat_current_mA = %d \r\n", hk.fields.bat_current);
	printf(" \t bat_voltage_mV = %d \r\n", hk.fields.bat_voltage);
	printf(" \t bat_temp_degC = %d \r\n", hk.fields.bat_temp);

	printf(" \t mppt0_in_curr_mA = %d \r\n", hk.fields.mppt0_in_curr);
	printf(" \t mppt0_in_volt_mV = %d \r\n", hk.fields.mppt0_in_volt);
	printf(" \t mppt0_out_curr_mA = %d \r\n", hk.fields.mppt0_out_curr);
	printf(" \t mppt0_out_volt_mV = %d \r\n", hk.fields.mppt0_out_volt);

	printf(" \t mppt1_in_curr_mA = %d \r\n", hk.fields.mppt1_in_curr);
	printf(" \t mppt1_in_volt_mV = %d \r\n", hk.fields.mppt1_in_volt);
	printf(" \t mppt1_out_curr_mA = %d \r\n", hk.fields.mppt1_out_curr);
	printf(" \t mppt1_out_volt_mV = %d \r\n", hk.fields.mppt1_out_volt);

	return TRUE;
}

static void show_LOState(ieps_lostate_t* los)
{
	printf(" \t pgood_bf = %d \r\n", los->fields.pgood.raw);
	printf(" \t latch off state_bf = %d \r\n", los->fields.los.raw);
	printf(" \t latch off count[0] = %d \r\n", los->fields.los_cnt[0]);
	printf(" \t latch off count[1] = %d \r\n", los->fields.los_cnt[1]);
	printf(" \t latch off count[2] = %d \r\n", los->fields.los_cnt[2]);
	printf(" \t latch off count[3] = %d \r\n", los->fields.los_cnt[3]);
	printf(" \t latch off count[4] = %d \r\n", los->fields.los_cnt[4]);
	printf(" \t latch off count[5] = %d \r\n", los->fields.los_cnt[5]);
	printf(" \t latch off count[6] = %d \r\n", los->fields.los_cnt[6]);
	printf(" \t latch off count[7] = %d \r\n", los->fields.los_cnt[7]);
}

static Boolean _getLOState3V3(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	ieps_lostate_t los;
	unsigned int rv;

	if(info)
	{
		printf("\r\n Information getLOState3V3 \r\n");
		printf(" Sends the getLOState3V3 command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_GET_OUTBUS_3V3_STATE);
		printf(" Provides 3V3 output bus on/off and overcurrent latch off state information. \r\n");
		printf(" The following information is returned: \r\n");
		printf(" pgood_bf = %s \r\n", "bitflag with a 1 bit when channel voltage >90% of 3.3V");
		printf(" latch off state_bf = %s \r\n", "bitflag with a 1 bit when channel was latched off due to over-current.");
		printf(" latch off count[0] = %s \r\n", "amount of latch off events on channel C0");
		printf(" latch off count[1] = %s \r\n", "amount of latch off events on channel C1");
		printf(" latch off count[2] = %s \r\n", "amount of latch off events on channel C2");
		printf(" latch off count[3] = %s \r\n", "amount of latch off events on channel C3");
		printf(" latch off count[4] = %s \r\n", "amount of latch off events on channel C4");
		printf(" latch off count[5] = %s \r\n", "amount of latch off events on channel C5");
		printf(" latch off count[6] = %s \r\n", "amount of latch off events on channel C6");
		printf(" latch off count[7] = %s \r\n", "amount of latch off events on channel C7");

		return TRUE;
	}

	printf("\r\n Perform getLOState3V3 \r\n");

	rv = IsisEPS_getLOState3V3(0, &los, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)	// non-zero return value means error!
		return TRUE;								// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	show_LOState(&los);

	return TRUE;
}


static Boolean _getLOState5V(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	ieps_lostate_t los;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getLOState5V *** \r\n");
		printf(" Sends the getLOState5V command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_GET_OUTBUS_5V_STATE);
		printf(" Provides 5V output bus on/off and overcurrent latch off state information. \r\n");
		printf(" The following information is returned: \r\n");
		printf(" pgood_bf = %s \r\n", "bitflag with a 1 bit when channel voltage >90% of 5V");
		printf(" latch off state_bf = %s \r\n", "bitflag with a 1 bit when channel was latched off due to over-current.");
		printf(" latch off count[0] = %s \r\n", "amount of latch off events on channel C0");
		printf(" latch off count[1] = %s \r\n", "amount of latch off events on channel C1");
		printf(" latch off count[2] = %s \r\n", "amount of latch off events on channel C2");
		printf(" latch off count[3] = %s \r\n", "amount of latch off events on channel C3");
		printf(" latch off count[4] = %s \r\n", "amount of latch off events on channel C4");
		printf(" latch off count[5] = %s \r\n", "amount of latch off events on channel C5");
		printf(" latch off count[6] = %s \r\n", "amount of latch off events on channel C6");
		printf(" latch off count[7] = %s \r\n", "amount of latch off events on channel C7");

		return TRUE;
	}

	printf("\r\n Perform getLOState5V \r\n");

	rv = IsisEPS_getLOState5V(0, &los, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)	// non-zero return value means error!
		return TRUE;								// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	show_LOState(&los);

	return TRUE;
}


static Boolean _getLOStateBV(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	ieps_lostate_t los;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getLOStateBV *** \r\n");
		printf(" Sends the getLOStateBV command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_GET_OUTBUS_BV_STATE);
		printf(" Provides battery voltage (BV) output bus on/off and overcurrent latch off state information. \r\n");
		printf(" The following information is returned: \r\n");
		printf(" pgood_bf = %s \r\n", "bitflag with a 1 bit when channel voltage >90% of 8V");
		printf(" latch off state_bf = %s \r\n", "bitflag with a 1 bit when channel was latched off due to over-current.");
		printf(" latch off count[0] = %s \r\n", "amount of latch off events on channel C0");
		printf(" latch off count[1] = %s \r\n", "amount of latch off events on channel C1");
		printf(" latch off count[2] = %s \r\n", "amount of latch off events on channel C2");
		printf(" latch off count[3] = %s \r\n", "amount of latch off events on channel C3");
		printf(" latch off count[4] = %s \r\n", "amount of latch off events on channel C4");
		printf(" latch off count[5] = %s \r\n", "amount of latch off events on channel C5");
		printf(" latch off count[6] = %s \r\n", "amount of latch off events on channel C6");
		printf(" latch off count[7] = %s \r\n", "amount of latch off events on channel C7");

		return TRUE;
	}

	printf("\r\n Perform getLOStateBV \r\n");

	rv = IsisEPS_getLOStateBV(0, &los, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)	// non-zero return value means error!
		return TRUE;								// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	show_LOState(&los);

	return TRUE;
}


static Boolean _getLOStateHV(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	ieps_lostate_t los;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information getLOStateHV *** \r\n");
		printf(" Sends the getLOStateHV command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_GET_OUTBUS_HV_STATE);
		printf(" Provides high-voltage (HV) output bus on/off and overcurrent latch off state information. \r\n");
		printf(" The following information is returned: \r\n");
		printf(" pgood_bf = %s \r\n", "bitflag with a 1 bit when channel voltage >90% of target voltage");
		printf(" latch off state_bf = %s \r\n", "bitflag with a 1 bit when channel was latched off due to over-current.");
		printf(" latch off count[0] = %s \r\n", "amount of latch off events on channel C0");
		printf(" latch off count[1] = %s \r\n", "amount of latch off events on channel C1");
		printf(" latch off count[2] = %s \r\n", "amount of latch off events on channel C2");
		printf(" latch off count[3] = %s \r\n", "amount of latch off events on channel C3");
		printf(" latch off count[4] = %s \r\n", "amount of latch off events on channel C4");
		printf(" latch off count[5] = %s \r\n", "amount of latch off events on channel C5");
		printf(" latch off count[6] = %s \r\n", "amount of latch off events on channel C6");
		printf(" latch off count[7] = %s \r\n", "amount of latch off events on channel C7");

		return TRUE;
	}

	printf("\r\n Perform getLOStateHV \r\n");

	rv = IsisEPS_getLOStateHV(0, &los, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)	// non-zero return value means error!
		return TRUE;								// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	show_LOState(&los);

	return TRUE;
}

static Boolean _getParameter(Boolean info)
{
	unsigned short par_id;							// storage for the param-id
	unsigned char par_data[8];						// byte array for storing our parameter data
	ieps_statcmd_t rsp_stat;						// storage for the command response
	unsigned int rv;

	if(info)
	{
		printf("\r\n Information getParameter \r\n");
		printf(" Sends the getParameter command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_GET_PARAMETER);
		printf(" Used to get configuration parameter values from the IEPS. \r\n");
		printf(" Execution is performed and completed immediately. \r\n");
		printf(" The following information is returned: \r\n");
		printf(" param-id = %s \r\n", "parameter-id of the parameter under consideration.");
		printf(" param-value = %s \r\n", "value of the parameter under consideration. Between 1 and 8 bytes.");

		return TRUE;
	}

	printf("\r\n Perform getParameter \r\n");

	if(!config_param_info(CONFIG_PARAM_OP_ask_parid, &par_id, NULL)) return TRUE;	// get the param-id from the user

	rv = IsisEPS_getParameter(0, par_id, par_data, &rsp_stat);	// get the parameter from the IEPS
	if(rv)
	{
		TRACE_ERROR("return value=%d \r\n", rv)		// non-zero return value means error!
		return TRUE;								// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&rsp_stat);							// parse the command response and show that to the user

	config_param_info(CONFIG_PARAM_OP_print, &par_id, &par_data);	// show the param-id and corresponding value that we received back

	return TRUE;
}


static Boolean _setParameter(Boolean info)
{
	unsigned short par_id;							// storage for the param-id
	unsigned char par_data[8] = {0};				// byte array for storing the parameter data we want to set; our maximum size is 8 bytes (e.g. double, long ...)
	unsigned char par_data_out[8] = {0};			// byte array for storing the parameter data that was actually set; our maximum size is 8 bytes (e.g. double, long ...)
	ieps_statcmd_t rsp_stat;						// storage for the command response
	unsigned int rv;

	if(info)
	{
		printf("\r\n Information setParameter \r\n");
		printf("Sends the setParameter command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_SET_PARAMETER);
		printf("Used to set configuration parameter values within the IEPS. \r\n");
		printf("Execution is performed and completed immediately. \r\n");
		printf("The following information is returned: \r\n");
		printf("param-id = %s \r\n", "parameter-id of the parameter under consideration.");
		printf("param-value = %s \r\n", "new value of the parameter under consideration. Between 1 and 8 bytes.");

		return TRUE;
	}

	printf("\r\n Perform setParameter \r\n");

	if(!config_param_info(CONFIG_PARAM_OP_ask_parid_and_data, &par_id, &par_data)) return TRUE;	// get the param-id and new value from the user

	rv = IsisEPS_setParameter(0, par_id, &par_data, &par_data_out, &rsp_stat);		// send the new parameter data to the IEPS
	if(rv)
	{
		TRACE_ERROR("return value=%d \r\n", rv)		// non-zero return value means error!
		return TRUE;								// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&rsp_stat);							// parse the command response and show that to the user

	if(!config_param_info(CONFIG_PARAM_OP_print, &par_id, &par_data_out)) return TRUE;	// show the param-id and corresponding value that we received back

	// slightly abuse strncmp to perform the comparison for us ... abuse because without the terminating NUL it could be considered not to be a string.
	if(strncmp((char *) par_data, (char *) par_data_out, sizeof(par_data)) != 0)		// is the returned parameter value the same?
	{
		unsigned int i;

		printf("Warning: the resulting parameter value differs from the one send in! \r\n");
		printf("this is generally caused because the supplied variable was not in the range \r\n");
		printf("of valid values and has been set to closest valid value by the IEPS. \r\n");
		printf("I2C bus noise might be another possible culprit. \r\n");
		printf("Consult the manual and test the I2C bus to determine the cause. \r\n");

		printf("send     = %#04x",  par_data[0]);
		for(i = 1; i < sizeof(par_data); i++)
		{
			printf(", %#04x",  par_data[i]);
		}
		printf("\r\n");
		printf("received = %#04x",  par_data_out[0]);
		for(i = 1; i < sizeof(par_data_out); i++)
		{
			printf(", %#04x",  par_data_out[i]);
		}
		printf("\r\n");
	}

	return TRUE;
}


static Boolean _resetParameter(Boolean info)
{
	unsigned short par_id;							// storage for the param-id
	unsigned char par_data[8];						// byte array for storing our parameter data
	ieps_statcmd_t rsp_stat;						// storage for the command response
	unsigned int rv;

	if(info)
	{
		printf("\r\n Information resetParameter \r\n");
		printf(" Sends the resetParameter command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_RESET_PARAMETER);
		printf(" Used to reset configuration parameter values back to its hard coded default within the IEPS. \r\n");
		printf(" note: this is different from the value stored in non-volatile memory; if required use loadConfig instead. \r\n");
		printf(" Execution is performed and completed immediately. \r\n");
		printf(" The following information is returned: \r\n");
		printf(" param-id = %s \r\n", "parameter-id of the parameter under consideration.");
		printf(" param-value = %s \r\n", "value of the parameter under consideration. Between 1 and 8 bytes.");

		return TRUE;
	}

	printf("\r\n Perform resetParameter \r\n");

	if(!config_param_info(CONFIG_PARAM_OP_ask_parid, &par_id, NULL)) return TRUE;	// get the param-id from the user

	rv = IsisEPS_resetParameter(0, par_id, &par_data, &rsp_stat);	// command the IEPS to reset the parameter, and receive the value that it was rest to
	if(rv)
	{
		TRACE_ERROR("return value=%d \r\n", rv)		// non-zero return value means error!
		return TRUE;								// indicates we should not exit to the higher demo menu
	}

	printf(" response: \r\n");
	_parse_resp(&rsp_stat);							// parse the command response and show that to the user

	config_param_info(CONFIG_PARAM_OP_print, &par_id, &par_data);	// show the param-id and corresponding value that we received back

	return TRUE;
}


static Boolean _resetConfig(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information resetConfig *** \r\n");
		printf(" Sends the resetConfig command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_RESET_CONF);
		printf(" Used to reset all configuration parameter values back to their hard coded defaults within the IEPS. \r\n");
		printf(" note: this is different from the value stored in non-volatile memory; if required use loadConfig instead. \r\n");
		printf(" Execution is performed and completed immediately. \r\n");

		return TRUE;
	}


	printf("\r\n Perform resetConfig \r\n");

	rv = IsisEPS_resetConfig(0, &rsp_stat);				// reset the IEPS configuration to hardcoded defaults
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	return TRUE;
}

static Boolean _loadConfig(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information loadConfig *** \r\n");
		printf(" Sends the loadConfig command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_LOAD_CONF);
		printf(" Used to load all configuration parameters values from non-volatile memory within the IEPS. \r\n");
		printf(" Execution is not started immediately but only once every 500 ms. It takes about 50 ms to complete. \r\n");

		return TRUE;
	}

	printf("\r\n Perform loadConfig \r\n");

	rv = IsisEPS_loadConfig(0, &rsp_stat);				// (re-)load the IEPS configuration from non-volatile memory
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)		// non-zero return value means error!
		return TRUE;									// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	return TRUE;
}

static Boolean _saveConfig(Boolean info)
{
	ieps_statcmd_t rsp_stat;
	unsigned short checksum;		// checksum to send over to the ISIS-EPS. Must match the checksum taken over the current state of the read/write parameters; Use 0 to force save the current config regardless of checksum
	unsigned int rv;

	if(info)
	{
		printf("\r\n *** Information saveConfig *** \r\n");
		printf(" Sends the saveConfig command %#04x to the IEPS \r\n", ISIS_EPS_I2CCMD_SAVE_CONF);
		printf(" Used to save all configuration parameters values to non-volatile memory within the IEPS. \r\n");
		printf(" Requires a checksum to be supplied that was gotten from a CRC16 operation over \r\n");
		printf(" all the read/write parameters that are going to be saved. \r\n");
		printf(" The checksum ensures no intermediate memory corruption occurred between updating the parameters \r\n");
		printf(" and executing this save command. \r\n");
		printf(" The CRC check can be skipped using a force-save by supplying 0 as checksum value. \r\n");
		printf(" During this operation no I2C interaction with the EPS should be attempted. \r\n");
		printf(" Execution is not started immediately but only once every 500 ms. It takes about 50 ms to complete. \r\n");

		return TRUE;
	}

	printf("\r\n Perform saveConfig \r\n");

	{
		double value;

		printf(" Provide checksum of the currently active configuration \r\n");
		printf(" or enter 0 to force-save without CRC check. \r\n");

		while(1)
		{
			rv = ing(" Provide checksum [0]: ", &value, 0, 65535, 0);								// request info from the user
			if(rv == INGRV_esc) {printf("<ESC> \r\n"); return TRUE;}								// esc? exit function
			else if(rv == INGRV_val) {printf("\r\n"); break;}										// valid? continue
		}

		checksum = (unsigned short) value;
	}

	rv = IsisEPS_saveConfig(0, checksum, &rsp_stat);
	if(rv)
	{
		TRACE_ERROR(" return value=%d \r\n", rv)	// non-zero return value means error!
		return TRUE;								// indicates we should not exit to the higher demo menu
	}

	_parse_resp(&rsp_stat);

	return TRUE;
}

///***************************************************************************************************************************
///
/// end IsisEPS command demo functions
///
///***************************************************************************************************************************

static Boolean _selectAndExecuteDemoTest(void)
{
	double value;
	unsigned int selection = 0;
	Boolean offerMoreTests = TRUE;
	static Boolean toggle_is_info = FALSE;

	if(toggle_is_info)
	{
		printf( "\n\r ******************* Information Mode ******************* \n\r");
		printf( "\n\r While in this mode you can select commands about which \n\r");
		printf( " you would like to get information without issuing \n\r");
		printf( " the actual command. To exit information mode select \n\r");
		printf( " option 1. \n\r");
		printf( "\n\r Choose which command to show information for: \n\r");
	}
	else
	{
		printf( "\n\r Select a test to perform: \n\r");
	}

	printf("\t 0) Return to main menu \n\r");

	if(toggle_is_info)	printf("\t 1) Switch back to test mode \n\r");
	else 				printf("\t 1) Show command information \n\r");

	printf(" \t 2) hardReset \n\r");
	printf(" \t 3) noOperation \n\r");
	printf(" \t 4) cancelOperation \n\r");
	printf(" \t 5) resetWDT \n\r");
	printf(" \t 6) outputBusGroupOn \n\r");
	printf(" \t 7) outputBusGroupOff \n\r");
	printf(" \t 8) outputBusGroupState \n\r");
	printf(" \t 9) outputBus3v3On \n\r");
	printf(" \t 10) outputBus3v3Off \n\r");
	printf(" \t 11) outputBus5vOn \n\r");
	printf(" \t 12) outputBus5vOff \n\r");
	printf(" \t 13) outputBusHvOn \n\r");
	printf(" \t 14) outputBusHvOff \n\r");
	printf(" \t 15) getSystemState \n\r");
	printf(" \t 16) getRawHKDataMB \n\r");
	printf(" \t 17) getEngHKDataMB \n\r");
	printf(" \t 18) getRAEngHKDataMB \n\r");
	printf(" \t 19) getRawHKDataCDB \n\r");
	printf(" \t 20) getEngHKDataCDB \n\r");
	printf(" \t 21) getRAEngHKDataCDB \n\r");
	printf(" \t 22) getLOState3V3 \n\r");
	printf(" \t 23) getLOState5V \n\r");
	printf(" \t 24) getLOStateBV \n\r");
	printf(" \t 25) getLOStateHV \n\r");
	printf(" \t 26) getParameter \n\r");
	printf(" \t 27) setParameter \n\r");
	printf(" \t 28) resetParameter \n\r");
	printf(" \t 29) resetConfig \n\r");
	printf(" \t 30) loadConfig \n\r");
	printf(" \t 31) saveConfig \n\r");

	while(INGRV_val != ing("\r\n enter selection: ", &value, 0, 31, -1))
	{
		printf("\r\n invalid selection! Try again. \r\n");
	}

	printf("\r\n");

	selection = value;

	switch(selection)
	{
		case 0: offerMoreTests = FALSE; break;
		case 1: toggle_is_info = !toggle_is_info; offerMoreTests = TRUE; break;
		case 2: offerMoreTests = _hardReset(toggle_is_info); break;
		case 3: offerMoreTests = _noOperation(toggle_is_info); break;
		case 4: offerMoreTests = _cancelOperation(toggle_is_info); break;
		case 5: offerMoreTests = _resetWDT(toggle_is_info); break;
		case 6: offerMoreTests = _outputBusGroupOn(toggle_is_info); break;
		case 7: offerMoreTests = _outputBusGroupOff(toggle_is_info); break;
		case 8: offerMoreTests = _outputBusGroupState(toggle_is_info); break;
		case 9: offerMoreTests = _outputBus3v3On(toggle_is_info); break;
		case 10: offerMoreTests = _outputBus3v3Off(toggle_is_info); break;
		case 11: offerMoreTests = _outputBus5vOn(toggle_is_info); break;
		case 12: offerMoreTests = _outputBus5vOff(toggle_is_info); break;
		case 13: offerMoreTests = _outputBusHvOn(toggle_is_info); break;
		case 14: offerMoreTests = _outputBusHvOff(toggle_is_info); break;
		case 15: offerMoreTests = _getSystemState(toggle_is_info); break;
		case 16: offerMoreTests = _getRawHKDataMB(toggle_is_info); break;
		case 17: offerMoreTests = _getEngHKDataMB(toggle_is_info); break;
		case 18: offerMoreTests = _getRAEngHKDataMB(toggle_is_info); break;
		case 19: offerMoreTests = _getRawHKDataCDB(toggle_is_info); break;
		case 20: offerMoreTests = _getEngHKDataCDB(toggle_is_info); break;
		case 21: offerMoreTests = _getRAEngHKDataCDB(toggle_is_info); break;
		case 22: offerMoreTests = _getLOState3V3(toggle_is_info); break;
		case 23: offerMoreTests = _getLOState5V(toggle_is_info); break;
		case 24: offerMoreTests = _getLOStateBV(toggle_is_info); break;
		case 25: offerMoreTests = _getLOStateHV(toggle_is_info); break;
		case 26: offerMoreTests = _getParameter(toggle_is_info); break;
		case 27: offerMoreTests = _setParameter(toggle_is_info); break;
		case 28: offerMoreTests = _resetParameter(toggle_is_info); break;
		case 29: offerMoreTests = _resetConfig(toggle_is_info); break;
		case 30: offerMoreTests = _loadConfig(toggle_is_info); break;
		case 31: offerMoreTests = _saveConfig(toggle_is_info); break;
		default: TRACE_ERROR("Invalid selection"); break;
	}

	return offerMoreTests;
}

/***
 * Initializes the I2C interface driver and the IEPS subsystem driver
 * The IEPS subsystem driver is a layer that sits on top of the I2C interface driver
 * requiring the I2C interface driver to be initialized once before using any of the
 * subsystem library drivers
 */
Boolean IsisEPSdemoInit(void)
{
    unsigned char i2c_address = 0x11;
    int rv;

	rv = IsisEPS_initialize(&i2c_address, 1);
	if(rv != E_NO_SS_ERR && rv != E_IS_INITIALIZED)
	{
		// we have a problem. Indicate the error. But we'll gracefully exit to the higher menu instead of
		// hanging the code
		TRACE_ERROR("\n\r IsisEPS_initialize() failed; err=%d! Exiting ... \n\r", rv);
		return FALSE;
	}

	return TRUE;
}

void IsisEPSdemoLoop(void)
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = _selectAndExecuteDemoTest();		// show the demo command line interface and handle commands

		if(offerMoreTests == FALSE)							// was exit/back selected?
		{
			break;
		}
	}
}

Boolean IsisEPSdemoMain(void)
{
	if(IsisEPSdemoInit())									// initialize of I2C and IEPS subsystem drivers succeeded?
	{
		IsisEPSdemoLoop();									// show the main IEPS demo interface and wait for user input
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

Boolean IsisEPStest(void)
{
	IsisEPSdemoMain();
	return TRUE;
}
