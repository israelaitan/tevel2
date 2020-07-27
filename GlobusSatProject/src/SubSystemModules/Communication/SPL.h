
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

	ACK_MEMORY_DELETE = 0x83,				// when memory delete is completed success

	ACK_UPDATE_TIME = 0x12,					// after updating time
	ACK_UPDATE_BEACON_TIME_DELAY = 0x14,
	ACK_UPDATE_EPS_VOLTAGES = 0x15,
	ACK_RESET_EPS_WD = 0x16,
	ACK_UPDATE_EPS_ALPHA = 0x17,

	ACK_IDLE_ON = 0x18,
	ACK_IDLE_OFF = 0x19,
	ACK_MUTE = 0x8D,
	ACK_UNMUTE = 0x8E,
	ACK_TRANSPONDER_ON = 0x8F,
	ACK_TRANSPONDER_RSSI = 0x8A,
	ACK_TRANSPONDER_OFF = 0x8C,

	ACK_DUMP_START = 0x90,
	ACK_DUMP_ABORT = 0x91,
	ACK_DUMP_FINISHED = 0x92,

	ACK_GENERIC_I2C_CMD = 0x93,
	ACK_ARM_DISARM = 0x94,					//after changing arm state of the ants
	ACK_REDEPLOY = 0x95,
	ACK_ANT_CANCEL_DEP = 0x9E,
	ACK_FRAM_RESET = 0xA0,
	ACK_TLM_SET_COLL_CYCLE= 0xA1,

	ACK_FS_DELETE_ALL = 0x0D,
	ACK_FS_DELETE_FILE = 0x0E,

	ACK_PING = 0xAA,
	ACK_UNKNOWN_SUBTYPE = 0xBB,				//when the given subtype is unknown
	ACK_NO_ACK = 0xCC,						//Do not send ACK
	ACK_ERROR_MSG = 0XFF 					// send this ACK when error has occurred
}ack_subtype_t;


typedef enum __attribute__ ((__packed__)) trxvu_subtypes_t
{
	BEACON_SUBTYPE =		0x01,	//0b00000001
	MUTE_TRXVU = 			0x11,	//0b00010001
	UNMUTE_TRXVU = 			0x88,	//0b10001000
	TRXVU_IDLE_ON = 		0x87,
	TRXVU_IDLE_OFF = 		0x86,
	START_DUMP_SUBTYPE =    0x69,	//0b01101001
	STOP_DUMP_SUBTYPE= 		0x22,	//0b00100010
	GET_BEACON_INTERVAL = 	0x23,	//0b00100011
	SET_BEACON_INTERVAL = 	0x24,	//0b00100100
	GET_TX_UPTIME = 		0x66,	//0b01100110
	GET_RX_UPTIME = 		0x68,	//0b01101000
	GET_NUM_OF_ONLINE_CMD = 0xA7,	//0b10100111
	ANT_SET_ARM_STATUS = 	0xB0,	//0b10110000
	ANT_GET_ARM_STATUS = 	0xB2,	//0b10110010
	ANT_GET_UPTIME =		0xB3,	//0b10110011
	FORCE_ABORT_DUMP_SUBTYPE = 0x33,//0b00110011
	DELETE_DUMP_TASK = 0x44,		//0b00100010
	TRANSPONDER_ON = 0x45,
	TRANSPONDER_RSSI = 0x46,
	TRANSPONDER_OFF = 0x47
}trxvu_subtypes_t;


typedef enum __attribute__ ((__packed__)) eps_subtypes_t
{
	EPS_UPDATE_ALPHA = 0x00,
	EPS_RESET_WD = 0x01,
	EPS_GET_MODE = 0x02
}eps_subtypes_t;


typedef enum __attribute__ ((__packed__)) telemetry_subtypes_t
{
	FS_IS_CORRUPT_SUBTYPE = 0x0A,
	FS_GET_FREE_SPACE_SUBTYPE = 0x0B,
	FS_GET_LAST_ERR_SUBTYPE = 0x0C,
	FS_DELETE_ALL_SUBTYPE = 0x0D,
	FS_DELETE_FILE_BY_TYPE_SUBTYPE = 0x0E,
	FS_DELETE_FILE_BY_TIME_SUBTYPE = 0x0F,
	TLM_GET_EPS_SUBTYPE = 0x10,
	TLM_GET_SOLAR_SUBTYPE = 0x11,
	TLM_GET_TRXVU_SUBTYPE = 0x12,
	TLM_GET_ANTS_SUBTYPE = 0x13
}telemetry_subtypes_t;


typedef enum __attribute__ ((__packed__)) management_subtypes_t
{
	SOFT_RESET_SUBTYPE = 		0xA0,		//0b10101010
	HARD_RESET_SUBTYPE = 		0xA1,		//0b10101010
	TRXVU_SOFT_RESET_SUBTYPE =	0xA2,		//0b11000011
	TRXVU_HARD_RESET_SUBTYPE = 	0xA3,		//0b00111100
	EPS_RESET_SUBTYPE =			0xA4,		//0b10111011
	FS_RESET_SUBTYPE =			0xA5,		//0b11001100
	ANTS_SIDE_A_RESET_SUBTYPE=	0xA6,
	ANTS_SIDE_B_RESET_SUBTYPE=	0xA7,
	ANTS_TURN_OFF_AUTO_DEP_SUBTYPE= 0x87,
	ANTS_AUTO_DEPLOY_SUBTYPE=	0x90,		//144
	ANTS_CANCEL_DEPLOY_SUBTYPE = 0xB7,		//0b10110111
	I2C_GEN_CMD_SUBTYPE=		0x91,		//145
	FRAM_READ_SUBTYPE=  		0x92,		//146
	FRAM_WRITE_SUBTYPE=			0x93,		//147
	FRAM_RESTART_SUBTYPE=		0x94,		//148
	UPDATE_SAT_TIME_SUBTYPE= 	0x95,		//149
	GET_SAT_TIME_SUBTYPE= 		0x96,		//150
	GET_SAT_UP_TIME_SUBTYPE= 	0x97,		//151
	TLM_SET_COLL_CYCLE_SUBTYPE= 0x98		//152
}management_subtypes_t;
//-----------------

#endif /* SPL_H_ */
