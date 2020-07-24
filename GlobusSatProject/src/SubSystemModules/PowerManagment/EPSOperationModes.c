
#include "EPSOperationModes.h"
#include "GlobalStandards.h"
#include <stdio.h>

#include <satellite-subsystems/isis_eps_driver.h>
#include "SubSystemModules/Maintenance/Log.h"

EpsState_t state;
Boolean g_low_volt_flag = FALSE; // set to true if in low voltage

int EnterFullMode()
{
	logg(EPSInfo,"I:EPS enter FullMode\n");
	if(state == FullMode)
		return 0;
	state = FullMode;
	EpsSetLowVoltageFlag(FALSE);
	return 0;
}

int EnterCruiseMode()
{
	logg(EPSInfo,"I:EPS enter CruiseMode\n");
	if(state == CruiseMode)
		return 0;
	state = CruiseMode;
	EpsSetLowVoltageFlag(FALSE);
	return 0;
}

int EnterSafeMode()
{
	logg(EPSInfo,"I:EPS enter SafeMode\n");
	if(state == SafeMode)
		return 0;
	state = SafeMode;
	EpsSetLowVoltageFlag(FALSE);
	return 0;
}

int EnterCriticalMode()
{
	logg(EPSInfo, "I:EPS enter CriticalMode\n");
	if(state == CriticalMode)
		return 0;
	state = CriticalMode;
	EpsSetLowVoltageFlag(TRUE);
	return 0;
}

EpsState_t GetSystemState()
{
	return state;
}

Boolean EpsGetLowVoltageFlag()
{
	return g_low_volt_flag;
}

void EpsSetLowVoltageFlag(Boolean low_volt_flag)
{
	g_low_volt_flag = low_volt_flag;
}

