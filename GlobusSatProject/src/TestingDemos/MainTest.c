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
#include "SubSystemModules/Maintenance/Maintenance.h"

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

		WriteDefaultValuesToFRAM();
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
	isFirstActivation(&St);
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

	Boolean St;
	// call isFirstActivation check
	isFirstActivation(&St);
	if(St==TRUE)
	{
		printf("isFirstActivation()=TRUE\n");
	}
	else
	{
		printf("isFirstActivation()=FLASE\n");
	}

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
		//logg(OBCInfo, "I:--------------------Main loop: i= : %d  ------------\n",  i);
	//	voltage_t v;
		TRX_Logic();
		logg(DMPInfo, "I:Main:TRX_Logic\n");

		TelemetryCollectorLogic();
		logg(DMPInfo, "I:Main:TelemetryCollectorLogic\n");

		Maintenance();
		logg(DMPInfo, "I:Main:MaintenanaceLogic\n");

		//vTaskDelay(100);

		i++;
		//if (i == 10)
		TestDumpTelemetry();


		//0 0 5 8 0 69 A 0 5 0 C3 3B 3D 5F CD EA 3F 5F

		//0 0 5 8 4 9A 1 0 2   set log level debug
		//0 0 5 8 4 9A 1 0 4   set log level event
		//0 0 5 8 0 69 A 0 5 0 0 0 0 0 7E 05 40 5F dump eps eng tlm
		//0 0 5 8 3 0F 9 0 1 CD EA 3F 5F 7E 05 40 5F    delete files by time

		//0 0 5 8 3 0E 1 0 3 delete files by type

		//0 0 5 8 0 69 A 0 1 0 12 8f 42 5F 69 91 42 5F
		//0 0 5 8 0

		//0 0 5 8 4 91 8 0 61 2 0 0 0 38 2 generic i2c turn transponder on
		//0 0 5 8 4 91 8 0 61 2 0 0 0 38 1 generic i2c turn transponder off
		//0 0 5 8 4 91 8 0 61 3 0 0 0 52 c8 0 generic i2c set rssi 200
		//0 0 5 8 4 91 8 0 61 3 0 0 0 52 0 0 generic i2c set rssi 200


	}

}


