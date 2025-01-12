#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <satellite-subsystems/IsisSolarPanelv2.h>
#include <hal/errors.h>
#include <string.h>
#include <hal/Drivers/I2C.h>
#include "EPS.h"
#include "SubSystemModules/Maintenance/Log.h"
#include <satellite-subsystems/isismepsv2_ivid5_piu.h>

ISISMEPSV2_IVID5_PIU_t subsystem[1]; // One instance to be initialised.

// y[i] = a * x[i] +(1-a) * y[i-1]
voltage_t prev_filtered_voltage = 0;		// y[i-1]

float alpha = DEFAULT_ALPHA_VALUE;			//<! smoothing constant
unsigned char heaters_active_mode = DEFAULT_HEATERS_ACTIVE_MODE;
EpsThreshVolt_t eps_threshold_voltages = {.raw = DEFAULT_EPS_THRESHOLD_VOLTAGES};	// saves the current EPS logic threshold voltages
extern unsigned short gl_eps_state_linger = 0;
extern unsigned short gl_eps_next_state_linger = 0;

int GetHeatersActiveMode() {
	int err = FRAM_read((unsigned char*) heaters_active_mode, EPS_BAT_HITERRS_ACTIVE_MODE_ADDR,
			EPS_BAT_HITERRS_ACTIVE_MODE_SIZE);
	return err;
}

void print_byte_arr(unsigned char arr[], int size) {
	if (!arr && size <= 0)
		return;
	int i = 0;
	for (; i < size - 1; i++)
		logg(EPSInfo, "%d=%d,", i, arr[i]);
	logg(EPSInfo, "%d=%d\n", i, arr[i]);
}

uint16_t byte_arr_2_int16(unsigned char arr[], int size) {
	return  arr[0] + (arr[1] << 8);
}

int i2c_write_read(I2Ctransfer *tr) {
	int err = 0;
	int i = 0;
	for (i = 0; i < 10; i++) {
		err = I2C_writeRead(tr);
		if (err)
			return err;

		if (tr->readData[4] != NEW_RESP)
			continue;
		printf("i=%d\n", i);
		print_byte_arr(tr->writeData, tr->writeSize);
		print_byte_arr(tr->readData, tr->readSize);
		break;
	}
	return err;
}

int eps_i2c_comm_set_config(uint16_t param, unsigned char *value, unsigned int val_size, int write_sz, int read_sz){
	//STID IVID 0x84 BID 0x0A 0x48 read only
	I2Ctransfer tr = { 0 };
	tr.slaveAddress = EPS_I2C_ADDR;
	unsigned char *i2c_write_data = malloc(write_sz);
	int i = 0;
	i2c_write_data[i++] =STID;
	i2c_write_data[i++] = IVID;
	i2c_write_data[i++] = SET_CONF_W;
	i2c_write_data[i++] = BID;
	i2c_write_data[i++] = ((unsigned char *)&param)[0];
	i2c_write_data[i++] = ((unsigned char *)&param)[1];
	i2c_write_data[i++] = value[0];
	if (val_size == 2)
		i2c_write_data[i++] = value[1];
	//unsigned char i2c_write_data[] = { STID, IVID, GET_CONF_W, BID , 0x00, 0x48 };
	tr.writeData = i2c_write_data;
	tr.writeSize = write_sz;
	tr.readData = malloc(read_sz);
	tr.readSize = read_sz;
	tr.writeReadDelay = 20;
	int err = i2c_write_read(&tr);
	if (!err) {
		uint16_t val;
		if (read_sz - 8 == 1)
			val = *(tr.readData+8);
		else if (read_sz - 8 == 2)
			val = byte_arr_2_int16(tr.readData+8, 2);

		printf("val=%d\n", val);
	}
	free(tr.readData);
	free(tr.writeData);
	return err;
}

void eps_set_battary_heater_mode(int8_t set){
	eps_i2c_comm_set_config(AUTO_HEAT_ENA_BP1, (unsigned char *)&set, sizeof(int8_t), SET_CONF_SIZE_W_8, SET_CONF_SIZE_W_8 + 2);
}

int init_linger_thrs(){
	FRAM_read((unsigned char*)&gl_eps_state_linger, EPS_MIN_EPS_STATE_LINGER_TIME_ADDR, EPS_MIN_EPS_STATE_LINGER_TIME_SIZE);
	if (gl_eps_state_linger > 3600)
		gl_eps_state_linger = MIN_EPS_STATE_LINGER_TIME;
	FRAM_read((unsigned char*)&gl_eps_next_state_linger, EPS_MIN_EPS_NEXT_STATE_LINGER_TIME_ADDR, EPS_MIN_EPS_NEXT_STATE_LINGER_TIME_SIZE);
	if (gl_eps_next_state_linger > 3600)
		gl_eps_next_state_linger = MIN_EPS_NEXT_STATE_LINGER_TIME;
}

int EPS_Init()
{
	subsystem[0].i2cAddr = EPS_I2C_ADDR;
	int rv = ISISMEPSV2_IVID5_PIU_Init(subsystem, 1);
	if (rv != E_NO_SS_ERR) {
		logg(error, "E:EPS init failed\n");
		return -1;
	}

	rv = GetThresholdVoltages(&eps_threshold_voltages);
	if (0 != rv) {
		return -3;
	}

	rv= GetAlpha(&alpha);
	if (0 != rv) {
		alpha = DEFAULT_ALPHA_VALUE;
		return -4;
	}
	prev_filtered_voltage = 0;	//y[i-1]
	GetBatteryVoltage(&prev_filtered_voltage);

	GetHeatersActiveMode();
	eps_set_battary_heater_mode(heaters_active_mode);

	init_linger_thrs();

	EPS_Conditioning();

	return 0;
}

int eps_i2c_comm_get_config(uint16_t param, int write_sz, int read_sz){
	//STID IVID 0x84 BID 0x0A 0x48 read only
	I2Ctransfer tr = { 0 };
	tr.slaveAddress = EPS_I2C_ADDR;
	unsigned char i2c_write_data[] = { STID, IVID, GET_CONF_W, BID , ((unsigned char *)&param)[0], ((unsigned char *)&param)[1] };
	//unsigned char i2c_write_data[] = { STID, IVID, GET_CONF_W, BID , 0x00, 0x48 };
	tr.writeData = i2c_write_data;
	tr.writeSize = write_sz;
	tr.readData = malloc(read_sz);
	tr.readSize = read_sz;
	tr.writeReadDelay = 20;
	int err = i2c_write_read(&tr);
	if (err)
		return err;

	uint16_t val;
	if (read_sz - 8 == 1)
		val = *(tr.readData+8);
	else if (read_sz - 8 == 2)
		val = byte_arr_2_int16(tr.readData+8, 2);

	printf("val=%d\n", val);
	free(tr.readData);
	return err;
}

int eps_i2c_comm(CC, write_sz, read_sz){
	//STID IVID 0x84 BID 0x0A 0x48
	I2Ctransfer tr = { 0 };
	tr.slaveAddress = EPS_I2C_ADDR;
	unsigned char i2c_write_data[] = { STID, IVID, CC, BID };
	tr.writeData = i2c_write_data;
	tr.writeSize = write_sz;
	tr.readData = malloc(read_sz);
	tr.readSize = read_sz;
	tr.writeReadDelay = 20;
	int res = i2c_write_read(&tr);
	free(tr.readData);
	return res;
}


int eps_set_channels_on(isismepsv2_ivid5_piu__eps_channel_t channel){
	isismepsv2_ivid5_piu__replyheader_t response;
	int err = isismepsv2_ivid5_piu__outputbuschannelon(0, channel, &response);
	if (err)
		logg(error, "E:eps_set_5v_channels_on=%d failed\n", channel);
	return err;
}

int eps_set_channels_off(isismepsv2_ivid5_piu__eps_channel_t channel){
	isismepsv2_ivid5_piu__replyheader_t response;
	int err = isismepsv2_ivid5_piu__outputbuschanneloff(0, channel, &response);
	if (err)
		logg(error, "E:eps_set_5v_channels_on=%d failed\n", channel);
	return err;
}

int eps_get_channel_state(uint16_t *stat_obc_on){
	int err = 0;
	isismepsv2_ivid5_piu__gethousekeepingeng__from_t tlm_mb_eng;
	err = isismepsv2_ivid5_piu__gethousekeepingeng(EPS_I2C_BUS_INDEX, &tlm_mb_eng);
	if(err)
		logg(error, "E=%d eps_get_channel_state\n", err);
	else
		*stat_obc_on = tlm_mb_eng.fields.stat_obc_on;
	return err;
}

int eps_check_channel_state(uint16_t channel, Boolean *status){
	uint16_t stat_obc_on = 0;
	int err = eps_get_channel_state(&stat_obc_on);
	if (!err)
		*status = (stat_obc_on & channel) == channel;
	return err;
}

void eps_set_battary_heater_trh(int16_t low, int16_t high){
	eps_i2c_comm_set_config(LOTHR_BP1_HEATER, (unsigned char *)&low, sizeof(int16_t), SET_CONF_SIZE_W_16, SET_CONF_SIZE_W_16 + 2);
	eps_i2c_comm_set_config(HITHR_BP1_HEATER, (unsigned char *)&high, sizeof(int16_t), SET_CONF_SIZE_W_16, SET_CONF_SIZE_W_16 + 2);
}

void RUN_EPS_I2C_COMM(){
	//eps_i2c_comm(GET_SYSTEM_STATUS_W, GET_SYSTEM_STATUS_SIZE_W, GET_SYSTEM_STATUS_SIZE_R);
	//eps_i2c_comm_config(TTC_I2C_SLAVE_ADDR, GET_CONF_SIZE_W, GET_CONF_SIZE_R);
	//eps_i2c_comm_get_config(SAFETY_VOLT_LOTHR, GET_CONF_SIZE_W, GET_CONF_SIZE_R_16);
	//eps_i2c_comm_get_config(SAFETY_VOLT_HITHR, GET_CONF_SIZE_W, GET_CONF_SIZE_R_16);

	eps_set_battary_heater_trh(2200, 2400);
	eps_i2c_comm_get_config(LOTHR_BP1_HEATER, GET_CONF_SIZE_W, GET_CONF_SIZE_R_16);
	eps_i2c_comm_get_config(HITHR_BP1_HEATER, GET_CONF_SIZE_W, GET_CONF_SIZE_R_16);

	eps_set_battary_heater_mode(1);
	eps_i2c_comm_get_config(AUTO_HEAT_ENA_BP1, GET_CONF_SIZE_W, GET_CONF_SIZE_R_8);
}

void RUN_EPS_CHAN_STAT(){
	uint16_t channel = 0x0010;//isismepsv2_ivid5_piu__eps_channel__channel_5v_sw3
	//uint16_t channel = 0x0020;//isismepsv2_ivid5_piu__eps_channel__channel_5v_sw2
	Boolean status = FALSE;
	eps_set_channels_off(isismepsv2_ivid5_piu__eps_channel__channel_5v_sw3);
	eps_check_channel_state(channel, &status);
	printf("status=%d\n", status);//status == 0

	eps_set_channels_on(isismepsv2_ivid5_piu__eps_channel__channel_5v_sw3);
	eps_check_channel_state(channel, &status);
	printf("status=%d\n", status); //status == 1
}

#define GetFilterdVoltage(curr_voltage) (voltage_t) (alpha * curr_voltage + (1 - alpha) * prev_filtered_voltage)

int EPS_Conditioning()
{
	voltage_t curr_voltage = 0;
	GetBatteryVoltage(&curr_voltage);
	if (curr_voltage < eps_threshold_voltages.fields.Vdown_safe) {
			EnterCriticalMode(curr_voltage);
	}
	else if (curr_voltage < eps_threshold_voltages.fields.Vdown_cruise) {
			EnterSafeMode(curr_voltage);
	}
	else if (curr_voltage < eps_threshold_voltages.fields.Vdown_full) {
			EnterCruiseMode(curr_voltage);
	}
	else {
		EnterFullMode(curr_voltage);
	}
}

int GetBatteryVoltage(voltage_t *vbatt)
{
	int err = 0;
	isismepsv2_ivid5_piu__gethousekeepingeng__from_t hk_tlm;
	err = isismepsv2_ivid5_piu__gethousekeepingeng( EPS_I2C_BUS_INDEX,  &hk_tlm );
	*vbatt = hk_tlm.fields.volt_vd0;
	return err;
}

int UpdateAlpha(float new_alpha)
{
	if (new_alpha > 1 || new_alpha < 0) {
		return -2;
	}
	int err = FRAM_write((unsigned char*) &new_alpha,
			EPS_ALPHA_FILTER_VALUE_ADDR, EPS_ALPHA_FILTER_VALUE_SIZE);
	if (0 != err) {
		return err;
	}

	alpha = new_alpha;
	return 0;
}

int UpdateThresholdVoltages(EpsThreshVolt_t *thresh_volts)
{
	if (NULL == thresh_volts) {
		return E_INPUT_POINTER_NULL;
	}

	Boolean valid_dependancies = (thresh_volts->fields.Vup_safe 	< thresh_volts->fields.Vup_cruise
	                           && thresh_volts->fields.Vup_cruise	< thresh_volts->fields.Vup_full);

	Boolean valid_regions = (thresh_volts->fields.Vdown_full 	< thresh_volts->fields.Vup_full)
						&&  (thresh_volts->fields.Vdown_cruise	< thresh_volts->fields.Vup_cruise)
						&&  (thresh_volts->fields.Vdown_safe	< thresh_volts->fields.Vup_safe);

	if (!(valid_dependancies && valid_regions)) {
		return -2;
	}
	int err = FRAM_write((unsigned char*) thresh_volts,
			EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE);
	if (0 != err) {
		return err;
	}
	memcpy(eps_threshold_voltages.raw, thresh_volts, EPS_THRESH_VOLTAGES_SIZE);
	return E_NO_SS_ERR;
}

int GetThresholdVoltages(EpsThreshVolt_t *thresh_volts)
{
	if (NULL == thresh_volts) {
		return E_INPUT_POINTER_NULL;
	}
	int err = FRAM_read((unsigned char*) thresh_volts, EPS_THRESH_VOLTAGES_ADDR,
			EPS_THRESH_VOLTAGES_SIZE);
	return err;
}

int GetAlpha(float *alpha)
{
	if (NULL == alpha) {
		return E_INPUT_POINTER_NULL;
	}
	int err = FRAM_read((unsigned char*) alpha, EPS_ALPHA_FILTER_VALUE_ADDR,
			EPS_ALPHA_FILTER_VALUE_SIZE);
	return err;
}

int RestoreDefaultAlpha()
{
	int err = 0;
	float def_alpha = DEFAULT_ALPHA_VALUE;
	err = UpdateAlpha(def_alpha);
	return err;
}

int RestoreDefaultThresholdVoltages()
{
	int err = 0;
	EpsThreshVolt_t def_thresh =
	{.raw = DEFAULT_EPS_THRESHOLD_VOLTAGES};
	err = UpdateThresholdVoltages(&def_thresh);
	return err;
}
