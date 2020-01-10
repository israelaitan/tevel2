
#ifndef ACTUPONCOMMAND_H_
#define ACTUPONCOMMAND_H_

#include "GlobalStandards.h"
#include "SatCommandHandler.h"

/*!
 * @brief	sends the input command into sub-modules command routers.
 * 			The routers will send the command into its corresponding execution function.
 * @param[in] cmd the command to be executed
 * @return function execution error.
 * @note each routing function will send an ACK if execution error encountered.
 * @see SPL.h spl_command_type_t
 * @see CommandDictionary.h
 */
int ActUponCommand(sat_packet_t *cmd);

#endif /* ACTUPONCOMMAND_H_ */
