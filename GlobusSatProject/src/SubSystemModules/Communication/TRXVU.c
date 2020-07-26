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
#include "InitSystem.h"
#include "TRXVU.h"
#include "AckHandler.h"
#include "ActUponCommand.h"
#include "SatCommandHandler.h"
#include "TLM_management.h"
#include "Transponder.h"

#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Communication/SatCommandHandler.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "SubSystemModules/Communication/Beacon.h"
#include "SubSystemModules/Housekepping/Dump.h"
#include "SubSystemModules/Maintenance/Log.h"

//idle global variables
int				g_idle_period = 300 ; // idle time default = 5 min
Boolean 		g_idle_flag = FALSE;
time_unix 		g_idle_start_time = 0;

//ant open global variables
char g_antOpen= 0;
time_unix 		g_ants_last_dep_time = 0;
int				g_ants_dep_period =30*60 ; //30 min

void setLastAntsAutoDeploymentTime(time_unix time)
{
	g_ants_last_dep_time = time;
	logg(TRXInfo, "I: Last Antenas Auto deployment is: %d\n", time);
}

//setting trxvu idle off
int SetIdleOff()
{
	logg(TRXInfo, "I:inside SetIdleOff()\n");
	int err = 0;

	if(g_idle_flag==TRUE)
	{
		err=IsisTrxvu_tcSetIdlestate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_idle_state_off);
		if(err==0)
		{
			g_idle_flag=FALSE;
		}
		else
		{
			logg(error, "E:failed in setting trxvu idle off - %d\n", err);
		}
	}
	return err;
}

void HandleOpenAnts()
{
	if(g_antOpen == 0)
	{
		//if ants open period has passed
		if (CheckExecutionTime(g_ants_last_dep_time, g_ants_dep_period)==TRUE)
		{
			// ants auto deploy
			autoDeploy();
		}
		else
		{
			logg(TRXInfo, "I:ants end period not reached\n");
		}
	}
}

// Checking if in idle state and if need to return back to regular state
void HandleIdleAndMuteTime()
{
	//if in idle state
	if(g_idle_flag==TRUE)
	{
		//if idle period has passed
		if (CheckExecutionTime(g_idle_start_time, g_idle_period)==TRUE)
		{
			SetIdleOff();
		}
		else
		{
			logg(TRXInfo, "I:idle end period not reached\n");
		}
	}

	//if mute on
	if(GetMuteFlag())
	{
		if(CheckForMuteEnd())
		{
			UnMuteTRXVU();
		}
	}
}

void HandleTransponderTime()
{
	if(getTransponderMode()==TURN_TRANSPONDER_ON)
	{
		if(checkEndTransponderMode()==TRUE)
		{
			CMD_turnOffTransponder();
		}
	}
}

// Initialize TRXVU component
int InitTrxvu()
{
	logg(TRXInfo, "I:InitTrxvu() started\n");
	//set I2C addresses
	ISIStrxvuI2CAddress i2cAdress;
	i2cAdress.addressVu_rc=I2C_TRXVU_RC_ADDR;
	i2cAdress.addressVu_tc=I2C_TRXVU_TC_ADDR;

	//set max frame lengths
	ISIStrxvuFrameLengths framelengths;
	framelengths.maxAX25frameLengthRX=SIZE_RXFRAME;
	framelengths.maxAX25frameLengthTX=SIZE_TXFRAME;

	//set bitrate
	ISIStrxvuBitrate default_bitrates;
	default_bitrates=trxvu_bitrate_9600;

	//Initialize TRXVU driver
	int err = IsisTrxvu_initialize(&i2cAdress,&framelengths,&default_bitrates,1);
	if(err!=0)
	{
		logg(error, "E: in the initialization: %d\n", err);
		return err;
	}
	else
	{
		logg(TRXInfo, "I: IsisTrxvu_initialize succeeded\n");
	}


	vTaskDelay(100);
	err=IsisTrxvu_tcSetAx25Bitrate(ISIS_TRXVU_I2C_BUS_INDEX ,trxvu_bitrate_9600);
	if(err!=0)
	{
		logg(error, "E: Error in the IsisTrxvu_tcSetAx25Bitrate: %d\n", err);
		return err;
	}
	else
	{
		logg(TRXInfo, "I:IsisTrxvu_tcSetAx25Bitrate succeeded\n");
	}
	vTaskDelay(100);

	// get ant open flag from fram
	err=FRAM_read((unsigned char *)&g_antOpen, ANT_OPEN_FLAG_ADDR,  ANT_OPEN_FLAG_SIZE );
	if(err!=0)
	{
		g_antOpen=0;
	}

	if(g_antOpen==0)
	{
		//Set Antenas addresses for both sides
		ISISantsI2Caddress address;
		address.addressSideA = ANTS_I2C_SIDE_A_ADDR;
		address.addressSideB = ANTS_I2C_SIDE_B_ADDR;

		//initialize Antenas system
		err=IsisAntS_initialize(&address,1);
		if(err!=0)
		{
			logg(error, "Error in the initialization of the Antennas: %d\n", err);
			return err;
		}
		else
		{
			logg(TRXInfo, "I: Initialization of the Antennas succeeded\n");
		}
	}

	//Initialize TRXVU transmit lock
	InitTxModule();

	//init transponder
	initTransponder();

	//Initialize beacon parameters
	InitBeaconParams();

	return err;
}


//TRX VU main logic
 CommandHandlerErr TRX_Logic()
{
	logg(TRXInfo, "I:Inside TRX_Logic()\n");
	sat_packet_t *cmd = malloc(sizeof(sat_packet_t));
	int onCmdCount;
	unsigned char* data = NULL;
	unsigned int length=0;
	CommandHandlerErr  res =0;

	//check if we have online command (frames in buffer)
	onCmdCount = GetNumberOfFramesInBuffer();

	if(onCmdCount>0)
	{
		//get the online command
		res = GetOnlineCommand(cmd);
		if(res==cmd_command_found)
		{
			logg(TRXInfo, "I:Getting the online command success\n");
			//Reset WD communication with earth by saving current time as last communication time in FRAM
			ResetGroundCommWDT();

			//Send Acknowledge to earth
			SendAckPacket(ACK_RECEIVE_COMM, cmd, data, length);

			//run command
			res = ActUponCommand(cmd);
		}
		else
		{
			logg(TRXInfo, "I: Status: %d in getting the online command. \n", res);
		}

		if(g_antOpen==0)
		{
			//update ant open to true
			g_antOpen=1;
			FRAM_write((unsigned char *)&g_antOpen, ANT_OPEN_FLAG_ADDR,  ANT_OPEN_FLAG_SIZE );
			logg(TRXInfo, "I: Antennas deployment flag set to TRUE as packet received from Earth******\n");
		}

	}

	//check idle timer and mute timer
	HandleIdleAndMuteTime();

	//check transponder timer
	HandleTransponderTime();

	// open ant if not open
    HandleOpenAnts();

	//handle beacon
	BeaconLogic();

	return res;
}

 /*
  * TRXVU Online Commands
  *
  */

// Command to set idle state to on
int CMD_SetIdleOn(sat_packet_t *cmd)
{
	logg(TRXInfo, "I:inside CMD_SetIdleOn()\n");
	int err=IsisTrxvu_tcSetIdlestate(ISIS_TRXVU_I2C_BUS_INDEX, trxvu_idle_state_on);
	if(err!=0)
	{
		logg(error, "E: Error in setting trxvu idle on - %d\n", err);
	}
	else
	{
		//get time of start idle period & set idle state flag to true and set idle period
		memcpy(&g_idle_period,cmd->data,sizeof(g_idle_period));
		Time_getUnixEpoch((unsigned int*)&g_idle_start_time);
		g_idle_flag=TRUE;
		logg(TRXInfo, "I:Set idle state for %d seconds. \n", g_idle_period);
	}
	return err;
}


// Command to set idle state to off
int CMD_SetIdleOff()
{
	logg(TRXInfo, "I:inside CMD_SetIdleOff()\n");
	return SetIdleOff();
}

