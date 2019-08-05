/*
 * gecko_use_cases.h
 *
 *  Created on: 2 jan. 2018
 *      Author: pbot
 */

#ifndef INCLUDE_SATELLITE_SUBSYSTEMS_SCS_GECKO_GECKO_USE_CASES_H_
#define INCLUDE_SATELLITE_SUBSYSTEMS_SCS_GECKO_GECKO_USE_CASES_H_

#include <hal/boolean.h>
#include <stdint.h>

int GECKO_UC_TakeImage( uint8_t adcGain, uint8_t pgaGain, uint32_t exposure, uint32_t frameAmount, uint32_t frameRate, uint32_t imageID );
int GECKO_UC_ReadImage( uint32_t imageID, uint32_t *buffer, Boolean fast );
int GECKO_UC_EraseBlock( uint32_t imageID );

#endif /* INCLUDE_SATELLITE_SUBSYSTEMS_SCS_GECKO_GECKO_USE_CASES_H_ */
