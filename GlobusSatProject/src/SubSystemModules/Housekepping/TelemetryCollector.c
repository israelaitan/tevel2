#include <hcc/api_fat.h>

#include "GlobalStandards.h"

#ifdef ISISEPS
	#include <satellite-subsystems/IsisEPS.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif

#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>
#include <satellite-subsystems/IsisSolarPanelv2.h>
#include <hal/Timing/Time.h>

#include <string.h>

#include "TelemetryCollector.h"
#include "TelemetryFiles.h"
#include "TLM_management.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"

int GetTelemetryFilenameByType(tlm_type_t tlm_type, char filename[MAX_F_FILE_NAME_SIZE])
{
	if (filename == NULL)
		return null_pointer_error;

	switch (tlm_type) {
			case tlm_wod:
				strcpy(filename, FILENAME_WOD_TLM);
				break;
			case tlm_eps_raw_mb:
				strcpy(filename, FILENAME_EPS_RAW_MB_TLM);
				break;
			case tlm_eps_eng_mb:
				strcpy(filename, FILENAME_EPS_ENG_MB_TLM);
				break;
			case tlm_eps_raw_cdb:
				strcpy(filename, FILENAME_EPS_RAW_CDB_TLM);
				break;
			case tlm_eps_eng_cdb:
				strcpy(filename, FILENAME_EPS_ENG_CDB_TLM);
				break;
			case tlm_solar:
				strcpy(filename, FILENAME_SOLAR_PANELS_TLM);
				break;
			case tlm_tx:
				strcpy(filename, FILENAME_TX_TLM);
				break;
			case tlm_tx_revc:
				strcpy(filename, FILENAME_TX_REVC);
				break;
			case tlm_rx:
				strcpy(filename, FILENAME_RX_TLM);
				break;
			case tlm_rx_revc:
				strcpy(filename, FILENAME_RX_REVC);
				break;
			case tlm_rx_frame:
				strcpy(filename, FILENAME_RX_FRAME);
				break;
			case tlm_antenna:
				strcpy(filename, FILENAME_ANTENNA_TLM);
				break;
			default:
				return E_PARAM_OUTOFBOUNDS;
	}
	return E_NO_SS_ERR;
}

void TelemetryCollectorLogic()
{
}

void TelemetrySaveEPS()
{
	unsigned char index = EPS_I2C_BUS_INDEX;
	ieps_rawhk_data_mb_t p_rawhk_data_mb = { 0 };
	ieps_enghk_data_mb_t p_enghk_data_mb = { 0 };
	ieps_rawhk_data_cdb_t p_rawhk_data_cdb = { 0 };
	ieps_enghk_data_cdb_t p_enghk_data_cdb = { 0 };
	ieps_statcmd_t p_rsp_code = { 0 };
	char filename[MAX_F_FILE_NAME_SIZE];
	int err = 0;

	err = IsisEPS_getRawHKDataMB(index, &p_rawhk_data_mb, &p_rsp_code);
	err = GetTelemetryFilenameByType( tlm_eps_raw_mb, filename );
	if (p_rsp_code.fields.cmd_error != 0)
		return;
	err = c_fileWrite(filename, &p_rawhk_data_mb);
	if (err)
		return;

	err = IsisEPS_getEngHKDataMB(index, &p_enghk_data_mb, &p_rsp_code);
	err = GetTelemetryFilenameByType( tlm_eps_eng_mb, filename );
	if (p_rsp_code.fields.cmd_error != 0)
		return;
	err = c_fileWrite(filename, &p_enghk_data_mb);
	if (err)
		return;

	err = IsisEPS_getRawHKDataCDB(index, ieps_board_mb, &p_rawhk_data_cdb, &p_rsp_code);
	err = GetTelemetryFilenameByType( tlm_eps_raw_cdb, filename );
	if (p_rsp_code.fields.cmd_error != 0)
		return;
	err = c_fileWrite(filename, &p_rawhk_data_cdb);
	if (err)
		return;

	err = IsisEPS_getEngHKDataCDB(index, ieps_board_mb, &p_enghk_data_cdb, &p_rsp_code);
	err = GetTelemetryFilenameByType( tlm_eps_eng_cdb, filename );
	if (p_rsp_code.fields.cmd_error != 0)
		return;
	err = c_fileWrite(filename, &p_enghk_data_cdb);
	if (err)
		return;

}

void TelemetrySaveTRXVU()
{
}

void TelemetrySaveANT()
{
}

void TelemetrySaveSolarPanels()
{
}

void TelemetrySaveWOD()
{
}

void GetCurrentWODTelemetry(WOD_Telemetry_t *wod)
{
}

