#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <satellite-subsystems/IsisSolarPanelv2.h>
#include <hal/errors.h>

#include <string.h>

#include "EPS.h"

#include <satellite-subsystems/isis_eps_driver.h>
#include "SubSystemModules/Maintenance/Log.h"


int EPS_Init()
{
	ISIS_EPS_t isis_eps = {EPS_I2C_ADDR};
	int err = ISIS_EPS_Init( &isis_eps, 1 );
	if (err != E_NO_SS_ERR)
		logg(error, "E:EPS init failed\n");

	err = IsisSolarPanelv2_initialize(slave0_spi);
	if (err != 0)
		logg(error, "E:Solar Panel init failed\n");

	//IsisSolarPanelv2_sleep();

	return err;
}


int GetBatteryVoltage(voltage_t *vbatt)
{
	int err = 0;

	isis_eps__gethousekeepingeng__from_t hk_tlm;
	err = isis_eps__gethousekeepingeng__tm( EPS_I2C_BUS_INDEX,  &hk_tlm );
	*vbatt = hk_tlm.fields.batt_input.fields.volt;

	return err;
}

