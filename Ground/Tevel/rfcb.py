import socket
import struct
from colorama import Fore

from Tevel.EpsRaw import *
from Tevel.EpsEng import *
from Tevel.PVTelemetry import *
from Tevel.AntTelemetry import *
from Tevel.TrxvuTelemetry import *
from Tevel.Types import *

kissPrefix = b'\xc0\x00'
kissSuffix = b'\xc0'
groundAddress = b'\xa8p\x8e\x84\xa6@\xe0'
satAddress = b'\xa8p\x8e\x84\xa6@c'
control = b'\x03'
pid = b'\xf0'
ax25headerLen = 2 + 7 + 7 + 2
splheaderLen = 1 + 1 + 2 + 1 + 1 + 2


def handelBeacon(header, data):
    beacon = Beacon._make(struct.unpack(BeaconFormat, data))
    print(f'{Fore.RED}{beacon}')

def handelLogDump(header, data):
    time = struct.unpack('I', data[0:4])
    logData = data[4:]
    print(f'{Fore.BLUE}{header}')
    print(f'{time[0]}:{logData}')

def handleEpsRaw(header, data):
    time = struct.unpack('I', data[0:4])
    epsData = data[4:]
    epsRaw = EpsRaw._make(struct.unpack(gethousekeepingrawFMT, epsData))
    print(f'{Fore.CYAN}{header}')
    print(f'{time[0]}:{epsRaw}')

def handleEpsEng(header, data):
    time = struct.unpack('I', data[0:4])
    epsData = data[4:]
    epsEng = EpsEng._make(struct.unpack(gethousekeepingengFMT, epsData))
    print(f'{Fore.CYAN}{header}')
    print(f'{time[0]}:{epsEng}')

def handleTx(header, data):
    time = struct.unpack('I', data[0:4])
    txData = data[4:]
    tx = TxTelemetry._make(struct.unpack(txTlmFormat, txData))
    print(f'{Fore.CYAN}{header}')
    print(f'{time[0]}:{tx}')

def handleRx(header, data):
    time = struct.unpack('I', data[0:4])
    rxData = data[4:]
    rx = RxTelemetry._make(struct.unpack(rxTlmFormat, rxData))
    print(f'{Fore.CYAN}{header}')
    print(f'{time[0]}:{rx}')

def handlePV(header, data):
    time = struct.unpack('I', data[0:4])
    pvData = data[4:]
    pv = PVTelemetry._make(struct.unpack(pvTlmFormat, pvData))
    print(f'{Fore.CYAN}{header}')
    print(f'{time[0]}:{pv}')

def handleAnt(header, data):
    time = struct.unpack('I', data[0:4])
    antData = data[4:]
    ant = AntTelemetry._make(struct.unpack(antTlmFormat, antData))
    print(f'{Fore.CYAN}{header}')
    print(f'{time[0]}:{ant}')

def handleAck(header, data):
    print(f'{Fore.GREEN}{header}:{AckSubtype(header.subtype)}:{data}')


def rcvPayload( headerStriped, socket ):
    header = SatPacketHeader._make(struct.unpack(SatPacketHeaderFormat, headerStriped))
    splData = socket.recv(header.length)
    if len(splData) != header.length:
        print('splData small')
        return
    if (header.type == Type.trxvu_cmd_type.value):
        if (header.subtype == Subtype.BEACON_SUBTYPE.value):
            handelBeacon(header, splData)
    elif (header.type == Type.ack_type.value):
        handleAck(header, splData)
    elif (header.type == Subtype.START_DUMP_SUBTYPE.value):
        if (header.subtype == Telemetry.tlm_log.value):
            handelLogDump(header, splData)
        elif (header.subtype == Telemetry.tlm_log_bckp.value):
            handelLogDump(header, splData)
        elif (header.subtype == Telemetry.tlm_eps_raw_mb.value):
            handleEpsRaw(header, splData)
        elif (header.subtype == Telemetry.tlm_eps_eng_mb.value):
            handleEpsEng(header, splData)
        elif (header.subtype == Telemetry.tlm_tx.value):
            handleTx(header, splData)
        elif (header.subtype == Telemetry.tlm_rx.value):
            handleRx(header, splData)
        elif (header.subtype == Telemetry.tlm_antenna.value):
            handleAnt(header, splData)
        elif (header.subtype == Telemetry.tlm_solar.value):
            handlePV(header, splData)
    supffix = socket.recv(1)
    if (supffix != kissSuffix):
        print('E:supffix')

def rcvPacket(packet, socket):
    packetLen = len(packet)
    if (packetLen != ax25headerLen):
        print('E:small packet')
        return
    packetPrefix = packet[0:2]
    if (packetPrefix != kissPrefix):
        print('E:kiss prefix')
        return
    destAddress = packet[2:9]
    if (destAddress != groundAddress):
        print('E:destination')
        return
    srcAddress = packet[9:16]

    payload = socket.recv(splheaderLen)
    if (len(payload) != splheaderLen):
        print('E:spl header small')
    rcvPayload(payload, socket)

def sendCommand(rfcb_socket):
    command = b'\x05\x00\x00\x08\x00\x69\x0A\x00\x0C\x00\x49\x1C\x0C\x5E\xA8\x1C\x0C\x5E'
    comm = b'%b%b%b%b%b%b%b' % (kissPrefix, satAddress, groundAddress, control, pid, command, kissSuffix)
    rfcb_socket.send(comm)

def connect():
    rfcb_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    rfcb_socket.connect(('127.0.0.1', 3211))
    #sendCommand(rfcb_socket)
    while 1:
        data = rfcb_socket.recv(ax25headerLen)
        rcvPacket(data, rfcb_socket)
def test():
    logg1 = b'\xc0\x00\xa8p\x8e\x84\xa6@\xe0\xa8p\x8e\x84\xa6@c\x03\xf0\x05\x00\x03\x08i\x0c\xc8\x00\n\x00g\n\x00\xe4\xed\x0b^I:Inside BeaconLogic()\n\x00\x00n \xe4\xed\x0b^I:TRASMITION ALLOWED\n\x00\n\x00\x00\xe4\xed\x0b^I:beacon time did not arrive\n\x00p: \xe4\xed\x0b^I:Main:TRX_Logic\n\x00not\xe4\xed\x0b^I:Main:TelemetryCollectorLogic\n\x00 i=\xe5\xed\x0b^I:--------------------Main loo\xc0'
    logg2 = b'\xc0\x00\xa8p\x8e\x84\xa6@\xe0\xa8p\x8e\x84\xa6@c\x03\xf0\x05\x00\x04\x08i\x0c\xc8\x00p: i= : 687  ------------\n\x00 da\xe5\xed\x0b^I:Main:EPS_Conditioning\n\x00n l\xe5\xed\x0b^I:Inside TRX_Logic()\n\x00g\n\x00\xe5\xed\x0b^I:Inside BeaconLogic()\n\x00\x00n \xe5\xed\x0b^I:TRASMITION ALLOWED\n\x00\n\x00\x00\xe5\xed\x0b^I:Command is: ID=65535, targetSat=8, type=0, \xc0'
    beacon = b"\xc0\x00\xa8p\x8e\x84\xa6@\xe0\xa8p\x8e\x84\xa6@c\x03\xf0\xff\xff\x00\x08\x00\x01\x1e\x00\xb5\x19\x0c^'\x1e\x13\x14\xff\x07\x01\x00\x95\x00\x00\x00$\x1e$\x1e\x00\xdb\xdc\xedv\x00\x00\x00\x00\x01\x00\xc0"
    rcvPacket(logg1)
    rcvPacket(beacon)
    rcvPacket(logg2)

def main():
    connect()


if __name__ == '__main__':
    main()