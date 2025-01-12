
/*
 * @file	EPS.h
 * @brief	EPS- Energy Powering System.This system is incharge of the energy consumtion
 * 			the satellite and switching on and off power switches(5V, 3V3)
 * @see		inspect logic flowcharts thoroughly in order to write the code in a clean manner.
 */

#include "GlobalStandards.h"
#include "EPSOperationModes.h"

#include <satellite-subsystems/isismepsv2_ivid5_piu.h>

/*
 	 	 	 	  ______
			  ___|		|___
 	 	 	 |				|
 	 	 	 |	 FULL MODE	|
 	 	 	 |- - - - - - -	|	-> FULL UP = 7400
 	 	 	 |- - - - - - - |	-> FULL DOWN = 7300
 	 	 	 |				|
 	 	 	 |	CRUISE MODE	|
 	 	 	 |- - - - - - -	|	-> CRUISE UP = 7200
 	 	 	 |- - - - - - - |	-> CRUISE DOWN = 7100
 	 	 	 |				|
 	 	 	 |	 SAFE MODE	|
 	 	 	 |- - - - - - -	| 	-> SAFE UP = 6600
 	 	 	 |- - - - - - - |	-> SAFE DOWN = 6500
 	 	 	 |				|
 	 	 	 |	 CRITICAL	|
 	 	 	 |______________|
 */
#define DEFAULT_ALPHA_VALUE 0.95
#define DEFAULT_HEATERS_ACTIVE_MODE 1

#define MIN_EPS_STATE_LINGER_TIME  10
#define MIN_EPS_NEXT_STATE_LINGER_TIME  10

#define NUMBER_OF_THRESHOLD_VOLTAGES 	6 		///< first 3 are charging voltages, last 3 are discharging voltages
#define DEFAULT_EPS_THRESHOLD_VOLTAGES 	{(voltage_t)6300, (voltage_t)6700, (voltage_t)7100,	 \
										  (voltage_t)6350, (voltage_t)6750, (voltage_t)7150}


unsigned short gl_eps_state_linger;
unsigned short gl_eps_next_state_linger;

typedef enum __attribute__ ((__packed__)){
	INDEX_DOWN_SAFE,
	INDEX_DOWN_CRUISE,
	INDEX_DOWN_FULL,
	INDEX_UP_SAFE,
	INDEX_UP_CRUISE,
	INDEX_UP_FULL
} EpsThresholdsIndex;

typedef enum __attribute__ ((__packed__)){
	STID = 26,
	IVID = 5,
	BID = 1
} EPS_I2C_HEADER;

typedef enum __attribute__ ((__packed__)){
	NOP_W = 0x02,
	NOP_R = 0x03,
	GET_SYSTEM_STATUS_W = 0x40,
	GET_SYSTEM_STATUS_R = 0x41,
	GET_PIU_HK_ENG_W = 0xA2,
	GET_PIU_HK_ENG_R = 0xA3,
	GET_CONF_W = 0x82,
	GET_CONF_R = 0x83,
	SET_CONF_W = 0x84,
	SET_CONF_R = 0x85,
	CONF_R = 0x85
} EPS_I2C_CMD;

typedef enum __attribute__ ((__packed__)){
	NOP_SIZE_W = 4,
	NOP_SIZE_R = 5,
	GET_SYSTEM_STATUS_SIZE_W = 4,
	GET_SYSTEM_STATUS_SIZE_R = 36,
	GET_CONF_SIZE_W = 6,
	GET_CONF_SIZE_R_8 = 9,
	GET_CONF_SIZE_R_16 = 10,
	GET_PIU_HK_ENG_SIZE_W = 4,
	GET_PIU_HK_ENG_SIZE_R = 116,
	SET_CONF_SIZE_W_8 = 7,
	SET_CONF_SIZE_W_16 = 8
} EPS_I2C_CMD_SIZE;

typedef enum __attribute__ ((__packed__)){
	SAFETY_VOLT_LOTHR = 0x4042,//uint16 6200
	SAFETY_VOLT_HITHR = 0x4043,//uint16 7000
	TTC_I2C_SLAVE_ADDR = 0x4800,//uint16
	EMLOPO_VOLT_LOTHR =  0x480A,//uint16 6000
	EMLOPO_VOLT_HITHR =  0x480B,//uint16 6200
	EMLOPO_PERIOD = 0x480C,//uint16 600s
	SAFETY_VOLT_LOTHR_USED = 0x480D,//uint16 6200
	SAFETY_VOLT_HITHR_USED = 0x480E,//uint16 7000
	SAFETY_LINGER =0x480F,//uint16 60s
	LOTHR_BP1_HEATER = 0x3000,//int16 200
	HITHR_BP1_HEATER = 0x3003,//int16 600
	AUTO_HEAT_ENA_BP1 = 0x1001//int8 0
} EPS_I2C_PRM;

typedef enum __attribute__ ((__packed__)){
	NEW_RESP = 0x80
} EPS_I2C_RESP;

typedef union __attribute__ ((__packed__)){
	voltage_t raw[NUMBER_OF_THRESHOLD_VOLTAGES];
	struct {
		voltage_t Vdown_safe;
		voltage_t Vdown_cruise;
		voltage_t Vdown_full;
		voltage_t Vup_safe;
		voltage_t Vup_cruise;
		voltage_t Vup_full;
	}fields;
}EpsThreshVolt_t;

/*!
 * @brief initializes the EPS subsystem.
 * @return	0 on success
 * 			-1 on EPS init error
 * 			-2 on Solar Panel init error
 * 			-3 on EPS threshold FRAM read error
 * 			-4 on EPS alpha FRAM read error
 * @note if FRAM read error than use default values of 'alpha' and 'eps_threshold_voltages'
 */
int EPS_Init();

int eps_set_channels_on(isismepsv2_ivid5_piu__eps_channel_t channel);

/*!
 * @brief EPS logic. controls the state machine of which subsystem
 * is on or off, as a function of only the battery voltage
 * @return	0 on success
 *  		Error code according to <hal/errors.h>
 */
int EPS_Conditioning();

/*!
 * @brief returns the current voltage on the battery
 * @param[out] vbat he current battery voltage
 * @return	0 on success
 * 			Error code according to <hal/errors.h>
 */
int GetBatteryVoltage(voltage_t *vbat);

/*!
 * @brief setting the new EPS logic threshold voltages on the FRAM.
 * @param[in] thresh_volts an array holding the new threshold values
 * @return	0 on success
 * 			-1 on failure setting new threshold voltages
 * 			-2 on invalid thresholds
 * 			ERR according to <hal/errors.h>
 */
int UpdateThresholdVoltages(EpsThreshVolt_t *thresh_volts);

/*!
 * @brief getting the EPS logic threshold  voltages on the FRAM.
 * @param[out] thresh_volts a buffer to hold the threshold values
 * @return	0 on success
 * 			-1 on NULL input array
 * 			-2 on FRAM read errors
 */
int GetThresholdVoltages(EpsThreshVolt_t *thresh_volts);

/*!
 * @brief getting the smoothing factor (alpha) from the FRAM.
 * @param[out] alpha a buffer to hold the smoothing factor
 * @return	0 on success
 * 			-1 on NULL input array
 * 			-2 on FRAM read errors
 */
int GetAlpha(float *alpha);

/*!
 * @brief setting the new voltage smoothing factor (alpha) on the FRAM.
 * @param[in] new_alpha new value for the smoothing factor alpha
 * @note new_alpha is a value in the range - (0,1)
 * @return	0 on success
 * 			-1 on failure setting new smoothing factor
 * 			-2 on invalid alpha
 * @see LPF- Low Pass Filter at wikipedia: https://en.wikipedia.org/wiki/Low-pass_filter#Discrete-time_realization
 */
int UpdateAlpha(float new_alpha);

/*!
 * @brief setting the new voltage smoothing factor (alpha) to be the default value.
 * @return	0 on success
 * 			-1 on failure setting new smoothing factor
 * @see DEFAULT_ALPHA_VALUE
 */
int RestoreDefaultAlpha();

/*!
 * @brief	setting the new EPS logic threshold voltages on the FRAM to the default.
 * @return	0 on success
 * 			-1 on failure setting smoothing factor
  * @see EPS_DEFAULT_THRESHOLD_VOLTAGES
 */
int RestoreDefaultThresholdVoltages();
