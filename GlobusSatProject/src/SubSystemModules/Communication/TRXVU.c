#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <hal/Timing/Time.h>
#include <hal/errors.h>

#include <satellite-subsystems/isis_vu_e.h>
#include <satellite-subsystems/isis_ants.h>

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
Boolean g_antOpen= FALSE;
time_unix 		g_ants_last_dep_time = 0;
int				g_ants_dep_period = ANT_DEPLOY_WAIT_PERIOD; //30 min
sat_packet_t cmd;


Boolean vu_getFrequenciesTest_revE(void)
{
	isis_vu_e__get_rx_freq__from_t rx_freq;
	uint32_t tx_freq;
	int rv = isis_vu_e__get_rx_freq(0, &rx_freq);
	if(rv) {
		logg(error, "E:Subsystem receiver call failed. rv = %d", rv);
		return TRUE;
	}

	rv = isis_vu_e__get_tx_freq(0, &tx_freq);
	if(rv) {
		logg(error, "E:Subsystem transmitter call failed. rv = %d", rv);
		return TRUE;
	}

	isis_vu_e__state__from_t response;
	rv =  isis_vu_e__state(0, &response);
	if(rv!=0)
			logg(error, "E:isis_vu_e__state: %d\n", rv);

	logg(event, "v:idle=%d bitrate=%d rx_freq=%lu rx_state-%d tx_freq=%lu\n", response.fields.idle, response.fields.bitrate, rx_freq.fields.freq, rx_freq.fields.status,  tx_freq);

	return rv;
}

void setLastAntsAutoDeploymentTime(time_unix time)
{
	g_ants_last_dep_time = time;
	logg(TRXInfo, "I: Last Antenas Auto deployment is: %d\n", time);
}

void HandleOpenAnts()
{
	if(g_antOpen == FALSE)
	{
		//if ants open period has passed
		if (CheckExecutionTime(g_ants_last_dep_time, g_ants_dep_period)==TRUE)
		{
			// ants auto deploy
			autoDeploy();
		}
	}
}

//setting trxvu idle off
int SetIdleOff()
{
	logg(TRXInfo, "I:inside SetIdleOff()\n");
	int err = isis_vu_e__set_idle_state(ISIS_TRXVU_I2C_BUS_INDEX, isis_vu_e__onoff__off);
	if(err == 0){
		g_idle_flag=FALSE;
		g_idle_start_time = 0;
		g_idle_period = 300 ;
	} else
		logg(error, "E:failed in setting trxvu idle off - %d\n", err);

	return err;
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

int InitAnts() {
	eps_set_channels_on(isismepsv2_ivid5_piu__eps_channel__channel_5v_sw2);
	ISIS_ANTS_t address[2];
	address[0].i2cAddr = ANTS_I2C_SIDE_A_ADDR;
	address[1].i2cAddr = ANTS_I2C_SIDE_B_ADDR;
	int err = ISIS_ANTS_Init(address, 2);
	if(err)
		logg(error, "E: Error in the initialization of the Antennas: %d\n", err);
	return err;
}

int SetFreqAndBitrate(){
		int err = 0;
		uint32_t rx_freq = TRXVU_RX_FREQ;//145970
		uint32_t tx_freq = TRXVU_TX_FREQ;//436400
		err = isis_vu_e__set_rx_freq(0, rx_freq);
		if(err!=0) {
				logg(error, "E: Error in the isis_vu_e__set_rx_freq: %d\n", err);
				return err;
		} else
			logg(TRXInfo, "I:isis_vu_e__get_rx_freq succeeded\n");

		err = isis_vu_e__set_tx_freq(0, tx_freq);
		if(err!=0) {
			logg(error, "E: Error in the isis_vu_e__set_tx_freq: %d\n", err);
			return err;
		} else
			logg(TRXInfo, "I:isis_vu_e__set_tx_freq succeeded\n");

		err = isis_vu_e__set_bitrate(0, isis_vu_e__bitrate__9600bps);
		if(err!=0) {
			logg(error, "E: Error in the isis_vu_e__set_bitrate: %d\n", err);
			return err;
		}
		else
			logg(TRXInfo, "I:IsisTrxvu_tcSetAx25Bitrate succeeded\n");
		vTaskDelay(100);
		vu_getFrequenciesTest_revE();
		return err;
}

int InitTrxvu() {
	logg(TRXInfo, "I:InitTrxvu() started\n");
	ISIS_VU_E_t myTRXVU[1];
	myTRXVU[0].rxAddr = I2C_TRXVU_RC_ADDR;
	myTRXVU[0].txAddr = I2C_TRXVU_TC_ADDR;
	myTRXVU[0].maxSendBufferLength = SIZE_TXFRAME;
	myTRXVU[0].maxReceiveBufferLength = SIZE_RXFRAME;

	driver_error_t err = ISIS_VU_E_Init(myTRXVU, 1);
	if(err!=0) {
		logg(error, "E: in the ISIS_VU_E_Init: %d\n", err);
		return err;
	}
	else
		logg(TRXInfo, "I: IsisTrxvu_initialize succeeded\n");

	SetFreqAndBitrate();

	SetIdleOff();

	areAntennasOpen();

	InitTxModule();

	initTransponder();

	InitBeaconParams();

	return err;
}


//TRX VU main logic
CommandHandlerErr TRX_Logic()
{
	logg(TRXInfo, "I:Inside TRX_Logic()\n");
	//sat_packet_t *cmd = malloc(sizeof(sat_packet_t));
	int onCmdCount;
	unsigned char* data = NULL;
	unsigned int length=0;
	CommandHandlerErr  res =0;
	isis_vu_e__state__from_t response;
	int rv =  isis_vu_e__state(0, &response);
	if ((!rv && response.fields.bitrate != isis_vu_e__bitrate_tlm__9600bps))//probably trxvu restarted by wdt
		SetFreqAndBitrate();

	onCmdCount = GetNumberOfFramesInBuffer();
	if(onCmdCount>0) {
		//printf("Command in bufffer =%d\n", onCmdCount);//TODO:remove
		//get the online command
		res = GetOnlineCommand(&cmd);
		if(res==cmd_command_found)
		{
			logg(TRXInfo, "I:Getting the online command success\n");
			//Reset WD communication with earth by saving current time as last communication time in FRAM
			ResetGroundCommWDT();

			if(g_antOpen==FALSE)
			{
				//update ant open to true
				g_antOpen=TRUE;
				logg(event, "V: TRXLogic.Antennas deployment stopped\n");
			}

			//Send Acknowledge to earth
			SendAckPacket(ACK_RECEIVE_COMM, cmd.ID, cmd.ordinal, data, length);

			//run command
			res = ActUponCommand(&cmd);
		}
		else
		{
			logg(TRXInfo, "I: Status: %d in getting the online command. \n", res);
		}
	}

	logg(TRXInfo, "I:TRX_Logic HandleIdleAndMuteTime\n");
	//check idle timer and mute timer
	HandleIdleAndMuteTime();

	logg(TRXInfo, "I:TRX_Logic HandleTransponderTime\n");
	//check transponder timer
	HandleTransponderTime();

	logg(TRXInfo, "I:TRX_Logic HandleOpenAnts\n");
	// open ant if not open
    HandleOpenAnts();

    logg(TRXInfo, "I:TRX_Logic BeaconLogic\n");
	//handle beacon
	BeaconLogic();

	return res;
}

 /* TRXVU Online Commands */

// Command to set idle state to on
int CMD_SetIdleOn(sat_packet_t *cmd)
{
	if (g_idle_flag==TRUE)
	{
		return 0;
	}
	else if (getTransponderMode() == TURN_TRANSPONDER_ON)
	{
		logg(TRXInfo, "W: Transponder is ON - Cannot turn ON idle \n");
		return -1;
	}

	logg(TRXInfo, "I:inside CMD_SetIdleOn()\n");
	int err = isis_vu_e__set_idle_state(ISIS_TRXVU_I2C_BUS_INDEX, isis_vu_e__onoff__on);
	if(err!=0)
		logg(error, "E: Error in setting trxvu idle on - %d\n", err);
	else {
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

Boolean areAntennasOpen()
{
	// get ant open flag from fram
	time_unix lastCommunicationTime;
	int err = FRAM_read((unsigned char *)&lastCommunicationTime, LAST_COMM_TIME_ADDR,  LAST_COMM_TIME_SIZE );
	if(err!=0 || lastCommunicationTime == UNIX_DEPLOY_DATE_JAN_D1_Y2020_SEC)
		g_antOpen=FALSE;
	else
		g_antOpen=TRUE;
	return g_antOpen;
}
