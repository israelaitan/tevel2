
#ifndef MAINTANENCE_COMMANDS_H_
#define MAINTANENCE_COMMANDS_H_

#include "SubSystemModules/Communication/SatCommandHandler.h"

#define ANTENNA_DEPLOYMENT_TIMEOUT 10 //<! in seconds

typedef enum __attribute__ ((__packed__)) reset_type_t
{
	reset_software,
	reset_hardware,
	reset_eps,
	reset_trxvu_hard,
	reset_trxvu_soft,
	reset_ant_SideA,
	reset_ant_SideB,
	reset_filesystem

}reset_type_t;

/*!
 * @breif executes an I2C command according to 'I2C_write'
 * @return errors according to 'I2C_write'
 * @see I2C.h
 */
int CMD_GenericI2C(sat_packet_t *cmd);

/*!
 * @brief reads a section from the FRAM and transmittes to GS
 * @return errors according to FRAM.h
 * @see FRAM.h
 */
int CMD_FRAM_ReadAndTransmitt(sat_packet_t *cmd);

/*!
 * @breif writes a section onto the FRAM, re-reads the section and transmittes to GS
 * @return errors zccording to FRAM.h
 * @see FRAM.h
 */
int CMD_FRAM_WriteAndTransmitt(sat_packet_t *cmd);

/*!
 * @brief re-executes the 'FRAM_start' operation.
 * @return errors according to FRAM.h
 * @see FRAM.h
 */
int CMD_FRAM_Start(sat_packet_t *cmd);

/*!
 * @brief executes the 'FRAM_stop' operation.
 * @return errors according to FRAM.h
 * @see FRAM.h
 */
int CMD_FRAM_Stop(sat_packet_t *cmd);

/*!
 * @brief executes the 'FRAM_getDeviceID' operation.
 * @return errors according to FRAM.h
 * @see FRAM.h
 */
int CMD_FRAM_GetDeviceID(sat_packet_t *cmd);

/*!
 * @brief executes the 'Time_setUnixEpoch' operation.
 * @return errors according to Time.h
 * @see Time.h
 */
int CMD_UpdateSatTime(sat_packet_t *cmd);

/*!
 * @brief executes the 'Time_getUnixEpoch' operation.
 * @return errors according to Time.h
 * @see Time.h
 */
int CMD_GetSatTime(sat_packet_t *cmd);

/*!
 * @brief executes the 'Time_getUptimeSeconds' operation.
 * @return errors according to Time.h
 * @see Time.h
 */
int CMD_GetSatUptime(sat_packet_t *cmd);

/*!
 * @brief executes a TRXVU software reset according to 'IsisTrxvu_componentSoftReset'
 * @returns errors according to '<hal/errors.h>'
 * @see <hal/errors.h>
 */
int CMD_SoftTRXVU_ComponenetReset(sat_packet_t *cmd);

/*!
 * @brief executes a TRXVU hardware reset according to 'CMD_HardTRXVU_ComponenetReset'
 * @returns errors according to '<ha/errors.h>'
 * @see <hal/errors.h>
 */
int CMD_HardTRXVU_ComponenetReset(sat_packet_t *cmd);

/*!
 * @brief 	re-starts the antenna deploy procedure.
 * @note 	In case of antenna deploy failure we can try to
 * 			transmit this command with more transmission power.
 * @returns errors according to '<ha/errors.h>'
 * @see <hal/errors.h>
 *
 */
int CMD_AntennaDeploy(sat_packet_t *cmd);


/*!
 * @brief 	starts a reset according to 'reset_type_t'
 * 			The function will raise the reset flag and send an ACK before starting the reset.
 * @return 	0 on success
 * 			Error code according to <hal/errors.h>
 */
int CMD_ResetComponent(reset_type_t rst_type);

#endif /* MAINTANENCE_COMMANDS_H_ */
