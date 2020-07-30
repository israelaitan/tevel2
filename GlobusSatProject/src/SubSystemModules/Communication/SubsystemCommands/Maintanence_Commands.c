#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include "GlobalStandards.h"

#include <hal/Timing/Time.h>

#include <satellite-subsystems/IsisTRXVU.h>
#include <satellite-subsystems/IsisAntS.h>
#include <satellite-subsystems/isis_eps_driver.h>
#include <hcc/api_fat.h>
#include <hal/Drivers/I2C.h>
#include <stdlib.h>
#include <string.h>
#include <hal/errors.h>
#include "TLM_management.h"
#include "InitSystem.h"
#include "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "SubSystemModules/Communication/AckHandler.h"
#include "SubSystemModules/Maintenance/Maintenance.h"
#include "SubSystemModules/Housekepping/TelemetryCollector.h"
#include "SubSystemModules/Maintenance/Log.h"
#include "Maintanence_Commands.h"

int CMD_GenericI2C(sat_packet_t *cmd)
{
	logg(MTNInfo, "I:inside CMD_GenericI2C()\n");

	if(cmd == NULL || cmd->data == NULL){
		logg(error, "E: Input is NULL");
		return E_INPUT_POINTER_NULL;
	}

	int err = 0;
	unsigned char slaveAddr = 0;
	unsigned int size = 0;
	unsigned char *i2c_data = malloc(size);

	memcpy(&slaveAddr,cmd->data,sizeof(slaveAddr));
	memcpy(&size,cmd->data + sizeof(slaveAddr),sizeof(size));

	unsigned int offset = sizeof(slaveAddr) + sizeof(size);

	err = I2C_write((unsigned int)slaveAddr,cmd->data + offset, (cmd->length - offset));
	err = I2C_read((unsigned int)slaveAddr,i2c_data,size);
	if (err == E_NO_SS_ERR){
		TransmitDataAsSPL_Packet(cmd, i2c_data, size);
	}
	free(i2c_data);

	return err;
}

int CMD_FRAM_ReadAndTransmitt(sat_packet_t *cmd)
{
	logg(MTNInfo, "I:inside CMD_FRAM_ReadAndTransmitt()\n");

	if(cmd == NULL || cmd->data == NULL){
		logg(error, "E: Input is NULL");
		return E_INPUT_POINTER_NULL;
	}

	int err = 0;
	unsigned int addr = 0;
	unsigned int size = 0;

	memcpy(&addr, cmd->data, sizeof(addr));
	memcpy(&size, cmd->data + sizeof(addr),sizeof(size));

	unsigned char *read_data = malloc(size);
	if(NULL == read_data){
		return E_MEM_ALLOC;
	}

	err = FRAM_read(read_data, addr, size);
	if (err != 0){
		return err;
	}

	TransmitDataAsSPL_Packet(cmd, read_data, size);
	free(read_data);
	return err;
}

int CMD_FRAM_WriteAndTransmitt(sat_packet_t *cmd)
{
	logg(MTNInfo, "I:inside CMD_FRAM_WriteAndTransmitt()\n");

	if(cmd == NULL || cmd->data == NULL){
		logg(error, "E: Input is NULL");
		return E_INPUT_POINTER_NULL;
	}

	int err = 0;
	unsigned int addr = 0;
	unsigned short length = cmd->length;
	unsigned char *data = cmd->data;

	memcpy(&addr, cmd->data, sizeof(addr));

	err = FRAM_write(data + sizeof(addr), addr, length - sizeof(addr));
	if (err != 0){
		return err;
	}
	err = FRAM_read(data, addr, length - sizeof(addr));
	if (err != 0){
		return err;
	}
	TransmitDataAsSPL_Packet(cmd, data, length);
	return err;
}

int CMD_FRAM_Start(sat_packet_t *cmd)
{
	logg(MTNInfo, "I:inside CMD_FRAM_Start()\n");

	int err = 0;
	err = FRAM_start();
	TransmitDataAsSPL_Packet(cmd, (unsigned char*)&err, sizeof(err));
	return err;
}

int CMD_FRAM_Stop(sat_packet_t *cmd)
{
	logg(MTNInfo, "I:inside CMD_FRAM_Stop()\n");
	(void)cmd;
	int err = 0;
	FRAM_stop();
	return err;
}

int CMD_FRAM_Restart(sat_packet_t *cmd)
{
	logg(MTNInfo, "I:inside CMD_FRAM_Restart()\n");

	CMD_FRAM_Stop(cmd);
	return CMD_FRAM_Start(cmd);
}

int CMD_FRAM_GetDeviceID(sat_packet_t *cmd)
{
	logg(MTNInfo, "I:inside CMD_FRAM_GetDeviceID()\n");

	int err = 0;
	unsigned char id;
	FRAM_getDeviceID(&id);
	TransmitDataAsSPL_Packet(cmd, &id, sizeof(id));
	return err;
}

int CMD_UpdateSatTime(sat_packet_t *cmd)
{
	logg(MTNInfo, "I:inside CMD_UpdateSatTime()\n");

	if(cmd == NULL || cmd->data == NULL){
		logg(error, "E: Input is NULL");
		return E_INPUT_POINTER_NULL;
	}

	int err = 0;
	time_unix set_time = 0;
	memcpy(&set_time, cmd->data, sizeof(set_time));
	err = Time_setUnixEpoch(set_time);
	TransmitDataAsSPL_Packet(cmd, (unsigned char*)&set_time, sizeof(set_time));
	return err;
}

int CMD_GetSatTime(sat_packet_t *cmd)
{
	logg(MTNInfo, "I:inside CMD_GetSatTime()\n");

	int err = 0;
	time_unix curr_time = 0;
	err = Time_getUnixEpoch((unsigned int *)&curr_time);
	if (err != 0)
	{
		return err;
	}
	TransmitDataAsSPL_Packet(cmd, (unsigned char*)&curr_time, sizeof(curr_time));

	return err;
}

int CMD_GetSatUptime(sat_packet_t *cmd)
{
	logg(MTNInfo, "I:inside CMD_GetSatUptime()\n");

	int err = 0;
	time_unix uptime = 0;
	uptime = Time_getUptimeSeconds();
	TransmitDataAsSPL_Packet(cmd, (unsigned char*)&uptime, sizeof(uptime));
	return err;
}

int CMD_SoftTRXVU_ComponenetReset(sat_packet_t *cmd)
{
	logg(MTNInfo, "I:inside CMD_SoftTRXVU_ComponenetReset()\n");

	if(cmd == NULL || cmd->data == NULL){
		logg(error, "E: Input is NULL");
		return E_INPUT_POINTER_NULL;
	}

	int err = 0;
	ISIStrxvuComponent component;
	memcpy(&component, cmd->data, sizeof(component));

	err = IsisTrxvu_componentSoftReset(ISIS_TRXVU_I2C_BUS_INDEX, component);
	return err;
}

int CMD_HardTRXVU_ComponenetReset(sat_packet_t *cmd)
{
	logg(MTNInfo, "I:inside CMD_HardTRXVU_ComponenetReset()\n");

	if (cmd == NULL || cmd->data == NULL)
	{
		logg(error, "E: Input is NULL");
		return E_INPUT_POINTER_NULL;
	}

	int err = 0;
	ISIStrxvuComponent component;
	memcpy(&component, cmd->data, sizeof(component));

	err = IsisTrxvu_componentHardReset(ISIS_TRXVU_I2C_BUS_INDEX, component);
	return err;
}

int CMD_AntennaDeploy(sat_packet_t *cmd)
{
	logg(MTNInfo, "I:inside CMD_AntennaDeploy()\n");

	(void)cmd;
	int err = 0;
	err = autoDeploy();
	return err;
}

int CMD_setLogLevel(sat_packet_t *cmd)
{
	logg(MTNInfo, "I:inside CMD_setLogLevel()\n");

	if (cmd == NULL || cmd->data == NULL)
	{
		logg(error, "E: Input is NULL");
		return E_INPUT_POINTER_NULL;
	}

	//get log level
	char logLevel;
	memcpy(&logLevel,cmd->data,sizeof(logLevel));

	//set log level
	setLogLevel(logLevel);
	logg(event, "I: Set log level to: %d", logLevel);
	return 0;
}

int CMD_SetTLM_CollectionCycle(sat_packet_t *cmd)
{
	logg(MTNInfo, "I:inside CMD_SetTLM_CollectionCycle()\n");

	if (cmd == NULL || cmd->data == NULL)
	{
		logg(error, "E: Input is NULL");
		return E_INPUT_POINTER_NULL;
	}

	int err=0;

	char tlmComponent;
	int tlmFramAddress;
	int period;

	//get component from command
	memcpy(&tlmComponent,cmd->data,sizeof(tlmComponent));

	//get period
	memcpy(&period,cmd->data+sizeof(tlmComponent),sizeof(period));

	if (tlmComponent == eps_tlm)
	{
		tlmFramAddress = EPS_SAVE_TLM_PERIOD_ADDR;
		logg(MTNInfo, "Setting EPS TLM with period: %d\n", period);
	}
	else if (tlmComponent == trxvu_tlm)
	{
		tlmFramAddress = TRXVU_SAVE_TLM_PERIOD_ADDR;
		logg(MTNInfo, "Setting TRXVU TLM with period: %d\n", period);
	}
	else if (tlmComponent == ant_tlm)
	{
		tlmFramAddress = ANT_SAVE_TLM_PERIOD_ADDR;
		logg(MTNInfo, "Setting ANTENNAS TLM with period: %d\n", period);
	}
	else if (tlmComponent == solar_panel_tlm)
	{
		tlmFramAddress = SOLAR_SAVE_TLM_PERIOD_ADDR;
		logg(MTNInfo, "Setting SOLAR TLM with period: %d\n", period);
	}
	else if (tlmComponent == wod_tlm)
	{
		tlmFramAddress = WOD_SAVE_TLM_PERIOD_ADDR;
		logg(MTNInfo, "Setting WOD TLM with period: %d\n", period);
	}

	//Set periods in FRAM
	FRAM_write((unsigned char*)period, tlmFramAddress, sizeof(period));
	return err;
}

int CMD_GetTLM_CollectionCycle(sat_packet_t *cmd)
{
	logg(MTNInfo, "I:inside CMD_GetTLM_CollectionCycle()\n");

	if (cmd == NULL || cmd->data == NULL)
	{
		logg(error, "E: Input is NULL");
		return E_INPUT_POINTER_NULL;
	}

	int err=0;

	char tlmComponent;
	int tlmFramAddress;

	//get component from command
	memcpy(&tlmComponent,cmd->data,sizeof(tlmComponent));

	if (tlmComponent == eps_tlm)
	{
		tlmFramAddress = EPS_SAVE_TLM_PERIOD_ADDR;
		logg(MTNInfo, "Getting EPS TLM with period\n");
	}
	else if (tlmComponent == trxvu_tlm)
	{
		tlmFramAddress = TRXVU_SAVE_TLM_PERIOD_ADDR;
		logg(MTNInfo, "Getting TRXVU TLM with period.\n");
	}
	else if (tlmComponent == ant_tlm)
	{
		tlmFramAddress = ANT_SAVE_TLM_PERIOD_ADDR;
		logg(MTNInfo, "Getting ANTENNAS TLM with period.\n");
	}
	else if (tlmComponent == solar_panel_tlm)
	{
		tlmFramAddress = SOLAR_SAVE_TLM_PERIOD_ADDR;
		logg(MTNInfo, "Getting SOLAR TLM with period\n");
	}
	else if (tlmComponent == wod_tlm)
	{
		tlmFramAddress = WOD_SAVE_TLM_PERIOD_ADDR;
		logg(MTNInfo, "Getting WOD TLM with period\n");
	}

	//Get periods in FRAM
	int period;
	FRAM_read((unsigned char*)period, tlmFramAddress, sizeof(period));

	//sending period
	TransmitDataAsSPL_Packet(cmd, (unsigned char*)&period, sizeof(period));
	return err;
}

int CMD_ResetComponent(reset_type_t rst_type)
{
	logg(MTNInfo, "I:inside CMD_ResetComponent()\n");

	int err = 0;

	Boolean8bit reset_flag = TRUE_8BIT;

	switch (rst_type)
	{
	case reset_software:
		SendAnonymosAck(ACK_SOFT_RESET);
		FRAM_write(&reset_flag, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE);
		vTaskDelay(10);
		restart();
		break;

	case reset_hardware:
		SendAnonymosAck(ACK_HARD_RESET);
		//Rest hardware is performed by EPS reset - so no break to execute next case as well

	case reset_eps:
		SendAnonymosAck(ACK_EPS_RESET);
		FRAM_write(&reset_flag, RESET_CMD_FLAG_ADDR, RESET_CMD_FLAG_SIZE);
		vTaskDelay(10);

		isis_eps__reset__to_t params;
		params.fields.rst_key = 0xA6;
		isis_eps__reset__from_t response;
		err = isis_eps__reset__tmtc( EPS_I2C_BUS_INDEX, &params , &response );
		break;

	case reset_trxvu_hard:
		SendAnonymosAck(ACK_TRXVU_HARD_RESET);
		err = IsisTrxvu_hardReset(ISIS_TRXVU_I2C_BUS_INDEX);
		vTaskDelay(100);
		break;

	case reset_trxvu_soft:
		SendAnonymosAck(ACK_TRXVU_SOFT_RESET);
		err = IsisTrxvu_softReset(ISIS_TRXVU_I2C_BUS_INDEX);
		vTaskDelay(100);
		break;

	case reset_filesystem:
		DeInitializeFS();
		vTaskDelay(10);
		err = (unsigned int) InitializeFS(FALSE);
		vTaskDelay(10);
		SendAckPacket(ACK_FS_RESET, 0xffff, 0xffff, (unsigned char*) &err, sizeof(err));
		break;

	case reset_ant_SideA:
		err = IsisAntS_reset(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideA);
		SendAckPacket(ACK_ANTS_RESET, 0xffff, 0xffff, (unsigned char*) &err, sizeof(err));
		break;

	case reset_ant_SideB:
		err = IsisAntS_reset(ISIS_TRXVU_I2C_BUS_INDEX, isisants_sideB);
		SendAckPacket(ACK_ANTS_RESET, 0xffff, 0xffff, (unsigned char*) &err, sizeof(err));
		break;

	default:
		SendAnonymosAck(ACK_UNKNOWN_SUBTYPE);
		break;
	}
	vTaskDelay(10);
	return err;
}
