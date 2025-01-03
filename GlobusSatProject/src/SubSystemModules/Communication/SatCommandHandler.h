
#ifndef SATCOMMANDS_H_
#define SATCOMMANDS_H_

#define T3GBS 3 //GIVAT SHMUEL SAT ID IN PACKETS
#define T0ALL 0 //ALL SATS

#define MAX_COMMAND_DATA_LENGTH 235 //SIZE_TXFRAME maximum AX25 data field available for downlink
#define SAT_PACKET_HEADER_LENGTH 10 // short*2+char*4

//<! how many command can be saved in the buffer
#define MAX_NUM_OF_DELAYED_CMD (100)

typedef enum __attribute__ ((__packed__)) CommandHandlerErr{
	cmd_command_succsess = 0,				///< a successful operation. no errors
	cmd_command_found = 0,					///< a command was found
	cmd_no_command_found ,					///< no commands were found in command buffers
	cmd_index_out_of_bound,					///< index out of bound error
	cmd_null_pointer_error,					///< input parameter pointer is null
	cmd_execution_error 					///< an execution error has occured
}CommandHandlerErr;

typedef struct __attribute__ ((__packed__)) sat_packet_t
{
	//!!!change in header size requires update to SAT_PACKET_HEADER_LENGTH
	unsigned short ordinal;								///< ord number of packet in sequence
	unsigned char ID;							///< ID of the received/transmitted command
	unsigned char targetSat;                             ///< packet target satelite
	unsigned char cmd_type;								///< type of the command. according to SPL protocol
	unsigned char cmd_subtype;							///< sub-type of the command. according to SPL protocol
	unsigned short length;						///< length of the received data.
	unsigned short total;
	unsigned char data[MAX_COMMAND_DATA_LENGTH];///< data buffer

}sat_packet_t;

/*!
 * @brief parses given frame from TRXVU into 'sat_command_t' structure.
 * @param[in] data raw data from which to parse the SPL packet
 * @param[out] cmd pointer to parsed command buffer
 * @return	errors according to CommandHandlerErr
 */
CommandHandlerErr ParseDataToCommand(unsigned char * data, sat_packet_t *cmd);

/*!
 * @brief parses given frame from TRXVU into 'sat_command_t' structure.
 * @param[in] data data field of the SPL packet
 * @param[in] data_length length of data packet in bytes
 * @param[in] type command type
 * @param[in] subtype command subtype
 * @param[in] id the id of the specific command
 * @param[in] ord the order of the specific command
 * @param[out] cmd pointer to parsed command buffer
 * @return	errors according to CommandHandlerErr
 * @note helpful when assembling assembling a cmd for downlink. assemble
 */
CommandHandlerErr AssembleCommand(unsigned char *data, unsigned char data_length, unsigned char type,
		unsigned char subtype,unsigned short id, unsigned short ord, unsigned char targetSat, unsigned short total, sat_packet_t *cmd);

/*!
 * @brief returns an online command to be executed if there is one in the RX buffer.
 * @param[out] cmd pointer to parsed command from online TRXVU frame buffer
 * @return	errors according to CommandHandlerErr
 */
CommandHandlerErr GetOnlineCommand(sat_packet_t *cmd);

#endif /* SATCOMMANDS_H_ */
