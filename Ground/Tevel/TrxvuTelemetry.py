from typing import NamedTuple


class TxTelemetry(NamedTuple):
    tx_reflpwr : int
    tx_fwrdpwr : int
    bus_volt : int
    vutotal_curr : int
    vutx_curr : int
    vurx_curr : int
    vupa_curr : int
    pa_temp : int
    board_temp: int

txTlmFormat = 'HHHHHHHHH'

class RxTelemetry(NamedTuple):
    rx_doppler : int
    rx_rssi : int
    bus_volt : int
    vutotal_curr : int
    vutx_curr : int
    vurx_curr : int
    vupa_curr : int
    pa_temp : int
    board_temp : int

rxTlmFormat = 'HHHHHHHHH'