
#ifndef SATCOMMANDS_H_
#define SATCOMMANDS_H_

#define MAX_COMMAND_DATA_LENGTH 200 ///< maximum AX25 data field available for downlink

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
	unsigned short ID;							///< ID of the received/transmitted command
	unsigned short ordinal;						///< ord number of packet in sequence
	char cmd_type;								///< type of the command. according to SPL protocol
	char cmd_subtype;							///< sub-type of the command. according to SPL protocol
	unsigned short length;						///< length of the received data.
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
CommandHandlerErr AssembleCommand(unsigned char *data, unsigned int data_length, char type,
		char subtype,unsigned short id, unsigned short ord, sat_packet_t *cmd);

/*!
 * @brief returns a command to be executed if there is one in the delayed command buffer
 * @param[out] cmd pointer to parsed command
 * @return	errors according to CommandHandlerErr
 */
CommandHandlerErr GetDelayedCommand(sat_packet_t *cmd);

/*!
 * @brief sets the command to the first empty slot or the command with the farthest execution time
 * @param[in] cmd command to be added into the delated command buffer
 * @return	errors according to CommandHandlerErr
 */
CommandHandlerErr AddDelayedCommand(sat_packet_t *cmd);

/*!
 * @brief returns number of commands in the delayed command buffer
 * @return	#number number of delayed commands in buffer
 * @return	errors according to CommandHandlerErr
 */
CommandHandlerErr GetDelayedCommandBufferCount();

/*!
 * @brief returns an online command to be executed if there is one in the RX buffer.
 * @param[out] cmd pointer to parsed command from online TRXVU frame buffer
 * @return	errors according to CommandHandlerErr
 */
CommandHandlerErr GetOnlineCommand(sat_packet_t *cmd);

/*!
 * @brief returns a command in the delayed buffer at a specified index.
 * @param[in] index the index of the requested delayed command.
 * @param[out] cmd the output buffer for the requested delayed command.
 * @return	errors according to CommandHandlerErr
 */
CommandHandlerErr GetDelayedCommandByIndex(unsigned int index, sat_packet_t *cmd);

/*!
 * @brief deletes a command in the delayed buffer at a specified index.
 * @param[in] index index of the requested delayed command to be deleted.
 * @return	errors according to CommandHandlerErr
 */

CommandHandlerErr DeleteDelayedCommandByIndex(unsigned int index);


/*!
 * @brief deletes the entire delayed command buffer.
 * @warning deletes the ENTIRE delayed command buffer.
 * @return	errors according to CommandHandlerErr
 */
CommandHandlerErr DeleteDelayedBuffer();

#endif /* SATCOMMANDS_H_ */
