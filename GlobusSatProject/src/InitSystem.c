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
#include "SubSystemModules/Payload/payload_drivers.h"
#include "TLM_management.h"
#include <satellite-subsystems/isismepsv2_ivid5_piu.h>
#include <satellite-subsystems/IsisSolarPanelv2.h>


#define I2c_Timeout 10
#define I2c_SPEED_Hz 100000
#define I2c_TimeoutTest portMAX_DELAY
#define ANTENNA_DEPLOYMENT_TIMEOUT 30 //<! in seconds

time_unix expected_deploy_time_sec = 0;

#define _PIN_RESET PIN_GPIO08
#define _PIN_INT   PIN_GPIO00

Boolean SolarPanelv2Init(void) {

	IsisSolarPanelv2_Error_t error = ISIS_SOLAR_PANEL_ERR_NONE;
	Pin solarpanelv2_pins[2] = {_PIN_RESET, _PIN_INT};
	error = IsisSolarPanelv2_initialize(slave0_spi, &solarpanelv2_pins[0], &solarpanelv2_pins[1]);
	if(error != ISIS_SOLAR_PANEL_ERR_NONE)
		logg(error, "IsisSolarPaneltest: IsisSolarPanelv2_initialize returned %dn", error);

	error = IsisSolarPanelv2_sleep();
	if(error != ISIS_SOLAR_PANEL_ERR_NONE)
		logg(error, "E:IsisSolarPaneltest: IsisSolarPanelv2_sleep returned %d\n", error);

	return TRUE;
}

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

//בדיקה שעברו  מספר דק מהשיגור. טיפול במקרה שיש אתחול חוזר של המערכת
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
		TelemetryCollectorLogic();//should kick tx and rx implicitly
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

//Initialize method to set FRAM to initial phase before first Init of satelite
 int IntializeFRAM() {
	int err = 0;
	err = StartSPI();
	err = StartI2C();
	err = StartFRAM();
	if(err)
		return err;
	int status = 1;
	FRAM_write((unsigned char*)&status,FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE );

	int a = 0;
	FRAM_write((unsigned char*)&a ,SECONDS_SINCE_DEPLOY_ADDR,SECONDS_SINCE_DEPLOY_SIZE);
	WriteDefaultValuesToFRAM();
	return err;
 }


//שמירת ערכי ברירת מחדל בזיכרון.
void WriteDefaultValuesToFRAM()
{
	unsigned int sat_cmd_id = 0;
	FRAM_write((unsigned char*)&sat_cmd_id, SAT_CMD_ID_ADDR, SAT_CMD_ID_SIZE);

	int DefNoCom=DEFAULT_NO_COMM_WDT_KICK_TIME;
	FRAM_write((unsigned char*)&DefNoCom, NO_COMM_WDT_KICK_TIME_ADDR,sizeof(DefNoCom));

	unsigned int lastWakeUpTime = 0;
	FRAM_write((unsigned char*)&lastWakeUpTime, LAST_WAKEUP_TIME_ADDR, LAST_WAKEUP_TIME_SIZE);

	unsigned int noCom = UNIX_DEPLOY_DATE_JAN_D1_Y2020_SEC;
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

	time_unix pic32 = DEFAULT_PIC32_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&pic32, PIC32_SAVE_TLM_PERIOD_ADDR,sizeof(pic32));
	time_unix radfet = DEFAULT_RADFET_SAVE_TLM_TIME;
	FRAM_write((unsigned char*)&radfet, RADFET_SAVE_TLM_PERIOD_ADDR,sizeof(radfet));

	int beacon = DEFAULT_BEACON_INTERVAL_TIME;
	FRAM_write((unsigned char*)&beacon,BEACON_INTERVAL_TIME_ADDR ,BEACON_INTERVAL_TIME_SIZE);

	unsigned short resets = 0;
	FRAM_write((unsigned char*)&resets,NUMBER_OF_RESETS_ADDR ,NUMBER_OF_RESETS_SIZE);

	unsigned short resets_cmd = 0;
	FRAM_write((unsigned char*) &resets_cmd, NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE);

	unsigned char reset_flag = FALSE_8BIT;
	FRAM_write(&reset_flag, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE);

	//uint16_t rssi = (uint16_t)(__builtin_bswap32(2500) >> 16u);
	uint16_t rssi = 2500;
	FRAM_write((unsigned char*) &rssi, TRANSPONDER_RSSI_ADDR, TRANSPONDER_RSSI_SIZE);

	unsigned int transponder_state = 0;
	FRAM_write((unsigned char*) &transponder_state, TRANSPONDER_STATE_ADDR, TRANSPONDER_STATE_SIZE);

	unsigned int mute_flag = 0;
	FRAM_write((unsigned char*) &mute_flag, MUTE_FLAG_ADRR, MUTE_FLAG_SIZE);

	time_unix mute_end_time = 0;
	FRAM_write((unsigned char*) &mute_end_time, MUTE_ON_END_TIME_ADRR, MUTE_ON_END_TIME_SIZE);

	Boolean turn_on_payload_in_init = FALSE;
	FRAM_write((unsigned char*) &turn_on_payload_in_init, TURN_ON_PAYLOAD_IN_INIT, TURN_ON_PAYLOAD_IN_INIT_SIZE);

	int turn_off_payload_by_command = 0;
	FRAM_write((unsigned char*)&turn_off_payload_by_command, PAYLOAD_TURN_OFF_BY_COMMAND, PAYLOAD_TURN_OFF_BY_COMMAND_SIZE);

	Boolean payload_on = FALSE;
	FRAM_write((unsigned char*)&payload_on, PAYLOAD_ON, PAYLOAD_ON_SIZE);

	float alpha = DEFAULT_ALPHA_VALUE;
	FRAM_write((unsigned char*)&alpha, EPS_ALPHA_FILTER_VALUE_ADDR, sizeof(alpha));
	EpsThreshVolt_t _eps_threshold_voltages = DEFAULT_EPS_THRESHOLD_VOLTAGES;
	FRAM_write((unsigned char*)&_eps_threshold_voltages, EPS_THRESH_VOLTAGES_ADDR, EPS_THRESH_VOLTAGES_SIZE);

	uint8_t tx_bitrate = DEFAULT_BITRATE_VALUE;
	FRAM_write((unsigned char*)&tx_bitrate, TX_BITRATE_ADDR, sizeof(uint8_t));

	unsigned int sel_firstactiv = 446;//5/1/2025 10:48
	FRAM_write((unsigned char*)&sel_firstactiv, FIRST_ACTIV_NUM_PAYLOAD_RESET, FIRST_ACTIV_NUM_PAYLOAD_RESET_SIZE);

	unsigned char heaters_mode = DEFAULT_HEATERS_ACTIVE_MODE;
	FRAM_write((unsigned char*)&heaters_mode, EPS_BAT_HITERRS_ACTIVE_MODE_ADDR, EPS_BAT_HITERRS_ACTIVE_MODE_SIZE);

}

void ReadDefaultValuesToFRAM()
{
	logg(OBCInfo, "I:Inside ReadDefaultValuesToFRAM()\n");
	int DefNoCom=DEFAULT_NO_COMM_WDT_KICK_TIME;
	FRAM_read((unsigned char*)&DefNoCom, NO_COMM_WDT_KICK_TIME_ADDR, NO_COMM_WDT_KICK_TIME_SIZE);
	printf("FRAM NO_COMM_WDT_KICK_TIME_ADDR %d=%d  \n", NO_COMM_WDT_KICK_TIME_ADDR, DefNoCom);

	unsigned int lastWakeUpTime = 0;
	FRAM_read((unsigned char*)&lastWakeUpTime, LAST_WAKEUP_TIME_ADDR, LAST_WAKEUP_TIME_SIZE);
	printf("FRAM LAST_WAKEUP_TIME_ADDR %d=%d  \n", LAST_WAKEUP_TIME_ADDR, lastWakeUpTime);

	unsigned int noCom = UNIX_DEPLOY_DATE_JAN_D1_Y2020_SEC;
	FRAM_read((unsigned char*) &noCom, LAST_COMM_TIME_ADDR, LAST_COMM_TIME_SIZE);
	printf("FRAM LAST_COMM_TIME_ADDR %d=%d  \n", LAST_COMM_TIME_ADDR, noCom);

	time_unix eps= DEFAULT_EPS_SAVE_TLM_TIME;
	FRAM_read((unsigned char*)&eps,EPS_SAVE_TLM_PERIOD_ADDR,sizeof(eps));
	time_unix trxvu=DEFAULT_TRXVU_SAVE_TLM_TIME;
	FRAM_read((unsigned char*)&trxvu,TRXVU_SAVE_TLM_PERIOD_ADDR,sizeof(trxvu));
	time_unix ant=DEFAULT_ANT_SAVE_TLM_TIME;
	FRAM_read((unsigned char*)&ant, ANT_SAVE_TLM_PERIOD_ADDR,sizeof(ant));
	time_unix solar=DEFAULT_SOLAR_SAVE_TLM_TIME;
	FRAM_read((unsigned char*)&solar,SOLAR_SAVE_TLM_PERIOD_ADDR,sizeof(solar));
	time_unix wod=DEFAULT_WOD_SAVE_TLM_TIME;
	FRAM_read((unsigned char*)&wod,WOD_SAVE_TLM_PERIOD_ADDR,sizeof(wod));

	time_unix pic32 = DEFAULT_PIC32_SAVE_TLM_TIME;
	FRAM_read((unsigned char*)&pic32, PIC32_SAVE_TLM_PERIOD_ADDR,sizeof(pic32));
	time_unix radfet = DEFAULT_RADFET_SAVE_TLM_TIME;
	FRAM_read((unsigned char*)&radfet, RADFET_SAVE_TLM_PERIOD_ADDR,sizeof(radfet));

	int beacon = DEFAULT_BEACON_INTERVAL_TIME;
	FRAM_read((unsigned char*)&beacon,BEACON_INTERVAL_TIME_ADDR ,BEACON_INTERVAL_TIME_SIZE);

	unsigned short resets = 0;
	FRAM_read((unsigned char*)&resets,NUMBER_OF_RESETS_ADDR ,NUMBER_OF_RESETS_SIZE);

	unsigned short resets_cmd = 0;
	FRAM_read((unsigned char*) &resets_cmd, NUMBER_OF_CMD_RESETS_ADDR, NUMBER_OF_CMD_RESETS_SIZE);

	unsigned char reset_flag = FALSE_8BIT;
	FRAM_read(&reset_flag, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE);

	unsigned short rssi;
	FRAM_read((unsigned char*) &rssi, TRANSPONDER_RSSI_ADDR, TRANSPONDER_RSSI_SIZE);
	printf("FRAM TRANSPONDER_RSSI_ADDR %d=%d  \n", TRANSPONDER_RSSI_ADDR, rssi);

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
	Time expected_deploy_time = UNIX_DEPLOY_DATE_JAN_D1_Y2025;
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
	resArm = isis_ants__arm(0);

	if(resArm == 0) {
		logg(event, "V:Deploying: Side A\n");
		//resDeploy = isis_ants__start_auto_deploy(0, ANTENNA_DEPLOYMENT_TIMEOUT);//TODO:REMOVE
		if (resDeploy!=0)
			logg(error, "E:Failed deploy: Side A res=%d\n", resDeploy);
	} else
		logg(error, "E:Failed Arming Side A with error: %d\n", resArm);

	// antenata auto deploy - sides B
	resArm = isis_ants__arm(1);
	if(resArm == 0) {
		logg(event, "V:Deploying: Side B\n");
		//resDeploy = isis_ants__start_auto_deploy(1, ANTENNA_DEPLOYMENT_TIMEOUT);//TODO:REMOVE
		if (resDeploy!=0)
			logg(error, "E:Failed deploy: Side B res=%d\n", resDeploy);
	} else
		logg(error, "E:Failed Arming Side B with error: %d\n", resArm);


	// update last deploy time
	time_unix deploy_time;
	int err = Time_getUnixEpoch((unsigned int *)&deploy_time);
	if(0 != err)
		logg(error, "E:Time_getUnixEpoch failed to set ants last deploy time\n");
	else {
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

		// waiting for 30 min (collect telemetry every x sec)-
		firstActivationProcedure();


		//Initialize the Antenas performed in trx init performing auto deploy
		res = autoDeploy();
	}
	return res;
}

//Init sub system

int InitSubsystems() {

	//dont logg anythin brfore time init
	int errSPI = StartSPI();
	int errI2C = StartI2C();
	int errRTC = RTC_start();
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
	if ( errRTC != 0 )
		logg(error, "E:%d Failed in StartRTC\n", errRTC);
	else
		logg(event, "V: StartRTC() - success\n");
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

	EPS_Init();

	//RUN_EPS_I2C_COMM();//TODO: remove

	InitAnts();//TODO: turn off somewhere after deploy

	// initialize TRXVU (communication) component and ants
	int err=InitTrxvu();
	if (err!=0)
		logg(error, "E:%d Failed in InitTrxvu\n", err);
	else
		logg(event, "V: InitTrxvu was successful\n");


	SolarPanelv2Init();
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
	// Deploy system - open Antetnas
	err = DeploySystem();
	if (err)
		logg(error, "E:%d Failed in DeploySystem\n", err);
	else
		logg(event, "V: DeploySystem was successful\n");

	//increase the number of total resets
	WakeUpFromReset();//must happen before payload init for short circuit protection
	payloadInit(TRUE);
	return 0;
}
