from enum import Enum
from typing import NamedTuple


class Type(Enum):
    trxvu_cmd_type = 0
    eps_cmd_type = 1
    telemetry_cmd_type = 2
    filesystem_cmd_type = 3
    managment_cmd_type = 4
    ack_type = 5

class Subtype(Enum):
    BEACON_SUBTYPE =		    0x01	#0b00000001
    MUTE_TRXVU = 			    0x11	#0b00010001
    UNMUTE_TRXVU = 			    0x88	#0b10001000
    TRXVU_IDLE_ON = 		    0x87
    TRXVU_IDLE_OFF = 		    0x86
    START_DUMP_SUBTYPE =        0x69	#0b01101001
    STOP_DUMP_SUBTYPE= 		    0x22	#0b00100010
    GET_BEACON_INTERVAL = 	    0x23	#0b00100011
    SET_BEACON_INTERVAL = 	    0x24	#0b00100100
    GET_TX_UPTIME = 		    0x66	#0b01100110
    GET_RX_UPTIME = 		    0x68	#0b01101000
    GET_NUM_OF_ONLINE_CMD =     0xA7	#0b10100111
    ANT_SET_ARM_STATUS = 	    0xB0	#0b10110000
    ANT_GET_ARM_STATUS = 	    0xB2	#0b10110010
    ANT_GET_UPTIME =		    0xB3    #0b10110011
    FORCE_ABORT_DUMP_SUBTYPE =  0x33    #0b00110011
    DELETE_DUMP_TASK =          0x44    #0b00100010
    TRANSPONDER_ON =            0x45
    TRANSPONDER_OFF =           0x46

class Telemetry(Enum):
    tlm_eps_raw_mb = 0
    tlm_eps_eng_mb = 1
    tlm_tx = 2
    tlm_rx = 3
    tlm_antenna = 4
    tlm_log = 5
    tlm_wod = 6

class AckSubtype(Enum):
    ACK_RECEIVE_COMM =      0x00			# when receive any packet

    ACK_RESET_WAKEUP =      0x7F,		    # after waking up from reset

    ACK_FS_RESET = 			0x70			# before going to filesystem reset
    ACK_TRXVU_SOFT_RESET = 	0x71			# before going to trxvu reset
    ACK_TRXVU_HARD_RESET =	0x72			# before going to trxvu reset
    ACK_ANTS_RESET = 		0x73			# before reseting antennas
    ACK_EPS_RESET = 		0x80			# before going to eps reset
    ACK_SOFT_RESET = 		0x81			# before going to reset
    ACK_HARD_RESET = 		0x82			# before starting hardware reset

    ACK_MEMORY_DELETE = 0x83			    # when memory delete is completed success

    ACK_UPDATE_TIME = 0x12					# after updating time
    ACK_UPDATE_BEACON_BIT_RATE = 0x13
    ACK_UPDATE_BEACON_TIME_DELAY = 0x14
    ACK_UPDATE_EPS_VOLTAGES = 0x15
    ACK_UPDATE_EPS_HEATER_VALUES = 0x16
    ACK_UPDATE_EPS_ALPHA = 0x17

    ACK_IDLE_ON = 0x18
    ACK_IDLE_OFF = 0x19
    ACK_MUTE = 0x8D
    ACK_UNMUTE = 0x8E
    ACK_TRANSPONDER_ON = 0x8F
    ACK_TRANSPONDER_OFF = 0x8C

    ACK_DUMP_START = 0x90
    ACK_DUMP_ABORT = 0x91
    ACK_DUMP_FINISHED = 0x92

    ACK_GENERIC_I2C_CMD = 0x93
    ACK_ARM_DISARM = 0x94					#after changing arm state of the ants
    ACK_REDEPLOY = 0x95
    ACK_ANT_CANCEL_DEP = 0x9E
    ACK_ANT_AUTO_DEP = 0x96
    ACK_FRAM_RESET = 0xA0
    ACK_TLM_SET_COLL_CYCLE = 0xA1
    ACK_SET_LOG_LEVEL = 0xA2

    ACK_PING = 0xAA
    ACK_UNKNOWN_SUBTYPE = 0xBB				#when the given subtype is unknown
    ACK_NO_ACK = 0xCC						#Do not send ACK
    ACK_ERROR_MSG = 0XFF 					# send this ACK when error has occurred

class SatPacketHeader(NamedTuple):
    id: int
    type: int
    target: int
    ord: int
    subtype: int
    length: int
SatPacketHeaderFormat = 'HBBHBB'

class Beacon(NamedTuple):
    sat_time: int			#< current Unix time of the satellites clock [sec]
    vbat: int				#< the current voltage on the battery [mV]
    volt_v0: int
    volt_5V: int			#< the current voltage on the 5V bus [mV]
    volt_3V3: int			#< the current voltage on the 3V3 bus [mV]
    charging_power: int		#< the current charging power [mW]
    consumed_power: int		#< the power consumed by the satellite [mW]
    electric_current: int	#< the up-to-date electric current of the battery [mA]
    current_v0: int
    current_3V3: int		#< the up-to-date 3.3 Volt bus current of the battery [mA]
    current_5V: int			#< the up-to-date 5 Volt bus current of the battery [mA]
    mcu_temp: int
    bat_temp: int
    volt_in_mppt1: int
    curr_in_mppt1: int
    volt_in_mppt2: int
    curr_in_mppt2: int
    volt_in_mppt3: int
    curr_in_mppt3: int
    sol_pan_t_0: int
    sol_pan_t_1: int
    sol_pan_t_2: int
    sol_pan_t_3: int
    sol_pan_t_4: int
    sol_pan_pd_0: int
    sol_pan_pd_1: int
    sol_pan_pd_2: int
    sol_pan_pd_3: int
    sol_pan_pd_4: int
    free_memory: int		#< number of bytes free in the satellites SD [byte]
    corrupt_bytes: int		#< number of currpted bytes in the memory	[bytes]
    number_of_resets: int   #< counts the number of resets the satellite has gone through [#]
BeaconFormat = 'IHHHHHHHHHHHHhhhhhhiiiiiIIIIIIIH'