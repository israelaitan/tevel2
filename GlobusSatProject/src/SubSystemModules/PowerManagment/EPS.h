/*
 * @file	EPS.h
 * @brief	EPS- Energy Powering System.This system is incharge of the energy consumtion
 * 			the satellite and switching on and off power switches(5V, 3V3)
 * @see		inspect logic flowcharts thoroughly in order to write the code in a clean manner.
 */

#include "GlobalStandards.h"

/*!
 * @brief initializes the EPS subsystem.
 * @return	0 on success
 * 			-1 on EPS init error
 * 			-2 on Solar Panel init error
 * 			-3 on EPS threshold FRAM read error
 * 			-4 on EPS alpha FRAM read error
 * @note if FRAM read error than use default values of 'alpha' and 'eps_threshold_voltages'
 */
int EPS_Init();

/*!
 * @brief returns the current voltage on the battery
 * @param[out] vbat he current battery voltage
 * @return	0 on success
 * 			Error code according to <hal/errors.h>
 */
int GetBatteryVoltage(voltage_t *vbat);


