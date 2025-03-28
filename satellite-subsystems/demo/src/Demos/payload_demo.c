
#include "Demos/payload/payload_drivers.h"
#include "Demos/isismeps_ivid7_piu_demo.h"

#include <hal/boolean.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <hal/Utility/util.h>



static Boolean checkPayloadReadEnvironment() {
    printf("Testing payloadReadEnvironment...\n");
    PayloadEnvironmentData environment_data;
    SoreqResult result = payloadReadEnvironment(&environment_data);

    if (result == PAYLOAD_SUCCESS) {
        printf("payloadReadEnvironment: SUCCESS\n");
        printf("  RADFET Voltage 1: %d (0x%X)\n",
               environment_data.adc_conversion_radfet1,
               environment_data.adc_conversion_radfet1);
        printf("  RADFET Voltage 2: %d (0x%X)\n",
               environment_data.adc_conversion_radfet2,
               environment_data.adc_conversion_radfet2);
        printf("  Temperature: %lf\n", environment_data.temperature);
        return TRUE;
    } else {
        printf("payloadReadEnvironment: ERROR (%d)\n", result);
        return FALSE;
    }
}


static Boolean checkPayloadReadEvents() {
    printf("Testing payloadReadEvents...\n");
    PayloadEventData event_data;
    SoreqResult result = payloadReadEvents(&event_data);
    if (result == PAYLOAD_SUCCESS) {
        printf("payloadReadEvents: SUCCESS\n");
        printf("  SEL Count: %d\n", event_data.sel_count);
        printf("  SEU Count: %d\n", event_data.seu_count);
        return TRUE;
    } else {
        printf("payloadReadEvents: ERROR (%d)\n", result);
        return FALSE;
    }
}

static Boolean checkPayloadSoftReset() {
    printf("Testing payloadSoftReset...\n");
    SoreqResult result = payloadSoftReset();
    if (result == PAYLOAD_SUCCESS) {
        printf("payloadSoftReset: SUCCESS\n");
        return TRUE;
    } else {
        printf("payloadSoftReset: ERROR (%d)\n", result);
        return FALSE;
    }
}

static Boolean checkPayloadTurnOff() {
    printf("Testing payloadTurnOff...\n");
    SoreqResult result = payloadTurnOff();
    if (result == PAYLOAD_SUCCESS) {
        printf("payloadTurnOff: SUCCESS\n");
        return TRUE;
    } else {
        printf("payloadTurnOff: ERROR (%d)\n", result);
        return FALSE;
    }
}

static Boolean selectAndExecutePayloadDemoTest(void)
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform: \n\r");
	printf("\t 1) Check Payload Read Environment \n\r");
	printf("\t 2) Check Payload Read Events \n\r");
	printf("\t 3) Check Payload Soft Reset \n\r");
	printf("\t 4) Check Payload Turn Off \n\r");
	printf("\t 5) Return to main menu \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 15) == 0);

	switch(selection) {
	case 1:
		offerMoreTests = checkPayloadReadEnvironment();
		break;
	case 2:
		offerMoreTests = checkPayloadReadEvents();
		break;
	case 3:
		offerMoreTests = checkPayloadSoftReset();
		break;
	case 4:
		offerMoreTests = checkPayloadTurnOff();
		break;
	case 5:
		offerMoreTests = FALSE;
		break;

	default:
		break;
	}

	return offerMoreTests;
}

void PayloadDemoLoop(void)
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecutePayloadDemoTest();	// show the demo command line interface and handle commands

		if(offerMoreTests == FALSE)							// was exit/back selected?
		{
			break;
		}
	}
}

Boolean PayloadDemoMain(void)
{
	if(isismepsv2_ivid7_piu__demo__init() == TRUE && payloadInit() == PAYLOAD_SUCCESS)
	{
		PayloadDemoLoop();
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

Boolean Payloadtest(void)
{
	PayloadDemoMain();
	return TRUE;
}
