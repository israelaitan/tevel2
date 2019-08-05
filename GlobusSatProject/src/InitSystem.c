#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/SPI.h>
#include <hal/Timing/Time.h>
#include <hal/errors.h>
#include <satellite-subsystems/IsisAntS.h>
#include <at91/utility/exithandler.h>
#include <string.h>
#include "GlobalStandards.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "InitSystem.h"
#include "TLM_management.h"
#include <hal/Storage/FRAM.h>

#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif
#ifdef ISISEPS
	#include <satellite-subsystems/IsisEPS.h>
#endif
#define I2c_SPEED_Hz 100000
#define I2c_Timeout 10
#define I2c_TimeoutTest portMAX_DELAY
#define TIME_Sync_Interval 0
#define TIME_SATELLITE_LAUNCH_TIME {\
		.seconds = 0,\
		.minutes = 35,\
		.hours = 13,\
		.day = 4,\
		.date = 1,\
		.month = 8,\
		.year = 19,\
		.secondsOfYear = 0 }
#define ANTS_SATELLITE_ADDR {\
		.addressSideA = ANTS_I2C_ADDR_SIDE_A,\
		.addressSideB = ANTS_I2C_ADDR_SIDE_B }
#define PRINT_IF_ERR(method) if(0 != err)printf("error in '" #method  "' err = %d\n",err);
#define Deployment_Time 10
#define N_MAX_DEPLOY_ATTEMPT 3
#define N_ANTS_PER_SIDE 2

Boolean isFirstActivation()//not good what about error?
{
	int activated;
	int err = FRAM_read(&activated, FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE);
	if (!err && activated == 1)
		return TRUE;
	return FALSE;
}

void firstActivationProcedure()
{
	//do it every minute instead and write to fram e.g. to avoid restart every 15 minute
	// collect telemetry also every 30 sec app
	const portTickType xDelay = 1000 * 60 * 30 / portTICK_RATE_MS;
	vTaskDelay( xDelay );
	//deploy ants
	ISISantsI2Caddress antsAddr = ANTS_SATELLITE_ADDR;
	int err = IsisAntS_initialize(&antsAddr, N_ANTS_PER_SIDE);
	//in practice it will continue until ground says its ok
	for (int i = 0; i < N_MAX_DEPLOY_ATTEMPT; i++) {
		IsisAntS_autoDeployment(isisants_antenna1, isisants_sideA, Deployment_Time);
		IsisAntS_autoDeployment(isisants_antenna2, isisants_sideA, Deployment_Time);
		IsisAntS_autoDeployment(isisants_antenna1, isisants_sideB, Deployment_Time);
		IsisAntS_autoDeployment(isisants_antenna2, isisants_sideB, Deployment_Time);
	}

	//fram write deploy time
	unsigned int epochTime;
	int err = Time_getUnixEpoch(&epochTime);
	err = FRAM_write(&epochTime, DEPLOYMENT_TIME_ADDR, DEPLOYMENT_TIME_SIZE);
	int firstActivation = 0;
	err = FRAM_write(&firstActivation, FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE);
}

void WriteDefaultValuesToFRAM()
{

}

int StartFRAM()
{
	int err = FRAM_start();
	PRINT_IF_ERR(StartFRAM);
	return err;
}

int StartI2C()
{
	int err = I2C_start(I2c_SPEED_Hz, I2c_Timeout);
	PRINT_IF_ERR(StartI2C);
	return err;
}

int StartSPI()
{
	int err = SPI_start(bus1_spi, slave1_spi);
	PRINT_IF_ERR(StartSPI);
	return err;
}

int StartTIME()
{
	Time curr_time = TIME_SATELLITE_LAUNCH_TIME;
	int err = TIME_start(&curr_time, TIME_Sync_Interval);
	PRINT_IF_ERR(StartTIME);
	if (!err && !isFirstActivation()) {
		time_unix time_before_restart = 0;
		err = FRAM_read(&time_before_restart, MOST_UPDATED_SAT_TIME_ADDR, MOST_UPDATED_SAT_TIME_SIZE);
		if (!err)
			err = Time_setUnixEpoch(time_before_restart);
	}
	return err;
}

int DeploySystem()
{
	if (isFirstActivation()) {
		firstActivationProcedure();
	}
	return 0;
}

int InitSubsystems()
{
	if (StartFRAM())
		return -1;
	if (WriteDefaultValuesToFRAM())
			return -1;
	if (StartTIME())
			return -1;
	if (StartSPI())
			return -1;
	if (StartI2C())
			return -1;
	if (isFirstActivation())
		DeploySystem();
	return 0;
}

