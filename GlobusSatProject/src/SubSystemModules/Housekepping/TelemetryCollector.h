#ifndef TELEMETRYCOLLECTOR_H_
#define TELEMETRYCOLLECTOR_H_

#include "GlobalStandards.h"
#include "TelemetryFiles.h"
#include "TLM_management.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Maintenance/Log.h"
#include "SubSystemModules/Payload/payload_drivers.h"

#include <satellite-subsystems/isismepsv2_ivid5_piu_types.h>
#include <satellite-subsystems/isis_vu_e_types.h>
#include <satellite-subsystems/isis_ants_types.h>



#define NUMBER_OF_SOLAR_PANELS 6


typedef union __attribute__((__packed__)) _eps_eng__from_t
{
    unsigned char raw[84];
    struct __attribute__ ((__packed__))
    {
        int16_t volt_brdsup; /*!< Voltage of internal board supply in raw form */
        int16_t temp; /*!< Measured temperature provided by a sensor internal to the MCU in raw form \note conversion: eng. value [in °C] = 0.01 * raw */
        isismepsv2_ivid5_piu__vipdeng_t dist_input; /*!< Input V, I and P data taken at the input of the distribution part of the unit in raw form. Negative values indicate output flow. */
        isismepsv2_ivid5_piu__vipdeng_t batt_input; /*!< Input V, I and P data taken at the input of the battery part of the unit in raw form. Negative values indicate output flow. */
        uint16_t stat_obc_on; /*!< Bitflag field indicating channel-on status for the output bus channels. */
        uint16_t stat_obc_ocf; /*!< Bitflag field indicating overcurrent latch-off fault status for the output bus channels. */
        isismepsv2_ivid5_piu__batterypackstatus_t bat_stat; /*!< Bitflag field indicating BP board */
        int16_t temp2; /*!< 2 and 4 cell battery pack: Battery pack temperature in between the center battery cells. \note conversion: eng. value [in °C] = 0.01 * raw */
        int16_t volt_vd0; /*!< Voltage of voltage domain 0 */
        int16_t volt_vd1; /*!< Voltage of voltage domain 0 */
        int16_t volt_vd2; /*!< Voltage of voltage domain 0 */
        isismepsv2_ivid5_piu__vipdeng_t vip_obc00; /*!< Output V, I and P of output bus channel */
        isismepsv2_ivid5_piu__vipdeng_t vip_obc01; /*!< Output V, I and P of output bus channel */
        isismepsv2_ivid5_piu__vipdeng_t vip_obc03; /*!< Output V, I and P of output bus channel */
        isismepsv2_ivid5_piu__vipdeng_t vip_obc04; /*!< Output V, I and P of output bus channel */
        isismepsv2_ivid5_piu__vipdeng_t vip_obc05; /*!< Output V, I and P of output bus channel */
        isismepsv2_ivid5_piu__ccsdeng_t cc1; /*!< Data on conditioning chain */
        isismepsv2_ivid5_piu__ccsdeng_t cc2; /*!< Data on conditioning chain */
        isismepsv2_ivid5_piu__ccsdeng_t cc3; /*!< Data on conditioning chain */
    } fields;
} hk_eps_eng;

typedef union __attribute__((__packed__)) _eps_avg__from_t
{
    unsigned char raw[84];
    struct __attribute__ ((__packed__))
    {
        int16_t volt_brdsup; /*!< Voltage of internal board supply in raw form */
        int16_t temp; /*!< Measured temperature provided by a sensor internal to the MCU in raw form \note conversion: eng. value [in °C] = 0.01 * raw */
        isismepsv2_ivid5_piu__vipdeng_t dist_input; /*!< Input V, I and P data taken at the input of the distribution part of the unit in raw form. Negative values indicate output flow. */
        isismepsv2_ivid5_piu__vipdeng_t batt_input; /*!< Input V, I and P data taken at the input of the battery part of the unit in raw form. Negative values indicate output flow. */
        uint16_t stat_obc_on; /*!< Bitflag field indicating channel-on status for the output bus channels. */
        uint16_t stat_obc_ocf; /*!< Bitflag field indicating overcurrent latch-off fault status for the output bus channels. */
        isismepsv2_ivid5_piu__batterypackstatus_t bat_stat; /*!< Bitflag field indicating BP board */
        int16_t temp2; /*!< 2 and 4 cell battery pack: Battery pack temperature in between the center battery cells. \note conversion: eng. value [in °C] = 0.01 * raw */
        int16_t volt_vd0; /*!< Voltage of voltage domain 0 */
        int16_t volt_vd1; /*!< Voltage of voltage domain 0 */
        int16_t volt_vd2; /*!< Voltage of voltage domain 0 */
        isismepsv2_ivid5_piu__vipdeng_t vip_obc00; /*!< Output V, I and P of output bus channel */
        isismepsv2_ivid5_piu__vipdeng_t vip_obc01; /*!< Output V, I and P of output bus channel */
        isismepsv2_ivid5_piu__vipdeng_t vip_obc03; /*!< Output V, I and P of output bus channel */
        isismepsv2_ivid5_piu__vipdeng_t vip_obc04; /*!< Output V, I and P of output bus channel */
        isismepsv2_ivid5_piu__vipdeng_t vip_obc05; /*!< Output V, I and P of output bus channel */
        isismepsv2_ivid5_piu__ccsdeng_t cc1; /*!< Data on conditioning chain */
        isismepsv2_ivid5_piu__ccsdeng_t cc2; /*!< Data on conditioning chain */
        isismepsv2_ivid5_piu__ccsdeng_t cc3; /*!< Data on conditioning chain */
    } fields;
} hk_eps_avg;

typedef union __attribute__((__packed__)) _eps_raw__from_t
{
    unsigned char raw[84];
    struct __attribute__ ((__packed__))
    {
	   uint16_t volt_brdsup; /*!< Voltage of internal board supply in raw form */
	   uint16_t temp; /*!< Measured temperature provided by a sensor internal to the MCU in raw form \note conversion: eng. value [in C] = 0.32234 * raw + -280*/
	   isismepsv2_ivid5_piu__vipdvd_t dist_input; /*!< Input V, I and P data taken at the input of the distribution part of the unit in raw form. Negative values indicate output flow. */
	   isismepsv2_ivid5_piu__vipdvd_t batt_input; /*!< Input V, I and P data taken at the input of the battery part of the unit in raw form. Negative values indicate output flow. */
	   uint16_t stat_obc_on; /*!< Bitflag field indicating channel-on status for the output bus channels. */
	   uint16_t stat_obc_ocf; /*!< Bitflag field indicating overcurrent latch-off fault status for the output bus channels. */
	   isismepsv2_ivid5_piu__batterypackstatus_t bat_stat; /*!< Bitflag field indicating BP board */
	   uint16_t temp2; /*!< 2 and 4 cell battery pack: Battery pack temperature in between the center battery cells. \note conversion: eng. value = -0.00000000334856 * raw<sup>3</sup> + 0.0000221414 * raw<sup>2</sup> + -0.066728 * raw + 93.3244*/
	   uint16_t volt_vd0; /*!< Voltage of voltage domain 0 in raw form */
	   uint16_t volt_vd1; /*!< Voltage of voltage domain 1 in raw form */
	   uint16_t volt_vd2; /*!< Voltage of voltage domain 2 in raw form */
	   isismepsv2_ivid5_piu__vipdch_t vip_obc00; /*!< Output V, I and P of output bus channel in raw form */
	   isismepsv2_ivid5_piu__vipdch_t vip_obc01; /*!< Output V, I and P of output bus channel in raw form */
	   isismepsv2_ivid5_piu__vipdch_t vip_obc03; /*!< Output V, I and P of output bus channel in raw form */
	   isismepsv2_ivid5_piu__vipdch_t vip_obc04; /*!< Output V, I and P of output bus channel in raw form */
	   isismepsv2_ivid5_piu__vipdch_t vip_obc05; /*!< Output V, I and P of output bus channel in raw form */
	   isismepsv2_ivid5_piu__ccsdraw_t cc1; /*!< Data on conditioning chain in raw form. */
	   isismepsv2_ivid5_piu__ccsdraw_t cc2; /*!< Data on conditioning chain in raw form. */
	   isismepsv2_ivid5_piu__ccsdraw_t cc3; /*!< Data on conditioning chain in raw form. */
    } fields;
} hk_eps_raw;

#define SIZE_BEACON_SPARE 10 //235 - 16 - 4 - 205

typedef union __attribute__((__packed__)) _WOD_Telemetry_t
{
	unsigned char raw[219];
	struct __attribute__ ((__packed__))
	{
		time_unix sat_time;
		int32_t solar_panels[NUMBER_OF_SOLAR_PANELS]; // temp of each solar panel
		unsigned int free_memory;		///< number of bytes free in the satellites SD [byte]
		unsigned int corrupt_bytes;		///< number of currpted bytes in the memory	[bytes]
		unsigned int sat_wakeup_time;
		unsigned char last_error_msg[SIZE_BEACON_SPARE];
		unsigned short number_of_sat_resets;
		unsigned short number_of_cmd_resets;
		int8_t eps_state;
		PayloadEventData pic32;
		PayloadEnvironmentData radfet;
		hk_eps_eng eps_eng;
		isis_vu_e__get_tx_telemetry__from_t tx;
		isis_vu_e__get_rx_telemetry__from_t rx;
		isis_ants__get_all_telemetry__from_t antA;
		isis_ants__get_all_telemetry__from_t antB;
	} fields;
} WOD_Telemetry_t;


typedef enum{
	eps_tlm,
	trxvu_tlm,
	ant_tlm,
	solar_panel_tlm,
	wod_tlm,
	pic32_tlm,
	radfet_tlm
}subsystem_tlm;

#define NUMBER_OF_TELEMETRIES 8	///< number of telemetries the satellite saves

#define NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS 7			///<

int InitTelemetryCollector();
/*!
 * @brief copies the corresponding filename into a buffer.
 * @return	-1 on NULL input
 * 			-2 on unknown  tlm_type
 */
int GetTelemetryFilenameByType(tlm_type tlm_type,char filename[MAX_F_FILE_NAME_SIZE]);


/*!
 * @brief Creates all telemetry files,
 * @param[out]	tlms_created states whether the files were created successful
 */
void TelemetryCreateFiles(Boolean8bit tlms_created[NUMBER_OF_TELEMETRIES]);

/*!
 * @brief saves all telemetries into the appropriate TLM files
 */
void TelemetryCollectorLogic();

/*!
 *  @brief saves current EPS telemetry into file
 */
void TelemetrySaveEPS();

/*!
 *  @brief saves current TRXVU telemetry into file
 */
void TelemetrySaveTRXVU();

/*!
 *  @brief saves current Antenna telemetry into file
 */
void TelemetrySaveANT();

/*!
 *  @brief saves current solar panel telemetry into file
 */
void TelemetrySaveSolarPanels();

/*!
 *  @brief saves current WOD telemetry into file
 */
void TelemetrySaveWOD();

/*!
 * @brief Gets all necessary telemetry and arranges it into a WOD structure
 * @param[out] output WOD telemetry. If an error occurred while getting TLM the fields will be zero
 */
WOD_Telemetry_t* GetCurrentWODTelemetry();

/*!
 *  @brief Gets all EPS telemetry
 * @param[out] output EPS telemetry and sends them to ground control
 */
int CMD_getEPS_ENG_TLM(sat_packet_t *cmd);
int CMD_getEPS_RAW_TLM(sat_packet_t *cmd);
int CMD_getEPS_AVG_TLM(sat_packet_t *cmd);

/*!
 *  @brief Gets all Solar panels telemetry
 * @param[out] output Solar panels telemetry and sends them to ground control
 */
int CMD_getSolar_TLM(sat_packet_t *cmd);

/*!
 * @brief Gets all TRXVU telemetry
 * @param[out] output TRXVU telemetry and sends them to ground control
 */
int CMD_getTX_TLM(sat_packet_t *cmd);
int CMD_getRX_TLM(sat_packet_t *cmd);

/*!
 * @brief Gets all Antennas telemetry
 * @param[out] output Antennas telemetry and sends them to ground control
 */
int CMD_getAnts_A_TLM(sat_packet_t *cmd);
int CMD_getAnts_B_TLM(sat_packet_t *cmd);
int CMD_getWOD_TLM(sat_packet_t *cmd);
int CMD_getLOG_TLM(sat_packet_t *cmd);
/*!
 * @brief Gets all Pic32 telemetry
 * @param[out] output Pic32 telemetry and sends them to ground control
 */
int CMD_getPic32_TLM(sat_packet_t *cmd);

/*!
 * @brief Gets all Radfet telemetry
 * @param[out] output Radfet telemetry and sends them to ground control
 */
int CMD_getRadfet_TLM(sat_packet_t *cmd);

/*!
 *  @brief saves current pic32 telemetry into file
 */
void TelemetrySavePIC32();

/*!
 *  @brief saves current radfet telemetry into file
 */
void TelemetrySaveRADFET();
int SaveSolar_TLM();

Boolean checkPayloadReadEvents();

Boolean checkPayloadReadEnvironment();


#endif /* TELEMETRYCOLLECTOR_H_ */
