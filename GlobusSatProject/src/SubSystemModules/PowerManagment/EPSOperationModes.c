
#include "EPSOperationModes.h"
#include "GlobalStandards.h"



//TODO: update functions to only the relevant channels
channel_t g_system_state;
EpsState_t state;
Boolean g_low_volt_flag = FALSE; // set to true if in low voltage

int EnterFullMode()
{
	if(state == FullMode)
		return 0;
	int err = SetEPS_Channels((channel_t) CHANNELS_OFF);
	state = FullMode;
	EpsSetLowVoltageFlag(FALSE);
	return err;
}

int EnterCruiseMode()
{
	if(state == CruiseMode)
		return 0;
	int err = SetEPS_Channels((channel_t) CHANNELS_OFF);
	state = CruiseMode;
	EpsSetLowVoltageFlag(FALSE);
	return err;
}

int EnterSafeMode()
{
	if(state == SafeMode)
		return 0;
	int err = SetEPS_Channels((channel_t) CHANNELS_OFF);
	state = SafeMode;
	EpsSetLowVoltageFlag(FALSE);
	return err;
}

int EnterCriticalMode()
{
	if(state == CriticalMode)
		return 0;
	int err = SetEPS_Channels((channel_t) CHANNELS_OFF);
	state = CriticalMode;

	EpsSetLowVoltageFlag(TRUE);
	return err;
}

int SetEPS_Channels(channel_t channel)
{
	(void)channel;
#ifdef ISISEPS
	//TODO:adjust code to new driver
	/*ieps_statcmd_t code;
	ieps_obus_channel_t chnl;
	chnl.raw = g_system_state;
	int err = IsisEPS_outputBusGroupOn(EPS_I2C_BUS_INDEX, chnl, chnl, &code);
	if (err != 0){
		return err;
	}
	g_system_state = channel;

	chnl.raw = ~g_system_state;	//flip all bits in 'system_state'
	err = IsisEPS_outputBusGroupOff(EPS_I2C_BUS_INDEX, chnl, chnl, &code);
	if (err != 0){
		return err;
	}*/
	return 0;
#endif
#ifdef GOMEPS
#ifdef SET_EPS_CHANNELS
	gom_eps_channelstates_t st = {0};
	st.raw = channel;
	int err = GomEpsSetOutput(EPS_I2C_BUS_INDEX,st);
	if(0 != err){
		return err;
	}
	g_system_state = channel;
	return err;
#endif
#endif
	return 0;
}

EpsState_t GetSystemState()
{
	return state;
}

channel_t GetSystemChannelState()
{
	return g_system_state;
}

Boolean EpsGetLowVoltageFlag()
{
	return g_low_volt_flag;
}

void EpsSetLowVoltageFlag(Boolean low_volt_flag)
{
	g_low_volt_flag = low_volt_flag;
}

