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
#define SAT_CMD_ID_ADDR					0x00		//0,1,2,3
#define SAT_CMD_ID_SIZE					4

#define LAUNCH_TIME_ADDR				0X04		//4,5,6,7<! time at which the satellites starts finished 30 min after lauanch
#define LAUNCH_TIME_SIZE				4			//<! size of parameter in bytes

#define SECONDS_SINCE_DEPLOY_ADDR		0x08		//8,9,A,B <! counts how many seconds has past since wakup for use in deployment.
#define SECONDS_SINCE_DEPLOY_SIZE		4			//<! size of the parameter in bytes
#define LAST_EPS_TLM_SAVE_TIME_ADDR		0x0C		//C,D,E,F <! time of last EPS TLM save inot files
#define NO_COMM_WDT_KICK_TIME_ADDR		0x10		//10,11,12,13/< number of seconds of no communications before GS WDT kick
#define NO_COMM_WDT_KICK_TIME_SIZE		4

#define TLM_SAVE_PERIOD_START_ADDR		0x14		//14,15,16,17 //<! start of the save periods in the FRAM
#define EPS_SAVE_TLM_PERIOD_ADDR		0x14		//<! address where the save tlm period will be

#define TRXVU_SAVE_TLM_PERIOD_ADDR		0x18		//18,19,1A,1B //<! address where the save tlm period will be
#define ANT_SAVE_TLM_PERIOD_ADDR		0x1C		//1C,1D,1E,1F //<! address where the save tlm period will be

#define SOLAR_SAVE_TLM_PERIOD_ADDR		0x20		//20,21,22,23 <! address where the save tlm period will be
#define WOD_SAVE_TLM_PERIOD_ADDR  	    0x24		//24,25,26,27
#define PIC32_SAVE_TLM_PERIOD_ADDR		0x28		//28,29,2A,2B
#define RADFET_SAVE_TLM_PERIOD_ADDR 	0x2C		//2C,2D,2E,2F
#define TURN_ON_PAYLOAD_IN_INIT			0X30 		// (30,31,32,33) the flag tells us whether or not to turn on the payload during the init process
#define TURN_ON_PAYLOAD_IN_INIT_SIZE	4			// size of the parameter in bytes
#define PAYLOAD_TURN_OFF_BY_COMMAND		0x34		//(34,35) counter of all restarts that are done due to a command
#define PAYLOAD_TURN_OFF_BY_COMMAND_SIZE 2 			// size of parameter in bytes
#define FIRST_ACTIV_NUM_PAYLOAD_RESET	0x36		//(36,37)
#define FIRST_ACTIV_NUM_PAYLOAD_RESET_SIZE 2 		// size of parameter in bytes
#define PAYLOAD_ON						0x38		//flag that indicates wheather paylod is on (38,39,3A,3B)
#define PAYLOAD_ON_SIZE					4
#define aadd_ADDR						0x3C		//3C,3D,3E,3F

#define LAST_WAKEUP_TIME_ADDR			0X40		//40,41,42,43 <! saves the first time after satellites wakeup from reset
#define LAST_WAKEUP_TIME_SIZE			4			//<! size of the parameter in bytes

#define FIRST_ACTIVATION_FLAG_ADDR		0x4F		//<! is this the first activation after launch flag
#define FIRST_ACTIVATION_FLAG_SIZE		4			//<! length in bytes of FIRST_ACTIVATION_FLAG

#define MOST_UPDATED_SAT_TIME_ADDR		0x60		//<! this parameters saves the sat time to be read after resets
#define MOST_UPDATED_SAT_TIME_SIZE		4			//<! size of the parameter in bytes

#define NUMBER_OF_RESETS_ADDR			0x64		//<! counts how many restarts did the satellite endure
#define NUMBER_OF_RESETS_SIZE			2			//<! size of the parameter in bytes

#define NUMBER_OF_CMD_RESETS_ADDR		0x66
#define NUMBER_OF_CMD_RESETS_SIZE		2

#define LAST_ANT_DEP_TIME_ADDR			0X68		//<! saves the first time after satellites wakeup from reset
#define LAST_ANT_DEP_TIME_SIZE			4			//<! size of the parameter in bytes

#define RESET_CMD_FLAG_ADDR				0x105		//<! the flag is raised whenever a restart is commissioned
#define RESET_CMD_FLAG_SIZE				1			//<! size of the parameter in bytes

#define TRANS_ABORT_FLAG_ADDR			0x500		//<! transmission abort request flag
#define TRANS_ABORT_FLAG_SIZE			1			//<! size of mute flag in bytes

#define TRANSPONDER_STATE_ADDR				0x502		// transponder state - active or not
#define TRANSPONDER_STATE_SIZE				4			// size of transponder state

#define TRANSPONDER_TURN_ON_END_TIME_ADRR	0x508		//transponder turn off time
#define TRANSPONDER_TURN_ON_END_TIME_SIZE	4

#define MUTE_FLAG_ADRR						0x514		// mute state - active or not
#define MUTE_FLAG_SIZE						4			// size of mute state

#define MUTE_ON_END_TIME_ADRR				0x520		//mute turn off time
#define MUTE_ON_END_TIME_SIZE				4

#define EPS_BAT_HITERRS_ACTIVE_MODE_ADDR     0x540
#define EPS_BAT_HITERRS_ACTIVE_MODE_SIZE     sizeof(unsigned char)

#define EPS_BAT_HITERRS_LOW_TRH_MODE_ADDR     0x542
#define EPS_BAT_HITERRS_LOW_TRH_MODE_SIZE     sizeof(unsigned short)

#define EPS_BAT_HITERRS_HIGH_TRH_MODE_ADDR     0x544
#define EPS_BAT_HITERRS_HIGH_TRH_MODE_SIZE     sizeof(unsigned short)

#define EPS_ALPHA_FILTER_VALUE_ADDR     0x550			//<! filtering value in the LPF formula
#define EPS_ALPHA_FILTER_VALUE_SIZE     sizeof(float)	//<! size of double (alpha) 8 byte

#define EPS_THRESH_VOLTAGES_ADDR		0x666		//<! starting address for eps threshold voltages array
#define EPS_THRESH_VOLTAGES_SIZE (NUMBER_OF_THRESHOLD_VOLTAGES * sizeof(voltage_t)) //<! number of bytes in eps threshold voltages array

#define TRANSPONDER_RSSI_ADDR				0x700		//transponder RSSI
#define TRANSPONDER_RSSI_SIZE				2

#define TX_BITRATE_ADDR						0x702		//tx bitrate
#define TX_BITRATE_ADDR_SIZE				1


#define BEACON_INTERVAL_TIME_ADDR 		0x4590		//<! address of value of the delay between 2 beacons
#define BEACON_INTERVAL_TIME_SIZE 		4			//<! size of parameter in bytes

#define LAST_COMM_TIME_ADDR 			0X9485		//<! saves the last unix time at which communication has occured
#define LAST_COMM_TIME_SIZE				4			//<! size of last communication time in bytes

#define FSFRAM 							0x10000
#define FSFRAM_SIZE						2 * sizeof(int)

#define DEFAULT_EPS_SAVE_TLM_TIME		5			//<! save EPS TLM every 20 seconds
#define DEFAULT_TRXVU_SAVE_TLM_TIME		5			//<! save TRXVU TLM every 20 seconds
#define DEFAULT_ANT_SAVE_TLM_TIME		5			//<! save antenna TLM every 20 seconds
#define DEFAULT_SOLAR_SAVE_TLM_TIME		5			//<! save solar panel TLM every 20 seconds
#define DEFAULT_WOD_SAVE_TLM_TIME		5			//<! save WOD TLM every 20 seconds

#define DEFAULT_PIC32_SAVE_TLM_TIME		20
#define DEFAULT_RADFET_SAVE_TLM_TIME	60 * 15

#define DEFAULT_LOG_SAVE_TLM_TIME		5			//
#define DEFAULT_NO_COMM_WDT_KICK_TIME  (7*24*60*60)	//<! number of seconds in 7 days
#define DEFALUT_BEACON_BITRATE_CYCLE	3			//<! default value
#define DEFAULT_BEACON_INTERVAL_TIME 	20			//<! how many seconds between two beacons [sec]
#define MAX_BEACON_INTERVAL				60			// beacon every 1 minute
#define MIN_BEACON_INTERVAL				5			// beacon every 10 seconds
#define TRANSPONDER_MAX_DURATION 		(72*60*60)  // max transponder duration is 72 hours
#define ANT_DEPLOY_WAIT_PERIOD			(45*60)		// 45 minutes TODO:set to 45*60
#define DEFAULT_BITRATE_VALUE			0x8
#endif /* FRAM_FLIGHTPARAMETERS_H_ */
