/*
 * trxuv_operations.c
 *
 *  Created on: Jul 4, 2012
 *      Author: marcoalfer
 */

#include "IsisTRXUVdemo.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>

#include <at91/utility/exithandler.h>
#include <at91/commons.h>
#include <at91/utility/trace.h>
#include <at91/peripherals/cp15/cp15.h>

#include <hal/Utility/util.h>
#include <hal/Timing/WatchDogTimer.h>
#include <hal/Drivers/I2C.h>
#include <hal/Drivers/LED.h>
#include <hal/boolean.h>
#include <hal/errors.h>

#include <satellite-subsystems/GomEPS.h>
#include <satellite-subsystems/cspaceADCS.h>
#include <satellite-subsystems/cspaceADCS_types.h>


#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if USING_GOM_EPS == 1
#include <SatelliteSubsystems/GomEPS.h>
#endif

////General Variables
#define TX_UPBOUND				30
#define TIMEOUT_UPBOUND			10

#define SIZE_RXFRAME	16
#define SIZE_TXFRAME	235

#define EPS_CHANNEL_ADCS_5V     gomeps_channel_0
#define EPS_CHANNEL_ADCS_3V3    gomeps_channel_3


/// Selection devices
typedef enum __attribute__ ((__packed__)) _adcs_pwrchoice_t
{
	sel_pwrsigccontrol = 0,
	sel_pwrmotccontrol = 1,
	sel_pwrcsense = 2,
	sel_pwrcubestar = 3,
	sel_pwrwheels = 4,
	sel_pwrmotor = 5,
	sel_pwrgps = 6
} adcs_pwrchoice_t;


static Boolean eslADC_ADCSRunModeTest(void)
{
	cspace_adcs_statetlm_t adcs_state;

	cspaceADCS_setRunMode(0, runmode_enabled);

	vTaskDelay(1000 / portTICK_RATE_MS);

	cspaceADCS_getStateTlm(0,&adcs_state);

	printf("Run mode enabled: %d \r\n", adcs_state.fields.curr_state.fields.run_mode);

	return TRUE;
}

static Boolean eslADC_ADCSGenInfo(void)
{
	cspace_adcs_geninfo_t info_data;

	cspaceADCS_getGeneralInfo(0, &info_data);

	printf("Note type: %d \r\n", info_data.fields.node_type);
	printf("Version Interface: %d \r\n", info_data.fields.version_interface);
	printf("Version major: %d \r\n", info_data.fields.version_major);
	printf("Version minor: %d \r\n", info_data.fields.version_minor);
	printf("Uptime secs: %d \r\n", info_data.fields.uptime_secs);
	printf("Uptime millisecs: %d \r\n", info_data.fields.uptime_millisecs);

	return TRUE;
}

static Boolean eslADC_ADCSCommStatusTest(void)
{
	cspace_adcs_commstat_t comm_status;

	cspaceADCS_getCommStatus(0, &comm_status);

	printf("tc counter: %d \r\n", comm_status.fields.tc_counter);
	printf("tlm_reqcounter: %d \r\n", comm_status.fields.tlm_reqcounter);
	if(comm_status.fields.tcbuffer_overrun)
	{
		printf("tcbuffer_overrun flag enable \r\n");
	}
	if(comm_status.fields.uart_protoc_err)
	{
		printf("uart_protoc_err flag enable\r\n");
	}
	if(comm_status.fields.uart_incomp_message)
	{
		printf("uart_incomp_message flag enable \r\n");
	}
	if(comm_status.fields.i2cTLM_readerr)
	{
		printf("i2cTLM_readerr flag enable \r\n");
	}
	if(comm_status.fields.i2cTC_bufferr)
	{
		printf("i2cTC_bufferr flag enable \r\n");
	}
	if(comm_status.fields.canTC_bufferr)
	{
		printf("canTC_bufferr flag enable \r\n");
	}

	return TRUE;
}

static Boolean eslADC_ADCSPowCtrlCmdTest(unsigned char pwr_choice)
{
	static cspace_adcs_powerdev_t set_ctrldev;
	static cspace_adcs_powerdev_t get_ctrldev;

	switch(pwr_choice)
	{
		case sel_pwrsigccontrol:
			set_ctrldev.fields.signal_cubecontrol = selection_on;
			cspaceADCS_setPwrCtrlDevice(0, set_ctrldev);
			vTaskDelay(1000 / portTICK_RATE_MS);
			cspaceADCS_getPwrCtrlDevice(0, &get_ctrldev);
			printf("Signal Cube Control: %d \r\n", get_ctrldev.fields.signal_cubecontrol);
			break;
		case sel_pwrmotccontrol:
			set_ctrldev.fields.motor_cubecontrol = selection_on;
			cspaceADCS_setPwrCtrlDevice(0, set_ctrldev);
			vTaskDelay(1000 / portTICK_RATE_MS);
			cspaceADCS_getPwrCtrlDevice(0, &get_ctrldev);
			printf("Motor Cube Control: %d \r\n", get_ctrldev.fields.motor_cubecontrol);
			break;
		case sel_pwrcsense:
			set_ctrldev.fields.pwr_cubesense = selection_on;
			cspaceADCS_setPwrCtrlDevice(0, set_ctrldev);
			vTaskDelay(1000 / portTICK_RATE_MS);
			cspaceADCS_getPwrCtrlDevice(0, &get_ctrldev);
			printf("Cube Sense: %d \r\n", get_ctrldev.fields.pwr_cubesense);
			break;
		case sel_pwrcubestar:
			set_ctrldev.fields.pwr_cubestar = selection_on;
			cspaceADCS_setPwrCtrlDevice(0, set_ctrldev);
			vTaskDelay(1000 / portTICK_RATE_MS);
			cspaceADCS_getPwrCtrlDevice(0, &get_ctrldev);
			printf("Cube Star: %d \r\n", get_ctrldev.fields.pwr_cubestar);
			break;
		case sel_pwrwheels:
			set_ctrldev.fields.pwr_cubewheel1 = selection_on;
			set_ctrldev.fields.pwr_cubewheel2 = selection_on;
			set_ctrldev.fields.pwr_cubewheel3 = selection_on;
			cspaceADCS_setPwrCtrlDevice(0, set_ctrldev);
			vTaskDelay(1000 / portTICK_RATE_MS);
			cspaceADCS_getPwrCtrlDevice(0, &get_ctrldev);
			printf("Power Wheel 1: %d \r\n", get_ctrldev.fields.pwr_cubewheel1);
			printf("Power Wheel 2: %d \r\n", get_ctrldev.fields.pwr_cubewheel2);
			printf("Power Wheel 3: %d \r\n", get_ctrldev.fields.pwr_cubewheel3);
			break;
		case sel_pwrmotor:
			set_ctrldev.fields.pwr_motor = selection_on;
			cspaceADCS_setPwrCtrlDevice(0, set_ctrldev);
			vTaskDelay(1000 / portTICK_RATE_MS);
			cspaceADCS_getPwrCtrlDevice(0, &get_ctrldev);
			printf("ADCS Motor: %d \r\n", get_ctrldev.fields.pwr_motor);
			break;
		case sel_pwrgps:
			set_ctrldev.fields.pwr_gps = selection_on;
			cspaceADCS_setPwrCtrlDevice(0, set_ctrldev);
			vTaskDelay(1000 / portTICK_RATE_MS);
			cspaceADCS_getPwrCtrlDevice(0, &get_ctrldev);
			printf("ADCS GPS: %d \r\n", get_ctrldev.fields.pwr_gps);
			break;
	}

	return TRUE;
}

static Boolean eslADC_setGetTimeTest(void)
{
	cspace_adcs_unixtm_t test_time;
	cspace_adcs_unixtm_t rx_time;

	test_time.fields.unix_time_sec = 10;
	test_time.fields.unix_time_millsec = 100;

	// Set time in cspaceADCS
	cspaceADCS_setCurrentTime(0, test_time);

	vTaskDelay(1000 / portTICK_RATE_MS);

	// Get time in cspaceADCS
	cspaceADCS_getCurrentTime(0, &rx_time);
	printf("Unix time ms: %d \r\n ", rx_time.fields.unix_time_millsec);
	printf("Unix time sec: %d \r\n ", rx_time.fields.unix_time_sec);

	return TRUE;
}

static Boolean eslADC_setResetTest(void)
{
	cspaceADCS_componentReset(0);

	return TRUE;
}

static Boolean eslADC_setGetWheelspeedTlmTest(void)
{
	cspace_adcs_wspeed_t wheelspeed;
	cspace_adcs_wspeed_t cmdwheelspeed;
	cspace_adcs_wspeed_t getwheelspeed;

	wheelspeed.fields.speedX = -1000;
	wheelspeed.fields.speedY = 2000;
	wheelspeed.fields.speedZ = -1000;

	cspaceADCS_setWheelSpeed(0, wheelspeed);

	vTaskDelay(1000 / portTICK_RATE_MS);

	cspaceADCS_getWheelSpeedCmd(0, &cmdwheelspeed);

	printf("Wheelspeed CMD x: %d [rpm]\r\n ", cmdwheelspeed.fields.speedX);
	printf("Wheelspeed CMD y: %d [rpm]\r\n ", cmdwheelspeed.fields.speedY);
	printf("Wheelspeed CMD z: %d [rpm]\r\n ", cmdwheelspeed.fields.speedZ);

	vTaskDelay(5000 / portTICK_RATE_MS);

	cspaceADCS_getWheelSpeed(0, &getwheelspeed);

	printf("Wheelspeed x: %d [rpm]\r\n ", getwheelspeed.fields.speedX);
	printf("Wheelspeed y: %d [rpm]\r\n ", getwheelspeed.fields.speedY);
	printf("Wheelspeed z: %d [rpm]\r\n ", getwheelspeed.fields.speedZ);

	vTaskDelay(5000 / portTICK_RATE_MS);

	wheelspeed.fields.speedX = 0;
	wheelspeed.fields.speedY = 0;
	wheelspeed.fields.speedZ = 0;

	cspaceADCS_setWheelSpeed(0, wheelspeed);

	return TRUE;
}

static Boolean eslADC_setMTQTest(void)
{
	static unsigned char toggle_flag1 = 1;
	cspace_adcs_magnetorq_t magpulse;
	double duty_cycle = 0.90;


	if(toggle_flag1 == 1)
	{
		magpulse.fields.magX = 0;
		magpulse.fields.magY = 0;
		magpulse.fields.magZ = (short)(duty_cycle * 1000.0);

		cspaceADCS_setMagOutput(0, magpulse);

		printf("Magnetorquer on \n\r");

		toggle_flag1 = 0;
	}
	else
	{
		magpulse.fields.magX = 0;
		magpulse.fields.magY = 0;
		magpulse.fields.magZ = 0;

		cspaceADCS_setMagOutput(0, magpulse);

		printf("Magnetorquer off \n\r");

		toggle_flag1 = 1;
	}

	return TRUE;
}

static Boolean eslADC_getMTQTest(void)
{
	cspace_adcs_magtorqcmd_t mag_cmd;

	cspaceADCS_getMagnetorquerCmd(0, &mag_cmd);

	printf("cmd magX: %d \r\n ", mag_cmd.fields.magXcmd);
	printf("cmd magY: %d \r\n ", mag_cmd.fields.magYcmd);
	printf("cmd magZ: %d \r\n ", mag_cmd.fields.magZcmd);

	return TRUE;
}

static Boolean eslADC_getMTMtest(void)
{
	cspace_adcs_magfieldvec_t magmeter;
	double conv_magfield = 0.0;

	cspaceADCS_getMagneticFieldVec(0, &magmeter);

	conv_magfield = (double) magmeter.fields.x_magfield * 0.01; //Conversion from raw to eng values
	printf("magX: %f [uT]\r\n ", conv_magfield);
	conv_magfield = (double) magmeter.fields.y_magfield * 0.01; //Conversion from raw to eng values
	printf("magY: %f [uT]\r\n ", conv_magfield);
	conv_magfield = (double) magmeter.fields.z_magfield * 0.01; //Conversion from raw to eng values
	printf("magZ: %f [uT]\r\n ", conv_magfield);

	return TRUE;
}

static Boolean eslADC_getCurrState(void)
{
	cspace_adcs_currstate_t curr_state;

	cspaceADCS_getCurrentState(0, &curr_state);

	if(curr_state.fields.run_mode)
	{
		printf("Run mode \r\n");
	}
	if(curr_state.fields.cctrlsig_enabled)
	{
		printf("Cube control signal enabled \r\n");
	}
	if(curr_state.fields.cctrlmot_enabled)
	{
		printf("Cube control motor enabled \r\n");
	}
	if(curr_state.fields.csense_enabled)
	{
		printf("Cube sense enabled \r\n");
	}
	if(curr_state.fields.cwheel1_enabled)
	{
		printf("Wheel 1 enabled \r\n");
		printf("CWheel1 error: 0x%x \r\n", curr_state.fields.cwheel1_comerr);
	}
	if(curr_state.fields.cwheel2_enabled)
	{
		printf("Wheel 2 enabled \r\n");
		printf("CWheel2 error: 0x%x \r\n", curr_state.fields.cwheel2_comerr);
	}
	if(curr_state.fields.cwheel3_enabled)
	{
		printf("Wheel 3 enabled \r\n");
		printf("CWheel3 error: 0x%x \r\n", curr_state.fields.cwheel3_comerr);
	}
	if(curr_state.fields.motdriv_enabled)
	{
		printf("Motor drive enabled \r\n");
	}

	return TRUE;
}

static Boolean eslADC_ADCSPowCtrlAllOffTest()
{
	cspace_adcs_powerdev_t set_ctrldev;
	cspace_adcs_powerdev_t get_ctrldev;

	set_ctrldev.fields.signal_cubecontrol = selection_off;
	set_ctrldev.fields.motor_cubecontrol = selection_off;
	set_ctrldev.fields.pwr_cubesense = selection_off;
	set_ctrldev.fields.pwr_cubestar = selection_off;
	set_ctrldev.fields.pwr_cubewheel1 = selection_off;
	set_ctrldev.fields.pwr_cubewheel2 = selection_off;
	set_ctrldev.fields.pwr_cubewheel3 = selection_off;
	set_ctrldev.fields.pwr_motor = selection_off;
	set_ctrldev.fields.pwr_gps = selection_off;
	cspaceADCS_setPwrCtrlDevice(0, set_ctrldev);

	vTaskDelay(1000 / portTICK_RATE_MS);
	cspaceADCS_getPwrCtrlDevice(0, &get_ctrldev);

	printf("Signal Cube Control: %d \r\n", get_ctrldev.fields.signal_cubecontrol);
	printf("Motor Cube Control: %d \r\n", get_ctrldev.fields.motor_cubecontrol);
	printf("Cube Sense: %d \r\n", get_ctrldev.fields.pwr_cubesense);
	printf("Cube Star: %d \r\n", get_ctrldev.fields.pwr_cubestar);
	printf("Power Wheel 1: %d \r\n", get_ctrldev.fields.pwr_cubewheel1);
	printf("Power Wheel 2: %d \r\n", get_ctrldev.fields.pwr_cubewheel2);
	printf("Power Wheel 3: %d \r\n", get_ctrldev.fields.pwr_cubewheel3);
	printf("ADCS Motor: %d \r\n", get_ctrldev.fields.pwr_motor);
	printf("ADCS GPS: %d \r\n", get_ctrldev.fields.pwr_gps);

	return TRUE;
}

static Boolean eslADC_ADCSPowTempTlm()
{
	cspace_adcs_pwtempms_t sensor_meas;
	double curr_conv = 0.0, temp_conv =0.0;

	cspaceADCS_getPowTempMeasTLM(0, &sensor_meas);

	curr_conv = (double) sensor_meas.fields.csense_curr.fields.cs_3v3_current * 0.1; //Conversion from raw to eng values
	printf("Cube Sense 3V3 current: %f [mA]\r\n", curr_conv);
	curr_conv = (double) sensor_meas.fields.csense_curr.fields.cs_cam1_sram_current * 0.1; //Conversion from raw to eng values
	printf("Cube Sense Cam1 SRAM current: %f [mA]\r\n", curr_conv);
	curr_conv = (double) sensor_meas.fields.csense_curr.fields.cs_cam2_sram_current * 0.1; //Conversion from raw to eng values
	printf("Cube Sense Cam2 SRAM current: %f [mA]\r\n", curr_conv);

	curr_conv = (double) sensor_meas.fields.cctrl_curr.fields.cc_3v3_current * 0.48828125; //Conversion from raw to eng values
	printf("Cube Control 3V3 current: %f [mA]\r\n", curr_conv);
	curr_conv = (double) sensor_meas.fields.cctrl_curr.fields.cc_5v_current * 0.48828125; //Conversion from raw to eng values
	printf("Cube Control 5V current: %f [mA]\r\n", curr_conv);
	curr_conv = (double) sensor_meas.fields.cctrl_curr.fields.cc_Vbat_current * 0.48828125; //Conversion from raw to eng values
	printf("Cube Control Batery Voltage current: %f [mA]\r\n", curr_conv);

	curr_conv = (double) sensor_meas.fields.wheel_curr.fields.wheel1Curr * 0.01; //Conversion from raw to eng values
	printf("Wheel current 1: %f [mA]\r\n", curr_conv);
	curr_conv = (double) sensor_meas.fields.wheel_curr.fields.wheel2Curr * 0.01; //Conversion from raw to eng values
	printf("Wheel current 2: %f [mA]\r\n", curr_conv);
	curr_conv = (double) sensor_meas.fields.wheel_curr.fields.wheel3Curr * 0.01; //Conversion from raw to eng values
	printf("Wheel current 3: %f [mA]\r\n", curr_conv);

	curr_conv = (double) sensor_meas.fields.misc_curr.fields.mag_curr * 0.01; //Conversion from raw to eng values
	printf("Magnetometer current: %f [mA]\r\n", curr_conv);

	printf("MCU temperature: %d [C]\r\n", sensor_meas.fields.misc_temp.fields.mcu_temp);
	temp_conv = (double) sensor_meas.fields.misc_temp.fields.mag_temp / 10; //Conversion from raw to eng values
	printf("Magnetometer temperature: %f [C]\r\n", temp_conv);

	printf("Sensor rate temperature x: %d [C]\r\n", sensor_meas.fields.rate_sentemp.fields.x_ratesen_temp);
	printf("Sensor rate temperature y: %d [C]\r\n", sensor_meas.fields.rate_sentemp.fields.y_ratesen_temp);
	printf("Sensor rate temperature z: %d [C]\r\n", sensor_meas.fields.rate_sentemp.fields.z_ratesen_temp);

	return TRUE;
}

static Boolean eslADC_getGeneralTelemetry(void)
{
	cspace_adcs_magfieldvec_t magfield_data;
	cspace_adcs_sunvec_t coarsesun_data;
	cspace_adcs_sunvec_t finesun_data;
	cspace_adcs_nadirvec_t nadirvec_data;
	cspace_adcs_angrate_t angrate_data;

	double conv_magfield = 0, conv_sunsen = 0, conv_nadir = 0, conv_angrate = 0;

	cspaceADCS_getMagneticFieldVec(0, &magfield_data);
	cspaceADCS_getCoarseSunVec(0, &coarsesun_data);
	cspaceADCS_getFineSunVec(0, &finesun_data);
	cspaceADCS_getNadirVector(0, &nadirvec_data);
	cspaceADCS_getSensorRates(0, &angrate_data);

	conv_magfield = (double) magfield_data.fields.x_magfield * 0.01; //Conversion from raw to eng values
	printf("Magnetic field x: %f [uT] \r\n", conv_magfield);
	conv_magfield = (double) magfield_data.fields.y_magfield * 0.01; //Conversion from raw to eng values
	printf("Magnetic field y: %f [uT] \r\n", conv_magfield);
	conv_magfield = (double) magfield_data.fields.z_magfield * 0.01; //Conversion from raw to eng values
	printf("Magnetic field z: %f [uT] \r\n", conv_magfield);

	conv_sunsen = (double) coarsesun_data.fields.x_sun_cmp * 0.0001; //Conversion from raw to eng values
	printf("Coarse Sun Vector x: %f \r\n", conv_sunsen);
	conv_sunsen = (double) coarsesun_data.fields.y_sun_cmp * 0.0001; //Conversion from raw to eng values
	printf("Coarse Sun Vector y: %f \r\n", conv_sunsen);
	conv_sunsen = (double) coarsesun_data.fields.z_sun_cmp * 0.0001; //Conversion from raw to eng values
	printf("Coarse Sun Vector z: %f \r\n", conv_sunsen);

	conv_sunsen = (double) finesun_data.fields.x_sun_cmp * 0.0001; //Conversion from raw to eng values
	printf("Fine Sun Vector x: %f \r\n", conv_sunsen);
	conv_sunsen = (double) finesun_data.fields.y_sun_cmp * 0.0001; //Conversion from raw to eng values
	printf("Fine Sun Vector y: %f \r\n", conv_sunsen);
	conv_sunsen = (double) finesun_data.fields.z_sun_cmp * 0.0001; //Conversion from raw to eng values
	printf("Fine Sun Vector z: %f \r\n", conv_sunsen);

	conv_nadir = (double) nadirvec_data.fields.x_nadir * 0.0001; //Conversion from raw to eng values
	printf("Nadir Vector x: %f \r\n", conv_nadir);
	conv_nadir = (double) nadirvec_data.fields.y_nadir * 0.0001; //Conversion from raw to eng values
	printf("Nadir Vector y: %f \r\n", conv_nadir);
	conv_nadir = (double) nadirvec_data.fields.z_nadir * 0.0001; //Conversion from raw to eng values
	printf("Nadir Vector z: %f \r\n", conv_nadir);

	conv_angrate = (double) angrate_data.fields.x_angrate * 0.01; //Conversion from raw to eng values
	printf("Sensor rates x: %f [deg/s]\r\n", conv_angrate);
	conv_angrate = (double) angrate_data.fields.y_angrate * 0.01; //Conversion from raw to eng values
	printf("Sensor rates y: %f [deg/s]\r\n", conv_angrate);
	conv_angrate = (double) angrate_data.fields.z_angrate * 0.01; //Conversion from raw to eng values
	printf("Sensor rates z: %f [deg/s]\r\n", conv_angrate);

	return TRUE;
}

static Boolean eslADC_getRawGeneralTlmSeveralCmd(void)
{
	cspace_adcs_rawcam1_t rawcam1;
	cspace_adcs_rawcam2_t rawcam2;
	cspace_adcs_rawcss1_6_t rawcss_group1;
	cspace_adcs_rawcss7_10_t rawcss_group2;
	cspace_adcs_rawmagmeter_t rawmagmeter;

	cspaceADCS_getRawCam1Sensor(0, &rawcam1);
	cspaceADCS_getRawCam2Sensor(0, &rawcam2);
	cspaceADCS_getRawCss1_6Measurements(0, &rawcss_group1);
	cspaceADCS_getRawCss7_10Measurements(0, &rawcss_group2);
	cspaceADCS_getRawMagnetometerMeas(0, &rawmagmeter);

	printf("Raw Cam1 Centroid x: %d \r\n", rawcam1.fields.cam1_centroid_x);
	printf("Raw Cam1 Centroid y: %d \r\n", rawcam1.fields.cam1_centroid_y);
	printf("Raw Cam1 Capture Status: %d \r\n", rawcam1.fields.capture_status);
	printf("Raw Cam1 Detection Results: %d \r\n", rawcam1.fields.detection_result);

	printf("Raw Cam2 Centroid x: %d \r\n", rawcam2.fields.cam2_centroid_x);
	printf("Raw Cam2 Centroid y: %d \r\n", rawcam2.fields.cam2_centroid_y);
	printf("Raw Cam2 Capture Status: %d \r\n", rawcam2.fields.capture_status);
	printf("Raw Cam2 Capture Status: %d \r\n", rawcam2.fields.detection_result);

	printf("Raw Course Sun Sensor 1: %d \r\n", rawcss_group1.fields.css_1);
	printf("Raw Course Sun Sensor 2: %d \r\n", rawcss_group1.fields.css_2);
	printf("Raw Course Sun Sensor 3: %d \r\n", rawcss_group1.fields.css_3);
	printf("Raw Course Sun Sensor 4: %d \r\n", rawcss_group1.fields.css_4);
	printf("Raw Course Sun Sensor 5: %d \r\n", rawcss_group1.fields.css_5);
	printf("Raw Course Sun Sensor 6: %d \r\n", rawcss_group1.fields.css_6);

	printf("Raw Course Sun Sensor 7: %d \r\n", rawcss_group2.fields.css_7);
	printf("Raw Course Sun Sensor 8: %d \r\n", rawcss_group2.fields.css_8);
	printf("Raw Course Sun Sensor 9: %d \r\n", rawcss_group2.fields.css_9);
	printf("Raw Course Sun Sensor 10: %d \r\n", rawcss_group2.fields.css_10);

	printf("Raw Magnetometer x: %d \r\n", rawmagmeter.fields.magnetic_x);
	printf("Raw Magnetometer y: %d \r\n", rawmagmeter.fields.magnetic_y);
	printf("Raw Magnetometer z: %d \r\n", rawmagmeter.fields.magnetic_z);

	return TRUE;
}

static Boolean eslADC_getRawGeneralTlmSingleCmd(void)
{
	cspace_adcs_rawsenms_t rawsensors;

	cspaceADCS_getRawSensorMeasurements(0, &rawsensors);


	printf("Raw Cam1 Centroid x: %d \r\n", rawsensors.fields.cam1_raw.fields.cam1_centroid_x);
	printf("Raw Cam1 Centroid y: %d \r\n", rawsensors.fields.cam1_raw.fields.cam1_centroid_y);
	printf("Raw Cam1 Capture Status: %d \r\n", rawsensors.fields.cam1_raw.fields.capture_status);
	printf("Raw Cam1 Detection Results: %d \r\n", rawsensors.fields.cam1_raw.fields.detection_result);

	printf("Raw Cam2 Centroid x: %d \r\n", rawsensors.fields.cam2_raw.fields.cam2_centroid_x);
	printf("Raw Cam2 Centroid y: %d \r\n", rawsensors.fields.cam2_raw.fields.cam2_centroid_y);
	printf("Raw Cam2 Capture Status: %d \r\n", rawsensors.fields.cam2_raw.fields.capture_status);
	printf("Raw Cam2 Capture Status: %d \r\n", rawsensors.fields.cam2_raw.fields.detection_result);

	printf("Raw Course Sun Sensor 1: %d \r\n", rawsensors.fields.css_raw1_6.fields.css_1);
	printf("Raw Course Sun Sensor 2: %d \r\n", rawsensors.fields.css_raw1_6.fields.css_2);
	printf("Raw Course Sun Sensor 3: %d \r\n", rawsensors.fields.css_raw1_6.fields.css_3);
	printf("Raw Course Sun Sensor 4: %d \r\n", rawsensors.fields.css_raw1_6.fields.css_4);
	printf("Raw Course Sun Sensor 5: %d \r\n", rawsensors.fields.css_raw1_6.fields.css_5);
	printf("Raw Course Sun Sensor 6: %d \r\n", rawsensors.fields.css_raw1_6.fields.css_6);

	printf("Raw Course Sun Sensor 7: %d \r\n", rawsensors.fields.css_raw7_10.fields.css_7);
	printf("Raw Course Sun Sensor 8: %d \r\n", rawsensors.fields.css_raw7_10.fields.css_8);
	printf("Raw Course Sun Sensor 9: %d \r\n", rawsensors.fields.css_raw7_10.fields.css_9);
	printf("Raw Course Sun Sensor 10: %d \r\n", rawsensors.fields.css_raw7_10.fields.css_10);

	printf("Raw Magnetometer x: %d \r\n", rawsensors.fields.magmeter_raw.fields.magnetic_x);
	printf("Raw Magnetometer y: %d \r\n", rawsensors.fields.magmeter_raw.fields.magnetic_y);
	printf("Raw Magnetometer z: %d \r\n", rawsensors.fields.magmeter_raw.fields.magnetic_z);

	return TRUE;
}

static Boolean eslADC_getCurrentMeasurements(void)
{
	cspace_adcs_csencurrms_t csense_curr;
	cspace_adcs_cctrlcurrms_t ccontrol_curr;
	double curr_conv = 0.0;

	cspaceADCS_getCSenseCurrentMeasurements(0, &csense_curr);
	cspaceADCS_getCControlCurrentMeasurements(0, &ccontrol_curr);

	curr_conv = (double) csense_curr.fields.cs_3v3_current * 0.1; //Conversion from raw to eng values
	printf("Cube Sense 3V3 current: %f [mA]\r\n", curr_conv);
	curr_conv = (double) csense_curr.fields.cs_cam1_sram_current * 0.1; //Conversion from raw to eng values
	printf("Cube Sense Cam1 SRAM current: %f [mA]\r\n", curr_conv);
	curr_conv = (double) csense_curr.fields.cs_cam2_sram_current * 0.1; //Conversion from raw to eng values
	printf("Cube Sense Cam2 SRAM current: %f [mA]\r\n", curr_conv);

	curr_conv = (double) ccontrol_curr.fields.cc_3v3_current * 0.48828125; //Conversion from raw to eng values
	printf("Cube Control 3V3 current: %f [mA]\r\n", curr_conv);
	curr_conv = (double) ccontrol_curr.fields.cc_5v_current * 0.48828125; //Conversion from raw to eng values
	printf("Cube Control 5V current: %f [mA]\r\n", curr_conv);
	curr_conv = (double) ccontrol_curr.fields.cc_Vbat_current * 0.48828125; //Conversion from raw to eng values
	printf("Cube Control Batery Voltage current: %f [mA]\r\n", curr_conv);

	return TRUE;
}

static Boolean eslADC_getADCActuators(void)
{
	cspace_adcs_actcmds_t actcmds;

	cspaceADCS_getActuatorsCmds(0, &actcmds);

	printf("Magnetorquer cmd x: %d \r\n ", actcmds.fields.magtorquer_cmds.fields.magX);
	printf("Magnetorquer cmd y: %d \r\n ", actcmds.fields.magtorquer_cmds.fields.magY);
	printf("Magnetorquer cmd z: %d \r\n ", actcmds.fields.magtorquer_cmds.fields.magZ);

	printf("Wheelspeed cmd x: %d [rpm]\r\n ", actcmds.fields.wheel_speed_cmds.fields.speedX);
	printf("Wheelspeed cmd y: %d [rpm]\r\n ", actcmds.fields.wheel_speed_cmds.fields.speedY);
	printf("Wheelspeed cmd z: %d [rpm]\r\n ", actcmds.fields.wheel_speed_cmds.fields.speedZ);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
static Boolean eslADC_getGPSStatusTime(void)
{
	cspace_adcs_rawgpssta_t curr_status;
	cspace_adcs_rawgpstm_t gps_time;

	cspaceADCS_getRawGpsStatus(0, &curr_status);

	printf("gps_logsetupmsg: %d \r\n", curr_status.fields.gps_logsetupmsg);
	printf("gps_counter_rangelog: %d \r\n", curr_status.fields.gps_counter_rangelog);
	printf("gps_counter_xyzlog: %d \r\n", curr_status.fields.gps_counter_xyzlog);
	printf("gps_satellites_used: %d \r\n", curr_status.fields.gps_satellites_used);
	printf("gps_satellites_tracked: %d \r\n", curr_status.fields.gps_satellites_tracked);
	printf("gps_status: \r\n");

	switch(curr_status.fields.gps_status)
	{
		case gps_solution_computed:
			printf("-- gps_solution_computed \r\n");
			break;
		case gps_solution_insufficientobs:
			printf("-- gps_solution_insufficientobs \r\n");
			break;
		case gps_solution_noconvergence:
			printf("-- gps_solution_noconvergence \r\n");
			break;
		case gps_solution_singularity:
			printf("-- gps_solution_singularity \r\n");
			break;
		case gps_solution_covtrace_exceed:
			printf("-- gps_solution_covtrace_exceed \r\n");
			break;
		case gps_solution_notyetconverged:
			printf("-- gps_solution_notyetconverged \r\n");
			break;
		case gps_solution_heightorvel_exceed:
			printf("-- gps_solution_heightorvel_exceed \r\n");
			break;
		case gps_solution_variance_exceed:
			printf("-- gps_solution_variance_exceed \r\n");
			break;
		case gps_solution_largeresidual:
			printf("-- gps_solution_largeresidual \r\n");
			break;
		case gps_solution_calccomparison:
			printf("-- gps_solution_calccomparison \r\n");
			break;
		case gps_solution_fixedposinvalid:
			printf("-- gps_solution_fixedposinvalid \r\n");
			break;
		case gps_solution_postypeunauthorized:
			printf("-- gps_solution_postypeunauthorized \r\n");
			break;
	}

	cspaceADCS_getRawGpsTime(0, &gps_time);

	printf("gps_refweeks: %d \r\n", gps_time.fields.gps_refweeks);
	printf("gps_timemillisecs: %ld \r\n", gps_time.fields.gps_timemillisecs);

	return TRUE;
}

static Boolean eslADC_getGPSRawData(void)
{
	cspace_adcs_rawgpsx_t raw_x;
	cspace_adcs_rawgpsy_t raw_y;
	cspace_adcs_rawgpsz_t raw_z;

	cspaceADCS_getRawGpsX(0, &raw_x);
	cspaceADCS_getRawGpsY(0, &raw_y);
	cspaceADCS_getRawGpsZ(0, &raw_z);

	printf("ecef_position_x: %ld \r\n", raw_x.fields.ecef_position_x);
	printf("ecef_velocity_x: %d \r\n", raw_x.fields.ecef_velocity_x);

	printf("ecef_position_x: %ld \r\n", raw_y.fields.ecef_position_y);
	printf("ecef_velocity_x: %d \r\n", raw_y.fields.ecef_velocity_y);

	printf("ecef_position_z: %ld \r\n", raw_z.fields.ecef_position_z);
	printf("ecef_velocity_z: %d \r\n", raw_z.fields.ecef_velocity_z);

	return TRUE;
}

static Boolean eslADC_getGPSRawMeasurements(void)
{
	cspace_adcs_rawgpsms_t raw_meas;

	cspaceADCS_getADCSRawGPSMeas(0, &raw_meas);

	printf("gps_logsetupmsg: %d \r\n", raw_meas.fields.gps_status.fields.gps_logsetupmsg);
	printf("gps_counter_rangelog: %d \r\n", raw_meas.fields.gps_status.fields.gps_counter_rangelog);
	printf("gps_counter_xyzlog: %d \r\n", raw_meas.fields.gps_status.fields.gps_counter_xyzlog);
	printf("gps_satellites_used: %d \r\n", raw_meas.fields.gps_status.fields.gps_satellites_used);
	printf("gps_satellites_tracked: %d \r\n", raw_meas.fields.gps_status.fields.gps_satellites_tracked);
	printf("gps_status: \r\n");

	switch(raw_meas.fields.gps_status.fields.gps_status)
	{
		case gps_solution_computed:
			printf("-- gps_solution_computed \r\n");
			break;
		case gps_solution_insufficientobs:
			printf("-- gps_solution_insufficientobs \r\n");
			break;
		case gps_solution_noconvergence:
			printf("-- gps_solution_noconvergence \r\n");
			break;
		case gps_solution_singularity:
			printf("-- gps_solution_singularity \r\n");
			break;
		case gps_solution_covtrace_exceed:
			printf("-- gps_solution_covtrace_exceed \r\n");
			break;
		case gps_solution_notyetconverged:
			printf("-- gps_solution_notyetconverged \r\n");
			break;
		case gps_solution_heightorvel_exceed:
			printf("-- gps_solution_heightorvel_exceed \r\n");
			break;
		case gps_solution_variance_exceed:
			printf("-- gps_solution_variance_exceed \r\n");
			break;
		case gps_solution_largeresidual:
			printf("-- gps_solution_largeresidual \r\n");
			break;
		case gps_solution_calccomparison:
			printf("-- gps_solution_calccomparison \r\n");
			break;
		case gps_solution_fixedposinvalid:
			printf("-- gps_solution_fixedposinvalid \r\n");
			break;
		case gps_solution_postypeunauthorized:
			printf("-- gps_solution_postypeunauthorized \r\n");
			break;
	}

	printf("ecef_position_x: %ld \r\n", raw_meas.fields.gps_x.fields.ecef_position_x);
	printf("ecef_velocity_x: %d \r\n", raw_meas.fields.gps_x.fields.ecef_velocity_x);

	printf("ecef_position_x: %ld \r\n", raw_meas.fields.gps_y.fields.ecef_position_y);
	printf("ecef_velocity_x: %d \r\n", raw_meas.fields.gps_y.fields.ecef_velocity_y);

	printf("ecef_position_z: %ld \r\n", raw_meas.fields.gps_z.fields.ecef_position_z);
	printf("ecef_velocity_z: %d \r\n", raw_meas.fields.gps_z.fields.ecef_velocity_z);

	printf("posX_stand_dev: %d \r\n", raw_meas.fields.pos_stdev.fields.posX_stand_dev);
	printf("posY_stand_dev: %d \r\n", raw_meas.fields.pos_stdev.fields.posY_stand_dev);
	printf("posZ_stand_dev: %d \r\n", raw_meas.fields.pos_stdev.fields.posZ_stand_dev);

	printf("velX_stand_dev: %d \r\n", raw_meas.fields.vel_stdev.fields.velX_stand_dev);
	printf("velY_stand_dev: %d \r\n", raw_meas.fields.vel_stdev.fields.velY_stand_dev);
	printf("velZ_stand_dev: %d \r\n", raw_meas.fields.vel_stdev.fields.velZ_stand_dev);

	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

static Boolean eslADC_getADCGroup1(void)
{
	cspace_adcs_wheelcurr_t wheelcurr;
	cspace_adcs_msctemp_t temptlm;
	cspace_adcs_ratesen_temp_t ratesens_temptlm;
	double curr_conv = 0.0, temp_conv = 0.0;

	cspaceADCS_getWheelCurrentsTlm(0, &wheelcurr);
	cspaceADCS_getADCSTemperatureTlm(0, &temptlm);
	cspaceADCS_getRateSensorTempTlm(0, &ratesens_temptlm);

	curr_conv = (double) wheelcurr.fields.wheel1Curr * 0.01; //Conversion from raw to eng values
	printf("Wheel current 1: %f [mA]\r\n", curr_conv);
	curr_conv = (double) wheelcurr.fields.wheel2Curr * 0.01; //Conversion from raw to eng values
	printf("Wheel current 2: %f [mA]\r\n", curr_conv);
	curr_conv = (double) wheelcurr.fields.wheel3Curr * 0.01; //Conversion from raw to eng values
	printf("Wheel current 3: %f [mA]\r\n", curr_conv);

	temp_conv = (double) temptlm.fields.mag_temp / 10; //Conversion from raw to eng values

	printf("MCU temperature: %d \r\n", temptlm.fields.mcu_temp);
	printf("Magnetometer temperature: %f \r\n", temp_conv);

	printf("Sensor rate temperature x: %d [C]\r\n", ratesens_temptlm.fields.x_ratesen_temp);
	printf("Sensor rate temperature y: %d [C]\r\n", ratesens_temptlm.fields.y_ratesen_temp);
	printf("Sensor rate temperature z: %d [C]\r\n", ratesens_temptlm.fields.z_ratesen_temp);

	return TRUE;
}

static Boolean eslADC_getADCGroup2(void)
{
	cspace_adcs_measure_t adcs_meas;

	double conv_magfield = 0.0, conv_sunsen = 0.0, conv_nadir = 0.0, conv_angrate = 0.0;

	cspaceADCS_getADCSMeasurements(0, &adcs_meas);

	conv_magfield = (double) adcs_meas.fields.magfield.fields.x_magfield * 0.01; //Conversion from raw to eng values
	printf("Magnetic field x: %f [uT] \r\n", conv_magfield);
	conv_magfield = (double) adcs_meas.fields.magfield.fields.y_magfield * 0.01; //Conversion from raw to eng values
	printf("Magnetic field y: %f [uT] \r\n", conv_magfield);
	conv_magfield = (double) adcs_meas.fields.magfield.fields.z_magfield * 0.01; //Conversion from raw to eng values
	printf("Magnetic field z: %f [uT] \r\n", conv_magfield);

	conv_sunsen = (double) adcs_meas.fields.course_sun.fields.x_sun_cmp * 0.0001; //Conversion from raw to eng values
	printf("Coarse Sun Vector x: %f \r\n", conv_sunsen);
	conv_sunsen = (double) adcs_meas.fields.course_sun.fields.y_sun_cmp * 0.0001; //Conversion from raw to eng values
	printf("Coarse Sun Vector y: %f \r\n", conv_sunsen);
	conv_sunsen = (double) adcs_meas.fields.course_sun.fields.z_sun_cmp * 0.0001; //Conversion from raw to eng values
	printf("Coarse Sun Vector z: %f \r\n", conv_sunsen);

	conv_sunsen = (double) adcs_meas.fields.fine_sun.fields.x_sun_cmp * 0.0001; //Conversion from raw to eng values
	printf("Fine Sun Vector x: %f \r\n", conv_sunsen);
	conv_sunsen = (double) adcs_meas.fields.fine_sun.fields.y_sun_cmp * 0.0001; //Conversion from raw to eng values
	printf("Fine Sun Vector y: %f \r\n", conv_sunsen);
	conv_sunsen = (double) adcs_meas.fields.fine_sun.fields.z_sun_cmp * 0.0001; //Conversion from raw to eng values
	printf("Fine Sun Vector z: %f \r\n", conv_sunsen);

	conv_nadir = (double) adcs_meas.fields.nadir_vector.fields.x_nadir * 0.0001; //Conversion from raw to eng values
	printf("Nadir Vector x: %f \r\n", conv_nadir);
	conv_nadir = (double) adcs_meas.fields.nadir_vector.fields.y_nadir * 0.0001; //Conversion from raw to eng values
	printf("Nadir Vector y: %f \r\n", conv_nadir);
	conv_nadir = (double) adcs_meas.fields.nadir_vector.fields.z_nadir * 0.0001; //Conversion from raw to eng values
	printf("Nadir Vector z: %f \r\n", conv_nadir);

	conv_angrate = (double) adcs_meas.fields.angular_rate.fields.x_angrate * 0.01; //Conversion from raw to eng values
	printf("Sensor rates x: %f [deg/s]\r\n", conv_angrate);
	conv_angrate = (double) adcs_meas.fields.angular_rate.fields.y_angrate * 0.01; //Conversion from raw to eng values
	printf("Sensor rates y: %f [deg/s]\r\n", conv_angrate);
	conv_angrate = (double) adcs_meas.fields.angular_rate.fields.z_angrate * 0.01; //Conversion from raw to eng values
	printf("Sensor rates z: %f [deg/s]\r\n", conv_angrate);

	printf("Wheelspeed x: %d [rpm]\r\n ", adcs_meas.fields.wheel_speed.fields.speedX);
	printf("Wheelspeed y: %d [rpm]\r\n ", adcs_meas.fields.wheel_speed.fields.speedY);
	printf("Wheelspeed z: %d [rpm]\r\n ", adcs_meas.fields.wheel_speed.fields.speedZ);

	return TRUE;
}

static Boolean eslADC_getADCGroup3(void)
{
	cspace_adcs_estmetadata_t metadata;
	double conv_tempdata = 0.0;

	cspaceADCS_getEstimationMetadata(0, &metadata);

	conv_tempdata = (double) metadata.fields.angratecov.fields.angratecov_x * 0.001; //Conversion from raw to eng values
	printf("Ang rate conv x: %f \r\n ", conv_tempdata);
	conv_tempdata = (double) metadata.fields.angratecov.fields.angratecov_y * 0.001; //Conversion from raw to eng values
	printf("Ang rate conv y: %f \r\n ", conv_tempdata);
	conv_tempdata = (double) metadata.fields.angratecov.fields.angratecov_z * 0.001; //Conversion from raw to eng values
	printf("Ang rate conv z: %f \r\n ", conv_tempdata);

	conv_tempdata = (double) metadata.fields.errquaternion.fields.errquaternion_q1 * 0.0001; //Conversion from raw to eng values
	printf("Err quaternion 1: %f \r\n ", conv_tempdata);
	conv_tempdata = (double) metadata.fields.errquaternion.fields.errquaternion_q2 * 0.0001; //Conversion from raw to eng values
	printf("Err quaternion 2: %f \r\n ", conv_tempdata);
	conv_tempdata = (double) metadata.fields.errquaternion.fields.errquaternion_q3 * 0.0001; //Conversion from raw to eng values
	printf("Err quaternion 3: %f \r\n ", conv_tempdata);

	conv_tempdata = (double) metadata.fields.estgyrobias.fields.estgyrobias_x * 0.001; //Conversion from raw to eng values
	printf("Estimation gyro bias x: %f \r\n ", conv_tempdata);
	conv_tempdata = (double) metadata.fields.estgyrobias.fields.estgyrobias_y * 0.001; //Conversion from raw to eng values
	printf("Estimation gyro bias y: %f \r\n ", conv_tempdata);
	conv_tempdata = (double) metadata.fields.estgyrobias.fields.estgyrobias_z * 0.001; //Conversion from raw to eng values
	printf("Estimation gyro bias z: %f \r\n ", conv_tempdata);

	conv_tempdata = (double) metadata.fields.igrf_magfield.fields.igrf_magfield_x * 0.01; //Conversion from raw to eng values
	printf("IGRF magfield x: %f \r\n ", conv_tempdata);
	conv_tempdata = (double) metadata.fields.igrf_magfield.fields.igrf_magfield_y * 0.01; //Conversion from raw to eng values
	printf("IGRF magfield y: %f \r\n ", conv_tempdata);
	conv_tempdata = (double) metadata.fields.igrf_magfield.fields.igrf_magfield_z * 0.01; //Conversion from raw to eng values
	printf("IGRF magfield z: %f \r\n ", conv_tempdata);

	conv_tempdata = (double) metadata.fields.innovationvec.fields.innovationvec_x * 0.0001; //Conversion from raw to eng values
	printf("Innovation vec x: %f \r\n ", conv_tempdata);
	conv_tempdata = (double) metadata.fields.innovationvec.fields.innovationvec_y * 0.0001; //Conversion from raw to eng values
	printf("Innovation vec y: %f \r\n ", conv_tempdata);
	conv_tempdata = (double) metadata.fields.innovationvec.fields.innovationvec_z * 0.0001; //Conversion from raw to eng values
	printf("Innocation vec z: %f \r\n ", conv_tempdata);

	conv_tempdata = (double) metadata.fields.quatcovariancerms.fields.quatcovariancerms_q1 * 0.001; //Conversion from raw to eng values
	printf("Quat covariance q1: %f \r\n ", conv_tempdata);
	conv_tempdata = (double) metadata.fields.quatcovariancerms.fields.quatcovariancerms_q2 * 0.001; //Conversion from raw to eng values
	printf("Quat covariance q2: %f \r\n ", conv_tempdata);
	conv_tempdata = (double) metadata.fields.quatcovariancerms.fields.quatcovariancerms_q3 * 0.001; //Conversion from raw to eng values
	printf("Quat covariance q3: %f \r\n ", conv_tempdata);

	conv_tempdata = (double) metadata.fields.sunvecmodel.fields.sunvecmodel_x * 0.0001; //Conversion from raw to eng values
	printf("Sun model vec x: %f \r\n ", conv_tempdata);
	conv_tempdata = (double) metadata.fields.sunvecmodel.fields.sunvecmodel_y * 0.0001; //Conversion from raw to eng values
	printf("Sun model vec y: %f \r\n ", conv_tempdata);
	conv_tempdata = (double) metadata.fields.sunvecmodel.fields.sunvecmodel_z * 0.0001; //Conversion from raw to eng values
	printf("Sun model vec z: %f \r\n ", conv_tempdata);

	return TRUE;
}

static Boolean eslADC_getADCGroup4(void)
{
	cspace_adcs_msctemp_t temp_tlm;
	double temp_conv = 0.0;

	cspaceADCS_getADCSTemperatureTlm(0, &temp_tlm);

	temp_conv = (double) temp_tlm.fields.mag_temp / 10;

	printf("MCU temperature: %d \r\n", temp_tlm.fields.mcu_temp);
	printf("Magnetometer temperature: %f \r\n", temp_conv);

	return TRUE;
}

static Boolean eslADC_getADCGroup5(void)
{
	cspace_adcs_exctm_t exc_tm;

	cspaceADCS_getADCSExcTimes(0, &exc_tm);

	printf("Time exc IGRF model: %d \r\n", exc_tm.fields.timeexc_igrfmodel);
	printf("Time exc SGP4Prop: %d \r\n", exc_tm.fields.timeexc_sgp4prop);
	printf("ADCS update time perf: %d \r\n", exc_tm.fields.timeperf_adcsupdate);
	printf("Act com time perf: %d \r\n", exc_tm.fields.timeperf_senactcom);

	return TRUE;
}

static Boolean eslADC_getADCGroup6(void)
{
	cspace_adcs_powerdev_t pwrdev;

	cspaceADCS_getPwrCtrlDevice(0, &pwrdev);

	if(pwrdev.fields.motor_cubecontrol)
	{
		printf("Cube control motor on \r\n");
	}
	if(pwrdev.fields.pwr_cubesense)
	{
		printf("Cube sense on\r\n");
	}
	if(pwrdev.fields.pwr_cubewheel1)
	{
		printf("Cube wheel 1 on \r\n");
	}
	if(pwrdev.fields.pwr_cubewheel2)
	{
		printf("Cube wheel 2 on \r\n");
	}
	if(pwrdev.fields.pwr_cubewheel3)
	{
		printf("Cube wheel 3 on \r\n");
	}
	if(pwrdev.fields.pwr_gps)
	{
		printf("GPS on \r\n");
	}
	if(pwrdev.fields.pwr_motor)
	{
		printf("Motor on \r\n");
	}
	if(pwrdev.fields.signal_cubecontrol)
	{
		printf("Cube control signal on \r\n");
	}

	return TRUE;
}

static Boolean eslADC_getADCGroup7(void)
{
	cspace_adcs_misccurr_t misc_curr;
	double curr_conv = 0.0;

	cspaceADCS_getMiscCurrentMeas(0, &misc_curr);

	curr_conv = (double) misc_curr.fields.mag_curr * 0.1; //Conversion from raw to eng values
	printf("Magnetometer current: %f \r\n", curr_conv);

	return TRUE;
}

static Boolean eslADC_getADCGroup8(void)
{
	cspace_adcs_rawmagmeter_t redmag;

	cspaceADCS_getADCRawRedMag(0, &redmag);

	printf("Magnetometer vector x: %d \r\n", redmag.fields.magnetic_x);
	printf("Magnetometer vector y: %d \r\n", redmag.fields.magnetic_y);
	printf("Magnetometer vector z: %d \r\n", redmag.fields.magnetic_z);

	return TRUE;
}

static Boolean eslADC_getADCGroup9(void)
{
	cspace_adcs_acp_info_t acploop_cmd;

	cspaceADCS_getACPExecutionState(0, &acploop_cmd);

	printf("Current execution: %d \r\n", acploop_cmd.fields.curr_exec);
	printf("Time to start: %d \r\n", acploop_cmd.fields.time_start);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////

static Boolean eslADC_ADCSOnOffTest(void)
{
	static unsigned char toggle_flag1 = 1;

	if(toggle_flag1 == 1)
	{
		// Turn on 3v3 for ADCS cube computer
		GomEpsSetSingleOutput(0, EPS_CHANNEL_ADCS_3V3, gomeps_channel_on, 0);
		// Turn on 5v for ADCS components
		GomEpsSetSingleOutput(0, EPS_CHANNEL_ADCS_5V, gomeps_channel_on, 0);

		printf("ADCS on \n\r");

		toggle_flag1 = 0;
	}
	else
	{
		// Turn on 3v3 for ADCS cube computer
		GomEpsSetSingleOutput(0, EPS_CHANNEL_ADCS_3V3, gomeps_channel_off, 0);
		// Turn on 5v for ADCS components
		GomEpsSetSingleOutput(0, EPS_CHANNEL_ADCS_5V, gomeps_channel_off, 0);

		printf("ADCS off \n\r");

		toggle_flag1 = 1;
	}

	return TRUE;
}

static Boolean selectAndExecute_cspaceADCSDemoTest(void)
{
	unsigned int selection = 0;
	Boolean offerMoreTests = TRUE;

	printf( "\n\r Select a test to perform: \n\r");
	printf("\t 1) ADCS Enable Run mode \n\r");
	printf("\t 2) ADCS General Information \n\r");
	printf("\t 3) ADCS Communication Status \n\r");
	printf("\t 4) Set get time Test \n\r");
	printf("\t 5) Set Reset Test \n\r");
	printf("\t 6) ADCS Power Signal CubeComputer segment \n\r");
	printf("\t 7) ADCS Power Motor CubeComputer segment \n\r");
	printf("\t 8) ADCS Power Signal CubeSense segment \n\r");
	printf("\t 9) ADCS Power Motor segment \n\r");
	printf("\t 10) Get Current state \n\r");
	printf("\t 11) Get Magnetometer result values \n\r");
	printf("\t 12) Set MTQ test \n\r");
	printf("\t 13) Get MTQ cmd test \n\r");
	printf("\t 14) ADCS Powers Wheels segment \n\r");
	printf("\t 15) Set get WheelspeedTlmTest \n\r");
	printf("\t 16) Get Actuators MTQ and Wheelspeed \n\r");
	printf("\t 17) ADCS Power and Temperature Telemetry \n\r");
	printf("\t 18) Get General Telemetry \n\r");
	printf("\t 19) Get Raw General Telemetry \n\r");
	printf("\t 20) Get Cube Current Measurements \n\r");
	printf("\t 21) ADCS Powers GPS segment \n\r");
	printf("\t 22) Get GPS status and time \n\r");
	printf("\t 23) Get GPS Raw data \n\r");
	printf("\t 24) Get GPS Complete Raw measurements \n\r");
	printf("\t 25) General tests \n\r");
	printf("\t 26) ADCS Power off all segments \n\r");
	printf("\t 27) ADCS on and off test (EPS lines) \n\r");
	printf("\t 28) Return to main menu \n\r");

	while(UTIL_DbguGetIntegerMinMax(&selection, 1, 28) == 0);

	switch(selection)
	{
		case 1:
			offerMoreTests = eslADC_ADCSRunModeTest();
			break;
		case 2:
			offerMoreTests = eslADC_ADCSGenInfo();
			break;
		case 3:
			offerMoreTests = eslADC_ADCSCommStatusTest();
			break;
		case 4:
			offerMoreTests = eslADC_setGetTimeTest();
			break;
		case 5:
			offerMoreTests = eslADC_setResetTest();
			break;
		case 6:
			offerMoreTests = eslADC_ADCSPowCtrlCmdTest(sel_pwrsigccontrol);
			break;
		case 7:
			offerMoreTests = eslADC_ADCSPowCtrlCmdTest(sel_pwrmotccontrol);
			break;
		case 8:
			offerMoreTests = eslADC_ADCSPowCtrlCmdTest(sel_pwrcsense);
			break;
		case 9:
			offerMoreTests = eslADC_ADCSPowCtrlCmdTest(sel_pwrmotor);
			break;
		case 10:
			offerMoreTests = eslADC_getCurrState();
			break;
		case 11:
			offerMoreTests = eslADC_getMTMtest();
			break;
		case 12:
			offerMoreTests = eslADC_setMTQTest();
			break;
		case 13:
			offerMoreTests = eslADC_getMTQTest();
			break;
		case 14:
			offerMoreTests = eslADC_ADCSPowCtrlCmdTest(sel_pwrwheels);
			break;
		case 15:
			offerMoreTests = eslADC_setGetWheelspeedTlmTest();
			break;
		case 16:
			offerMoreTests = eslADC_getADCActuators();
			break;
		case 17:
			offerMoreTests = eslADC_ADCSPowTempTlm();
			break;
		case 18:
			offerMoreTests = eslADC_getGeneralTelemetry();
			break;
		case 19:
			offerMoreTests = eslADC_getRawGeneralTlmSeveralCmd();//eslADC_getRawGeneralTlmSingleCmd();
			break;
		case 20:
			offerMoreTests = eslADC_getCurrentMeasurements();
			break;
		case 21:
			offerMoreTests = eslADC_ADCSPowCtrlCmdTest(sel_pwrgps);
			break;
		case 22:
			offerMoreTests = eslADC_getGPSStatusTime();
			break;
		case 23:
			offerMoreTests = eslADC_getGPSRawData();
			break;
		case 24:
			offerMoreTests = eslADC_getGPSRawMeasurements();
			break;
		case 25:
			offerMoreTests = eslADC_getADCGroup1();
			//offerMoreTests = eslADC_getADCGroup2();
			//offerMoreTests = eslADC_getADCGroup3();
			//offerMoreTests = eslADC_getADCGroup4();
			//offerMoreTests = eslADC_getADCGroup5();
			//offerMoreTests = eslADC_getADCGroup6();
			//offerMoreTests = eslADC_getADCGroup7();
			//offerMoreTests = eslADC_getADCGroup8();
			//offerMoreTests = eslADC_getADCGroup9();
			break;
		case 26:
			offerMoreTests = eslADC_ADCSPowCtrlAllOffTest();
			break;
		case 27:
			offerMoreTests = eslADC_ADCSOnOffTest();
			break;
		case 28:
			offerMoreTests = FALSE;
			break;

		default:
			break;
	}

	return offerMoreTests;
}

Boolean cspaceADCSdemoInit(void)
{
	unsigned char adcs_i2c[1] = {0x57};
	unsigned char gomEpsI2cAddress[1] = {0x02};
    int rv;

    rv = GomEpsInitialize(gomEpsI2cAddress, 1);
    if(rv != E_NO_SS_ERR && rv != E_IS_INITIALIZED)
    {
        // we have a problem. Indicate the error. But we'll gracefully exit to the higher menu instead of
        // hanging the code
        TRACE_ERROR("\n\r GomEpsInitialize() failed; err=%d! Exiting ... \n\r", rv);
        return FALSE;
    }

    rv = cspaceADCS_initialize(adcs_i2c, 1);
    if(rv != E_NO_SS_ERR && rv != E_IS_INITIALIZED)
    {
        // we have a problem. Indicate the error. But we'll gracefully exit to the higher menu instead of
        // hanging the code
        TRACE_ERROR("\n\r cspaceADCS_initialize() failed; err=%d! Exiting ... \n\r", rv);
        return FALSE;
    }

    return TRUE;
}

void cspaceADCSdemoLoop(void)
{
    Boolean offerMoreTests = FALSE;

    while(1)
    {
        offerMoreTests = selectAndExecute_cspaceADCSDemoTest(); // show the demo command line interface and handle commands

        if(offerMoreTests == FALSE)  // was exit/back
        {
            break;
        }
    }
}

Boolean cspaceADCSdemoMain(void)
{
    if(cspaceADCSdemoInit())                                 // initialize of I2C and IsisTRXVU subsystem drivers succeeded?
    {
    	cspaceADCSdemoLoop();                                // show the main IsisTRXVU demo interface and wait for user input
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    return TRUE;
}

Boolean cspaceADCStest(void)
{
	cspaceADCSdemoMain();

	return TRUE;
}
