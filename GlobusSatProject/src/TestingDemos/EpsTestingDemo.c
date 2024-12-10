#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "EpsTestingDemo.h"

#include <hal/Utility/util.h>

#include <SubSystemModules/PowerManagment/EPS.h>

#include <stdlib.h>





Boolean TestGetBatteryVoltage()
{
	voltage_t vbat = 0;
	int err = GetBatteryVoltage(&vbat);
	if(0!=err){
		printf("error in 'GetBatteryVoltage' = %d",err);
	}
	printf("battery voltage = %d\n",vbat);
	return TRUE;
}

/*
Boolean selectAndExecuteEpsDemoTest()
{
	unsigned int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform: \n\r");
	printf("\t 0) Return to main menu \n\r");
	printf("\t 1) Eps Conditioning \n\r");
	printf("\t 2) Get Battery Voltage \n\r");
	printf("\t 3) Update Threshold Voltages \n\r");
	printf("\t 4) Print Threshold Voltages \n\r");
	printf("\t 5) Update Alpha  \n\r");
	printf("\t 6) Print Alpha  \n\r");
	printf("\t 7) Restore Default Alpha \n\r");
	printf("\t 8) Restore Default Threshold Voltages \n\r");


	unsigned int number_of_tests = 8;
	while(UTIL_DbguGetIntegerMinMax(&selection, 0, number_of_tests) == 0);

	switch(selection) {
	case 0:
		offerMoreTests = FALSE;
		break;
	case 1:
		offerMoreTests = TestEpsConditioning();
		break;
	case 2:
		offerMoreTests = TestGetBatteryVoltage();
		break;
	case 3:
		offerMoreTests = TestUpdateThreshVoltages();
		break;
	case 4:
		offerMoreTests = TestGetThreshVoltages();
		break;
	case 5:
		offerMoreTests = TestUpdateAlpha();
		break;
	case 6:
		offerMoreTests = TestGetAlpha();
		break;
	case 7:
		offerMoreTests = TestRestoreDefaultAlpha();
		break;
	case 8:
		offerMoreTests = TestRestoreDefaultThresholdVoltages();
		break;

	default:
		break;
	}
	return offerMoreTests;
}
*/

Boolean MainEpsTestBench()
{
	TestGetBatteryVoltage();
	/*Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteEpsDemoTest();

		if(offerMoreTests == FALSE)
		{
			return FALSE;
		}
	}*/
	return FALSE;
}
