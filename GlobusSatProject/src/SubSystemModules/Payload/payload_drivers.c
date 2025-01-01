#include "payload_drivers.h"
#include "hal/Drivers/I2C.h"
#include "SysI2CAddr.h"
#include <string.h>
#include <hal/Timing/Time.h>
#include <satellite-subsystems/isismepsv2_ivid5_piu.h>
#include "FRAM_FlightParameters.h"
#include "SubSystemModules/PowerManagment/EPSOperationModes.h"
#include "SubSystemModules/Maintenance/Log.h"

#define CLEAR_WDT 0x3F
#define SOFT_RESET 0xF8
#define READ_RADFET_VOLTAGES 0x33
#define MEASURE_TEMPERATURE 0x77
#define READ_PIC32_SEU 0x47
#define READ_PIC32_SEL 0x66
#define TIMEOUT 4000
#define READ_DELAY 200

#define EPS_INDEX 0
#define PAYLOAD_BUS_CHANNEL 4

// Macro to convert endianess
#define CHANGE_ENDIAN(x) ((x) = ((x) >> 24 & 0xff) | ((x) << 8 & 0xff0000) | ((x) >> 8 & 0xff00) | ((x) << 24 & 0xff000000))


SoreqResult payloadInit(Boolean check_allowed_on_init) {

	if (!check_allowed_on_init)
		 return payloadTurnOn();

	Boolean turn_on_payload_in_init;
	int err = FRAM_read((unsigned char*) &turn_on_payload_in_init, TURN_ON_PAYLOAD_IN_INIT, TURN_ON_PAYLOAD_IN_INIT_SIZE);
	if (err){
		logg(error, "E: %d FRAMread failed\n", err);
		return err;
	}


	if(turn_on_payload_in_init) {
		err = payloadTurnOn();
		if (err)
				logg(error, "E:%d Failed in payloadInit\n", err);
		else
			logg(event, "V: payloadInit was successful\n");
	} else
			logg(event, "v: %d not to turn on payload during INIT\n", TURN_ON_PAYLOAD_IN_INIT);
    return err;
}

SoreqResult payloadRead(int size, unsigned char *buffer) {

    unsigned char wtd_and_read[] = {CLEAR_WDT};
    for (int i = 0; i < TIMEOUT / READ_DELAY; ++i) {
        if (I2C_write(PAYLOAD_I2C_ADDR, wtd_and_read, 1))
        	return PAYLOAD_I2C_WRITE_ERROR;

        vTaskDelay(READ_DELAY);

        if (!I2C_read(PAYLOAD_I2C_ADDR, buffer, size) && buffer[3] == 0)
        	return PAYLOAD_SUCCESS;
    }
    return PAYLOAD_TIMEOUT;
}

SoreqResult payloadSendCommand(char opcode, int size, unsigned char *buffer, int delay) {

	if (I2C_write(PAYLOAD_I2C_ADDR, (unsigned char *)&opcode, 1))
    	return PAYLOAD_I2C_WRITE_ERROR;
    vTaskDelay(delay);
    return payloadRead(size, buffer);
}


#define ADC_TO_VOLTAGE(R) ((2 * 4.096 * (R)) / (2 << 23))
#define VOLTAGE_TO_TEMPERATURE(V) (100 * ((V) * (5 / 2.0) - 2.73))

SoreqResult payloadReadEnvironment(PayloadEnvironmentData *environment_data) {
    unsigned char buffer[12];
    SoreqResult res;

    // Read RADFET voltages
    res = payloadSendCommand(READ_RADFET_VOLTAGES, sizeof(buffer), buffer, 1250 / portTICK_RATE_MS);
    if (res != PAYLOAD_SUCCESS)
        return res;

    memcpy(&environment_data->adc_conversion_radfet1, buffer + 4, 4);
    memcpy(&environment_data->adc_conversion_radfet2, buffer + 8, 4);
    CHANGE_ENDIAN(environment_data->adc_conversion_radfet1);
    CHANGE_ENDIAN(environment_data->adc_conversion_radfet2);

    // Read temperature ADC value
    int raw_temperature_adc;
    res = payloadSendCommand(MEASURE_TEMPERATURE, sizeof(buffer), buffer, 845 / portTICK_RATE_MS);
    if (res)
        return res;

    memcpy(&raw_temperature_adc, buffer + 4, 4);
    CHANGE_ENDIAN(raw_temperature_adc);

    // Extract and process ADC value
    int remove_extra_bits = (raw_temperature_adc & (~(1 << 29))) >> 5; // Mask and shift to remove redundant bits
    float voltage = ADC_TO_VOLTAGE(remove_extra_bits); // Convert ADC value to voltage
    float temperature = VOLTAGE_TO_TEMPERATURE(voltage); // Convert voltage to temperature
    environment_data->temperature = temperature;

    return PAYLOAD_SUCCESS;
}


SoreqResult payloadReadEvents(PayloadEventData *event_data) {
    unsigned char buffer[12];
    SoreqResult res;

    // Read SEL count
    res = payloadSendCommand(READ_PIC32_SEL, sizeof(buffer), buffer, 10 / portTICK_RATE_MS);
    if (res)
        return res;
    memcpy(&event_data->sel_count, buffer + 4, 4);
    if (event_data->sel_count == 0) memcpy(&event_data->sel_count, buffer + 8, 4);
    CHANGE_ENDIAN(event_data->sel_count);

    // Read SEU count
    res = payloadSendCommand(READ_PIC32_SEU, sizeof(buffer), buffer, 100 / portTICK_RATE_MS);
    if (res)
        return res;
    memcpy(&event_data->seu_count, buffer + 4, 4);
    CHANGE_ENDIAN(event_data->seu_count);

    int payload_turn_off_by_command;
    res = FRAM_read((unsigned char*)&payload_turn_off_by_command, PAYLOAD_TURN_OFF_BY_COMMAND, PAYLOAD_TURN_OFF_BY_COMMAND_SIZE);
    if(res)
    	return res;

    event_data ->payload_turn_off_by_command = payload_turn_off_by_command;

    int sat_number_of_resets;
    res = FRAM_read((unsigned char*)&sat_number_of_resets, NUMBER_OF_RESETS_ADDR, NUMBER_OF_RESETS_SIZE);
    if(res)
    	return res;
    event_data ->sat_number_of_resets = sat_number_of_resets;

    return PAYLOAD_SUCCESS;
}

SoreqResult payloadSoftReset() {
	SoreqResult res = payloadSendCommand(SOFT_RESET, 0, NULL, 0);
	if (!res)
		payloadCommandRestartCount();
    return res;
}

void payloadCommandRestartCount() {
	unsigned int reset_cnt = 0;
	int res = FRAM_read((unsigned char*)&reset_cnt, PAYLOAD_TURN_OFF_BY_COMMAND, PAYLOAD_TURN_OFF_BY_COMMAND_SIZE);
	if(!res) {
		reset_cnt++;
		FRAM_write((unsigned char*)&reset_cnt, PAYLOAD_TURN_OFF_BY_COMMAND, PAYLOAD_TURN_OFF_BY_COMMAND_SIZE);
	}
}

SoreqResult payloadTurnOff() {
	isismepsv2_ivid5_piu__replyheader_t response;
    int res =  isismepsv2_ivid5_piu__outputbuschanneloff(EPS_INDEX, PAYLOAD_BUS_CHANNEL, &response);
    if(!res){
    	Boolean flag = FALSE;
    	FRAM_write((unsigned char*)&flag, PAYLOAD_ON, PAYLOAD_ON_SIZE);
    	payloadCommandRestartCount();
    }
    return res;
}

SoreqResult payloadTurnOn() {
	int res = 1;
	if(GetEPSSystemState() == FullMode) {
		res = eps_set_channels_on(isismepsv2_ivid5_piu__eps_channel__channel_5v_sw3);
		if(!res) {
			Boolean flag = TRUE;
			FRAM_read((unsigned char*)&flag, PAYLOAD_ON, PAYLOAD_ON_SIZE);
		}
	} else
		logg(error, "E:notinFullModeT=%d\n", res);
	return res;
}



