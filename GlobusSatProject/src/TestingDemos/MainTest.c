#include "MainTest.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/WatchDogTimer.h>
#include <hal/boolean.h>
#include <hal/Utility/util.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/SPI.h>
#include <hal/Timing/Time.h>

#include <at91/utility/trace.h>
#include <at91/peripherals/cp15/cp15.h>
#include <at91/utility/exithandler.h>
#include <at91/commons.h>

#include <hcc/api_fat.h>

#include "InitSystem.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Communication/Beacon.h"
#include "SubSystemModules/HouseKepping/TelemetryCollector.h"
#include "TrxvuTestingDemo.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "SubSystemModules/Communication/Trxvu.h"
#include "SubSystemModules/Maintenance/Log.h"

Boolean selectAndExecuteTest()
{
	unsigned int selection = 0;
	Boolean offerMoreTests = TRUE;

#define RESTART_INDEX		0
#define TLM_TEST_INDEX		1
#define EPS_TEST_INDEX		2
#define TRXVU_TEST_INDEX	3
#define CMD_TEST_INDEX		4
#define MNGMNT_TEST_INDEX	5
#define FS_TEST_INDEX 		6


	unsigned int number_of_tests = 7;

	printf("\n\r Select the device to be tested to perform: \n\r");
	printf("\t 0) Restart\n\r");
	printf("\t 1) Telemetry Testing\n\r");
	printf("\t 2) EPS Testing\n\r");
	printf("\t 3) TRXVU Testing\n\r");
	printf("\t 4) Commands Handeling Testing\n\r");
	printf("\t 5) Managment Testing\n\r");
	printf("\t 6) File System Testing\n\r");


	while (UTIL_DbguGetIntegerMinMax(&selection, 0, number_of_tests) == 0);

	switch (selection) {
	case RESTART_INDEX:
		restart();
		vTaskDelay(10000);
		printf("what?? \n\n\nwhere am I???");
		break;

	case TLM_TEST_INDEX:
		offerMoreTests = MainTelemetryTestBench();
		break;

	case EPS_TEST_INDEX:
		offerMoreTests = MainEpsTestBench();
		break;

	case TRXVU_TEST_INDEX:
		offerMoreTests = MainTrxvuTestBench();
		break;

	case CMD_TEST_INDEX:
		offerMoreTests = MainCommandsTestBench();
		break;

	case MNGMNT_TEST_INDEX:
		offerMoreTests = MainMaintenanceTestBench();
		break;

	case FS_TEST_INDEX:
		offerMoreTests = MainFileSystemTestBench();
		break;

	default:
		printf("Undefined Test\n\r");
		offerMoreTests = TRUE;
		break;
	}

	return offerMoreTests;
}


//Idle Test
void testsIdle()
{
	InitSubsystems();
	int i = 0;
	while (i<60)
	{
		//GetBatteryVoltage(&curr_voltage);
		printf("GivatShmuel:main testing loop: i= : %d\n",  i);
		TRX_Logic();
		vTaskDelay(1000);
		i++;

		if(i == 5)
		{
			CMD_SetIdleOn();
			printf("********************************************************************** set idle\n");
		}
		else if (i==42)
		{
			CMD_SetIdleOff();
			printf("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv set idle off\n");
		}
	}
}

//Mute Test
void testsMute()
{
	InitSubsystems();
	int i = 0;
	while (i<60)
	{
		//GetBatteryVoltage(&curr_voltage);
		printf("GivatShmuel:main testing loop: i= : %d\n",  i);
		TRX_Logic();
		vTaskDelay(1000);
		i++;

		if(i == 25)
		{
			muteTRXVU(30);
			printf("********************************************************************** set mute\n");
		}
		else if (i==42)
		{
			UnMuteTRXVU();
			printf("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv set unmute");
		}
	}
}

//Initialize method to set FRAM to initial phase before first Init of satelite
 void IntializeFRAM()
 {
	int err = 0;
	err = StartSPI();
	err = StartI2C();
	err = StartFRAM();

	if(!err)
	{
		 //set first activation flag to true
		int status = 1;
		FRAM_write((unsigned char*)&status,FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE );

		 //set seconds since deploy to 0
		int a = 0;
		FRAM_write((unsigned char*)&a ,SECONDS_SINCE_DEPLOY_ADDR,SECONDS_SINCE_DEPLOY_SIZE);
	}
 }


//Test for firstActivationProcedure Logic
void TestFirstActivionProc()
{
	printf("i am starting\n");
	int err;
	IntializeFRAM();
	err = StartTIME();

	//added to allow Telemetry collector to work
	err = InitializeFS(TRUE);

	if(!err)
	{
		firstActivationProcedure();
	}
	printf("i am done\n");
}

void TestIsFirstAct(char status)
{
	Boolean St;

	//set flag status in FRAM
	FRAM_write((unsigned char*)&status,FIRST_ACTIVATION_FLAG_ADDR, FIRST_ACTIVATION_FLAG_SIZE );

	// call isFirstActivation check
	St=isFirstActivation();
	if(St==TRUE)
	{
		printf("isFirstActivation()=TRUE\n");
	}
	else
	{
		printf("isFirstActivation()=FLASE\n");
	}
	printf("i am done\n");
}


void TestisFirstActivation()
{
	printf("i am starting\n");
	int err;
	err = StartSPI();
	err = StartI2C();
	err = StartFRAM();

	if(!err)
	{
		printf("Setting isFirstActivation with FALSE\n");
		TestIsFirstAct(0);

		printf("Setting isFirstActivation with TRUE\n");
		TestIsFirstAct(1);
	}
}



void TestUpdateBeaconInterval()
{
	printf("i am starting\n");

	sat_packet_t cmd;
	cmd.data[0] = 120;

	int s;
	s=UpdateBeaconInterval(&cmd);
	if(s==E_PARAM_OUTOFBOUNDS)
	{
		printf("test succeeded - update interval failed\n");
	}
	else
	{
		printf("test failed - update interval succeeded\n");
	}

	printf("i am done\n");
}


//Test restart after deployment not performing deployment again
void TestRestartSkipDeployment()
{
	printf("i am starting\n");
	IntializeFRAM() ;
	InitSubsystems(); // run  init
	DeploySystem(); // check that deployment is skipped

	printf("i am done\n");
}

//Main testing task
void taskTesting()
{
	WDT_startWatchdogKickTask(10 / portTICK_RATE_MS, FALSE);


	//TestisFirstActivation();
	//TestFirstActivionProc();
	//TestRestartSkipDeployment();

	//testsIdle();
	//testsMute();
	//TestUpdateBeaconInterval();
	//TestDumpTelemetry();


	//IntializeFRAM();

	InitSubsystems();
	int i = 0;
	while (1)
	{
		logg(OBCInfo, "I:--------------------Main loop: i= : %d  ------------\n",  i);

		int res=EPS_Conditioning();
		logg(OBCInfo, "I:Main:EPS_Conditioning\n");
		if(res!=0)
			logg(error, "E:in EPS_Conditioning: %d\n", res);

		TRX_Logic();
		logg(OBCInfo, "I:Main:TRX_Logic\n");

		TelemetryCollectorLogic();
		logg(OBCInfo, "I:Main:TelemetryCollectorLogic\n");

		vTaskDelay(100);

		i++;
		//if (i == 10)
		//TestDumpTelemetry();
		//5 0 0 0 0 105 10 0 4 0 128 67 109 56 20 225 11 94
	}

}


