
#ifndef SATDATATX_H_
#define SATDATATX_H_


#define GS_MODE_TIME 	(120)	///< how long does the satellite is in GS mode since last received command
#define MAX_MUTE_TIME 	(5400) 	///< max mute duration will be 90 minutes = 60 * 90 [sec]
#define MUTE_ON 		TRUE	///< mute is on flag
#define MUTE_OFF 		FALSE	///< mute is off flag

#include <satellite-subsystems/isis_vu_e.h>
#include "SubSystemModules/Communication/SatCommandHandler.h"

/*!
 * @breif Initializes data filed for transmission - semaphores, parameters from the FRAM
 * @return
 */
void InitTxModule();

/*!
 * @brief checks if transmission is possible on grounds of low voltage and TX mute
 * @return	TRUE if transmission is allowed
 * 			FALSE if transmission is denied
 */
Boolean CheckTransmitionAllowed();

/*!
 * @brief checks if currently transmissiting
 * @return	TRUE if transmistting
 * 			FALSE if NOT transmission
 */
Boolean IsTransmitting();


/*!
 * @brief	mutes the TRXVU for a specified time frame
 * @param[in] duration for how long will the satellite be in mute state
 * @return	0 in successful
 * 			-1 in failure
 */
int muteTRXVU(time_unix duration);

/*!
 * @brief Cancels TRXVU mute - transmission is now enabled
 */
void UnMuteTRXVU();

/*!
 * @brief returns the current state of the mute flag
 * @return	MUTE_ON if mute is on
 * 			MUTE_OFF if mute is off
 */
Boolean GetMuteFlag();

/*!
 * @brief checks if the Trxvu mute time has terminated
 * @return	TRUE if the termination time has arrived
 * 			FALSE if mute has yet to end.
 */
Boolean CheckForMuteEnd();

/*!
 * @brief returns the current baud rate of the TX
 * @param[out] bitrate the current baud rate of the satellite
 * @return errors according to <hal/errors.h>
 */
int GetTrxvuBitrate(isis_vu_e__bitrate_t *bitrate);

/*!
 * @brief returns number of online frames are in the TRX frame buffer
 * @return	#number number of packets available
 * 			-1 in case of failure
 */
int GetNumberOfFramesInBuffer();


/*!
 * @brief transmits data as SPL packet.
 * retrieves information from the command to send to the ground.
 * @param[in] cmd the given command.
 * @param[in] data the outout data.
 * @param[in] length number of bytes in 'data' fields.
 * @return errors according to <hal/errors.h>
 * @note this function can be used to send data to the ground as a response to a TL
 */
int TransmitDataAsSPL_Packet(sat_packet_t *cmd, unsigned char *data, unsigned int length);

/*!
 * @brief 	Transmits a packet according to the SPL protocol
 * @param[in] packet packet to be transmitted
 * @param[out] avalFrames Number of the available slots in the transmission buffer
 * of the VU_TC after the frame has been added. Set NULL to skip available slot count read-back.
 * @return    Error code according to <hal/errors.h>
 */
int TransmitSplPacket(sat_packet_t *packet, uint8_t *avalFrames);

#endif /* SATDATATX_H_ */
