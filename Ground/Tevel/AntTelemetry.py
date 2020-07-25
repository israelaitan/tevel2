from typing import NamedTuple

class AntTelemetry(NamedTuple):
    ants_temperature : int
    ants_deployment : int
    ants_uptime : int

antTlmFormat = 'HHI'
