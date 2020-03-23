#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>
#include <hal/errors.h>

#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>

#include <stdlib.h>
#include <string.h>
#include "GlobalStandards.h"
#include "TRXVU.h"
#include "AckHandler.h"
#include "ActUponCommand.h"
#include "SatCommandHandler.h"
#include "TLM_management.h"

#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "SubSystemModules/Communication/Beacon.h"
#include "SubSystemModules/Housekepping/Dump.h"

int				g_idle_period = 301 ;				// idle time default = 5 min
Boolean 		g_idle_flag = FALSE;
time_unix 		g_idle_start_time = 0;

//setting trxvu idle off
int SetIdleOff()
{
	//TODO: remove print after testing complete
	print("inside SetIdleOff()");
	int err=IsisTrxvu_tcSetIdlestate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_idle_state_off);
	if(err!=0)
	{
		printf("failed in setting trxvu idle off - %d", err);
	}
	return err;
}

void HandleIdleTime()
{
	//TODO: remove print after testing complete
	print("inside HandleIdleTime()");
	if(g_idle_flag==TRUE)
	{
		if (CheckExecutionTime(g_idle_start_time, g_idle_period)==TRUE)
		{
			SetIdleOff();
			g_idle_flag=FALSE;
		}
		else
		{
			//TODO: remove print after testing complete
			print("idle end period not reached");
		}
	}
	else
	{
		//TODO: remove print after testing complete
		print("not in idle period");
	}
}


int InitTrxvu()
{
	//TODO: remove print after testing complete
	print("inside InitTrxvu()");

	ISIStrxvuI2CAddress i2cAdress;
	i2cAdress.addressVu_rc=I2C_TRXVU_RC_ADDR;
	i2cAdress.addressVu_tc=I2C_TRXVU_TC_ADDR;

	ISIStrxvuFrameLengths framelengths;

	framelengths.maxAX25frameLengthRX=SIZE_RXFRAME;
	framelengths.maxAX25frameLengthTX=SIZE_TXFRAME;
	ISIStrxvuBitrate default_bitrates;
	default_bitrates=trxvu_bitrate_9600;

	int err = IsisTrxvu_initialize(&i2cAdress,&framelengths,&default_bitrates,1);
	if(err!=0)
	{
		printf("there is error in the initialization");
		return err;
	}
	else
	{
		//TODO: remove print after testing complete
		print("initialization succeeded");
	}


	//sleep 0.1 sec and set birate to 9600 bps
	vTaskDelay(100);
	err=IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX ,trxvu_bitrate_9600);
	if(err!=0)
	{
		printf("there is error in the IsisTrxvu_tcSetAx25Bitrate");
		return err;
	}
	else
	{
		//TODO: remove print after testing complete
		print("IsisTrxvu_tcSetAx25Bitrate succeeded");
	}
	vTaskDelay(100);

	//initialize Antenas system
	ISISantsI2Caddress adress;
	adress.addressSideA = ANTS_I2C_SIDE_A_ADDR;
	adress.addressSideB = ANTS_I2C_SIDE_B_ADDR;
	err=IsisAntS_initialize(&adress,1);
	if(err!=0)
	{
		printf("there is error in the initialization of the Antennas");
		return err;
	}
	else
	{
		//TODO: remove print after testing complete
		print("initialization of the Antennas succeeded");
	}

	InitTxModule();
	InitBeaconParams();

	return err;
}


 CommandHandlerErr TRX_Logic()
{
	 //TODO: remove print after testing complete
	print("inside TRX_Logic()");
	sat_packet_t cmd={0};
	int onCmdCount, offBufferCount;
	unsigned char* data = NULL;
	unsigned int length=0;
	CommandHandlerErr  res;
	int result = 0;

	//check if we have online command
	onCmdCount = GetNumberOfFramesInBuffer();

	if(onCmdCount>0)
	{
		res = GetOnlineCommand(&cmd);//ask the teacher
		if(res!=0)
		{
			printf("there was an error in getting the online command ");
		}
		else
		{
			 //TODO: remove print after testing complete
			print("getting the online command success");
			ResetGroundCommWDT();
			SendAckPacket(ACK_RECEIVE_COMM, &cmd, data, length);
		}
	}
	else
	{
		offBufferCount=GetDelayedCommandBufferCount();
		if(offBufferCount> 0)
		{
			res=GetDelayedCommand(&cmd);
			if(res!=0)
			{
				printf("there was error in getting delayed command");
			}
			else
			{
				 //TODO: remove print after testing complete
				print("getting delayed command success");
			}
		}
	}

	if((onCmdCount>0 || offBufferCount>0) && res == 0)
	{
		result = ActUponCommand(&cmd);
	}

	//check idle timer
	HandleIdleTime();

	BeaconLogic();

	return result+res;

}


int CMD_SetIdleOn()
{
	//TODO: remove print after testing complete
	print("inside CMD_SetIdleOn()");
	int err=IsisTrxvu_tcSetIdlestate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_idle_state_on);
	if(err!=0)
	{
		printf("failed in setting trxvy idle on - %d", err);
	}
	else
	{
		Time_getUnixEpoch((unsigned int*)&g_idle_start_time);
		g_idle_flag=TRUE;
	}
	return err;
}



