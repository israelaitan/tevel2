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


#define I2c_Timeout 10
#define I2c_SPEED_Hz 100000
#define I2c_TimeoutTest portMAX_DELAY
#define ANTENNA_DEPLOYMENT_TIMEOUT 30 //<! in seconds


//האם זו האינטרקציה הראשונה
int isFirstActivation(Boolean * status)
{
	unsigned char FirstActivation = 0;

	int res = FRAM_read(&FirstActivation,FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE );
	if (!res && FirstActivation==0)
	{
		*status = FALSE;
	}
	else
	{
		*status = TRUE;
	}

	return res;
}

//בדיקה שעברו 30 דק מהשיגור. טיפול במקרה שיש אתחול חוזר של המערכת
void firstActivationProcedure()
{
	int err = 0;
	const int TotalWaitTime = 1000 * ANT_DEPLOY_WAIT_PERIOD;
	int AwaitedTime = 0;
	err = FRAM_read ((unsigned char *)&AwaitedTime ,SECONDS_SINCE_DEPLOY_ADDR,SECONDS_SINCE_DEPLOY_SIZE);
	if (err!=0)
	{
		logg(error, "E:could not read SECONDS_SINCE_DEPLOY from FRAM\n");
	}

	int i = 1;
	while (TotalWaitTime>AwaitedTime)
	{
		logg(event, "V:%d Total awaited time is: %d s should:%d\n", i++, AwaitedTime/1000, TotalWaitTime/1000);
		vTaskDelay(1000*10);

		AwaitedTime += 1000*10;
		FRAM_write((unsigned char*)&AwaitedTime ,SECONDS_SINCE_DEPLOY_ADDR,SECONDS_SINCE_DEPLOY_SIZE);
		TelemetryCollectorLogic();
	}

	//set deploment time in FRAM
	unsigned int deployTime;
	Time_getUnixEpoch(&deployTime);
	FRAM_write((unsigned char *)&deployTime, LAUNCH_TIME_ADDR, LAUNCH_TIME_SIZE);

	//update first activation flag to false if antennas are not Connected
	char firstactivation= 0;
	FRAM_write((unsigned char *)&firstactivation, FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE );
	logg(OBCInfo, "I:*****First Activation without Antenas deployed******\n");

}

//שמירת ערכי ברירת מחדל בזיכרון.
void WriteDefaultValuesToFRAM()
{
	logg(OBCInfo, "I:Inside WriteDefaultValuesToFRAM()\n");
	int DefNoCom=DEFAULT_NO_COMM_WDT_KICK_TIME;
	FRAM_write((unsigned char*)&DefNoCom, NO_COMM_WDT_KICK_TIME_ADDR,sizeof(DefNoCom));

	int noCom = 0;
	FRAM_write((unsigned char*) &noCom, LAST_COMM_TIME_ADDR, LAST_COMM_TIME_SIZE);

	time_unix eps= DEFAULT_EPS_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&eps,EPS_SAVE_TLM_PERIOD_ADDR,sizeof(eps));
	time_unix trxvu=DEFAULT_TRXVU_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&trxvu,TRXVU_SAVE_TLM_PERIOD_ADDR,sizeof(trxvu));
	time_unix ant=DEFAULT_ANT_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&ant, ANT_SAVE_TLM_PERIOD_ADDR,sizeof(ant));
	time_unix solar=DEFAULT_SOLAR_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&solar,SOLAR_SAVE_TLM_PERIOD_ADDR,sizeof(solar));
	time_unix wod=DEFAULT_WOD_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&wod,WOD_SAVE_TLM_PERIOD_ADDR,sizeof(wod));

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
	return FRAM_start();
}

	// i2c אתחול ה
int StartI2C()
{
	return I2C_start(I2c_SPEED_Hz, I2c_Timeout);
}

	//spI אתחול ה
int StartSPI()
{
	return SPI_start(bus1_spi, slave1_spi);
}

	//אתחול השעון של הלווין
int StartTIME()
{
	Time expected_deploy_time = UNIX_DEPLOY_DATE_JAN_D1_Y2020;
	int err = Time_start(&expected_deploy_time, 0);
	time_unix time_before_wakeup = 0;
	Boolean isFirstA;
	isFirstActivation(&isFirstA);
	if (!isFirstA)
	{
		FRAM_read((unsigned char*) &time_before_wakeup,MOST_UPDATED_SAT_TIME_ADDR, MOST_UPDATED_SAT_TIME_SIZE);
		int setError = Time_setUnixEpoch(time_before_wakeup);
		if (setError)
			logg(error, "E:%d Time_setUnixEpoch\n", setError);
		else
			logg(event, "V: Reset clock with %d\n", time_before_wakeup);
	}

	return err;
}

// antena auto deploy both sides
int autoDeploy()
{
	logg(event, "V: Inside autoDeploy()\n");
	int resArm=0, resDeploy = -1;

	// antena auto deploy - sides A
	resArm = IsisAntS_setArmStatus(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideA, isisants_arm);

	if(resArm==0)
	{
		logg(event, "V:Deploying: Side A\n");
		resDeploy=IsisAntS_autoDeployment(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideA, ANTENNA_DEPLOYMENT_TIMEOUT);
		if (resDeploy!=0)
			logg(event, "V:Failed deploy: Side A res=%d\n", resDeploy);
	}
	else
	{
		logg(error, "E:Failed Arming Side A with error: %d\n", resArm);
	}

	// unarm antenas side A - we decided not to use this method
	//resArm = IsisAntS_setArmStatus(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideA, isisants_disarm);

	// antenata auto deploy - sides B
	resArm = IsisAntS_setArmStatus(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideB, isisants_arm);
	if(resArm==0)
	{
		logg(event, "V:Deploying: Side B\n");
		resDeploy = IsisAntS_autoDeployment(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideB, ANTENNA_DEPLOYMENT_TIMEOUT);
		if (resDeploy!=0)
			logg(event, "V:Failed deploy: Side B res=%d\n", resDeploy);
	}
	else
	{
		logg(error, "E:Failed Arming Side B with error: %d\n", resArm);
	}

	// unarm antenas side B - we decided not to use this method
	//resArm = IsisAntS_setArmStatus(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideB, isisants_disarm);

	// update last deploy time
	time_unix deploy_time;
	int err = Time_getUnixEpoch((unsigned int *)&deploy_time);
	if(0 != err)
	{
		logg(error, "E:Time_getUnixEpoch failed to set ants last deploy time\n");
	}
	else
	{
		setLastAntsAutoDeploymentTime(deploy_time);
		FRAM_write((unsigned char*)&deploy_time, LAST_ANT_DEP_TIME_ADDR, LAST_ANT_DEP_TIME_SIZE);
		logg(event, "V:setLastAntsAutoDeploymentTime success\n");
	}
	return resDeploy;
}

	// פריסת אנטנות לאחר 30 דק שקט
int DeploySystem()
{
	int res=0;

	logg(event, "V: DeploySystem() here\n");
	Boolean isFirstA;
	isFirstActivation(&isFirstA);
	if(isFirstA)
	{

		// waiting for 30 min (collect telemetry every 10 sec)-
		firstActivationProcedure();


		//Initialize the Antenas performed in trx init performing auto deploy
		res = autoDeploy();
	}
	return res;
}



//Init sub system

int InitSubsystems()
{

	//dont logg anythin brfore time init
	int errSPI = StartSPI();
	int errI2C = StartI2C();
	int errFRAM = StartFRAM();
	int errTime = StartTIME();
	Boolean firstActivation;
	int resFirstActivation = isFirstActivation(&firstActivation);
	int errInitFS = InitializeFS(firstActivation);
	//int errInitFS = InitializeFS(1);
	initLog();

	if ( errSPI != 0 )
		logg(error, "E:%d Failed in StartSPI\n", errSPI);
	else
		logg(event, "V: StartSPI() - success\n");
	if ( errI2C != 0 )
		logg(error, "E:%d Failed in StartI2C\n", errI2C);
	else
		logg(event, "V: StartI2C() - success\n");
	if ( errFRAM != 0 )
		logg(error, "E:%d Failed in StartFRAM\n", errFRAM);
	else
		logg(event, "V:FRAM_start was successful\n");
	if ( errTime != 0 )
		logg(error, "E:%d Failed in StartTIME\n", errTime);
	else
		logg(event, "V:StartTIME was successful\n");
	if ( resFirstActivation != 0 )
		logg(error, "E:%d Failed in firstActivation\n", resFirstActivation);
	else
		logg(event, "V: firstActivation was successful\n");
	if ( errInitFS != 0 )
		logg(error, "E:%d Failed in InitializeFS\n", errInitFS);
	else
		logg(event, "V: InitializeFS was successful isFirstActive=%d\n", resFirstActivation);


	// initialize TRXVU (communication) component
	int err=InitTrxvu();
	if (err!=0)
		logg(error, "E:%d Failed in InitTrxvu\n", err);
	else
		logg(event, "V: InitTrxvu was successful\n");

	//Initialize the dump thread (queue and lock)
	err=InitDump();
	if (err!=0)
		logg(error, "E:%d Failed in InitDump\n", err);
	else
		logg(event, "V: InitDump was successful\n");

	err = InitTelemetryCollector();
	if (err!=0)
		logg(error, "E:%d Failed in InitTelemetryCollector\n", err);
	else
		logg(event, "V: InitTelemetryCollector was successful\n");

	EPS_Init();

	// Deploy system - open Antetnas
	err = DeploySystem();
	if (err)
		logg(error, "E:%d Failed in DeploySystem\n", err);
	else
		logg(event, "V: DeploySystem was successful\n");

	return 0;
}
