/*
 * isis_mtq_v2_types.h
 *
 * AUTOGENERATED CODE
 * Please do not perform manual edits
 * Generated using autogen v1.0.3
 *
 * Generated ICD version: 1.6
 * Generated from: isis_mtq_v2.yaml
 */

#ifndef ISIS_MTQ_V2_TYPES_H_
#define ISIS_MTQ_V2_TYPES_H_

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* DEFINES */

#define ISIS_MTQ_V2_NO_OP_ID {0x02}
#define ISIS_MTQ_V2_CANCEL_OP_ID {0x03}
#define ISIS_MTQ_V2_START_MTM_ID {0x04}
#define ISIS_MTQ_V2_START_ACTUATION_CURRENT_ID {0x05}
#define ISIS_MTQ_V2_START_ACTUATION_DIPOLE_ID {0x06}
#define ISIS_MTQ_V2_START_ACTUATION_PWM_ID {0x07}
#define ISIS_MTQ_V2_START_SELF_TEST_ID {0x08}
#define ISIS_MTQ_V2_START_BDOT_ID {0x09}
#define ISIS_MTQ_V2_GET_STATE_ID {0x41}
#define ISIS_MTQ_V2_GET_RAW_MTM_DATA_ID {0x42}
#define ISIS_MTQ_V2_GET_CAL_MTM_DATA_ID {0x43}
#define ISIS_MTQ_V2_GET_COIL_CURRENT_ID {0x44}
#define ISIS_MTQ_V2_GET_COIL_TEMPS_ID {0x45}
#define ISIS_MTQ_V2_GET_CMD_ACTUATION_DIPOLE_ID {0x46}
#define ISIS_MTQ_V2_GET_SELF_TEST_RESULT_ALL_ID {0x47}
#define ISIS_MTQ_V2_GET_SELF_TEST_RESULT_SINGLE_ID {0x47}
#define ISIS_MTQ_V2_GET_DETUMBLE_DATA_ID {0x48}
#define ISIS_MTQ_V2_GET_HOUSEKEEPING_ID {0x49}
#define ISIS_MTQ_V2_GET_HOUSEKEEPING_ENGINEERING_ID {0x4A}
#define ISIS_MTQ_V2_GET_PARAMETER_ID {0x81}
#define ISIS_MTQ_V2_SET_PARAMETER_ID {0x82}
#define ISIS_MTQ_V2_RESET_PARAMETER_ID {0x83}
#define ISIS_MTQ_V2_RESET_SW_ID {0xAA}


/* ENUMS */

/*!
 * Enumeration of iMTQ axis
 */
typedef enum __attribute__ ((__packed__)) isis_mtq_v2__axis_t
{
    isis_mtq_v2__axis__all = 0u,
    isis_mtq_v2__axis__x_positive = 1u,
    isis_mtq_v2__axis__x_negative = 2u,
    isis_mtq_v2__axis__y_positive = 3u,
    isis_mtq_v2__axis__y_negative = 4u,
    isis_mtq_v2__axis__z_positive = 5u,
    isis_mtq_v2__axis__z_negative = 6u,
} isis_mtq_v2__axis_t;

/*!
 * Enumeration of iMTQ response error codes
 */
typedef enum __attribute__ ((__packed__)) isis_mtq_v2__errorcode_t
{
    isis_mtq_v2__errorcode__accepted = 0u,
    isis_mtq_v2__errorcode__rejected = 1u,
    isis_mtq_v2__errorcode__invalid = 2u,
    isis_mtq_v2__errorcode__parmetermissing = 3u,
    isis_mtq_v2__errorcode__parameterinvalid = 4u,
    isis_mtq_v2__errorcode__cc_unavailable = 5u,
    isis_mtq_v2__errorcode__reserved = 6u,
    isis_mtq_v2__errorcode__internalerror = 7u,
} isis_mtq_v2__errorcode_t;

/*!
 * Enumeration of iMTQ modes
 */
typedef enum __attribute__ ((__packed__)) isis_mtq_v2__mode_t
{
    isis_mtq_v2__mode__idle = 0u,
    isis_mtq_v2__mode__selftest = 1u,
    isis_mtq_v2__mode__detumble = 2u,
} isis_mtq_v2__mode_t;

/*!
 * Enumeration of iMTQ self test error codes
 */
typedef enum __attribute__ ((__packed__)) isis_mtq_v2__selftesterror_t
{
    isis_mtq_v2__selftesterror__noerror = 0u,
    isis_mtq_v2__selftesterror__i2c_failure = 1u,
    isis_mtq_v2__selftesterror__spi_failure = 2u,
    isis_mtq_v2__selftesterror__adc_failure = 4u,
    isis_mtq_v2__selftesterror__pwm_failure = 8u,
    isis_mtq_v2__selftesterror__tc_failure = 16u,
    isis_mtq_v2__selftesterror__mtm_outofrange = 32u,
    isis_mtq_v2__selftesterror__coil_outofrange = 64u,
} isis_mtq_v2__selftesterror_t;

/*!
 * Enumeration of iMTQ self test steps
 */
typedef enum __attribute__ ((__packed__)) isis_mtq_v2__step_t
{
    isis_mtq_v2__step__init = 0u,
    isis_mtq_v2__step__x_positive = 1u,
    isis_mtq_v2__step__x_negative = 2u,
    isis_mtq_v2__step__y_positive = 3u,
    isis_mtq_v2__step__y_negative = 4u,
    isis_mtq_v2__step__z_positive = 5u,
    isis_mtq_v2__step__z_negative = 6u,
    isis_mtq_v2__step__fina = 7u,
} isis_mtq_v2__step_t;

/* STRUCTS */

/*!
 *  ISIS_MTQ_V2 instance structure
 */
typedef struct
{
    uint8_t i2cAddr; /*!< I2C address used for this instance */
} ISIS_MTQ_V2_t;

/*!
 * Union for storing the parameters for struct ReplyHeader.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__replyheader_t
{
    unsigned char raw[2];
    struct __attribute__ ((__packed__))
    {
        uint8_t command_code; /*!< Reply command code */
        isis_mtq_v2__errorcode_t cmd_error : 4; /*!< CMD ERROR NUMBER bits of STAT byte, determining if command was accepted for processing */
        uint8_t iva_x : 1; /*!< IVA bits of STAT byte, determines validity of X axis measurement */
        uint8_t iva_y : 1; /*!< IVA bits of STAT byte, determines validity of Y axis measurement */
        uint8_t iva_z : 1; /*!< IVA bits of STAT byte, determines validity of Z axis measurement */
        uint8_t new_flag : 1; /*!< NEW bit of STAT byte, determining if STAT value is received for the first time */
    } fields;
} isis_mtq_v2__replyheader_t;

/*!
 * Union for storing the parameters for struct SelfTestData.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__selftestdata_t
{
    unsigned char raw[40];
    struct __attribute__ ((__packed__))
    {
        isis_mtq_v2__replyheader_t reply_header; /*!< Generic iMTQ reply header */
        isis_mtq_v2__selftesterror_t error; /*!< ERR byte indicating the result of the self-test for that axis */
        isis_mtq_v2__step_t step; /*!< Represents the step of the self-test */
        int32_t raw_mag_x; /*!< X-direction raw MTM data */
        int32_t raw_mag_y; /*!< Y-direction raw MTM data */
        int32_t raw_mag_z; /*!< Z-direction raw MTM data */
        int32_t calibrated_mag_x; /*!< X-direction calibrated MTM data */
        int32_t calibrated_mag_y; /*!< Y-direction calibrated MTM data */
        int32_t calibrated_mag_z; /*!< Z-direction calibrated MTM data */
        int16_t coil_current_x; /*!< X-direction coil current */
        int16_t coil_current_y; /*!< Y-direction coil current */
        int16_t coil_current_z; /*!< Z-direction coil current */
        int16_t coil_temp_x; /*!< X-direction coil temperature */
        int16_t coil_temp_y; /*!< Y-direction coil temperature */
        int16_t coil_temp_z; /*!< Z-direction coil temperature */
    } fields;
} isis_mtq_v2__selftestdata_t;

/*!
 * Union for storing the parameters received by get_cal_mtm_data.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__get_cal_mtm_data__from_t
{
    unsigned char raw[15];
    struct __attribute__ ((__packed__))
    {
        isis_mtq_v2__replyheader_t reply_header; /*!< Generic iMTQ reply header */
        int32_t calibrated_mag_x; /*!< X-direction calibrated MTM data */
        int32_t calibrated_mag_y; /*!< Y-direction calibrated MTM data */
        int32_t calibrated_mag_z; /*!< Z-direction calibrated MTM data */
        int8_t coilact; /*!< Coils actuation status during measurement (1 for actuating, 0 for not actuating) */
    } fields;
} isis_mtq_v2__get_cal_mtm_data__from_t;

/*!
 * Union for storing the parameters received by get_cmd_actuation_dipole.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__get_cmd_actuation_dipole__from_t
{
    unsigned char raw[8];
    struct __attribute__ ((__packed__))
    {
        isis_mtq_v2__replyheader_t reply_header; /*!< Generic iMTQ reply header */
        int16_t cmd_act_dip_x; /*!< X-direction commanded actuation dipole */
        int16_t cmd_act_dip_y; /*!< Y-direction commanded actuation dipole */
        int16_t cmd_act_dip_z; /*!< Y-direction commanded actuation dipole */
    } fields;
} isis_mtq_v2__get_cmd_actuation_dipole__from_t;

/*!
 * Union for storing the parameters received by get_coil_current.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__get_coil_current__from_t
{
    unsigned char raw[8];
    struct __attribute__ ((__packed__))
    {
        isis_mtq_v2__replyheader_t reply_header; /*!< Generic iMTQ reply header */
        int16_t coil_current_x; /*!< X-direction coil current */
        int16_t coil_current_y; /*!< Y-direction coil current */
        int16_t coil_current_z; /*!< Z-direction coil current */
    } fields;
} isis_mtq_v2__get_coil_current__from_t;

/*!
 * Union for storing the parameters received by get_coil_temps.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__get_coil_temps__from_t
{
    unsigned char raw[8];
    struct __attribute__ ((__packed__))
    {
        isis_mtq_v2__replyheader_t reply_header; /*!< Generic iMTQ reply header */
        int16_t coil_temp_x; /*!< X-direction coil temperature */
        int16_t coil_temp_y; /*!< Y-direction coil temperature */
        int16_t coil_temp_z; /*!< Z-direction coil temperature */
    } fields;
} isis_mtq_v2__get_coil_temps__from_t;

/*!
 * Union for storing the parameters received by get_detumble_data.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__get_detumble_data__from_t
{
    unsigned char raw[56];
    struct __attribute__ ((__packed__))
    {
        isis_mtq_v2__replyheader_t reply_header; /*!< Generic iMTQ reply header */
        int32_t calibrated_mag_x; /*!< X-direction calibrated MTM data */
        int32_t calibrated_mag_y; /*!< Y-direction calibrated MTM data */
        int32_t calibrated_mag_z; /*!< Z-direction calibrated MTM data */
        int32_t filtered_mag_x; /*!< X-direction filtered MTM data */
        int32_t filtered_mag_y; /*!< Y-direction filtered MTM data */
        int32_t filtered_mag_z; /*!< Z-direction filtered MTM data */
        int32_t bdot_x; /*!< X-direction B-dot data */
        int32_t bdot_y; /*!< Y-direction B-dot data */
        int32_t bdot_z; /*!< Z-direction B-dot data */
        int16_t cmd_act_dip_x; /*!< X-direction commanded actuation dipole */
        int16_t cmd_act_dip_y; /*!< Y-direction commanded actuation dipole */
        int16_t cmd_act_dip_z; /*!< Y-direction commanded actuation dipole */
        int16_t cmd_current_x; /*!< X-direction command current */
        int16_t cmd_current_y; /*!< Y-direction command current */
        int16_t cmd_current_z; /*!< Z-direction command current */
        int16_t meas_current_x; /*!< X-direction coil current measurement */
        int16_t meas_current_y; /*!< Y-direction coil current measurement */
        int16_t meas_current_z; /*!< Z-direction coil current measurement */
    } fields;
} isis_mtq_v2__get_detumble_data__from_t;

/*!
 * Union for storing the parameters received by get_housekeeping.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__get_housekeeping__from_t
{
    unsigned char raw[24];
    struct __attribute__ ((__packed__))
    {
        isis_mtq_v2__replyheader_t reply_header; /*!< Generic iMTQ reply header */
        int16_t digital_voltage; /*!< Voltage measurement of the digital supply in raw ADC counts \note conversion: eng. value = 0.001221001 * raw */
        int16_t analog_voltage; /*!< Voltage measurement of the amalog supply in raw ADC counts \note conversion: eng. value = 0.001221001 * raw */
        int16_t digital_current; /*!< Current measurement of the digital supply in raw ADC counts \note conversion: eng. value = 0.0000611 * raw */
        int16_t analog_current; /*!< Current measurement of the analog supply in raw ADC counts \note conversion: eng. value = 0.0000611 * raw */
        int16_t meas_current_x; /*!< X-direction coil current measurement in raw ADC counts \note conversion: eng. value = 0.00030525 * raw + -0.515*/
        int16_t meas_current_y; /*!< Y-direction coil current measurement in raw ADC counts \note conversion: eng. value = 0.00030525 * raw + -0.515*/
        int16_t meas_current_z; /*!< Z-direction coil current measurement in raw ADC counts \note conversion: eng. value = 0.001271876 * raw + -2.145833333*/
        int16_t coil_temp_x; /*!< X-direction coil temperature in raw ADC counts \note conversion: eng. value = -0.075370446 * raw + 193.4567901*/
        int16_t coil_temp_y; /*!< Y-direction coil temperature in raw ADC counts \note conversion: eng. value = -0.075370446 * raw + 193.4567901*/
        int16_t coil_temp_z; /*!< Z-direction coil temperature in raw ADC counts \note conversion: eng. value = -0.075370446 * raw + 193.4567901*/
        int16_t mcu_temp; /*!< Temperature measurement of MCU in raw ADC counts \note conversion: eng. value = -0.271333605 * raw + 302.2222222*/
    } fields;
} isis_mtq_v2__get_housekeeping__from_t;

/*!
 * Union for storing the parameters received by get_housekeeping_engineering.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__get_housekeeping_engineering__from_t
{
    unsigned char raw[24];
    struct __attribute__ ((__packed__))
    {
        isis_mtq_v2__replyheader_t reply_header; /*!< Generic iMTQ reply header */
        int16_t digital_voltage; /*!< Voltage measurement of the digital supplys */
        int16_t analog_voltage; /*!< Voltage measurement of the amalog supply */
        int16_t digital_current; /*!< Current measurement of the digital supply */
        int16_t analog_current; /*!< Current measurement of the analog supply */
        int16_t meas_current_x; /*!< X-direction coil current measurement */
        int16_t meas_current_y; /*!< Y-direction coil current measurement */
        int16_t meas_current_z; /*!< Z-direction coil current measurement */
        int16_t coil_temp_x; /*!< X-direction coil temperature */
        int16_t coil_temp_y; /*!< Y-direction coil temperature */
        int16_t coil_temp_z; /*!< Z-direction coil temperature */
        int16_t mcu_temp; /*!< Temperature measurement of MCU */
    } fields;
} isis_mtq_v2__get_housekeeping_engineering__from_t;

/*!
 * Union for storing the parameters received by get_parameter.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__get_parameter__from_t
{
    unsigned char raw[12];
    struct __attribute__ ((__packed__))
    {
        isis_mtq_v2__replyheader_t reply_header; /*!< Generic iMTQ reply header */
        uint16_t param_id; /*!< Parameter ID */
        uint8_t param_value[8]; /*!< Parameter value (type and size depends on parameter) */
    } fields;
} isis_mtq_v2__get_parameter__from_t;

/*!
 * Union for storing the parameters received by get_raw_mtm_data.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__get_raw_mtm_data__from_t
{
    unsigned char raw[15];
    struct __attribute__ ((__packed__))
    {
        isis_mtq_v2__replyheader_t reply_header; /*!< Generic iMTQ reply header */
        int32_t raw_mag_x; /*!< X-direction raw MTM data */
        int32_t raw_mag_y; /*!< Y-direction raw MTM data */
        int32_t raw_mag_z; /*!< Z-direction raw MTM data */
        int8_t coilact; /*!< Coils actuation status during measurement (1 for actuating, 0 for not actuating) */
    } fields;
} isis_mtq_v2__get_raw_mtm_data__from_t;

/*!
 * Union for storing the parameters received by get_self_test_result_all.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__get_self_test_result_all__from_t
{
    unsigned char raw[320];
    struct __attribute__ ((__packed__))
    {
        isis_mtq_v2__selftestdata_t step_init; /*!< Step INIT result information; this step measures the initial magnetic field and coil currents while not torquing */
        isis_mtq_v2__selftestdata_t step_posx; /*!< Step +X result information; this step measures the magnetic field and coil currents while torquing +X */
        isis_mtq_v2__selftestdata_t step_negx; /*!< Step -X result information; this step measures the magnetic field and coil currents while torquing -X */
        isis_mtq_v2__selftestdata_t step_posy; /*!< Step +Y result information; this step measures the magnetic field and coil currents while torquing +Y */
        isis_mtq_v2__selftestdata_t step_negy; /*!< Step -Y result information; this step measures the magnetic field and coil currents while torquing -Y */
        isis_mtq_v2__selftestdata_t step_posz; /*!< Step +Z result information; this step measures the magnetic field and coil currents while torquing +Z */
        isis_mtq_v2__selftestdata_t step_negz; /*!< Step -Z result information; this step measures the magnetic field and coil currents while torquing -Z */
        isis_mtq_v2__selftestdata_t step_fina; /*!< Step FINA result information; this step measures the final magnetic field and coil currents while not torquing */
    } fields;
} isis_mtq_v2__get_self_test_result_all__from_t;

/*!
 * Union for storing the parameters received by get_self_test_result_single.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__get_self_test_result_single__from_t
{
    unsigned char raw[120];
    struct __attribute__ ((__packed__))
    {
        isis_mtq_v2__selftestdata_t step_init; /*!< Step INIT result information; this step measures the initial magnetic field and coil currents while not torquing */
        isis_mtq_v2__selftestdata_t step_axac; /*!< Axis actuation result information; this step measures the magnetic field and coil currents while torquing the requested axis-direction */
        isis_mtq_v2__selftestdata_t step_fina; /*!< Step FINA result information; this step measures the final magnetic field and coil currents while not torquing */
    } fields;
} isis_mtq_v2__get_self_test_result_single__from_t;

/*!
 * Union for storing the parameters received by get_state.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__get_state__from_t
{
    unsigned char raw[6];
    struct __attribute__ ((__packed__))
    {
        isis_mtq_v2__replyheader_t reply_header; /*!< Generic iMTQ reply header */
        isis_mtq_v2__mode_t mode; /*!< Gives current mode (0 for idle, 1 for selftest, 2 for detumble) */
        uint8_t err; /*!< Reports error encountered in previous iteration (0 for no error) */
        uint8_t conf; /*!< Returns the system configuration parameter status (0 if no parameter updated, 1 if any parameter updated) */
        uint8_t uptime; /*!< System uptime since power on */
    } fields;
} isis_mtq_v2__get_state__from_t;

/*!
 * Union for storing the parameters received by reset_parameter.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__reset_parameter__from_t
{
    unsigned char raw[12];
    struct __attribute__ ((__packed__))
    {
        isis_mtq_v2__replyheader_t reply_header; /*!< Generic iMTQ reply header */
        int16_t param_id; /*!< Parameter ID */
        uint8_t param_value[8]; /*!< Parameter reset value  (type and size depends on parameter) */
    } fields;
} isis_mtq_v2__reset_parameter__from_t;

/*!
 * Union for storing the parameters sent by set_parameter.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__set_parameter__to_t
{
    unsigned char raw[10];
    struct __attribute__ ((__packed__))
    {
        uint16_t param_id; /*!< Parameter ID */
        uint8_t param_value[8]; /*!< Parameter value (type and size depends on parameter) */
    } fields;
} isis_mtq_v2__set_parameter__to_t;

/*!
 * Union for storing the parameters received by set_parameter.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__set_parameter__from_t
{
    unsigned char raw[12];
    struct __attribute__ ((__packed__))
    {
        isis_mtq_v2__replyheader_t reply_header; /*!< Generic iMTQ reply header */
        uint16_t param_id; /*!< Parameter ID */
        uint8_t param_value_new[8]; /*!< New parameter value  (type and size depends on parameter) */
    } fields;
} isis_mtq_v2__set_parameter__from_t;

/*!
 * Union for storing the parameters sent by start_actuation_current.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__start_actuation_current__to_t
{
    unsigned char raw[8];
    struct __attribute__ ((__packed__))
    {
        int16_t input_x; /*!< X-direction setting */
        int16_t input_y; /*!< Y-direction setting */
        int16_t input_z; /*!< Z-direction setting */
        uint16_t duration; /*!< Actuation duration. 0 = infinite */
    } fields;
} isis_mtq_v2__start_actuation_current__to_t;

/*!
 * Union for storing the parameters sent by start_actuation_dipole.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__start_actuation_dipole__to_t
{
    unsigned char raw[8];
    struct __attribute__ ((__packed__))
    {
        int16_t input_x; /*!< X-direction setting */
        int16_t input_y; /*!< Y-direction setting */
        int16_t input_z; /*!< Z-direction setting */
        uint16_t duration; /*!< Actuation duration. 0 = infinite */
    } fields;
} isis_mtq_v2__start_actuation_dipole__to_t;

/*!
 * Union for storing the parameters sent by start_actuation_pwm.
 */
typedef union __attribute__((__packed__)) _isis_mtq_v2__start_actuation_pwm__to_t
{
    unsigned char raw[8];
    struct __attribute__ ((__packed__))
    {
        int16_t input_x; /*!< X-direction setting */
        int16_t input_y; /*!< Y-direction setting */
        int16_t input_z; /*!< Z-direction setting */
        uint16_t duration; /*!< Actuation duration. 0 = infinite */
    } fields;
} isis_mtq_v2__start_actuation_pwm__to_t;

#ifdef __cplusplus
}
#endif

#endif /* ISIS_MTQ_V2_TYPES_H_ */
