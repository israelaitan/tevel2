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
#include "SubSystemModules/PowerManagment/EPSOperationModes.h"

#define SAVE_FLAG_IF_FILE_CREATED(type)	if(FS_SUCCSESS != res &&NULL != tlms_created){tlms_created[(type)] = FALSE_8BIT;}

time_unix tlm_save_periods[NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS] = {0};
time_unix tlm_last_save_time[NUM_OF_SUBSYSTEMS_SAVE_FUNCTIONS]= {0};
WOD_Telemetry_t wod_beacon = { 0 };
hk_eps_eng hk_eps_eng_for_save = { 0 };
hk_eps_avg hk_eps_avg_for_save = { 0 };
hk_eps_raw hk_eps_raw_for_save = { 0 };
isismepsv2_ivid5_piu__gethousekeepingraw__from_t tlm_mb_raw = {0};
isismepsv2_ivid5_piu__gethousekeepingeng__from_t tlm_mb_eng = {0};
isismepsv2_ivid5_piu__gethousekeepingrunningavg__from_t tlm_mb_avg = {0};

unsigned char* AddTime(unsigned char* data, unsigned int sz){
	unsigned int curr_time;
	Time_getUnixEpoch(&curr_time);
	unsigned char* res = malloc(sizeof(curr_time) + sz);
	if (res) {
		memcpy(res, &curr_time, sizeof(curr_time));
		memcpy(res + sizeof(curr_time), data, sz);
	}
	return res;
}

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
	case tlm_eps_avg_mb:
		strcpy(filename,FILENAME_EPS_AVG_MB_TLM);
		break;
	case tlm_tx:
		strcpy(filename,FILENAME_TX_TLM);
		break;
	case tlm_rx:
		strcpy(filename,FILENAME_RX_TLM);
		break;
	case tlm_antA:
		strcpy(filename,FILENAME_ANTENNA_SIDE_A_TLM);
		break;
	case tlm_antB:
		strcpy(filename,FILENAME_ANTENNA_SIDE_B_TLM);
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
	case tlm_solar:
		strcpy(filename,FILENAME_SOLAR_TLM);
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

	if (CheckExecutionTime(tlm_last_save_time[solar_panel_tlm], tlm_save_periods[solar_panel_tlm])){
		SaveSolar_TLM();
		logg(TLMInfo, "I:TelemetrySaveSOLAR\n");
		Time_getUnixEpoch((unsigned int *)(&tlm_last_save_time[solar_panel_tlm]));
	}

	Boolean is_payload_on;
	FRAM_read((unsigned char*)&is_payload_on, PAYLOAD_ON, PAYLOAD_ON_SIZE);
	if(is_payload_on) {
		if (CheckExecutionTime(tlm_last_save_time[pic32_tlm], tlm_save_periods[pic32_tlm])){
				TelemetrySavePIC32();
				logg(TLMInfo, "I:TelemetrySavePIC_32\n");
				Time_getUnixEpoch((unsigned int *)(&tlm_last_save_time[pic32_tlm]));
		}

		if (CheckExecutionTime(tlm_last_save_time[radfet_tlm], tlm_save_periods[radfet_tlm])){
				TelemetrySaveRADFET();
				logg(TLMInfo, "I:TelemetrySaveRADFET\n");
				Time_getUnixEpoch((unsigned int *)(&tlm_last_save_time[radfet_tlm]));
		}
	}

}


void TelemetryCreateFiles(Boolean8bit tlms_created[NUMBER_OF_TELEMETRIES])
{
	logg(event, "V:TelemetryCreateFiles()\n");
	FileSystemResult res;
	// -- EPS files
	res = c_fileCreate(FILENAME_EPS_RAW_MB_TLM,sizeof(hk_eps_raw));
	SAVE_FLAG_IF_FILE_CREATED(tlm_eps_raw_mb)

	res = c_fileCreate(FILENAME_EPS_ENG_MB_TLM,sizeof(hk_eps_eng));
	SAVE_FLAG_IF_FILE_CREATED(tlm_eps_eng_mb);

	res = c_fileCreate(FILENAME_EPS_AVG_MB_TLM,sizeof(hk_eps_avg));
	SAVE_FLAG_IF_FILE_CREATED(tlm_eps_avg_mb);

	// -- TRXVU files
	res = c_fileCreate(FILENAME_TX_TLM,sizeof(isis_vu_e__get_tx_telemetry__from_t));
	SAVE_FLAG_IF_FILE_CREATED(tlm_tx);

	res = c_fileCreate(FILENAME_RX_TLM,sizeof(isis_vu_e__get_rx_telemetry__from_t));
	SAVE_FLAG_IF_FILE_CREATED(tlm_rx);

	// -- ANTs files
	res = c_fileCreate(FILENAME_ANTENNA_SIDE_A_TLM,sizeof(isis_ants__get_all_telemetry__from_t));
	SAVE_FLAG_IF_FILE_CREATED(tlm_antA);
	res = c_fileCreate(FILENAME_ANTENNA_SIDE_B_TLM,sizeof(isis_ants__get_all_telemetry__from_t));
		SAVE_FLAG_IF_FILE_CREATED(tlm_antB);

	//-- LOG files
	res = c_fileCreate(FILENAME_LOG_TLM, LOG_TLM_SIZE);
	SAVE_FLAG_IF_FILE_CREATED(tlm_log);

	// -- WOD file
	res = c_fileCreate(FILENAME_WOD_TLM, sizeof(WOD_Telemetry_t));
	SAVE_FLAG_IF_FILE_CREATED(tlm_wod);

	// -- PAYLOAS file
	res = c_fileCreate(FILENAME_PIC32_TLM, sizeof(PayloadEventData));
	SAVE_FLAG_IF_FILE_CREATED(tlm_pic32);

	res = c_fileCreate(FILENAME_RADFET_TLM, sizeof(PayloadEnvironmentData));
	SAVE_FLAG_IF_FILE_CREATED(tlm_radfet);

	res = c_fileCreate(FILENAME_SOLAR_TLM, sizeof(int32_t) * ISIS_SOLAR_PANEL_6);
	SAVE_FLAG_IF_FILE_CREATED(tlm_solar);
}

void to_hk_eps_eng(isismepsv2_ivid5_piu__gethousekeepingeng__from_t *tlm_mb_eng, hk_eps_eng *tlm_hk_eps_eng) {
	tlm_hk_eps_eng->fields.bat_stat = tlm_mb_eng->fields.bat_stat;
	tlm_hk_eps_eng->fields.batt_input = tlm_mb_eng->fields.batt_input;
	tlm_hk_eps_eng->fields.cc1 = tlm_mb_eng->fields.cc1;
	tlm_hk_eps_eng->fields.cc2 = tlm_mb_eng->fields.cc2;
	tlm_hk_eps_eng->fields.cc3 = tlm_mb_eng->fields.cc3;
	tlm_hk_eps_eng->fields.dist_input = tlm_mb_eng->fields.dist_input;
	tlm_hk_eps_eng->fields.stat_obc_ocf = tlm_mb_eng->fields.stat_obc_ocf;
	tlm_hk_eps_eng->fields.stat_obc_on = tlm_mb_eng->fields.stat_obc_on;
	tlm_hk_eps_eng->fields.temp = tlm_mb_eng->fields.temp;
	tlm_hk_eps_eng->fields.temp2 = tlm_mb_eng->fields.temp2;
	tlm_hk_eps_eng->fields.vip_obc00 = tlm_mb_eng->fields.vip_obc00;
	tlm_hk_eps_eng->fields.vip_obc01 = tlm_mb_eng->fields.vip_obc01;
	tlm_hk_eps_eng->fields.vip_obc03 = tlm_mb_eng->fields.vip_obc03;
	tlm_hk_eps_eng->fields.vip_obc04 = tlm_mb_eng->fields.vip_obc04;
	tlm_hk_eps_eng->fields.vip_obc05 = tlm_mb_eng->fields.vip_obc05;
	tlm_hk_eps_eng->fields.volt_brdsup = tlm_mb_eng->fields.volt_brdsup;
	tlm_hk_eps_eng->fields.volt_vd0 = tlm_mb_eng->fields.volt_vd0;
	tlm_hk_eps_eng->fields.volt_vd1 = tlm_mb_eng->fields.volt_vd1;
	tlm_hk_eps_eng->fields.volt_vd2 = tlm_mb_eng->fields.volt_vd2;
}

void to_hk_eps_avg(isismepsv2_ivid5_piu__gethousekeepingrunningavg__from_t *tlm_mb_eng, hk_eps_eng *tlm_hk_eps_eng) {
	tlm_hk_eps_eng->fields.bat_stat = tlm_mb_eng->fields.bat_stat;
	tlm_hk_eps_eng->fields.batt_input = tlm_mb_eng->fields.batt_input;
	tlm_hk_eps_eng->fields.cc1 = tlm_mb_eng->fields.cc1;
	tlm_hk_eps_eng->fields.cc2 = tlm_mb_eng->fields.cc2;
	tlm_hk_eps_eng->fields.cc3 = tlm_mb_eng->fields.cc3;
	tlm_hk_eps_eng->fields.dist_input = tlm_mb_eng->fields.dist_input;
	tlm_hk_eps_eng->fields.stat_obc_ocf = tlm_mb_eng->fields.stat_obc_ocf;
	tlm_hk_eps_eng->fields.stat_obc_on = tlm_mb_eng->fields.stat_obc_on;
	tlm_hk_eps_eng->fields.temp = tlm_mb_eng->fields.temp;
	tlm_hk_eps_eng->fields.temp2 = tlm_mb_eng->fields.temp2;
	tlm_hk_eps_eng->fields.vip_obc00 = tlm_mb_eng->fields.vip_obc00;
	tlm_hk_eps_eng->fields.vip_obc01 = tlm_mb_eng->fields.vip_obc01;
	tlm_hk_eps_eng->fields.vip_obc03 = tlm_mb_eng->fields.vip_obc03;
	tlm_hk_eps_eng->fields.vip_obc04 = tlm_mb_eng->fields.vip_obc04;
	tlm_hk_eps_eng->fields.vip_obc05 = tlm_mb_eng->fields.vip_obc05;
	tlm_hk_eps_eng->fields.volt_brdsup = tlm_mb_eng->fields.volt_brdsup;
	tlm_hk_eps_eng->fields.volt_vd0 = tlm_mb_eng->fields.volt_vd0;
	tlm_hk_eps_eng->fields.volt_vd1 = tlm_mb_eng->fields.volt_vd1;
	tlm_hk_eps_eng->fields.volt_vd2 = tlm_mb_eng->fields.volt_vd2;
}

void to_hk_eps_raw(isismepsv2_ivid5_piu__gethousekeepingraw__from_t *tlm_mb_raw, hk_eps_raw *tlm_hk_eps_raw) {
	tlm_hk_eps_raw->fields.bat_stat = tlm_mb_raw->fields.bat_stat;
	tlm_hk_eps_raw->fields.batt_input = tlm_mb_raw->fields.batt_input;
	tlm_hk_eps_raw->fields.cc1 = tlm_mb_raw->fields.cc1;
	tlm_hk_eps_raw->fields.cc2 = tlm_mb_raw->fields.cc2;
	tlm_hk_eps_raw->fields.cc3 = tlm_mb_raw->fields.cc3;
	tlm_hk_eps_raw->fields.dist_input = tlm_mb_raw->fields.dist_input;
	tlm_hk_eps_raw->fields.stat_obc_ocf = tlm_mb_raw->fields.stat_obc_ocf;
	tlm_hk_eps_raw->fields.stat_obc_on = tlm_mb_raw->fields.stat_obc_on;
	tlm_hk_eps_raw->fields.temp = tlm_mb_raw->fields.temp;
	tlm_hk_eps_raw->fields.temp2 = tlm_mb_raw->fields.temp2;
	tlm_hk_eps_raw->fields.vip_obc00 = tlm_mb_raw->fields.vip_obc00;
	tlm_hk_eps_raw->fields.vip_obc01 = tlm_mb_raw->fields.vip_obc01;
	tlm_hk_eps_raw->fields.vip_obc03 = tlm_mb_raw->fields.vip_obc03;
	tlm_hk_eps_raw->fields.vip_obc04 = tlm_mb_raw->fields.vip_obc04;
	tlm_hk_eps_raw->fields.vip_obc05 = tlm_mb_raw->fields.vip_obc05;
	tlm_hk_eps_raw->fields.volt_brdsup = tlm_mb_raw->fields.volt_brdsup;
	tlm_hk_eps_raw->fields.volt_vd0 = tlm_mb_raw->fields.volt_vd0;
	tlm_hk_eps_raw->fields.volt_vd1 = tlm_mb_raw->fields.volt_vd1;
	tlm_hk_eps_raw->fields.volt_vd2 = tlm_mb_raw->fields.volt_vd2;
}

void TelemetrySaveEPS()
{
	int err = 0;

	err = isismepsv2_ivid5_piu__gethousekeepingraw(EPS_I2C_BUS_INDEX, &tlm_mb_raw);
	if (err == 0) {
		to_hk_eps_raw(&tlm_mb_raw, &hk_eps_raw_for_save);
		c_fileWrite(FILENAME_EPS_RAW_MB_TLM, &hk_eps_raw_for_save);
	} else
		logg(error, "E=%d isis_eps__gethousekeepingraw__tm\n", err);

	err = isismepsv2_ivid5_piu__gethousekeepingeng(EPS_I2C_BUS_INDEX, &tlm_mb_eng);
	if (err == 0) {
		to_hk_eps_eng(&tlm_mb_eng, &hk_eps_eng_for_save);
		c_fileWrite(FILENAME_EPS_ENG_MB_TLM, &hk_eps_eng_for_save);
		memcpy(wod_beacon.fields.eps_eng.raw, hk_eps_eng_for_save.raw, sizeof(hk_eps_eng));
	} else
		logg(error, "E=%d isis_eps__gethousekeepingeng__tm\n", err);

	err = isismepsv2_ivid5_piu__gethousekeepingrunningavg(EPS_I2C_BUS_INDEX, &tlm_mb_avg);
	if (err == 0) {
		to_hk_eps_avg(&tlm_mb_avg, &hk_eps_avg_for_save);
		c_fileWrite(FILENAME_EPS_AVG_MB_TLM, &hk_eps_avg_for_save);
	} else
		logg(error, "E=%d isismepsv2_ivid5_piu__gethousekeepingrunningavg\n", err);

}

//get EPS TLM
int CMD_getEPS_ENG_TLM(sat_packet_t *cmd) {
	int err = 0;
	err = isismepsv2_ivid5_piu__gethousekeepingeng(EPS_I2C_BUS_INDEX, &tlm_mb_eng);
	if(err == 0){
		to_hk_eps_eng(&tlm_mb_eng, &hk_eps_eng_for_save);
		unsigned char *data = AddTime((unsigned char*)&hk_eps_eng_for_save, sizeof(hk_eps_eng_for_save));
		TransmitDataAsSPL_Packet(cmd, data, sizeof(unsigned int) + sizeof(hk_eps_eng_for_save));
	}
	else
			logg(error, "E=%d isis_eps__gethousekeepingeng__tm\n", err);

	return err;
}

int CMD_getEPS_RAW_TLM(sat_packet_t *cmd) {
	int err = 0;
	err = isismepsv2_ivid5_piu__gethousekeepingraw(EPS_I2C_BUS_INDEX, &tlm_mb_raw);
	if(err == 0){
		to_hk_eps_raw(&tlm_mb_raw, &hk_eps_raw_for_save);
		unsigned char *data = AddTime((unsigned char*)&hk_eps_raw_for_save, sizeof(hk_eps_raw_for_save));
		TransmitDataAsSPL_Packet(cmd, data, sizeof(unsigned int) + sizeof(hk_eps_raw_for_save));
	}
	else
			logg(error, "E=%d isis_eps__gethousekeepingraw__tm\n", err);

	return err;
}

int CMD_getEPS_AVG_TLM(sat_packet_t *cmd) {
	int err = 0;
	err = isismepsv2_ivid5_piu__gethousekeepingrunningavg(EPS_I2C_BUS_INDEX, &tlm_mb_avg);
	if(err == 0){
		to_hk_eps_avg(&tlm_mb_avg, &hk_eps_avg_for_save);
		unsigned char *data = AddTime((unsigned char*)&hk_eps_avg_for_save, sizeof(hk_eps_avg_for_save));
		TransmitDataAsSPL_Packet(cmd, data, sizeof(unsigned int) + sizeof(hk_eps_avg_for_save));
	}
	else
			logg(error, "E=%d isismepsv2_ivid5_piu__gethousekeepingrunningavg\n", err);

	return err;
}

void TelemetrySaveTRXVU()
{
	int err = 0;
	isis_vu_e__get_tx_telemetry__from_t tx_tlm;
	err = isis_vu_e__get_tx_telemetry(ISIS_TRXVU_I2C_BUS_INDEX, &tx_tlm);
	if (err == 0) {
		c_fileWrite(FILENAME_TX_TLM, &tx_tlm);
		memcpy(wod_beacon.fields.rx.raw, tx_tlm.raw, sizeof(isis_vu_e__get_tx_telemetry__from_t));
	}

	isis_vu_e__get_rx_telemetry__from_t rx_tlm;
	err = isis_vu_e__get_rx_telemetry(ISIS_TRXVU_I2C_BUS_INDEX, &rx_tlm);
	if (err == 0) {
		c_fileWrite(FILENAME_RX_TLM, &rx_tlm);
		memcpy(wod_beacon.fields.tx.raw, rx_tlm.raw, sizeof(isis_vu_e__get_rx_telemetry__from_t));
	}
}

int CMD_getTX_TLM(sat_packet_t *cmd)
{
	int err;
	unsigned char rawData[sizeof(unsigned int) + sizeof(isis_vu_e__get_tx_telemetry__from_t)];

	isis_vu_e__get_tx_telemetry__from_t tx_tlm;
	err = isis_vu_e__get_tx_telemetry(ISIS_TRXVU_I2C_BUS_INDEX, &tx_tlm);
	if (err)
		return err;
	unsigned int curr_time;
	Time_getUnixEpoch(&curr_time);
	memcpy(rawData, (unsigned char*)&curr_time, sizeof(unsigned int));
	memcpy(rawData + sizeof(unsigned int), (unsigned char*)&tx_tlm.raw, sizeof(isis_vu_e__get_tx_telemetry__from_t));
	TransmitDataAsSPL_Packet(cmd, (unsigned char*)rawData, sizeof(unsigned int) + sizeof(isis_vu_e__get_tx_telemetry__from_t));
	return err;
}

int CMD_getRX_TLM(sat_packet_t *cmd)
{
	int err;
	unsigned char rawData[sizeof(unsigned int) + sizeof(isis_vu_e__get_rx_telemetry__from_t)];

	isis_vu_e__get_tx_telemetry__from_t rx_tlm;
	err = isis_vu_e__get_rx_telemetry(ISIS_TRXVU_I2C_BUS_INDEX, &rx_tlm);
	if (err == 0) {
		unsigned int curr_time;
		Time_getUnixEpoch(&curr_time);
		memcpy(rawData, (unsigned char*)&curr_time, sizeof(unsigned int));
		memcpy(rawData + sizeof(unsigned int), (unsigned char*)&rx_tlm.raw, sizeof(isis_vu_e__get_rx_telemetry__from_t));
		TransmitDataAsSPL_Packet(cmd, (unsigned char*)rawData, sizeof(unsigned int) + sizeof(isis_vu_e__get_rx_telemetry__from_t));
	}
	return err;
}

void TelemetrySaveANT()
{
	int err = 0;
	if (GetEPSSystemState() >= SafeMode)
		return;
	isis_ants__get_all_telemetry__from_t ant_tlmA = { 0 };
	err = isis_ants__get_all_telemetry(ANTS_SIDE_A_BUS_INDEX, &ant_tlmA);
	if (err == 0) {
		c_fileWrite(FILENAME_ANTENNA_SIDE_A_TLM, &ant_tlmA);
		memcpy(wod_beacon.fields.antA.raw, ant_tlmA.raw, sizeof(isis_ants__get_all_telemetry__from_t));
	} else
		logg(TLMInfo, "E=%d TelemetrySaveANT side A\n", err);

	isis_ants__get_all_telemetry__from_t ant_tlmB = { 0 };
	err = isis_ants__get_all_telemetry(ANTS_SIDE_B_BUS_INDEX, &ant_tlmB);
	if (err == 0) {
		c_fileWrite(FILENAME_ANTENNA_SIDE_B_TLM, &ant_tlmB);
		memcpy(wod_beacon.fields.antB.raw, ant_tlmB.raw, sizeof(isis_ants__get_all_telemetry__from_t));
	} else
		logg(TLMInfo, "E=%d TelemetrySaveANT side B\n", err);
}

// Get Antennas TLM
int CMD_getAnts_A_TLM(sat_packet_t *cmd) {
	int err = 0;
	isis_ants__get_all_telemetry__from_t ant_tlm;
	err = isis_ants__get_all_telemetry(ANTS_SIDE_A_BUS_INDEX, &ant_tlm);

	if (err == 0) {
		unsigned char *data = AddTime(&ant_tlm, sizeof(isis_ants__get_all_telemetry__from_t));
		TransmitDataAsSPL_Packet(cmd, data, sizeof(unsigned int) + sizeof(ant_tlm));
	}

	return err;
}

int CMD_getAnts_B_TLM(sat_packet_t *cmd) {
	int err = 0;
	isis_ants__get_all_telemetry__from_t ant_tlm;
	err = isis_ants__get_all_telemetry(ANTS_SIDE_B_BUS_INDEX, &ant_tlm);

	if (err == 0) {
		unsigned char *data = AddTime(&ant_tlm, sizeof(isis_ants__get_all_telemetry__from_t));
		TransmitDataAsSPL_Packet(cmd, data, sizeof(unsigned int) + sizeof(ant_tlm));
	}

	return err;
}

Boolean SolarPanelv2_Temperature(int32_t paneltemps[]) {
	IsisSolarPanelv2_Error_t error = 0;
	int panel;
	uint8_t status = 0;
	float conv_temp;

	IsisSolarPanelv2_wakeup();

	for( panel = 0; panel < ISIS_SOLAR_PANEL_6; panel++ ) {
		error = IsisSolarPanelv2_getTemperature(panel, &paneltemps[panel], &status);
		if( error ) {
			logg(TLMInfo, "Panel %d : Error (%d), Status (0x%X)\n", panel, error, status);
			continue;
		}
		paneltemps[panel] = (float)(paneltemps[panel]) * ISIS_SOLAR_PANEL_CONV;
	}

	IsisSolarPanelv2_sleep();
	return error;
}

int SaveSolar_TLM(){
	int32_t panel_temps[ISIS_SOLAR_PANEL_6];
	int err = SolarPanelv2_Temperature(panel_temps);
	if (!err) {
		c_fileWrite(FILENAME_SOLAR_TLM, panel_temps);
	}
	return err;
}

// Get Solar Panels TLM
int CMD_getSolar_TLM(sat_packet_t *cmd) {
	int32_t panel_temps[ISIS_SOLAR_PANEL_6];
	int err = SolarPanelv2_Temperature(panel_temps);
	if (!err) {
		unsigned char *data = AddTime(panel_temps,  sizeof(int32_t) * ISIS_SOLAR_PANEL_6);
		TransmitDataAsSPL_Packet(cmd, data, sizeof(unsigned int) + sizeof(int32_t) * ISIS_SOLAR_PANEL_6);
	}
	return err;
}

WOD_Telemetry_t* GetCurrentWODTelemetry()
{
	//memset(wod_beacon,0,sizeof(wod_beacon));
	time_unix current_time = 0;
	Time_getUnixEpoch((unsigned int *)&current_time);
	wod_beacon.fields.sat_time = current_time;

	int errorSizeMsg = getLastErrorMsgSize();
	if (errorSizeMsg > 0) {
		copyLastErrorMsg(wod_beacon.fields.last_error_msg, SIZE_BEACON_SPARE);
		errorSizeMsg = SIZE_BEACON_SPARE;
	}
	wod_beacon.fields.eps_state = GetEPSSystemState();

	int err = 0;
	F_SPACE space = { 0 };
	int drivenum = f_getdrive();
	err = f_getlasterror();
	err = f_getfreespace(drivenum, &space);
	if (err == F_NO_ERROR){
		wod_beacon.fields.free_memory = space.free;
		wod_beacon.fields.corrupt_bytes = space.bad;
	}

	SolarPanelv2_Temperature(&wod_beacon.fields.solar_panels);

    //Get number of resets is not managed
	FRAM_read((unsigned char*)&wod_beacon.fields.number_of_sat_resets, NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);
	FRAM_read((unsigned char*)&wod_beacon.fields.number_of_cmd_resets, NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE);
	FRAM_read((unsigned char*)&wod_beacon.fields.sat_wakeup_time, LAST_WAKEUP_TIME_ADDR, LAST_WAKEUP_TIME_SIZE);

	return &wod_beacon;
}

int CMD_getWOD_TLM(sat_packet_t *cmd) {
	GetCurrentWODTelemetry();
	unsigned char *data = AddTime(&wod_beacon, sizeof(WOD_Telemetry_t));
	TransmitDataAsSPL_Packet(cmd, data, sizeof(WOD_Telemetry_t) + sizeof(unsigned int));
	return 0;
}

int CMD_getLOG_TLM(sat_packet_t *cmd) {
	unsigned char data[LOG_TLM_SIZE_WITH_TIME];
	getLog_TLM(data, LOG_TLM_SIZE_WITH_TIME);
	TransmitDataAsSPL_Packet(cmd, data, LOG_TLM_SIZE_WITH_TIME);
	return 0;
}

void TelemetrySavePIC32() {

	PayloadEventData event_data;
	SoreqResult result = payloadReadEvents(&event_data);
	if (!result) {
		c_fileWrite(FILENAME_PIC32_TLM, &event_data);
		memcpy(wod_beacon.fields.pic32.raw, event_data.raw, sizeof(event_data));
	} else
		logg(error, "E:payloadTelemtrySavePic32=%d\n", result);
}

void TelemetrySaveRADFET() {

	PayloadEnvironmentData environment_data;
	SoreqResult result = payloadReadEnvironment(&environment_data);
	if (!result) {
		c_fileWrite(FILENAME_RADFET_TLM, &environment_data);
		memcpy(wod_beacon.fields.radfet.raw, environment_data.raw, sizeof(environment_data));
	} else
		logg(error, "E:TelemetrySaveRADFET=%d\n", result);

}

// Get Pic32 TLM
int CMD_getPic32_TLM(sat_packet_t *cmd)
{
	PayloadEventData event_data;
	SoreqResult result = payloadReadEvents(&event_data);
	if (!result) {
		unsigned char *data = AddTime((unsigned char*)&event_data, sizeof(event_data));
		if (data) {
			TransmitDataAsSPL_Packet(cmd, data, sizeof(unsigned int) + sizeof(event_data));
			free(data);
		}
		else {
			result = -1;//TODO:no memory
			logg(error, "E:payloadGetPic32=%d\n", result);
		}
	}
	else
		logg(error, "E:payloadGetPic32=%d\n", result);
	return result;
}

// Get Radfet TLM
int CMD_getRadfet_TLM(sat_packet_t *cmd)
{
	PayloadEnvironmentData environment_data;
	SoreqResult result = payloadReadEnvironment(&environment_data);
	if (!result) {
		unsigned char *data = AddTime((unsigned char*)&environment_data, sizeof(environment_data));
		if(data) {
			TransmitDataAsSPL_Packet(cmd, data, sizeof(unsigned int) + sizeof(environment_data));
			free(data);
		}
		else {
			result = -1; // TODO: no memory;
			logg(error, "E:payloadRdafet=%d\n", result);
		}
	}
	else
		logg(error, "E:payloadGetRADFET=%d\n", result);
	return result;
}


