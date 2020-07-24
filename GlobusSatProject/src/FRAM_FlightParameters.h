/*
 *
 * @file	FRAM_FlightParameters.h
 * @brief	ordering all flight parameters(saved on the FRAM)  addresses and sizes
 * @note 	sizes are measured in chars = 1 byte. e.g size = 4, meaning 4 bytes(=int)
 */

#ifndef FRAM_FLIGHTPARAMETERS_H_
#define FRAM_FLIGHTPARAMETERS_H_

#include <hal/Storage/FRAM.h>

// <Satellite Management>

#define DEPLOYMENT_TIME_ADDR			0X05		//<! time at which the satellites starts deployment
#define DEPLOYMENT_TIME_SIZE			4			//<! size of parameter in bytes

#define SECONDS_SINCE_DEPLOY_ADDR		0x09		//<! counts how many seconds has past since wakup for use in deployment.
#define SECONDS_SINCE_DEPLOY_SIZE		4			//<! size of the parameter in bytes

#define LAST_EPS_TLM_SAVE_TIME_ADDR		0x20		//<! time of last EPS TLM save inot files
#define NO_COMM_WDT_KICK_TIME_ADDR  	0x24		///< number of seconds of no communications before GS WDT kick
#define NO_COMM_WDT_KICK_TIME_SIZE		4
#define TLM_SAVE_PERIOD_START_ADDR		0x28		//<! start of the save periods in the FRAM
#define EPS_SAVE_TLM_PERIOD_ADDR		0x28		//<! address where the save tlm period will be
#define TRXVU_SAVE_TLM_PERIOD_ADDR		0x2c		//<! address where the save tlm period will be
#define ANT_SAVE_TLM_PERIOD_ADDR		0x30		//<! address where the save tlm period will be
#define SOLAR_SAVE_TLM_PERIOD_ADDR		0x34		//<! address where the save tlm period will be
#define WOD_SAVE_TLM_PERIOD_ADDR		0x38		//<! address where the save tlm period will be
#define LOG_SAVE_TLM_PERIOD_ADDR		0x3B
#define LAST_WAKEUP_TIME_ADDR			0X3F		//<! saves the first time after satellites wakeup from reset
#define LAST_WAKEUP_TIME_SIZE			4			//<! size of the parameter in bytes

#define FIRST_ACTIVATION_FLAG_ADDR		0x5F		//<! is this the first activation after launch flag
#define FIRST_ACTIVATION_FLAG_SIZE		4			//<! length in bytes of FIRST_ACTIVATION_FLAG

#define ANT_OPEN_FLAG_ADDR				0X4587		//<! is the ants open flag
#define ANT_OPEN_FLAG_SIZE				4			//<! length of ants open flag
#define LAST_ANT_DEP_TIME_ADDR			0X66		//<! saves the first time after satellites wakeup from reset
#define LAST_ANT_DEP_TIME_SIZE			4			//<! size of the parameter in bytes

#define MOST_UPDATED_SAT_TIME_ADDR		0x60		//<! this parameters saves the sat time to be read after resets
#define MOST_UPDATED_SAT_TIME_SIZE		4			//<! size of the parameter in bytes

#define NUMBER_OF_RESETS_ADDR			0x64		//<! counts how many restarts did the satellite endure
#define NUMBER_OF_RESETS_SIZE			2			//<! size of the parameter in bytes

#define RESET_CMD_FLAG_ADDR				0x105		//<! the flag is raised whenever a restart is commissioned
#define RESET_CMD_FLAG_SIZE				1			//<! size of the parameter in bytes

#define TRANS_ABORT_FLAG_ADDR			0x500		//<! transmission abort request flag
#define TRANS_ABORT_FLAG_SIZE			1			//<! size of mute flag in bytes

#define TRANSPONDER_STATE_ADDR			0x502		// transponder state - active or not
#define TRANSPONDER_STATE_SIZE			4			// size of transponder state

#define TRANSPONDER_TURN_ON_END_TIME_ADRR	0x508		//transponder turn off time
#define TRANSPONDER_TURN_ON_END_TIME_SIZE	4

#define EPS_ALPHA_FILTER_VALUE_ADDR     0x550			//<! filtering value in the LPF formula
#define EPS_ALPHA_FILTER_VALUE_SIZE     sizeof(float)	//<! size of double (alpha)

#define EPS_THRESH_VOLTAGES_ADDR		0x666		//<! starting address for eps threshold voltages array
#define EPS_THRESH_VOLTAGES_SIZE (NUMBER_OF_THRESHOLD_VOLTAGES * sizeof(voltage_t)) //<! number of bytes in eps threshold voltages array

#define BEACON_INTERVAL_TIME_ADDR 		0x4590		//<! address of value of the delay between 2 beacons
#define BEACON_INTERVAL_TIME_SIZE 		4			//<! size of parameter in bytes

#define LAST_COMM_TIME_ADDR 			0X9485		//<! saves the last unix time at which communication has occured
#define LAST_COMM_TIME_SIZE				4			//<! size of last communication time in bytes

#define DEFAULT_EPS_SAVE_TLM_TIME		5			//<! save EPS TLM every 20 seconds
#define DEFAULT_TRXVU_SAVE_TLM_TIME		5		//<! save TRXVU TLM every 20 seconds
#define DEFAULT_ANT_SAVE_TLM_TIME		5			//<! save antenna TLM every 20 seconds
#define DEFAULT_SOLAR_SAVE_TLM_TIME		5			//<! save solar panel TLM every 20 seconds
#define DEFAULT_WOD_SAVE_TLM_TIME		5			//<! save WOD TLM every 20 seconds
#define DEFAULT_LOG_SAVE_TLM_TIME		5			//
#define DEFAULT_NO_COMM_WDT_KICK_TIME  (15*24*60*60)	//<! number of seconds in 15 days
#define DEFALUT_BEACON_BITRATE_CYCLE	3			//<! default value
#define DEFAULT_BEACON_INTERVAL_TIME 	20			//<! how many seconds between two beacons [sec]
#define MAX_BEACON_INTERVAL				60			// beacon every 1 minute
#define MIN_BEACON_INTERVAL				5			// beacon every 10 seconds


#endif /* FRAM_FLIGHTPARAMETERS_H_ */
