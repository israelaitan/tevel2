import socket
import struct
from typing import NamedTuple
from enum import Enum


kissPrefix = b'\xc0\x00'
kissSuffix = b'\xc0'
satAddress = b'\xa8p\x8e\x84\xa6@\xe0'
groundAddress = b'\xa8p\x8e\x84\xa6@c'
control = b'\x03'
pid = b'\xf0'
minPacketSize = 2 + 14 + 2 + 1

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


class SatPacketHeader(NamedTuple):
    id: int
    ord: int
    target: int
    type: int
    subtype: int
    length: int
SatPacketHeaderFormat = 'hbbbbh'

class Beacon(NamedTuple):
    sat_time: int			#< current Unix time of the satellites clock [sec]
    vbat: int				#< the current voltage on the battery [mV]
    volt_5V: int			#< the current voltage on the 5V bus [mV]
    volt_3V3: int			#< the current voltage on the 3V3 bus [mV]
    charging_power: int		#< the current charging power [mW]
    consumed_power: int		#< the power consumed by the satellite [mW]
    electric_current: int	#< the up-to-date electric current of the battery [mA]
    current_3V3: int		#< the up-to-date 3.3 Volt bus current of the battery [mA]
    current_5V: int			#< the up-to-date 5 Volt bus current of the battery [mA]
    free_memory: int		#< number of bytes free in the satellites SD [byte]
    corrupt_bytes: int		#< number of currpted bytes in the memory	[bytes]
    number_of_resets: int   #< counts the number of resets the satellite has gone through [#]
BeaconFormat = 'IHHHHHHHHIIH'

def handelBeacon(data):
    beacon = Beacon._make(struct.unpack(BeaconFormat, data))
    print(beacon)

def handelDump(data):
    print('bump')

def rcvPayload(payload):
    headerStriped = payload[0:8]
    header = SatPacketHeader._make(struct.unpack(SatPacketHeaderFormat, headerStriped))
    dataStripped = payload[8:8+header.length]
    if (header.type == Type.trxvu_cmd_type.value):
        if (header.subtype == Subtype.BEACON_SUBTYPE.value):
            handelBeacon(dataStripped)
        elif (header.subtype == Subtype.START_DUMP_SUBTYPE.value):
            handelDump(dataStripped)

def rcvPacket(packet):
    packetLen = len(packet)
    if (packetLen <= minPacketSize):
        print('E:small packet')
        return
    packetPrefix = packet[0:2]
    if (packetPrefix != kissPrefix):
        print('E:kiss prefix')
        return
    packetSuffix = packet[packetLen-1:packetLen]
    if (packetSuffix != kissSuffix):
        print('E:kiss suffix')
        return
    destAddress = packet[2:9]
    if (destAddress != satAddress):
        print('E:destination')
        return
    srcAddress = packet[9:16]
    payload = packet[18:packetLen-1]
    rcvPayload(payload)

def main():
    #rfcb_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    #rfcb_socket.connect(('127.0.0.1', 3211))
    #while 1:
    #    data = rfcb_socket.recv(1024)
    #    rcvPacket(data)
    #logg1 = b'\xc0\x00\xa8p\x8e\x84\xa6@\xe0\xa8p\x8e\x84\xa6@c\x03\xf0\x05\x00\x03\x08i\x0c\xc8\x00\n\x00g\n\x00\xe4\xed\x0b^I:Inside BeaconLogic()\n\x00\x00n \xe4\xed\x0b^I:TRASMITION ALLOWED\n\x00\n\x00\x00\xe4\xed\x0b^I:beacon time did not arrive\n\x00p: \xe4\xed\x0b^I:Main:TRX_Logic\n\x00not\xe4\xed\x0b^I:Main:TelemetryCollectorLogic\n\x00 i=\xe5\xed\x0b^I:--------------------Main loo\xc0'
    #logg2 = b'\xc0\x00\xa8p\x8e\x84\xa6@\xe0\xa8p\x8e\x84\xa6@c\x03\xf0\x05\x00\x04\x08i\x0c\xc8\x00p: i= : 687  ------------\n\x00 da\xe5\xed\x0b^I:Main:EPS_Conditioning\n\x00n l\xe5\xed\x0b^I:Inside TRX_Logic()\n\x00g\n\x00\xe5\xed\x0b^I:Inside BeaconLogic()\n\x00\x00n \xe5\xed\x0b^I:TRASMITION ALLOWED\n\x00\n\x00\x00\xe5\xed\x0b^I:Command is: ID=65535, targetSat=8, type=0, \xc0'
    beacon = b"\xc0\x00\xa8p\x8e\x84\xa6@\xe0\xa8p\x8e\x84\xa6@c\x03\xf0\xff\xff\x00\x08\x00\x01\x1e\x00\xb5\x19\x0c^'\x1e\x13\x14\xff\x07\x01\x00\x95\x00\x00\x00$\x1e$\x1e\x00\xdb\xdc\xedv\x00\x00\x00\x00\x01\x00\xc0"
    rcvPacket(beacon)

if __name__ == '__main__':
    main()