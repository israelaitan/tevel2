#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <satellite-subsystems/isis_vu_e.h>
#include <satellite-subsystems/isis_ants.h>

#include "GlobalStandards.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "SubSystemModules/Housekepping/Dump.h"
#include "TRXVU_Commands.h"

int CMD_StartDump(sat_packet_t *cmd)
{
	int err = 0;
	err = DumpTelemetry(cmd);
	return err;
}

int CMD_SendDumpAbortRequest(sat_packet_t *cmd)
{
	(void)cmd;
	SendDumpAbortRequest();
	return 0;
}

int CMD_ForceDumpAbort(sat_packet_t *cmd)
{
	(void)cmd;
	AbortDump();
	return 0;
}


int CMD_MuteTRXVU(sat_packet_t *cmd)
{

	int err = 0;
	time_unix mute_duaration = 0;
	memcpy(&mute_duaration,cmd->data,sizeof(mute_duaration));
	err = muteTRXVU(mute_duaration);
	return err;
}

int CMD_UnMuteTRXVU(sat_packet_t *cmd)
{
	(void)cmd;
	UnMuteTRXVU();
	return 0;
}


int CMD_GetBeaconInterval(sat_packet_t *cmd)
{
	int err = 0;
	time_unix beacon_interval = 0;
	err = FRAM_read((unsigned char*) &beacon_interval,
			BEACON_INTERVAL_TIME_ADDR,
			BEACON_INTERVAL_TIME_SIZE);

	TransmitDataAsSPL_Packet(cmd, (unsigned char*) &beacon_interval,
			sizeof(beacon_interval));

	return err;
}

int CMD_SetBeaconInterval(sat_packet_t *cmd)
{
	int err = 0;
	time_unix interval = 0;
	err =  FRAM_write((unsigned char*) &cmd->data,
			BEACON_INTERVAL_TIME_ADDR,
			BEACON_INTERVAL_TIME_SIZE);

	err += FRAM_read((unsigned char*) &interval,
			BEACON_INTERVAL_TIME_ADDR,
			BEACON_INTERVAL_TIME_SIZE);

	TransmitDataAsSPL_Packet(cmd, (unsigned char*) &interval,sizeof(interval));
	return err;
}


int CMD_GetTxUptime(sat_packet_t *cmd)
{
	int err = 0;
	uint32_t uptime = 0;
	err = isis_vu_e__tx_uptime(ISIS_TRXVU_I2C_BUS_INDEX, (uint32_t*)&uptime);
	TransmitDataAsSPL_Packet(cmd, (unsigned char*)&uptime, sizeof(uptime));

	return err;
}

int CMD_GetRxUptime(sat_packet_t *cmd)
{
	int err = 0;
	uint32_t uptime = 0;
	err = isis_vu_e__rx_uptime(ISIS_TRXVU_I2C_BUS_INDEX,(uint32_t*) &uptime);
	TransmitDataAsSPL_Packet(cmd, (unsigned char*) &uptime, sizeof(uptime));

	return err;
}

int CMD_GetNumOfOnlineCommands(sat_packet_t *cmd)
{
	uint16_t frame_count_out = 0;
	int err = isis_vu_e__get_frame_count(ISIS_TRXVU_I2C_BUS_INDEX, &frame_count_out);
	TransmitDataAsSPL_Packet(cmd, (unsigned char*) &frame_count_out, sizeof(frame_count_out));
	return err;
}

int CMD_AntSetArmStatus(sat_packet_t *cmd)
{
	if (cmd == NULL || cmd->data == NULL)
		return E_INPUT_POINTER_NULL;
	int err = 0;
	uint8_t ant_side = cmd->data[0];
	uint8_t status = cmd->data[1];
	if (status)
		err = isis_ants__arm(ant_side);
	else
		err = isis_ants__disarm(ant_side);

	return err;
}

int CMD_AntGetArmStatus(sat_packet_t *cmd)
{
	int err = 0;
	isis_ants__get_status__from_t status;
	uint8_t ant_side;
	memcpy(&ant_side, cmd->data, sizeof(ant_side));

	err = isis_ants__get_status(ant_side, &status);
	TransmitDataAsSPL_Packet(cmd, (unsigned char*) &status, sizeof(status));

	return err;
}

int CMD_AntGetUptime(sat_packet_t *cmd)
{
	int err = 0;
	uint32_t uptime = 0;
	uint8_t ant_side;
	memcpy(&ant_side, cmd->data, sizeof(ant_side));
	err = isis_ants__get_uptime(ant_side, &uptime);
	TransmitDataAsSPL_Packet(cmd, (unsigned char*) &uptime, sizeof(uptime));
	return err;
}

int CMD_AntCancelDeployment(sat_packet_t *cmd)
{
	char ant_side;
	memcpy(&ant_side, cmd->data, sizeof(ant_side));
	int err = isis_ants__cancel_deploy(ant_side);
	return err;
}

int CMD_AntStopAutoDeployment(sat_packet_t *cmd)
{
	int err = 0;

	Boolean flag = areAntennasOpen();

	TransmitDataAsSPL_Packet(cmd, (unsigned char*) &flag, sizeof(flag));

	return err;
}

