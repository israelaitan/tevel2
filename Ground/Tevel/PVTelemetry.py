from typing import NamedTuple

class PVTelemetry(NamedTuple):
    pv0: int
    pv1: int
    pv2: int
    pv3: int
    pv4: int

pvTlmFormat = 'IIIII'