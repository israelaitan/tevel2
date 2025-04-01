
#include "EPSOperationModes.h"
#include "GlobalStandards.h"
#include "SubSystemModules/Payload/payload_drivers.h"
#include "SubSystemModules/Maintenance/Log.h"
#include "SubSystemModules/PowerManagment/EPS.h"

time_unix state_transition_time = 0;
time_unix next_state_transition_time = 0;

channel_t g_system_state;
EpsState_t state = FullMode;
EpsState_t next_state = FullMode;
Boolean g_low_volt_flag = FALSE; // set to true if in low voltage

extern unsigned short gl_eps_state_linger;
extern unsigned short gl_eps_next_state_linger;


Boolean is_time_linger_passed() {
	time_unix curr_time;
	Time_getUnixEpoch(&curr_time);
	if ((curr_time - state_transition_time) > gl_eps_state_linger) {
		state_transition_time = curr_time;
		return TRUE;
	}
	return FALSE;
}

Boolean is_next_state_time_linger_passed(EpsState_t next_state_request) {
	time_unix curr_time;
	Time_getUnixEpoch(&curr_time);
	Boolean res =  FALSE;
	if (next_state == next_state_request) {
		if ((curr_time - next_state_transition_time) > gl_eps_next_state_linger) {
			next_state_transition_time = curr_time;
			res = TRUE;
		}
	} else {
		next_state = next_state_request;
		next_state_transition_time = curr_time;
	}
	return res;
}


int EnterFullMode(voltage_t curr_voltage)
{
	if(!is_time_linger_passed() || state == FullMode)
		return 0;
	if(!is_next_state_time_linger_passed(FullMode))
		return 0;
	logg(event, "V:EPS from state=%d -> state=%d v=%d\n", state, FullMode, curr_voltage);
	state = FullMode;
	int err = payloadInit(TRUE);
	EpsSetLowVoltageFlag(FALSE);
	return err;
}

int EnterCruiseMode(voltage_t curr_voltage)
{
	if(!is_time_linger_passed() || state == CruiseMode)
		return 0;
	if(!is_next_state_time_linger_passed(CruiseMode))
			return 0;
	logg(event, "V:EPS from state=%d -> state=%d v=%d\n", state, CruiseMode, curr_voltage);
	state = CruiseMode;
	int err = payloadTurnOff();
	EpsSetLowVoltageFlag(FALSE);
	return err;
}

int EnterSafeMode(voltage_t curr_voltage)
{
	if( !is_time_linger_passed() || state == SafeMode)
		return 0;
	if(!is_next_state_time_linger_passed(SafeMode))
		return 0;
	logg(event, "V:EPS from state=%d -> state=%d v=%d\n", state, SafeMode, curr_voltage);
	state = SafeMode;
	int err = payloadTurnOff();
	EpsSetLowVoltageFlag(FALSE);
	return err;
}

int EnterCriticalMode(voltage_t curr_voltage)
{
	if(!is_time_linger_passed() || state == CriticalMode)
		return 0;
	if(!is_next_state_time_linger_passed(CriticalMode))
			return 0;
	logg(event, "V:EPS from state=%d -> state=%d v=%d\n", state, CriticalMode, curr_voltage);
	state = CriticalMode;
	int err = payloadTurnOff();
	EpsSetLowVoltageFlag(TRUE);
	return err;
}

EpsState_t GetEPSSystemState()
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

