#include "GlobalStandards.h"
#include <satellite-subsystems/isismepsv2_ivid5_piu.h>
#include <satellite-subsystems/IsisSolarPanelv2.h>
#include <stdlib.h>
#include <string.h>

#include  "SubSystemModules/Communication/TRXVU.h"
#include "SubSystemModules/PowerManagment/EPS.h"
#include "SubSystemModules/Communication/SatDataTx.h"
#include "EPS_Commands.h"
#include <hal/errors.h>
#include "SubSystemModules/Maintenance/Log.h"


int CMD_EPS_NOP(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	/* TODO:complete
	isis_eps__nop__from_t ieps_cmd;
	err = isis_eps__nop__tm( EPS_I2C_BUS_INDEX, &ieps_cmd );
	 */
	return err;
}

int CMD_EPS_ResetWDT(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	/* TODO:complete
	isis_eps__watchdog__from_t ieps_cmd;
	err = isis_eps__watchdog__tm( EPS_I2C_BUS_INDEX, &ieps_cmd );
	*/
	return err;
}

int CMD_GetEpsParameter(sat_packet_t *cmd)
{
	if (cmd == NULL || cmd->data == NULL)
	{
		return E_INPUT_POINTER_NULL;
	}

	int err = 0;
	/* TODO:complete
	unsigned short int id = 0;
	isis_eps__getparameter__to_t parameter;
	memcpy(&parameter, cmd->data, sizeof(id));

	//unsigned int par_size;
	//err = IsisEPS_getParSize(id, &par_size);
	//if (err != E_NO_SS_ERR){
	//	return err;
	//}

	//unsigned char *parameter = malloc(par_size);
	//if (NULL == parameter){
	//	return E_MEM_ALLOC;
	//}

	isis_eps__getparameter__from_t rsp_cmd;

	err = isis_eps__getparameter__tmtc( EPS_I2C_BUS_INDEX, &parameter , &rsp_cmd );

	if (err == E_NO_SS_ERR){
		TransmitDataAsSPL_Packet(cmd, (unsigned char *)&rsp_cmd, sizeof(rsp_cmd));
	}
	*/
	return err;
}

int CMD_SetEpsParemeter(sat_packet_t *cmd)
{
	if (cmd == NULL || cmd->data == NULL){
		return E_INPUT_POINTER_NULL;
	}

	int err = 0;
	/* TODO:complete
	//unsigned short int id = 0;
	isis_eps__setparameter__to_t parameter;
	memcpy(&parameter, cmd->data, sizeof(parameter));

	//unsigned int par_size;
	//err = IsisEPS_getParSize(id, &par_size);
	//if (err != E_NO_SS_ERR){
	//	return err;
	//}

	//unsigned char *parameter = malloc(par_size);
	//unsigned char *out_param = malloc(par_size);
	//if (NULL == parameter || out_param == NULL){
	//	return E_MEM_ALLOC;
	//}
	//memcpy(parameter, cmd->data + sizeof(id), par_size);

	isis_eps__setparameter__from_t rsp_cmd;
	err = isis_eps__setparameter__tmtc( EPS_I2C_BUS_INDEX,  &parameter , &rsp_cmd );


	if (err == E_NO_SS_ERR){
		TransmitDataAsSPL_Packet(cmd, (unsigned char *)&rsp_cmd, sizeof(rsp_cmd));
	}
	//free(parameter);
	//free(out_param);
	 */
	return err;
}

int CMD_ResetParameter(sat_packet_t *cmd)
{
	if (cmd == NULL || cmd->data == NULL){
		return E_INPUT_POINTER_NULL;
	}
	int err = 0;
	/* TODO:complete
	//unsigned short int id = 0;
	isis_eps__resetparameter__to_t parameter;
	memcpy(&parameter, cmd->data, sizeof(parameter));

	//unsigned int par_size;
	//err = IsisEPS_getParSize(id, &par_size);
	//if (err != E_NO_SS_ERR){
	//	return err;
	//}

	//unsigned char *parameter = malloc(par_size);
	//if (NULL == parameter){
	//	return E_MEM_ALLOC;
	//}
	//memcpy(parameter, cmd->data + sizeof(id), par_size);

	isis_eps__resetparameter__from_t rsp_cmd;
	err = isis_eps__resetparameter__tmtc( ISIS_TRXVU_I2C_BUS_INDEX, &parameter , &rsp_cmd );

	if (err == E_NO_SS_ERR)
		TransmitDataAsSPL_Packet(cmd, (unsigned char *)&rsp_cmd, sizeof(rsp_cmd));
	 */
	return err;
}

int CMD_ResetConfig(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	/* TODO:complete
	isis_eps__resetall__from_t rsp_cmd;
	isis_eps__resetall__to_t  params = {{0}};
	isis_eps__resetall__tmtc( ISIS_TRXVU_I2C_BUS_INDEX,  &params , &rsp_cmd );
	*/
	return err;
}

int CMD_LoadConfig(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	/* TODO:complete
	isis_eps__loadall__from_t rsp_cmd;
	isis_eps__loadall__to_t params = {{0}};
	err = isis_eps__loadall__tmtc( ISIS_TRXVU_I2C_BUS_INDEX, &params , &rsp_cmd );
    */
	return err;
}

int CMD_SaveConfig(sat_packet_t *cmd)
{
	(void)cmd;
	int err = 0;
	//CMD_SaveConfig, includeing CRC16- checksum and whatnot
	return err;
}

int CMD_SolarPanelWake(sat_packet_t *cmd)
{

	IsisSolarPanelv2_State_t state = 0;
	state = IsisSolarPanelv2_wakeup();
	TransmitDataAsSPL_Packet(cmd, &state, sizeof(state));
	return state;
}

int CMD_SolarPanelSleep(sat_packet_t *cmd)
{
	IsisSolarPanelv2_State_t state = 0;
	state = IsisSolarPanelv2_sleep();
	TransmitDataAsSPL_Packet(cmd, &state, sizeof(state));
	return state;
}

int CMD_GetSolarPanelState(sat_packet_t *cmd) {
	IsisSolarPanelv2_State_t state = 0;
	state = IsisSolarPanelv2_getState();
	TransmitDataAsSPL_Packet(cmd, &state, sizeof(state));
	return state;
}

int CMD_EPS_SetChannelStateOn(sat_packet_t *cmd) {
	uint8_t channel = 0;
	int res;
	memcpy(&channel, cmd->data, sizeof(channel));
	isismepsv2_ivid5_piu__replyheader_t response;
	if(channel == isismepsv2_ivid5_piu__eps_channel__channel_5v_sw3)
		res = payloadInit(FALSE);
	else
	{
		res =  isismepsv2_ivid5_piu__outputbuschannelon(EPS_I2C_BUS_INDEX, channel, &response);
			if (res)
				logg(error, "E:CMD_SetChannelStateOn=%d\n", res);
	}
	TransmitDataAsSPL_Packet(cmd, &response, sizeof(response));
	return res;
}

int CMD_EPS_SetChannelStateOff(sat_packet_t *cmd) {
	uint8_t channel = 0;
	int res;
	memcpy(&channel, cmd->data, sizeof(channel));
	isismepsv2_ivid5_piu__replyheader_t response;
	if(channel == isismepsv2_ivid5_piu__eps_channel__channel_5v_sw3)
		res = payloadTurnOff();
	else
	{
			res =  isismepsv2_ivid5_piu__outputbuschanneloff(EPS_I2C_BUS_INDEX, channel, &response);
			if (res)
				logg(error, "E:CMD_SetChannelStateOn=%d\n", res);
	}
	TransmitDataAsSPL_Packet(cmd, &response, sizeof(response));
	return res;
}


