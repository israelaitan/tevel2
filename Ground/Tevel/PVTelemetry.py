from typing import NamedTuple

class PVTelemetry(NamedTuple):
    pv0: int
    pv1: int
    pv2: int
    pv3: int
    pv4: int
    pv5: int
    pv6: int
    pv7: int
    pv8: int

pvTlmFormat = 'IIIIIIIII'