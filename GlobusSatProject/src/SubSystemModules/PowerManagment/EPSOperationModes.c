
#include "EPSOperationModes.h"
#include "GlobalStandards.h"
#include "SubSystemModules/Payload/payload_drivers.h"



//TODO: update functions to only the relevant channels
channel_t g_system_state;
EpsState_t state;
Boolean g_low_volt_flag = FALSE; // set to true if in low voltage

int EnterFullMode()
{
	if(state == FullMode)
		return 0;
	int err = payloadTurnOn();
	state = FullMode;
	EpsSetLowVoltageFlag(FALSE);
	return err;
}

int EnterCruiseMode()
{
	if(state == CruiseMode)
		return 0;
	int err = payloadTurnOff();
	state = CruiseMode;
	EpsSetLowVoltageFlag(FALSE);
	return err;
}

int EnterSafeMode()
{
	if(state == SafeMode)
		return 0;
	int err = payloadTurnOff();
	state = SafeMode;
	EpsSetLowVoltageFlag(FALSE);
	return err;
}

int EnterCriticalMode()
{
	if(state == CriticalMode)
		return 0;
	int err = payloadTurnOff();
	state = CriticalMode;

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

