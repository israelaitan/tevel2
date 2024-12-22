#include <string.h>

#include <hcc/api_fat.h>
#include <satellite-subsystems/isismepsv2_ivid5_piu.h>

#include <satellite-subsystems/isis_vu_e.h>

#include <satellite-subsystems/isis_ants.h>
#include <satellite-subsystems/isis_ants_types.h>

#include <satellite-subsystems/IsisSolarPanelv2.h>
#include <hal/Timing/Time.h>
#include <hal/drivers/ADC.h>

#include "GlobalStandards.h"
#include "TelemetryCollector.h"
#include "TelemetryFiles.h"
#include "TLM_management.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Maintenance/Log.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "SubSystemModules/Payload/payload_drivers.h"

#define SAVE_FLAG_IF_FILE_CREATED(type)	if(FS_SUCCSESS != res &&NULL != tlms_created){tlms_created[(type)] = FALSE_8BIT;}

time_unix tlm_save_periods[NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS] = {0};
time_unix tlm_last_save_time[NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS]= {0};


int InitTelemetryCollector() {
	return FRAM_read((unsigned char*)tlm_save_periods, TLM_SAVE_PERIOD_START_ADDR, NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS*sizeof(time_unix));
}

int GetTelemetryFilenameByType(tlm_type tlm_type, char filename[MAX_F_FILE_NAME_SIZE])
{
	if(NULL == filename){
		return -1;
	}
	switch (tlm_type) {

	case tlm_eps_raw_mb:
		strcpy(filename,FILENAME_EPS_RAW_MB_TLM);
		break;
	case tlm_eps_eng_mb:
		strcpy(filename,FILENAME_EPS_ENG_MB_TLM);
		break;
	case tlm_tx:
		strcpy(filename,FILENAME_TX_TLM);
		break;
	case tlm_rx:
		strcpy(filename,FILENAME_RX_TLM);
		break;
	case tlm_antenna:
		strcpy(filename,FILENAME_ANTENNA_TLM);
		break;
	case tlm_log:
		strcpy(filename,FILENAME_LOG_TLM);
		break;
	case tlm_wod:
		strcpy(filename,FILENAME_WOD_TLM);
		break;
	case tlm_pic32:
		strcpy(filename,FILENAME_PIC32_TLM);
		break;
	case tlm_radfet:
		strcpy(filename,FILENAME_RADFET_TLM);
		break;

	default:
		return -2;
	}
	return 0;
}


void TelemetryCollectorLogic()
{
	if (CheckExecutionTime(tlm_last_save_time[eps_tlm],tlm_save_periods[eps_tlm])){
		TelemetrySaveEPS();
		Time_getUnixEpoch((unsigned int *)(&tlm_last_save_time[eps_tlm]));
		logg(TLMInfo, "I:TelemetrySaveEPS, time: %lu\n", tlm_last_save_time[eps_tlm]);
	}

	if (CheckExecutionTime(tlm_last_save_time[trxvu_tlm],tlm_save_periods[trxvu_tlm])){
		TelemetrySaveTRXVU();
		logg(TLMInfo, "I:TelemetrySaveTRXVU\n");
		Time_getUnixEpoch((unsigned int *)(&tlm_last_save_time[trxvu_tlm]));
	}

	if (CheckExecutionTime(tlm_last_save_time[ant_tlm],tlm_save_periods[ant_tlm])){
		TelemetrySaveANT();
		logg(TLMInfo, "I:TelemetrySaveANT\n");
		Time_getUnixEpoch((unsigned int *)(&tlm_last_save_time[ant_tlm]));
	}

	if (CheckExecutionTime(tlm_last_save_time[wod_tlm], tlm_save_periods[wod_tlm])){
		TelemetrySaveWOD();
		logg(TLMInfo, "I:TelemetrySaveWOD\n");
		Time_getUnixEpoch((unsigned int *)(&tlm_last_save_time[wod_tlm]));
	}

	if (CheckExecutionTime(tlm_last_save_time[seu_tlm], tlm_save_periods[seu_tlm])){
		checkPayloadReadEvents();
		logg(event, "V:TelemetrySaveSEU\n");
		Time_getUnixEpoch((unsigned int *)(&tlm_last_save_time[seu_tlm]));
	}

	if (CheckExecutionTime(tlm_last_save_time[pic32_tlm], tlm_save_periods[pic32_tlm])){
			checkPayloadReadEvents();
			logg(event, "V:TelemetrySavePIC_32\n");
			Time_getUnixEpoch((unsigned int *)(&tlm_last_save_time[pic32_tlm]));
	}

	if (CheckExecutionTime(tlm_last_save_time[radfet_tlm], tlm_save_periods[radfet_tlm])){
			checkPayloadReadEnvironment();
			logg(event, "V:TelemetrySaveRADFET\n");
			Time_getUnixEpoch((unsigned int *)(&tlm_last_save_time[radfet_tlm]));
	}


}


void TelemetryCreateFiles(Boolean8bit tlms_created[NUMBER_OF_TELEMETRIES])
{
	logg(event, "V:TelemetryCreateFiles()\n");
	FileSystemResult res;
	// -- EPS files
	res = c_fileCreate(FILENAME_EPS_RAW_MB_TLM,sizeof(isismepsv2_ivid5_piu__gethousekeepingraw__from_t));
	SAVE_FLAG_IF_FILE_CREATED(tlm_eps_raw_mb)

	res = c_fileCreate(FILENAME_EPS_ENG_MB_TLM,sizeof(isismepsv2_ivid5_piu__gethousekeepingeng__from_t));
	SAVE_FLAG_IF_FILE_CREATED(tlm_eps_eng_mb);

	// -- TRXVU files
	res = c_fileCreate(FILENAME_TX_TLM,sizeof(isis_vu_e__get_tx_telemetry__from_t));
	SAVE_FLAG_IF_FILE_CREATED(tlm_tx);

	res = c_fileCreate(FILENAME_RX_TLM,sizeof(isis_vu_e__get_rx_telemetry__from_t));
	SAVE_FLAG_IF_FILE_CREATED(tlm_rx);

	// -- ANT files
	res = c_fileCreate(FILENAME_ANTENNA_TLM,sizeof(isis_ants__get_status__from_t));
	SAVE_FLAG_IF_FILE_CREATED(tlm_antenna);

	//-- LOG files
	res = c_fileCreate(FILENAME_LOG_TLM, LOG_TLM_SIZE);
	SAVE_FLAG_IF_FILE_CREATED(tlm_log);

	// -- WOD file
	res = c_fileCreate(FILENAME_WOD_TLM, sizeof(WOD_Telemetry_t));
	SAVE_FLAG_IF_FILE_CREATED(tlm_wod);

	// -- PAYLOAS file
	//res = c_fileCreat(FILENAME_PIC32_TLM, sizeof(?));
	//SVAE_FLAG_IF_FILE_CREATED(tlm_pic32);

	//res = c_fileCreat(FILENAME_RADFET_TLM, sizeof(?));
	//SVAE_FLAG_IF_FILE_CREATED(tlm_radfet);
}

void TelemetrySaveEPS()
{
	int err = 0;

	isismepsv2_ivid5_piu__gethousekeepingraw__from_t tlm_mb_raw;
	err = isismepsv2_ivid5_piu__gethousekeepingraw(EPS_I2C_BUS_INDEX, &tlm_mb_raw);
	if (err == 0)
		c_fileWrite(FILENAME_EPS_RAW_MB_TLM, &tlm_mb_raw);
	else
		logg(error, "E=%d isis_eps__gethousekeepingraw__tm\n", err);

	isismepsv2_ivid5_piu__gethousekeepingeng__from_t tlm_mb_eng;
	err = isismepsv2_ivid5_piu__gethousekeepingeng(EPS_I2C_BUS_INDEX, &tlm_mb_eng);

	if (err == 0)
		c_fileWrite(FILENAME_EPS_ENG_MB_TLM, &tlm_mb_eng);
	else
		logg(error, "E=%d isis_eps__gethousekeepingeng__tm\n", err);

}

//get EPS TLM
int CMD_getEPS_TLM(sat_packet_t *cmd)
{
	int err = 0;
	isismepsv2_ivid5_piu__gethousekeepingeng__from_t tlm_mb_eng;
	err = isismepsv2_ivid5_piu__gethousekeepingeng(EPS_I2C_BUS_INDEX, &tlm_mb_eng);

	if(err == 0)
		TransmitDataAsSPL_Packet(cmd, (unsigned char*)&tlm_mb_eng.raw, sizeof(tlm_mb_eng));
	else
			logg(error, "E=%d isis_eps__gethousekeepingeng__tm\n", err);

	return err;
}

void TelemetrySaveTRXVU()
{
	int err = 0;
	isis_vu_e__get_tx_telemetry__from_t tx_tlm;
	err = isis_vu_e__get_tx_telemetry(ISIS_TRXVU_I2C_BUS_INDEX, &tx_tlm);
	if (err == 0)
		c_fileWrite(FILENAME_TX_TLM, &tx_tlm);

	isis_vu_e__get_rx_telemetry__from_t rx_tlm;
	err = isis_vu_e__get_rx_telemetry(ISIS_TRXVU_I2C_BUS_INDEX, &rx_tlm);
	if (err == 0)
		c_fileWrite(FILENAME_RX_TLM, &rx_tlm);

}

int CMD_getTRXVU_TLM(sat_packet_t *cmd)
{
	int err1, err2;
	unsigned char rawData[sizeof(isis_vu_e__get_tx_telemetry__from_t) + sizeof(isis_vu_e__get_tx_telemetry_last__from_t)];

	isis_vu_e__get_tx_telemetry__from_t tx_tlm;
	err1 = isis_vu_e__get_tx_telemetry(ISIS_TRXVU_I2C_BUS_INDEX, &tx_tlm);
	if (err1 == 0)
		memcpy(&rawData, (unsigned char*)&tx_tlm.raw, sizeof(isis_vu_e__get_tx_telemetry__from_t));

	isis_vu_e__get_tx_telemetry_last__from_t rx_tlm;
	err2 = isis_vu_e__get_tx_telemetry_last(ISIS_TRXVU_I2C_BUS_INDEX, &rx_tlm);
	if (err2 == 0)
		memcpy(&rawData + sizeof(isis_vu_e__get_tx_telemetry__from_t), (unsigned char*)&rx_tlm.raw, sizeof(isis_vu_e__get_tx_telemetry_last__from_t));

	if(err1 == 0 || err2 == 0)
	{
		TransmitDataAsSPL_Packet(cmd, (unsigned char*)rawData, sizeof(isis_vu_e__get_tx_telemetry__from_t) + sizeof(isis_vu_e__get_tx_telemetry_last__from_t));
		return 0;
	}

	return err1 || err2;
}

void TelemetrySaveANT()
{
	int err = 0;
	isis_ants__get_all_telemetry__from_t ant_tlmA;
	err = isis_ants__get_all_telemetry(ISIS_TRXVU_I2C_BUS_INDEX, &ant_tlmA);
	if (err == 0)
		c_fileWrite(FILENAME_ANTENNA_TLM, &ant_tlmA);
}

// Get Antennas TLM
int CMD_getAnts_TLM(sat_packet_t *cmd)
{
	int err = 0;
	isis_ants__get_all_telemetry__from_t ant_tlm;
	err = isis_ants__get_all_telemetry(ISIS_TRXVU_I2C_BUS_INDEX, &ant_tlm);

	if (err == 0)
		TransmitDataAsSPL_Packet(cmd, (unsigned char*) &ant_tlm,sizeof(ant_tlm));

	return err;
}

int getSolarPanelsTLM(int32_t *t)
{
	int err = 0;
	uint8_t fault;

	err = IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_0, &t[0], &fault);
	err = IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_1, &t[1], &fault);
	err = IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_2, &t[2], &fault);
	err = IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_3, &t[3], &fault);
	err = IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_4, &t[4], &fault);

	return err;
}

// Get Solar Panels TLM
int CMD_getSolar_TLM(sat_packet_t *cmd)
{
	int err = 0;
	int32_t t[ISIS_SOLAR_PANEL_COUNT];

	if (IsisSolarPanelv2_getState() == ISIS_SOLAR_PANEL_STATE_AWAKE)
	{
		err = getSolarPanelsTLM(t);
		if (err == ISIS_SOLAR_PANEL_STATE_AWAKE * ISIS_SOLAR_PANEL_COUNT)
		{
			TransmitDataAsSPL_Packet(cmd, (unsigned char*) t, sizeof(int32_t)*ISIS_SOLAR_PANEL_COUNT);
		}
	}

	return err;
}


void TelemetrySaveWOD()
{
	WOD_Telemetry_t wod = { 0 };
	GetCurrentWODTelemetry(&wod);
	c_fileWrite(FILENAME_WOD_TLM, &wod);
}

void GetCurrentWODTelemetry(WOD_Telemetry_t *wod)
{
	if (NULL == wod)
		return;

	memset(wod,0,sizeof(*wod));
	int err = 0;

	F_SPACE space = { 0 };
	int drivenum = f_getdrive();
	err = f_getlasterror();
	err = f_getfreespace(drivenum, &space);
	if (err == F_NO_ERROR){
		wod->free_memory = space.free;
		wod->corrupt_bytes = space.bad;
	}
	time_unix current_time = 0;
	Time_getUnixEpoch((unsigned int *)&current_time);
	wod->sat_time = current_time;

	isismepsv2_ivid5_piu__gethousekeepingeng__from_t hk_tlm;
	err += isismepsv2_ivid5_piu__gethousekeepingeng(EPS_I2C_BUS_INDEX, &hk_tlm);

	if(err == 0){
		wod->vbat = hk_tlm.fields.dist_input.fields.volt;
		wod->electric_current =  hk_tlm.fields.dist_input.fields.current;
		wod->consumed_power = hk_tlm.fields.dist_input.fields.power;

		wod->charging_power = hk_tlm.fields.batt_input.fields.power;

		wod->current_v0 = hk_tlm.fields.vip_obc00.fields.current;
		wod->volt_v0 = hk_tlm.fields.vip_obc00.fields.volt;

		wod->current_3V3 = hk_tlm.fields.vip_obc05.fields.current;
		wod->volt_3V3 = hk_tlm.fields.vip_obc05.fields.volt;

		wod->current_5V = hk_tlm.fields.vip_obc01.fields.current;;
		wod->volt_5V =hk_tlm.fields.vip_obc01.fields.volt;

		wod->mcu_temp = hk_tlm.fields.temp;
		wod->bat_temp = hk_tlm.fields.temp2;

		wod->volt_in_mppt1 = hk_tlm.fields.cc1.fields.volt_in_mppt;
		wod->curr_in_mppt1 = hk_tlm.fields.cc1.fields.curr_in_mppt;
		wod->volt_in_mppt2 = hk_tlm.fields.cc2.fields.volt_in_mppt;
		wod->curr_in_mppt2 = hk_tlm.fields.cc2.fields.curr_in_mppt;
		wod->volt_in_mppt3 = hk_tlm.fields.cc3.fields.volt_in_mppt;
		wod->curr_in_mppt3 = hk_tlm.fields.cc3.fields.curr_in_mppt;
	}

	uint8_t status;
	IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_0,&wod->solar_panels[0],&status);
	IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_1,&wod->solar_panels[1],&status);
	IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_2,&wod->solar_panels[2],&status);
	IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_3,&wod->solar_panels[3],&status);
	IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_4,&wod->solar_panels[4],&status);
	IsisSolarPanelv2_getTemperature(ISIS_SOLAR_PANEL_5,&wod->solar_panels[5],&status);

	// get ADC channels vlaues (include the photo diodes mV values)
	unsigned short adcSamples[8];
	ADC_SingleShot( adcSamples );
	int i;
	for(i=0; i < NUMBER_OF_SOLAR_PANELS; i++ )
		wod->photo_diodes[i] = ADC_ConvertRaw10bitToMillivolt( adcSamples[i] ); // convert to mV data


    //Get number of resets is not managed
	FRAM_read((unsigned char*)&wod->number_of_resets, NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);
	FRAM_read((unsigned char*)&wod->number_of_cmd_resets, NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE);
}

void TelemetrySavePIC32()
{

}

void TelemetrySaveRADFET()
{

}

Boolean checkPayloadReadEvents() {
    printf("Testing payloadReadEvents...\n");
    PayloadEventData event_data;
    SoreqResult result = payloadReadEvents(&event_data);
    if (result == PAYLOAD_SUCCESS) {
        printf("payloadReadEvents: SUCCESS\n");
        printf("  SEL Count: %d\n", event_data.sel_count);
        printf("  SEU Count: %d\n", event_data.seu_count);
        return TRUE;
    } else {
        printf("payloadReadEvents: ERROR (%d)\n", result);
        return FALSE;
    }
}

Boolean checkPayloadReadEnvironment() {
    printf("Testing payloadReadEnvironment...\n");
    PayloadEnvironmentData environment_data;
    SoreqResult result = payloadReadEnvironment(&environment_data);

    if (result == PAYLOAD_SUCCESS) {
        printf("payloadReadEnvironment: SUCCESS\n");
        printf("  RADFET Voltage 1: %d (0x%X)\n",
               environment_data.adc_conversion_radfet1,
               environment_data.adc_conversion_radfet1);
        printf("  RADFET Voltage 2: %d (0x%X)\n",
               environment_data.adc_conversion_radfet2,
               environment_data.adc_conversion_radfet2);
        printf("  Temperature: %lf\n", environment_data.temperature);
        return TRUE;
    } else {
        printf("payloadReadEnvironment: ERROR (%d)\n", result);
        return FALSE;
    }
}


// Get Pic32 TLM
int CMD_getPic32_TLM(sat_packet_t *cmd)
{
	cmd = NULL;
	return 0;
}

// Get Radfet TLM
int CMD_getRadfet_TLM(sat_packet_t *cmd)
{
	cmd = NULL;
	return 0;
}

