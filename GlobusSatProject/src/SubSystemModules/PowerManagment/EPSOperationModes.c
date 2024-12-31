
#include "EPSOperationModes.h"
#include "GlobalStandards.h"
#include "SubSystemModules/Payload/payload_drivers.h"
#include "SubSystemModules/Maintenance/Log.h"



//TODO: update functions to only the relevant channels
channel_t g_system_state;
EpsState_t state;
Boolean g_low_volt_flag = FALSE; // set to true if in low voltage

int EnterFullMode()
{
	if(state == FullMode)
		return 0;
	logg(event, "V:EPS from state=%d -> state=%d\n", state, FullMode);
	state = FullMode;
	int err = payloadTurnOn();
	EpsSetLowVoltageFlag(FALSE);
	return err;
}

int EnterCruiseMode()
{
	if(state == CruiseMode)
		return 0;
	logg(event, "V:EPS from state=%d -> state=%d\n", state, CruiseMode);
	state = CruiseMode;
	int err = payloadTurnOff();
	EpsSetLowVoltageFlag(FALSE);
	return err;
}

int EnterSafeMode()
{
	if(state == SafeMode)
		return 0;
	logg(event, "V:EPS from state=%d -> state=%d\n", state, SafeMode);
	state = SafeMode;
	int err = payloadTurnOff();
	EpsSetLowVoltageFlag(FALSE);
	return err;
}

int EnterCriticalMode()
{
	if(state == CriticalMode)
		return 0;
	logg(event, "V:EPS from state=%d -> state=%d\n", state, CriticalMode);
	state = CriticalMode;
	int err = payloadTurnOff();
	EpsSetLowVoltageFlag(TRUE);
	return err;
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

