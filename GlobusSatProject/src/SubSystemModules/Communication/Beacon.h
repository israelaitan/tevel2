#ifndef BEACON_H_
#define BEACON_H_

//#define AWATEDTIME 20000
#include "SatCommandHandler.h"

/*!
 *	@brief initializes the relevant parameters for
 *	the beacon module ongoing work
 */
void InitBeaconParams();

/*!
 * @brief transmits beacon according to beacon logic
 */
void BeaconLogic();


/*!
 * @brief updates the time period between two beacons.
 * @param[in] intrvl the time to be set
 * @return	Errors according to <hal/errors.h>
 * 			E_PARAM_OUTOFBOUNDS if 'intrvl' is not legal(too big\too small)
 * @note updates in the FRAM as well as the private global variable.
 */
int UpdateBeaconInterval(sat_packet_t *cmd);




#endif /* BEACON_H_*/
