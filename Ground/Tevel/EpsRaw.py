from typing import NamedTuple

class EpsRaw(NamedTuple):
    #unsigned char raw[116]   
    reply_header_stid : int
    reply_header_ivid : int
    reply_header_rc : int
    reply_header_bid : int
    reply_header_cmderr : int
    reply_header_stat : int

    #reserved : int
    
    volt_brdsup : int
    temp : int
    
    dist_input_volt : int
    dist_input_current : int
    dist_input_power : int
    batt_input_input_volt : int
    batt_input_current : int
    batt_input_power : int

    stat_obc_on : int
    stat_obc_ocf : int
    bat_stat : int
    temp2 : int
    temp3 : int
    volt_vd0 : int
    volt_vd1 : int
    volt_vd2 : int
    
    vip_obc00_volt : int
    vip_obc00_current : int
    vip_obc00_power  : int
    
    vip_obc01_volt : int
    vip_obc01_current : int
    vip_obc01_power : int
    
    vip_obc02_volt : int
    vip_obc02_current : int
    vip_obc02_power : int

    vip_obc03_volt : int
    vip_obc03_current : int
    vip_obc03_power : int
    
    vip_obc04_volt : int
    vip_obc04_current : int
    vip_obc04_power : int
    
    vip_obc05_volt : int
    vip_obc05_current : int
    vip_obc05_power : int
    
    vip_obc06_volt : int
    vip_obc06_current : int
    vip_obc06_power : int
    
    vip_obc07_volt : int
    vip_obc07_current : int
    vip_obc07_power : int
    
    vip_obc08_volt : int
    vip_obc08_current : int
    vip_obc08_power : int

    cc1_volt_in_mppt : int
    cc1_curr_in_mppt : int
    cc1_volt_out_mpt : int
    cc1_curr_out_mppt : int
     
    cc2_volt_in_mppt : int
    cc2_curr_in_mppt : int
    cc2_volt_out_mppt : int
    cc2_curr_out_mppt : int
     
    cc3_volt_in_mppt : int
    cc3_curr_in_mppt : int
    cc3_volt_out_mppt : int
    cc3_curr_out_mppt : int

gethousekeepingrawFMT = 'BBBBBBHHhhhhhhHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHHH'