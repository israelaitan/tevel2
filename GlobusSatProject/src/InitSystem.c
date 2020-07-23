#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/SPI.h>
#include <hal/Timing/Time.h>
#include <at91/utility/exithandler.h>
#include <string.h>
#include "GlobalStandards.h"
#include "InitSystem.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Maintenance/Log.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Housekepping/DUMP.h"
#include <satellite-subsystems/IsisAntS.h>
#include "TLM_management.h"
#include <satellite-subsystems/isis_eps_driver.h>

//TODO change it after testing to 30.
#define ANT_AWAITED_TIME_MIN 4
#define I2c_Timeout 10
#define I2c_SPEED_Hz 100000
#define I2c_TimeoutTest portMAX_DELAY
#define ANTENNA_DEPLOYMENT_TIMEOUT 10 //<! in seconds
#define PRINT_IF_ERR(method) if(0 != err)printf("error in '" #method  "' err = %d\n",err);

//האם זו האינטרקציה הראשונה
Boolean isFirstActivation()
{
	unsigned char FirstActivation=0;
	int res=0;

	//TODO: remove print after testing
	logg(OBCInfo, "I:Inside isFirstActivation()\n");

	res = FRAM_read(&FirstActivation,FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE );
	if (res==-2)
	{
		logg(error, "E: specified address and size are out of range of the FRAM space\n");
	}
	if  (res==-1)
	{
		logg(error, "E:  obtaining lock for FRAM access fails\n");
	}

	if (FirstActivation==1)
	{
		logg(OBCInfo, "I:Inside isFirstActivation() return TRUE\n");
		return TRUE;
	}
	else
	{
		logg(OBCInfo, "I:Inside isFirstActivation() - %c return FAlSE\n", FirstActivation);
		return FALSE;
	}
}

//בדיקה שעברו 30 דק מהשיגור. טיפול במקרה שיש אתחול חוזר של המערכת
void firstActivationProcedure()
{
	int err = 0;
	const int TotalWaitTime = 1000 * 60 * ANT_AWAITED_TIME_MIN;
	int AwaitedTime = 0;
	err = FRAM_read ((unsigned char *)&AwaitedTime ,SECONDS_SINCE_DEPLOY_ADDR,SECONDS_SINCE_DEPLOY_SIZE);
	if (err!=0)
	{
		logg(error, "E:could not read SECONDS_SINCE_DEPLOY from FRAM\n");
	}

	int i = 1;
	while (TotalWaitTime>AwaitedTime)
	{
		logg(OBCInfo, "I:%d Total awaited time is: %d seconds\n", i++, AwaitedTime/1000 );
		vTaskDelay(1000*10);

		AwaitedTime += 1000*10;
		FRAM_write((unsigned char*)&AwaitedTime ,SECONDS_SINCE_DEPLOY_ADDR,SECONDS_SINCE_DEPLOY_SIZE);
		TelemetryCollectorLogic();
	}

	//write default parms to fram.
	WriteDefaultValuesToFRAM();

	//set deploment time in FRAM
	unsigned int deployTime;
	Time_getUnixEpoch(&deployTime);
	FRAM_write((unsigned char *)&deployTime, DEPLOYMENT_TIME_ADDR, DEPLOYMENT_TIME_SIZE);

	//update first activation flag to false if antenas are not Connected
	char firstactivation= 0;
	FRAM_write((unsigned char *)&firstactivation, FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE );
	logg(OBCInfo, "I:*****First Activation without Antenas deployed******\n");

}

//שמירת ערכי ברירת מחדל בזיכרון.
void WriteDefaultValuesToFRAM()
{
	//TODO: remove print after testing
	logg(OBCInfo, "I:Inside WriteDefaultValuesToFRAM()\n");
	int DefNoCom=DEFAULT_NO_COMM_WDT_KICK_TIME;
	FRAM_write((unsigned char*)&DefNoCom, NO_COMM_WDT_KICK_TIME_ADDR,sizeof(DefNoCom));
	EpsThreshVolt_t thresh_volts = { DEFAULT_EPS_THRESHOLD_VOLTAGES };
	FRAM_write((unsigned char*)&thresh_volts, EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE);
	float alpha = DEFAULT_ALPHA_VALUE;
	FRAM_write((unsigned char*)&alpha,EPS_ALPHA_FILTER_VALUE_ADDR,EPS_ALPHA_FILTER_VALUE_SIZE);
	int eps= DEFAULT_EPS_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&eps,EPS_SAVE_TLM_PERIOD_ADDR,sizeof(eps));
	int trxvu=DEFAULT_TRXVU_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&trxvu,TRXVU_SAVE_TLM_PERIOD_ADDR,sizeof(trxvu));
	int ant=DEFAULT_ANT_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&ant, ANT_SAVE_TLM_PERIOD_ADDR,sizeof(ant));
	int solar=DEFAULT_SOLAR_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&solar,SOLAR_SAVE_TLM_PERIOD_ADDR,sizeof(solar));
	int wod=DEFAULT_WOD_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&wod,WOD_SAVE_TLM_PERIOD_ADDR,sizeof(wod));
	int logg = DEFAULT_LOG_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&logg, LOG_SAVE_TLM_PERIOD_ADDR, sizeof(logg));
	int beacon = DEFAULT_BEACON_INTERVAL_TIME;
	FRAM_write((unsigned char*)&beacon,BEACON_INTERVAL_TIME_ADDR ,BEACON_INTERVAL_TIME_SIZE);
	unsigned short resets = 0;
	FRAM_write((unsigned char*)&resets,NUMBER_OF_RESETS_ADDR ,NUMBER_OF_RESETS_SIZE);
	unsigned char reset_flag = FALSE_8BIT;
	FRAM_write(&reset_flag, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE);
}

	//אתחול ה FRAM
int StartFRAM()
{
	int result=0;
	logg(OBCInfo, "I:Inside StartFRAM()\n");
	result=FRAM_start();
	if(-1==result)
	{
		logg(error, "E:failed to creaT semaforeS\n");
	}
	else if(-2==result)
	{
		logg(error, "E:failed to initializing SPI\n");
	}
	else
	{
		logg(OBCInfo, "I:FRAM_start was successful\n");
	}

	return result;
}

	// i2c אתחול ה
int StartI2C()
{
	int result=0;

	//TODO: remove after finish testing
	logg(OBCInfo, "I:Inside StartI2C() - calling I2C_start driver\n");

	result=I2C_start(I2c_SPEED_Hz, I2c_Timeout);
	if(result==-3)
	{
		logg(error, "E:the driver uses a timeout of 1\n");
	}
	else if(result==-2)
	{
		logg(error, "E:TWI peripheral fails\n");
	}
	else if(result==-1)
	{
		logg(error, "E:creating the task that consumes I2C transfer requests failed\n");
	}

	if(result==0)
	{
		logg(OBCInfo, "I: StartI2C.success\n");
	}

	return result;
}

	//spI אתחול ה
int StartSPI()
{
	int result= 0;
	logg(OBCInfo, "I:Inside StartSPI()\n");
	result = SPI_start(bus1_spi, slave1_spi);

	if(result==0)
	{
		logg(OBCInfo, "I:StartSPI.success\n");
	}
	else if(result==-1)
	{
		logg(error, "E:StartSPI.error\n");
	}
	return result;
}

	//אתחול השעון של הלווין
int StartTIME()
{
	int err = 0;
	logg(OBCInfo, "I:Inside StartTIME()\n");
	Time expected_deploy_time = UNIX_DEPLOY_DATE_JAN_D1_Y2020;
	err = Time_start(&expected_deploy_time, 0);
	if (0 != err)
	{
		return err;
	}

	time_unix time_before_wakeup = 0;
	if (!isFirstActivation())
	{
		logg(OBCInfo, "I:reset clock\n");
		FRAM_read((unsigned char*) &time_before_wakeup,MOST_UPDATED_SAT_TIME_ADDR, MOST_UPDATED_SAT_TIME_SIZE);
		Time_setUnixEpoch(time_before_wakeup);
	}
	return 0;
}

// antena auto deploy both sides
int autoDeploy()
{
	int res=0;
	// antena auto deploy - sides A
	res = IsisAntS_setArmStatus(ANTS_I2C_SIDE_A_ADDR, isisants_sideA, isisants_arm);

	if(res==0)
	{
		logg(OBCInfo, "I:Deploying: Side A\n");
		res=IsisAntS_autoDeployment(ANTS_I2C_SIDE_A_ADDR, isisants_sideA, ANTENNA_DEPLOYMENT_TIMEOUT);
	}
	else
	{
		logg(error, "E:Failed Arming Side A\n");
	}

	// unarm antenas side A
	res = IsisAntS_setArmStatus(ANTS_I2C_SIDE_A_ADDR, isisants_sideA, isisants_disarm);

	// antenata auto deploy - sides B
	res = IsisAntS_setArmStatus(ANTS_I2C_SIDE_B_ADDR, isisants_sideB, isisants_arm);
	if(res==0)
	{
		logg(OBCInfo, "I:Deploying: Side B\n");
		res = IsisAntS_autoDeployment(ANTS_I2C_SIDE_B_ADDR, isisants_sideB, ANTENNA_DEPLOYMENT_TIMEOUT);
	}
	else
	{
		logg(error, "E:Failed Arming Side B\n");
	}

	// unarm antenas side B
	res = IsisAntS_setArmStatus(ANTS_I2C_SIDE_B_ADDR, isisants_sideB, isisants_disarm);

	// update last deploy time
	time_unix deploy_time;
	int err = Time_getUnixEpoch((unsigned int *)&deploy_time);
	if(0 != err)
	{
		logg(OBCInfo, "I:Time_getUnixEpoch failed to set ants last deploy time\n");
	}
	else
	{
		setLastDeploymentTime(deploy_time);
		FRAM_write((unsigned char*)&deploy_time, LAST_ANT_DEP_TIME_ADDR, LAST_ANT_DEP_TIME_SIZE);
	}
	return res;
}

	// פריסת אנטנות לאחר 30 דק שקט
int DeploySystem()
{
	int res=0;

	//TODO: remove print after testing
	logg(OBCInfo, "I: DeploySystem() here\n");

	if(isFirstActivation())
	{

		// waiting for 30 min (collect telemetry every 10 sec)-
		firstActivationProcedure();


		//Initialize the Antenas performed in trx init performing auto deploy
		res = autoDeploy();
	}
	return res;
}


//אתחול כלל המערכות
int InitSubsystems()
{
	//TODO: remove print after testing
	logg(OBCInfo, "I:InitSubsystems()  here\n");

	//TODO: not sure we should stop if something fails
	int err;
	err = StartSPI();
	if (err!=0)
		return err;

	// start the I2C component
	err = StartI2C();
	if (err!=0)
		return err;

	// start the FRAM
	err = StartFRAM();
	if (err!=0)
		return err;

	// Start the clock
	err = StartTIME();
	if (err!=0)
		return err;

	// initialize TRXVU (communication) component
	err=InitTrxvu();
	if (err!=0)
		return err;

	// Initalize the file system
	Boolean firstActivation= isFirstActivation();
	err=InitializeFS(firstActivation);
	if (err!=0)
		return err;

	//Initialize the dump thread (queue and lock)
	err=InitDump();
	if (err!=0)
		return err;

	err = InitTelemetryCollector();
	//TODO: log if error


	// Initialize EPS
	err=EPS_Init();
	if (err!=0)
		return err;

	// Deploy system - open Antetnas
	err = DeploySystem();
	if (err!=0)
		return err;

	return 0;
}
