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
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include <satellite-subsystems/IsisAntS.h>
#include "TLM_management.h"

#ifdef GOMEPS
	#include <satellite-subsystems/GomEPS.h>
#endif
#ifdef ISISEPS
	#include <satellite-subsystems/isis_eps_driver.h>
#endif
#define I2c_SPEED_Hz 100000
#define I2c_Timeout 10
#define I2c_TimeoutTest portMAX_DELAY
#define PRINT_IF_ERR(method) if(0 != err)printf("error in '" #method  "' err = %d\n",err);

//האם זו האינטרקציה הראשונה
Boolean isFirstActivation()
{
	unsigned char FirstActivation=0;
	int res=0;
	res = FRAM_read(&FirstActivation,FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE );
	 if (res==-2)
	 {
		 printf(" specified address and size are out of range of the FRAM space");
	 }
	 if  (res==-1)
	 {
		 printf(" obtaining lock for FRAM access fails");
	 }

	 if (FirstActivation==1)
	 {
		 return TRUE;
	 }
	 else
	 {
		 return FALSE;
	 }
}

//בדיקה שעברו 30 דק מהשיגור. טיפול במקרה שיש אתחול חוזר של המערכת
void firstActivationProcedure()
{
	int err = 0;
	const int TotalWaitTime = 1000 * 60 * 30 / portTICK_RATE_MS;
	int AwaitedTime = 0;
	err = FRAM_read ((unsigned char *)&AwaitedTime ,MOST_UPDATED_SAT_TIME_ADDR , MOST_UPDATED_SAT_TIME_SIZE	 );
	if (!err)
	{
		while (TotalWaitTime>AwaitedTime)
		{

			vTaskDelay(1000*10);
			AwaitedTime += 1000*10;
			FRAM_write(&AwaitedTime , MOST_UPDATED_SAT_TIME_ADDR , MOST_UPDATED_SAT_TIME_SIZE);
			TelemetryCollectorLogic();
		}
	}
}

	//שמירת ערכי ברירת מחדל בזיכרון.
void WriteDefaultValuesToFRAM()
{
	int DefNoCom=DEFAULT_NO_COMM_WDT_KICK_TIME;
	FRAM_write(&DefNoCom, NO_COMM_WDT_KICK_TIME_ADDR,sizeof(DefNoCom));
	int NumberVoltages=NUMBER_OF_THRESHOLD_VOLTAGES;
	FRAM_write(&NumberVoltages,EPS_THRESH_VOLTAGES_ADDR,EPS_THRESH_VOLTAGES_SIZE);
	int alpha=DEFAULT_ALPHA_VALUE;
	FRAM_write(&alpha,EPS_ALPHA_FILTER_VALUE_ADDR,EPS_ALPHA_FILTER_VALUE_SIZE);
	int eps= DEFAULT_EPS_SAVE_TLM_TIME;
	FRAM_write(&eps,EPS_SAVE_TLM_PERIOD_ADDR,sizeof(eps));
	int trxvu=DEFAULT_TRXVU_SAVE_TLM_TIME;
	FRAM_write(&trxvu,TRXVU_SAVE_TLM_PERIOD_ADDR,sizeof(trxvu));
	int ant=DEFAULT_ANT_SAVE_TLM_TIME;
	FRAM_write(&ant, ANT_SAVE_TLM_PERIOD_ADDR,sizeof(ant));
	int solar=DEFAULT_SOLAR_SAVE_TLM_TIME;
	FRAM_write(&solar,SOLAR_SAVE_TLM_PERIOD_ADDR,sizeof(solar));
	int wod=DEFAULT_WOD_SAVE_TLM_TIME;
	FRAM_write(&wod,WOD_SAVE_TLM_PERIOD_ADDR,sizeof(wod));
	//TODO:FRAM_write(add beacon);
}

	//אתחול ה FRAM
int StartFRAM()
{
	int result=0;
	result=FRAM_start();
	if(-1==result)
	{
		printf("failed to creaT semaforeS");}

	else if(-2==result)

		printf("failed to initializing SPI");

	return result;
}

	// i2c אתחול ה
int StartI2C()
{
	int result=0;
	result=I2C_start(I2c_Timeout,I2c_Timeout);
	if(result==-3)
	{
		printf("the driver uses a timeout of 1");
	}
	else if(result==-2)
	{
		printf("TWI peripheral fails");
	}
	else if(result==-1)
	{
		printf("creating the task that consumes I2C transfer requests failed");
	}

	if(result==0)
	{
		printf("success");}
		return result;
}

	//spI אתחול ה
int StartSPI()
{

	int result= 0;
	result = SPI_start(bus1_spi, slave1_spi);

	if(result==0)
	{
		printf("success");
	}
	else if(result==-1)
	{
		printf("error");
	}
	return result;
}

	//אתחול השעון של הלווין
int StartTIME()
{
	int err = 0;
	Time expected_deploy_time = UNIX_DEPLOY_DATE_JAN_D1_Y2020;
	err = Time_start(&expected_deploy_time, 0);
	if (0 != err)
	{
		return err;
	}

	time_unix time_before_wakeup = 0;
	if (!isFirstActivation())
	{
		FRAM_read((unsigned char*) &time_before_wakeup,
				MOST_UPDATED_SAT_TIME_ADDR, MOST_UPDATED_SAT_TIME_SIZE);
		Time_setUnixEpoch(time_before_wakeup);
	}
	return 0;
}

	// פריסת אנטנות לאחר 30 דק שקט
int DeploySystem()
{
#ifdef TESTING
		printf(" DeploySystem here");
#endif
	if(isFirstActivation())
	{
		firstActivationProcedure();

		unsigned int deployTime;
		Time_getUnixEpoch(&deployTime);
		FRAM_write((unsigned char *)&deployTime, DEPLOYMENT_TIME_ADDR, DEPLOYMENT_TIME_SIZE);

		ISISantsI2Caddress addressab;
		addressab.addressSideA=0;//TODO: take from SysI2CAddr.h
		addressab.addressSideB=0;//TODO: take from SysI2CAddr.h
		int res=IsisAntS_initialize( &addressab, 1);//לוודא שיש מערכת 1

		if(res==0)
		{
			res=IsisAntS_autoDeployment(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideA, 10);//TODO: define interval
		}

		if(res==0)
			res = IsisAntS_autoDeployment(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideB, 10);//TODO: define interval

		if(res==0)
		{
			Boolean firstactivation= FALSE;
			FRAM_write((unsigned char *)&firstactivation, FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE );
		}
	}

	return 0;
}

	//אתחול כלל המערכות
int InitSubsystems()
{

	int err;
	err = StartSPI();
	if (err!=0)
		return err;

	err = StartI2C();
	if (err!=0)
		return err;

	err = StartFRAM();
	if (err!=0)
		return err;

	WriteDefaultValuesToFRAM();

	err = StartTIME();
	if (err!=0)
		return err;

	err=InitTrxvu();
	if (err!=0)
		return err;

	err=InitializeFS(TRUE);//TODO:set first time properly
	if (err!=0)
		return err;

	err=EPS_Init();
	if (err!=0)

		return err;

	err = DeploySystem();
	if (err!=0)
		return err;

	return 0;
}
