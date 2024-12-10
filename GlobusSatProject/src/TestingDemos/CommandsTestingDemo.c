
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "CommandsTestingDemo.h"

#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/ActUponCommand.h"
#include "SubSystemModules/Communication/CommandDictionary.h"
#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Communication/SPL.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"

#include <stdio.h>
#include <string.h>
#include <hal/Utility/util.h>

Boolean TestActUponCommand()
{
	printf("\nPlease insert number of minutes to test(1 to 10)\n");
	int minutes = 0;
	int err = 0;
	while(UTIL_DbguGetIntegerMinMax((int*)&minutes,1,10) == 0);

	portTickType curr_time = xTaskGetTickCount();
	portTickType end_time = MINUTES_TO_TICKS(minutes) + curr_time;

	sat_packet_t cmd = {0};
	while(end_time - curr_time > 0)
	{
		curr_time = xTaskGetTickCount();
		err = GetOnlineCommand(&cmd);
		if(cmd_command_found == err){
			printf("Online Command Found!!\n");
			err = ActUponCommand(&cmd);
			if(0 != err){
				printf("error in 'ActUponCommand' = %d\n",err);
			}
		}
		vTaskDelay(100);
	}
	return TRUE;
}

Boolean TestAssmbleCommand()
{
	unsigned char data[] = {1,2,3,4,5,6,7,8,9,10,
							11,12,13,14,15,16,17,18,19,20};
	unsigned int length = sizeof(data);
	unsigned char type = 0x42;
	unsigned char subtype = 0x43;
	unsigned int id = 0x12345678;

	sat_packet_t cmd = {0};
	int err = AssembleCommand(data,length,type, subtype, id, 0, 8, &cmd);
	if(0 != err){
		printf("error in 'AssmbleCommand' = %d\n",err);
		return TRUE;
	}
	printf("id = %d\n",			cmd.ID);
	printf("type = %d\n",		cmd.cmd_type);
	printf("subtype = %d\n",	cmd.cmd_subtype);
	printf("data length = %d\n",cmd.length);

	printf("data =\t");
	unsigned int i;
	for(i= 0;i<cmd.length;i++)
	{
		printf("%X\t",cmd.data[i]);
	}
	printf("\n");
	return TRUE;
}



Boolean TestGetOnlineCommand()
{
	sat_packet_t cmd = {0};
	int err = 0;
	err = GetOnlineCommand(&cmd);
	if(cmd_command_found != err){
		printf("error in 'GetOnlineCommand' = %d",err);
		return TRUE;
	}

	printf("data of the online command:\n");
	unsigned int i;
	for(i = 0; i < sizeof(sat_packet_t); i++){
		printf("%x\t",((unsigned char*)(&cmd))[i]);
	}
	printf("\n");
	return TRUE;
}



Boolean selectAndExecuteCommandsDemoTest()
{
	int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf("\n\r Select a test to perform: \n\r");
	printf("\t 0) Return to main menu \n\r");
	printf("\t 1) Test Act Upon Command\n\r");
	printf("\t 2) Test Assmble Command\n\r");
	printf("\t 3) Get Online Command\n\r");

	unsigned int number_of_tests = 8;
	while(UTIL_DbguGetIntegerMinMax(&selection, 0, number_of_tests) == 0);

	switch(selection) {
	case 0:
		offerMoreTests = FALSE;
		break;
	case 1:
		offerMoreTests = TestActUponCommand();
		break;
	case 2:
		offerMoreTests = TestAssmbleCommand();
		break;
	case 3:
		offerMoreTests = TestGetOnlineCommand();
		break;
	}
	return offerMoreTests;
}

Boolean MainCommandsTestBench()
{
	Boolean offerMoreTests = FALSE;

	while(1)
	{
		offerMoreTests = selectAndExecuteCommandsDemoTest();

		if(offerMoreTests == FALSE)
		{
			break;
		}
	}
	return FALSE;
}
