
#ifndef SPL_H_
#define SPL_H_

typedef enum __attribute__ ((__packed__)) spl_command_type_t {
	trxvu_cmd_type,
	eps_cmd_type,
	telemetry_cmd_type,
	filesystem_cmd_type,
	managment_cmd_type,
	ack_type
}spl_command_type;


typedef enum __attribute__ ((__packed__)) ack_subtype_t
{
	ACK_RECEIVE_COMM = 		0x00,			// when receive any packet

	ACK_RESET_WAKEUP = 		0x7F,			// after waking up from reset

	ACK_FS_RESET = 			0x70,			// before going to filesystem reset
	ACK_TRXVU_SOFT_RESET = 	0x71,			// before going to trxvu reset
	ACK_TRXVU_HARD_RESET =	0x72,			// before going to trxvu reset
	ACK_ANTS_RESET = 		0x73,			// before reseting antennas
	ACK_EPS_RESET = 		0x80,			// before going to eps reset
	ACK_SOFT_RESET = 		0x81,			// before going to reset
	ACK_HARD_RESET = 		0x82,			// before starting hardware reset

	ACK_MEMORY_DELETE = 	0x83,				// when memory delete is completed success

	ACK_UPDATE_TIME = 		0x12,					// after updating time
	ACK_UPDATE_BEACON_TIME_DELAY = 0x14,
	ACK_UPDATE_EPS_VOLTAGES = 0x15,
	ACK_RESET_EPS_WD = 		0x16,
	ACK_UPDATE_EPS_ALPHA = 	0x17,

	ACK_IDLE_ON = 			0x18,
	ACK_IDLE_OFF = 			0x19,
	ACK_MUTE = 				0x8D,
	ACK_UNMUTE = 			0x8E,
	ACK_TRANSPONDER_ON = 	0x8F,
	ACK_TRANSPONDER_RSSI = 	0x8A,
	ACK_TRANSPONDER_OFF = 	0x8C,

	ACK_DUMP_START = 		0x90,
	ACK_DUMP_ABORT = 		0x91,
	ACK_DUMP_FINISHED = 	0x92,

	ACK_GENERIC_I2C_CMD = 	0x93,
	ACK_ARM_DISARM = 		0x94,					//after changing arm state of the ants
	ACK_REDEPLOY = 			0x95,
	ACK_ANT_CANCEL_DEP = 	0x9E,
	ACK_FRAM_RESET = 		0xA0,
	ACK_TLM_SET_COLL_CYCLE= 0xA1,
	ACK_SET_LOG_LEVEL = 	0xA2,
	ACK_GET_LOG_LEVEL = 	0xA3,

	ACK_FS_DELETE_ALL = 	0x0D,
	ACK_FS_DELETE_FILE = 	0x0E,

	ACK_PING = 				0xAA,
	ACK_UNKNOWN_SUBTYPE = 	0xBB,				//when the given subtype is unknown
	ACK_NO_ACK = 			0xCC,						//Do not send ACK
	ACK_ERROR_MSG = 		0XFF 					// send this ACK when error has occurred
}ack_subtype_t;


typedef enum __attribute__ ((__packed__)) trxvu_subtypes_t {
	BEACON_SUBTYPE,
	MUTE_TRXVU,
	UNMUTE_TRXVU,
	TRXVU_IDLE_ON,
	TRXVU_IDLE_OFF,
	START_DUMP_SUBTYPE,
	STOP_DUMP_SUBTYPE,
	GET_BEACON_INTERVAL,
	SET_BEACON_INTERVAL,
	GET_TX_UPTIME,
	GET_RX_UPTIME,
	GET_NUM_OF_ONLINE_CMD,
	ANT_SET_ARM_STATUS,
	ANT_GET_ARM_STATUS,
	ANT_GET_UPTIME,
	FORCE_ABORT_DUMP_SUBTYPE,
	DELETE_DUMP_TASK,
	TRANSPONDER_ON,
	TRANSPONDER_RSSI,
	TRANSPONDER_OFF
} trxvu_subtypes_t;


typedef enum __attribute__ ((__packed__)) eps_subtypes_t
{
	EPS_UPDATE_ALPHA,
	EPS_RESET_WD,
	EPS_GET_MODE
}eps_subtypes_t;


typedef enum __attribute__ ((__packed__)) telemetry_subtypes_t {
	TLM_GET_EPS_SUBTYPE,
	TLM_GET_SOLAR_SUBTYPE,
	TLM_GET_TRXVU_SUBTYPE,
	TLM_GET_ANTS_SUBTYPE
} telemetry_subtypes_t;

typedef enum __attribute__ ((__packed__)) files_subtypes_t {
	FS_IS_CORRUPT_SUBTYPE,
	FS_GET_FREE_SPACE_SUBTYPE,
	FS_GET_LAST_ERR_SUBTYPE,
	FS_DELETE_ALL_SUBTYPE,
	FS_DELETE_FILE_BY_TYPE_SUBTYPE,
	FS_DELETE_FILE_BY_TIME_SUBTYPE
} files_subtypes_t;


typedef enum __attribute__ ((__packed__)) management_subtypes_t {
	SOFT_RESET_SUBTYPE,
	HARD_RESET_SUBTYPE,
	TRXVU_SOFT_RESET_SUBTYPE,
	TRXVU_HARD_RESET_SUBTYPE,
	EPS_RESET_SUBTYPE,
	FS_RESET_SUBTYPE,
	ANTS_SIDE_A_RESET_SUBTYPE,
	ANTS_SIDE_B_RESET_SUBTYPE,
	ANTS_TURN_OFF_AUTO_DEP_SUBTYPE,
	ANTS_AUTO_DEPLOY_SUBTYPE,
	ANTS_CANCEL_DEPLOY_SUBTYPE,
	I2C_GEN_CMD_SUBTYPE,
	FRAM_READ_SUBTYPE,
	FRAM_WRITE_SUBTYPE,
	FRAM_RESTART_SUBTYPE,
	UPDATE_SAT_TIME_SUBTYPE,
	GET_SAT_TIME_SUBTYPE,
	GET_SAT_UP_TIME_SUBTYPE,
	TLM_SET_COLL_CYCLE_SUBTYPE,
	TLM_GET_COLL_CYCLE_SUBTYPE,
	SET_LOG_SUBTYPE,
	GET_LOG_SUBTYPE,
} management_subtypes_t;
//-----------------

#endif /* SPL_H_ */
