
#include <satellite-subsystems/IsisSolarPanelv2.h>
#include <hal/errors.h>

#include <string.h>

#include "EPS.h"
#include "EPSOperationModes.h"
#ifdef ISISEPS
	#include <satellite-subsystems/IsisEPS.h>
#endif
#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif

// y[i] = a * x[i] +(1-a) * y[i-1]
voltage_t prev_avg = 0;		// y[i-1]
voltage_t curr_avg = 0;
float alpha = 0;			//<! smoothing constant

voltage_t eps_threshold_voltages[NUMBER_OF_THRESHOLD_VOLTAGES];	// saves the current EPS logic threshold voltages

int GetBatteryVoltage(voltage_t *vbatt)
{
	ieps_enghk_data_cdb_t data = {.raw ={0}};
	ieps_statcmd_t status = {.raw ={0}};
	int err = IsisEPS_getEngHKDataCDB(0, ieps_board_mb, &data, &status);
	if (err)
		return err;
	if(status.fields.cmd_error != 0)
		return status.fields.cmd_error;
	*vbatt = data.fields.bat_voltage;
	return 0;
}

int EPS_Init()
{
	unsigned char addr = EPS_I2C_ADDR;
	int err = IsisEPS_initialize( &addr, NUMBER_OF_EPS );
	if (err)
		return err;
	IsisSolarPanelv2_State_t state = IsisSolarPanelv2_initialize( slave0_spi );
	if (state == ISIS_SOLAR_PANEL_STATE_NOINIT)
			return E_NOT_INITIALIZED;
	IsisSolarPanelv2_sleep();
	err = GetThresholdVoltages(eps_threshold_voltages);
	if (err)
		return err;
	voltage_t temp[] = DEFAULT_EPS_THRESHOLD_VOLTAGES;
	memcpy(temp, eps_threshold_voltages, NUMBER_OF_THRESHOLD_VOLTAGES);
	err = GetAlpha(&alpha);
	if (err)
			return err;

	err = EPS_Conditioning();
	return err;
}

int EPS_Conditioning()
{
	voltage_t currV = 0;
	int err = GetBatteryVoltage(&currV);
	if (err)
		return err;
	curr_avg = alpha * currV + (1 - alpha) * prev_avg;

	if (curr_avg > prev_avg) {
		if (curr_avg > eps_threshold_voltages[INDEX_DOWN_FULL])
			EnterFullMode();
		else if (curr_avg > eps_threshold_voltages[INDEX_DOWN_CRUISE])
			EnterCruiseMode();
		else if (curr_avg > eps_threshold_voltages[INDEX_DOWN_SAFE])
			EnterSafeMode();
		else
			EnterCriticalMode();
	} else if (curr_avg < prev_avg) {
		if (curr_avg < eps_threshold_voltages[INDEX_DOWN_SAFE])
			EnterCriticalMode();
		else if (curr_avg < eps_threshold_voltages[INDEX_UP_SAFE])
			EnterSafeMode();
		else if (curr_avg < eps_threshold_voltages[INDEX_UP_CRUISE])
			EnterCruiseMode();
		else
			EnterFullMode();
	}
	prev_avg = curr_avg;
	return 0;
}

int UpdateAlpha(float new_alpha)
{
	return 0;
}

int UpdateThresholdVoltages(voltage_t thresh_volts[NUMBER_OF_THRESHOLD_VOLTAGES])
{
	return 0;
}

int GetThresholdVoltages(voltage_t thresh_volts[NUMBER_OF_THRESHOLD_VOLTAGES])
{
	int err = FRAM_read((unsigned char *)thresh_volts, EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE);
	return err;
}

int GetAlpha(float *alpha)
{
	int err = FRAM_read((unsigned char *)alpha, EPS_ALPHA_FILTER_VALUE_ADDR, EPS_ALPHA_FILTER_VALUE_SIZE);
	return err;
}

int RestoreDefaultAlpha()
{
	return 0;
}

int RestoreDefaultThresholdVoltages()
{
	return 0;
}

