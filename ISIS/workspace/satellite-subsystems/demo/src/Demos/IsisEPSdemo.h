/*
 * IsisEPSdemo.h
 *
 *  Created on: 22 sep. 2015
 *      Author: lrot
 */

#ifndef ISISEPSDEMO_H_
#define ISISEPSDEMO_H_

#include <hal/boolean.h>


/***
 * Starts demo.
 * Calls Init and Menu in sequence.
 * Returns FALSE on failure to initialize.
 */
Boolean IsisEPSdemoMain(void);

/***
 * Initializes the IEPS subsystem driver.
 * Returns FALSE on failure.
 *
 * note:
 * Depends on an initialized I2C driver.
 * Initialize the I2C interface once before using
 * any of the subsystem library drivers
 */
Boolean IsisEPSdemoInit(void);

/***
 * Loop producing an interactive
 * text menu for invoking subsystem functions
 * note:
 * Depends on an initialized IsisEPS subsystem driver.
 */
void IsisEPSdemoLoop(void);

/***
 * (obsolete) Legacy function to start interactive session
 * Always returns TRUE
 *
 * Note:
 * Use IsisEPSdemoMain instead.
 */
Boolean IsisEPStest(void);

#endif /* ISISEPSDEMO_H_ */
