/***************************************************************************//**
 *   @file    ltc2672_iio.c
 *   @brief   Implementation of LTC2672 IIO application interfaces
********************************************************************************
 * Copyright (c) 2023-25 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "app_config.h"
#include "ltc2672_iio.h"
#include "ltc2672_user_config.h"
#include "common.h"
#include "no_os_util.h"
#include "no_os_delay.h"
#include "no_os_alloc.h"

/******** Forward declaration of getter/setter functions ********/
static int ltc2672_iio_attr_get(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv);

static int ltc2672_iio_attr_set(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv);

static int ltc2672_iio_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int ltc2672_iio_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

/******************************************************************************/
/************************ Macros and Constants ********************************/
/******************************************************************************/

#define LTC2672_CHN_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = ltc2672_iio_attr_get,\
		.store = ltc2672_iio_attr_set\
}

#define LTC2672_CHN_AVAIL_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = ltc2672_iio_attr_available_get,\
		.store = ltc2672_iio_attr_available_set\
}

#define LTC2672_CH(_name, _idx, _type) {\
	.name = _name, \
	.ch_type = _type,\
	.ch_out = true,\
	.indexed = true,\
	.channel = _idx,\
	.scan_index = _idx,\
	.scan_type = &ltc2672_iio_scan_type,\
	.attributes = ltc2672_iio_ch_attributes\
}

/*	Number of IIO devices */
#define NUM_OF_IIO_DEVICES	1

#define	BYTES_PER_SAMPLE	sizeof(uint16_t)

#define BYTE_SIZE	(uint32_t)8
#define	BYTE_MASK	(uint32_t)0xff

/* Number of data storage bits (needed for IIO client to send buffer of data) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

/* Reference voltage limits */
#define LTC2672_MIN_REF_VOLTAGE      1.225
#define LTC2672_MAX_REF_VOLTAGE	     1.275

/* Full scale adjust resistor limits */
#define LTC2672_MIN_FSADJ_RESISTOR   19
#define LTC2672_MAX_FSADJ_RESISTOR   41

/* Shifts the boolean variables to their positions in the config command */
#define CONFIG_COMMAND(oc, pl, ts, rd)	(oc << 3) | (pl << 2) | (ts << 1) | (rd << 0)

/* Converts index number to device ID */
#define INDEX_TO_DEV_ID(x) ((x * 2) + 1)

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/
/* LTC2672 devices descriptor */
struct ltc2672_dev *ltc2672_dev_desc;

/* LTC2672 IIO descriptor */
static struct iio_desc *ltc2672_iio_desc;

/* Attribute IDs */
enum ltc2672_iio_attr_id {
	// Channels attributes
	DAC_CH_RAW,
	DAC_CH_OFFSET,
	DAC_CH_SCALE,
	DAC_CH_SPAN,
	DAC_CH_CURRENT,
	DAC_CH_INPUT_A,
	DAC_CH_INPUT_B,
	DAC_CH_POWERDOWN,
	DAC_CH_SW_LDAC,
	DAC_CH_WRITE_TO_N_UPDATE_ALL,
	DAC_CH_TOGGLE_SEL,
	DAC_CH_OPEN_CIRCUIT_FAULT,

	// Device attributes
	DAC_RAW,
	DAC_SPAN,
	DAC_CURRENT,
	DAC_MUX,
	DAC_READBACK,
	DAC_RESET,
	DAC_HW_TOGGLE_STATE,
	DAC_TOGGLE_PWM,
	DAC_SAMPLE_RATE,
	DAC_CHIP_POWERDOWN,
	DAC_INPUT_A,
	DAC_INPUT_B,
	DAC_HW_LDAC,
	DAC_SW_LDAC,
	DAC_FAULT,
	DAC_OPEN_CIRCUIT_CONFIG,
	DAC_POWER_LIMIT_CONFIG,
	DAC_THERMAL_SHUTDOWN_CONFIG,
	DAC_EXTERNAL_REFERENCE_CONFIG,
	DAC_SW_TOGGLE_STATE,
	DAC_OVER_TEMP_FAULT,
	DAC_POWER_LIMIT_FAULT,
	DAC_SPI_LENGTH_FAULT,
	DAC_REFERENCE,
	DAC_RESISTOR,
	DAC_NO_OP
};

/* IIO channels scan structure */
static struct scan_type ltc2672_iio_scan_type = {
	.sign = 'u',
	.realbits = DAC_RESOLUTION,
	.storagebits = CHN_STORAGE_BITS,
	.shift = 0,
	.is_big_endian = false
};

/* Current spans */
static const char *ltc2672_current_spans[] = {
	"off_mode",
	"3.125mA",
	"6.25mA",
	"12.5mA",
	"25mA",
	"50mA",
	"100mA",
	"200mA",
	"MVREF",
	"300mA"
};

/* Mux output select options for LTC2672 */
static const char *ltc2672_mux_select[] = {
	"disable",
	"iout0",
	"iout1",
	"iout2",
	"iout3",
	"iout4",
	"vcc",
	"vref",
	"vref_lo",
	"die_temperature",
	"vdd0",
	"vdd1",
	"vdd2",
	"vdd3",
	"vdd4",
	"v_minus",
	"gnd",
	"vout0",
	"vout1",
	"vout2",
	"vout3",
	"vout4"
};

/* Mux output select options for LTC2662 */
static const char *ltc2662_mux_select[] = {
	"disable",
	"iout0",
	"iout1",
	"iout2",
	"iout3",
	"iout4",
	"vcc",
	"vref",
	"vref_lo",
	"die_temperature",
	"vdd0",
	"vdd1",
	"vdd2",
	"vdd3",
	"vdd4",
	"v_plus",
	"v_minus",
	"gnd",
	"vout0",
	"vout1",
	"vout2",
	"vout3",
	"vout4"
};

/* Fault pin state options */
static const char *ltc2672_fault_pins_states[] = {
	"fault_detected",
	"no_fault"
};

/* Fault options */
static const char *ltc2672_fault_options[] = {
	"no_fault",
	"fault_detected"
};

/* Global Toggle bit options */
static const char *ltc2672_gobal_toggle_options[] = {
	"low",
	"high"
};

/* Toggle select options */
static const char *ltc2672_toggle_sel_options[] = {
	"disable",
	"enable"
};

/* Toggle pin state options */
static const char *ltc2672_toggle_pins_states[] = {
	"low",
	"high"
};

/* Toggle pin pwm options */
static const char *ltc2672_toggle_pwm_options[] = {
	"disable",
	"enable"
};

/* Powerdown options */
static const char *ltc2672_powerdown_options[] = {
	"powerdown"
};

/* Update options */
static const char *ltc2672_update_options[] = {
	"update"
};

/* Reset options */
static const char *ltc2672_reset_options[] = {
	"reset"
};

/* No operation command options */
static const char *ltc2672_no_op_options[] = {
	"send"
};

/* Fault detection config options */
static const char *fault_detection_options[] = {
	"enable",
	"disable"
};

/* External reference config options */
static const char *external_reference_options[] = {
	"disable",
	"enable"
};

/* Mux command array for LTC2672 */
static enum ltc2672_mux_commands ltc2672_mux_map[] =  {
	LTC2672_MUX_DISABLED,
	LTC2672_MUX_IOUT0,
	LTC2672_MUX_IOUT1,
	LTC2672_MUX_IOUT2,
	LTC2672_MUX_IOUT3,
	LTC2672_MUX_IOUT4,
	LTC2672_MUC_VCC,
	LTC2672_MUX_VREF,
	LTC2672_MUX_VREF_LO,
	LTC2672_MUX_DIE_TEMP,
	LTC2672_MUX_VDD0,
	LTC2672_MUX_VDD1,
	LTC2672_MUX_VDD2,
	LTC2672_MUX_VDD3,
	LTC2672_MUX_VDD4,
	LTC2672_MUX_VMINUS,
	LTC2672_MUX_GND,
	LTC2672_MUX_VOUT0,
	LTC2672_MUX_VOUT1,
	LTC2672_MUX_VOUT2,
	LTC2672_MUX_VOUT3,
	LTC2672_MUX_VOUT4
};

/* Mux command array for LTC2662 */
static enum ltc2672_mux_commands ltc2662_mux_map[] = {
	LTC2672_MUX_DISABLED,
	LTC2672_MUX_IOUT0,
	LTC2672_MUX_IOUT1,
	LTC2672_MUX_IOUT2,
	LTC2672_MUX_IOUT3,
	LTC2672_MUX_IOUT4,
	LTC2672_MUC_VCC,
	LTC2672_MUX_VREF,
	LTC2672_MUX_VREF_LO,
	LTC2672_MUX_DIE_TEMP,
	LTC2672_MUX_VDD0,
	LTC2672_MUX_VDD1,
	LTC2672_MUX_VDD2,
	LTC2672_MUX_VDD3,
	LTC2672_MUX_VDD4,
	LTC2672_MUX_VPLUS,
	LTC2672_MUX_VMINUS,
	LTC2672_MUX_GND,
	LTC2672_MUX_VOUT0,
	LTC2672_MUX_VOUT1,
	LTC2672_MUX_VOUT2,
	LTC2672_MUX_VOUT3,
	LTC2672_MUX_VOUT4
};

/* IIO channels attributes list */
static struct iio_attribute ltc2672_iio_ch_attributes[] = {
	LTC2672_CHN_ATTR("raw", DAC_CH_RAW),
	LTC2672_CHN_ATTR("scale", DAC_CH_SCALE),
	LTC2672_CHN_ATTR("offset", DAC_CH_OFFSET),
	LTC2672_CHN_ATTR("input_register_and_update", DAC_CH_CURRENT),
	LTC2672_CHN_ATTR("input_register_a", DAC_CH_INPUT_A),
	LTC2672_CHN_ATTR("input_register_b", DAC_CH_INPUT_B),
	LTC2672_CHN_ATTR("span", DAC_CH_SPAN),
	LTC2672_CHN_AVAIL_ATTR("span_available", DAC_CH_SPAN),
	LTC2672_CHN_ATTR("powerdown", DAC_CH_POWERDOWN),
	LTC2672_CHN_AVAIL_ATTR("powerdown_available", DAC_CH_POWERDOWN),
	LTC2672_CHN_ATTR("sw_update", DAC_CH_SW_LDAC),
	LTC2672_CHN_AVAIL_ATTR("sw_update_available", DAC_CH_SW_LDAC),
	LTC2672_CHN_ATTR("input_register_and_update_all_chns", DAC_CH_WRITE_TO_N_UPDATE_ALL),
	LTC2672_CHN_ATTR("toggle_select", DAC_CH_TOGGLE_SEL),
	LTC2672_CHN_AVAIL_ATTR("toggle_select_available", DAC_CH_TOGGLE_SEL),
	LTC2672_CHN_ATTR("open_circuit_fault", DAC_CH_OPEN_CIRCUIT_FAULT),
	LTC2672_CHN_AVAIL_ATTR("open_circuit_fault_available", DAC_CH_OPEN_CIRCUIT_FAULT),
	END_ATTRIBUTES_ARRAY
};

/* IIO global attributes list */
static struct iio_attribute ltc2672_iio_global_attributes[] = {
	LTC2672_CHN_ATTR("all_chns_raw", DAC_RAW),
	LTC2672_CHN_ATTR("sampling_frequency", DAC_SAMPLE_RATE),
	LTC2672_CHN_ATTR("all_chns_input_register_and_update", DAC_CURRENT),
	LTC2672_CHN_ATTR("all_chns_span", DAC_SPAN),
	LTC2672_CHN_AVAIL_ATTR("all_chns_span_available", DAC_SPAN),
	LTC2672_CHN_ATTR("mux", DAC_MUX),
	LTC2672_CHN_AVAIL_ATTR("mux_available", DAC_MUX),
	LTC2672_CHN_ATTR("readback", DAC_READBACK),
	LTC2672_CHN_ATTR("reset", DAC_RESET),
	LTC2672_CHN_AVAIL_ATTR("reset_available", DAC_RESET),
	LTC2672_CHN_ATTR("toggle_pin_state", DAC_HW_TOGGLE_STATE),
	LTC2672_CHN_AVAIL_ATTR("toggle_pin_state_available", DAC_HW_TOGGLE_STATE),
	LTC2672_CHN_ATTR("toggle_pwm", DAC_TOGGLE_PWM),
	LTC2672_CHN_AVAIL_ATTR("toggle_pwm_available", DAC_TOGGLE_PWM),
	LTC2672_CHN_ATTR("powerdown_chip", DAC_CHIP_POWERDOWN),
	LTC2672_CHN_AVAIL_ATTR("powerdown_chip_available", DAC_CHIP_POWERDOWN),
	LTC2672_CHN_ATTR("all_chns_input_register_a", DAC_INPUT_A),
	LTC2672_CHN_ATTR("all_chns_input_register_b", DAC_INPUT_B),
	LTC2672_CHN_ATTR("hw_ldac_update", DAC_HW_LDAC),
	LTC2672_CHN_AVAIL_ATTR("hw_ldac_update_available", DAC_HW_LDAC),
	LTC2672_CHN_ATTR("all_chns_sw_update", DAC_SW_LDAC),
	LTC2672_CHN_AVAIL_ATTR("all_chns_sw_update_available", DAC_SW_LDAC),
	LTC2672_CHN_ATTR("fault_alert", DAC_FAULT),
	LTC2672_CHN_AVAIL_ATTR("fault_alert_available", DAC_FAULT),
	LTC2672_CHN_ATTR("open_circuit_detection", DAC_OPEN_CIRCUIT_CONFIG),
	LTC2672_CHN_AVAIL_ATTR("open_circuit_detection_available", DAC_OPEN_CIRCUIT_CONFIG),
	LTC2672_CHN_ATTR("thermal_shutdown_protection", DAC_THERMAL_SHUTDOWN_CONFIG),
	LTC2672_CHN_AVAIL_ATTR("thermal_shutdown_protection_available", DAC_THERMAL_SHUTDOWN_CONFIG),
	LTC2672_CHN_ATTR("external_reference", DAC_EXTERNAL_REFERENCE_CONFIG),
	LTC2672_CHN_AVAIL_ATTR("external_reference_available", DAC_EXTERNAL_REFERENCE_CONFIG),
	LTC2672_CHN_ATTR("sw_toggle_state", DAC_SW_TOGGLE_STATE),
	LTC2672_CHN_AVAIL_ATTR("sw_toggle_state_available", DAC_SW_TOGGLE_STATE),
	LTC2672_CHN_ATTR("over_temperature_fault", DAC_OVER_TEMP_FAULT),
	LTC2672_CHN_AVAIL_ATTR("over_temperature_fault_available", DAC_OVER_TEMP_FAULT),
	LTC2672_CHN_ATTR("invalid_spi_seq_length", DAC_SPI_LENGTH_FAULT),
	LTC2672_CHN_AVAIL_ATTR("invalid_spi_seq_length_available", DAC_SPI_LENGTH_FAULT),
	LTC2672_CHN_ATTR("reference_in_volts", DAC_REFERENCE),
	LTC2672_CHN_ATTR("fsadj_res_in_kohm", DAC_RESISTOR),
	LTC2672_CHN_ATTR("no_op_cmd", DAC_NO_OP),
	LTC2672_CHN_ATTR("no_op_cmd_available", DAC_NO_OP),
	END_ATTRIBUTES_ARRAY
};

static struct iio_attribute ltc2662_iio_global_attributes[] = {
	LTC2672_CHN_ATTR("all_chns_raw", DAC_RAW),
	LTC2672_CHN_ATTR("sampling_frequency", DAC_SAMPLE_RATE),
	LTC2672_CHN_ATTR("all_chns_input_register_and_update", DAC_CURRENT),
	LTC2672_CHN_ATTR("all_chns_span", DAC_SPAN),
	LTC2672_CHN_AVAIL_ATTR("all_chns_span_available", DAC_SPAN),
	LTC2672_CHN_ATTR("mux", DAC_MUX),
	LTC2672_CHN_AVAIL_ATTR("mux_available", DAC_MUX),
	LTC2672_CHN_ATTR("readback", DAC_READBACK),
	LTC2672_CHN_ATTR("reset", DAC_RESET),
	LTC2672_CHN_AVAIL_ATTR("reset_available", DAC_RESET),
	LTC2672_CHN_ATTR("toggle_pin_state", DAC_HW_TOGGLE_STATE),
	LTC2672_CHN_AVAIL_ATTR("toggle_pin_state_available", DAC_HW_TOGGLE_STATE),
	LTC2672_CHN_ATTR("toggle_pwm", DAC_TOGGLE_PWM),
	LTC2672_CHN_AVAIL_ATTR("toggle_pwm_available", DAC_TOGGLE_PWM),
	LTC2672_CHN_ATTR("powerdown_chip", DAC_CHIP_POWERDOWN),
	LTC2672_CHN_AVAIL_ATTR("powerdown_chip_available", DAC_CHIP_POWERDOWN),
	LTC2672_CHN_ATTR("all_chns_input_register_a", DAC_INPUT_A),
	LTC2672_CHN_ATTR("all_chns_input_register_b", DAC_INPUT_B),
	LTC2672_CHN_ATTR("hw_ldac_update", DAC_HW_LDAC),
	LTC2672_CHN_AVAIL_ATTR("hw_ldac_update_available", DAC_HW_LDAC),
	LTC2672_CHN_ATTR("all_chns_sw_update", DAC_SW_LDAC),
	LTC2672_CHN_AVAIL_ATTR("all_chns_sw_update_available", DAC_SW_LDAC),
	LTC2672_CHN_ATTR("fault_alert", DAC_FAULT),
	LTC2672_CHN_AVAIL_ATTR("fault_alert_available", DAC_FAULT),
	LTC2672_CHN_ATTR("open_circuit_detection", DAC_OPEN_CIRCUIT_CONFIG),
	LTC2672_CHN_AVAIL_ATTR("open_circuit_detection_available", DAC_OPEN_CIRCUIT_CONFIG),
	LTC2672_CHN_ATTR("power_limit_protection", DAC_POWER_LIMIT_CONFIG),
	LTC2672_CHN_AVAIL_ATTR("power_limit_protection_available", DAC_POWER_LIMIT_CONFIG),
	LTC2672_CHN_ATTR("thermal_shutdown_protection", DAC_THERMAL_SHUTDOWN_CONFIG),
	LTC2672_CHN_AVAIL_ATTR("thermal_shutdown_protection_available", DAC_THERMAL_SHUTDOWN_CONFIG),
	LTC2672_CHN_ATTR("external_reference", DAC_EXTERNAL_REFERENCE_CONFIG),
	LTC2672_CHN_AVAIL_ATTR("external_reference_available", DAC_EXTERNAL_REFERENCE_CONFIG),
	LTC2672_CHN_ATTR("sw_toggle_state", DAC_SW_TOGGLE_STATE),
	LTC2672_CHN_AVAIL_ATTR("sw_toggle_state_available", DAC_SW_TOGGLE_STATE),
	LTC2672_CHN_ATTR("over_temperature_fault", DAC_OVER_TEMP_FAULT),
	LTC2672_CHN_AVAIL_ATTR("over_temperature_fault_available", DAC_OVER_TEMP_FAULT),
	LTC2672_CHN_ATTR("power_limit_fault", DAC_POWER_LIMIT_FAULT),
	LTC2672_CHN_AVAIL_ATTR("power_limit_fault_available", DAC_POWER_LIMIT_FAULT),
	LTC2672_CHN_ATTR("invalid_spi_seq_length", DAC_SPI_LENGTH_FAULT),
	LTC2672_CHN_AVAIL_ATTR("invalid_spi_seq_length_available", DAC_SPI_LENGTH_FAULT),
	LTC2672_CHN_ATTR("reference_in_volts", DAC_REFERENCE),
	LTC2672_CHN_ATTR("fsadj_res_in_kohm", DAC_RESISTOR),
	LTC2672_CHN_ATTR("no_op_cmd", DAC_NO_OP),
	LTC2672_CHN_ATTR("no_op_cmd_available", DAC_NO_OP),
	END_ATTRIBUTES_ARRAY
};

/* IIO channels info */
static struct iio_channel ltc2672_iio_channels[] = {
	LTC2672_CH("Chn0", 0, IIO_CURRENT),
	LTC2672_CH("Chn1", 1, IIO_CURRENT),
	LTC2672_CH("Chn2", 2, IIO_CURRENT),
	LTC2672_CH("Chn3", 3, IIO_CURRENT),
	LTC2672_CH("Chn4", 4, IIO_CURRENT)
};

#if defined(DC2903A)
/* IIO context attributes */
static struct iio_ctx_attr ctx_attrs[] = {
	{
		.name = "hw_carrier",
		.value = STR(HW_CARRIER_NAME)
	},
	{
		.name = "hw_mezzanine",
		.value = HW_MEZZANINE_NAME
	},
	{
		.name = "hw_name",
		.value = ACTIVE_DEVICE_NAME
	},
};
#endif

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

/* All channels dac code */
static uint32_t all_chs_dac_code;

/* All channels dac code register A */
static uint32_t all_chs_dac_code_reg_a;

/* All channels dac code register B */
static uint32_t all_chs_dac_code_reg_b;

/* All channels span */
static enum ltc2672_out_range all_chs_span;

/* All channels scale */
static float all_chs_scale;

/* Channel wise dac code array */
static uint32_t ch_dac_codes[LTC2672_TOTAL_CHANNELS];

/* Channel wise dac code array of register A */
static uint32_t ch_dac_codes_reg_a[LTC2672_TOTAL_CHANNELS];

/* Channel wise dac code array register B */
static uint32_t ch_dac_codes_reg_b[LTC2672_TOTAL_CHANNELS];

/* Pointer to mux character array of active device */
static const char **mux_select = ltc2672_mux_select;

/* Pointer to mux map of active device */
static enum ltc2672_mux_commands *mux_map = ltc2672_mux_map;

/* Variable to store mux total count */
static uint8_t mux_count = NO_OS_ARRAY_SIZE(ltc2672_mux_select);

/* Variable to store mux output select */
static uint8_t mux_val;

/* Scale attribute value */
static float attr_scale_val[LTC2672_TOTAL_CHANNELS];

/* Reference Voltage of the device */
float ref_voltage = DAC_VREF;

/* Full Scale Adjust Resistor Value */
float resistor_fsadj = DAC_FSADJ_RESISTOR;

/* Offset attribute value */
static uint16_t attr_offset_val;

/* Variable to store which channels are selected for toggling */
uint8_t toggle_sel_bits = 0;

/* Variable to store fault register contents */
uint8_t fault_register = 0;

/* Flag to check if PWM has been enabled */
static bool ltc2672_tgp_pwm_enabled = false;

/* Variable to hold the state of TGP */
static bool tgp_state = true;

/* DAC toggle rate */
uint32_t ltc2672_toggle_rate = LTC2672_MAX_TOGGLE_RATE;

/* Boolean to store the Open-Circuit Detection bit value */
static bool config_oc = false;

/* Boolean to store the Power Limit Protection Disable bit value */
static bool config_pl = false;

/* Boolean to store the Thermal Shutdown Disable bit value */
static bool config_ts = false;

/* Boolean to store the Reference Disable bit value */
static bool config_rd = false;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/
/**
 * @brief Append options to the buffer and get the length of the buffer
 * @param buf[in, out] - Buffer to append options to
 * @param options[in] - Array of options
 * @param options_size[in] - Size of array
 * @return len in case of success, negative error code otherwise
 */
static uint32_t append_options_to_buffer(char *buf, const char **options,
		uint8_t options_size)
{
	uint32_t len;

	for (size_t i = 0; i < options_size; i++) {
		strcat(buf, options[i]);
		strcat(buf, " ");
	}

	/* Remove extra trailing space at the end of the buffer string */
	len = strlen(buf);
	buf[len - 1] = '\0';

	return strlen(buf);
}

/**
 * @brief Set the toggling rate and get the updated value supported by MCU platform and the device
 * @param toggling_rate[in,out] - Update rate value
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ltc2672_set_toggling_rate(uint32_t* toggling_rate)
{
	int32_t ret;
	uint32_t pwm_period_ns;

	if (!toggling_rate) {
		return -EINVAL;
	}

	if (*toggling_rate > LTC2672_MAX_TOGGLE_RATE) {
		*toggling_rate = LTC2672_MAX_TOGGLE_RATE;
	}

	/* Configure the TGP PWM period and duty ratio */
	ret = no_os_pwm_set_period(toggle_pwm_desc,
				   FREQ_TO_NSEC(*toggling_rate));
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_set_duty_cycle(toggle_pwm_desc,
				       DUTY_CYCLE_NSEC(FREQ_TO_NSEC(*toggling_rate)));
	if (ret) {
		return ret;
	}

	/* Get the updated value set by hardware */
	ret = no_os_pwm_get_period(toggle_pwm_desc, &pwm_period_ns);
	if (ret) {
		return ret;
	}

	/* Convert period (nsec) to frequency (hertz) */
	*toggling_rate = (1.0 / pwm_period_ns) * 1000000000;

	return 0;
}

/**
 * @brief	Get the IIO scale for input channel
 * @param 	chn[in] - Input channel
 * @param	scale[in, out] - Channel IIO scale value
 * @return	0 in case of success, negative error code otherwise
 */
static int ltc2672_get_scale(uint8_t chn, float *scale)
{
	if (!scale) {
		return -EINVAL;
	}

	if (ltc2672_dev_desc->id == LTC2672_12 || ltc2672_dev_desc->id == LTC2662_12) {
		*scale = ((float)ltc2672_dev_desc->max_currents[chn] / LTC2672_12BIT_RESO) /
			 1000;
	} else {
		*scale = ((float)ltc2672_dev_desc->max_currents[chn] / LTC2672_16BIT_RESO) /
			 1000;
	}

	return 0;
}

/*!
* @brief	Getter function for LTC2672 attributes.
* @param	device[in, out]- Pointer to IIO device instance.
* @param	buf[in]- IIO input data buffer.
* @param	len[in]- Number of expected bytes.
* @param	channel[in] - input channel.
* @param	priv[in] - Attribute private ID.
* @return	len in case of success, negative error code otherwise.
*/
static int ltc2672_iio_attr_get(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv)
{
	int ret;
	uint32_t read_val = 0;
	uint8_t gpio_state;
	float current = 0;

	switch (priv) {
	case DAC_CH_RAW:
		return sprintf(buf, "%lu", ch_dac_codes[channel->ch_num]);

	case DAC_CH_OFFSET:
		return sprintf(buf, "%u", attr_offset_val);

	case DAC_CH_SCALE:
		return sprintf(buf, "%0.10f", attr_scale_val[channel->ch_num]);

	case DAC_CH_CURRENT:
		if (ltc2672_dev_desc->out_spans[channel->ch_num] == LTC2672_VMINUS_VREF) {
			current = LTC2672_VMINUS_FIXED_CURRENT;
		} else {
			/* Convert the dac code to current in mA */
			current = (ch_dac_codes[channel->ch_num] + attr_offset_val)
				  * attr_scale_val[channel->ch_num] * (ref_voltage / DAC_VREF) *
				  (DAC_FSADJ_RESISTOR / resistor_fsadj);
		}

		return sprintf(buf, "%5.4fmA", current);

	case DAC_CH_INPUT_A:
		if (ltc2672_dev_desc->out_spans[channel->ch_num] == LTC2672_VMINUS_VREF) {
			return sprintf(buf, "%5.4fmA", current);
		} else {
			/* Convert the dac code to current in mA */
			current = (ch_dac_codes_reg_a[channel->ch_num] + attr_offset_val)
				  * attr_scale_val[channel->ch_num] * (ref_voltage / DAC_VREF) *
				  (DAC_FSADJ_RESISTOR / resistor_fsadj);

			return sprintf(buf, "%5.4fmA", current);
		}

	case DAC_CH_INPUT_B:
		if (ltc2672_dev_desc->out_spans[channel->ch_num] == LTC2672_VMINUS_VREF) {
			return sprintf(buf, "%5.4fmA", current);
		} else {
			/* Convert the dac code to current in mA */
			current = (ch_dac_codes_reg_b[channel->ch_num] + attr_offset_val)
				  * attr_scale_val[channel->ch_num] * (ref_voltage / DAC_VREF) *
				  (DAC_FSADJ_RESISTOR / resistor_fsadj);

			return sprintf(buf, "%5.4fmA", current);
		}

	case DAC_CH_SPAN:
		read_val = ltc2672_dev_desc->out_spans[channel->ch_num];

		if (read_val == LTC2672_4800VREF) {
			read_val = LTC2672_NUM_CURRENT_SPANS - 1;
		}

		return sprintf(buf,
			       "%s",
			       ltc2672_current_spans[read_val]);

	case DAC_CH_POWERDOWN:
		return sprintf(buf, "%s", ltc2672_powerdown_options[0]);

	case DAC_CH_SW_LDAC:
		return sprintf(buf, "%s", ltc2672_update_options[0]);

	case DAC_CH_WRITE_TO_N_UPDATE_ALL:
		if (ltc2672_dev_desc->out_spans[channel->ch_num] == LTC2672_VMINUS_VREF) {
			current = LTC2672_VMINUS_FIXED_CURRENT;
		} else {
			/* Convert the dac code to current in mA */
			current = (ch_dac_codes[channel->ch_num] + attr_offset_val)
				  * attr_scale_val[channel->ch_num] * (ref_voltage / DAC_VREF) *
				  (DAC_FSADJ_RESISTOR / resistor_fsadj);
		}

		return sprintf(buf, "%5.4fmA", current);

	case DAC_CH_TOGGLE_SEL:
		return sprintf(buf, "%s",
			       ltc2672_toggle_sel_options[no_os_test_bit(channel->ch_num, &toggle_sel_bits)]);

	case DAC_CH_OPEN_CIRCUIT_FAULT:
		/* Get fault register bitfield from previous command */
		fault_register = no_os_field_get(LTC2672_FAULT_REG_MASK,
						 ltc2672_dev_desc->prev_command);

		return sprintf(buf, "%s", ltc2672_fault_options[no_os_test_bit(channel->ch_num,
				&fault_register)]);

	case DAC_CURRENT:
		if (all_chs_span == LTC2672_VMINUS_VREF) {
			current = LTC2672_VMINUS_FIXED_CURRENT;
		} else {
			/* Convert the dac code to current in mA */
			current = (all_chs_dac_code + attr_offset_val)
				  * all_chs_scale * (ref_voltage / DAC_VREF) * (DAC_FSADJ_RESISTOR /
						  resistor_fsadj);
		}

		return sprintf(buf, "%5.4fmA", current);

	case DAC_RAW:
		return sprintf(buf, "%lu", all_chs_dac_code);

	case DAC_SPAN:
		read_val = all_chs_span;

		if (read_val == LTC2672_4800VREF) {
			read_val = LTC2672_NUM_CURRENT_SPANS - 1;
		}

		return sprintf(buf,
			       "%s",
			       ltc2672_current_spans[read_val]);

	case DAC_MUX:
		return sprintf(buf, "%s", mux_select[mux_val]);

	case DAC_SAMPLE_RATE:
		return sprintf(buf, "%ld", ltc2672_toggle_rate);

	case DAC_READBACK:
		return sprintf(buf, "0x%08lx", ltc2672_dev_desc->prev_command);

	case DAC_RESET:
		return sprintf(buf, "%s", ltc2672_reset_options[0]);

	case DAC_HW_TOGGLE_STATE:
		return sprintf(buf, "%s", ltc2672_toggle_pins_states[tgp_state]);

	case DAC_TOGGLE_PWM:
		return sprintf(buf, "%s", ltc2672_toggle_pwm_options[ltc2672_tgp_pwm_enabled]);

	case DAC_CHIP_POWERDOWN:
		return sprintf(buf, "%s", ltc2672_powerdown_options[0]);

	case DAC_INPUT_A:
		if (all_chs_span == LTC2672_VMINUS_VREF) {
			return sprintf(buf, "%5.4fmA", current);
		} else {
			/* Convert the dac code to current in mA */
			current = (all_chs_dac_code_reg_a + attr_offset_val)
				  * all_chs_scale * (ref_voltage / DAC_VREF) * (DAC_FSADJ_RESISTOR /
						  resistor_fsadj);

			return sprintf(buf, "%5.4fmA", current);
		}

	case DAC_INPUT_B:
		if (all_chs_span == LTC2672_VMINUS_VREF) {
			return sprintf(buf, "%5.4fmA", current);
		} else {
			/* Convert the dac code to current in mA */
			current = (all_chs_dac_code_reg_b + attr_offset_val)
				  * all_chs_scale * (ref_voltage / DAC_VREF) * (DAC_FSADJ_RESISTOR /
						  resistor_fsadj);

			return sprintf(buf, "%5.4fmA", current);
		}

	case DAC_HW_LDAC:
		return sprintf(buf, "%s", ltc2672_update_options[0]);

	case DAC_SW_LDAC:
		return sprintf(buf, "%s", ltc2672_update_options[0]);

	case DAC_FAULT:
		ret = no_os_gpio_get_value(ltc2672_dev_desc->gpio_fault, &gpio_state);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%s", ltc2672_fault_pins_states[gpio_state]);

	case DAC_OPEN_CIRCUIT_CONFIG:
		return sprintf(buf, "%s", fault_detection_options[config_oc]);

	case DAC_POWER_LIMIT_CONFIG:
		return sprintf(buf, "%s", fault_detection_options[config_pl]);

	case DAC_THERMAL_SHUTDOWN_CONFIG:
		return sprintf(buf, "%s", fault_detection_options[config_ts]);

	case DAC_EXTERNAL_REFERENCE_CONFIG:
		return sprintf(buf, "%s", external_reference_options[config_rd]);

	case DAC_SW_TOGGLE_STATE:
		return sprintf(buf, "%s",
			       ltc2672_gobal_toggle_options[ltc2672_dev_desc->global_toggle]);

	case DAC_OVER_TEMP_FAULT:
		/* Get fault register bitfield from previous command */
		fault_register = no_os_field_get(LTC2672_FAULT_REG_MASK,
						 ltc2672_dev_desc->prev_command);

		return sprintf(buf, "%s",
			       ltc2672_fault_options[no_os_test_bit(LTC2672_OVER_TEMP, &fault_register)]);

	case DAC_POWER_LIMIT_FAULT:
		/* Get fault register bitfield from previous command */
		fault_register = no_os_field_get(LTC2672_FAULT_REG_MASK,
						 ltc2672_dev_desc->prev_command);

		return sprintf(buf, "%s", ltc2672_fault_options[no_os_test_bit(LTC2672_POW_LIM,
				&fault_register)]);

	case DAC_SPI_LENGTH_FAULT:
		/* Get fault register bitfield from previous command */
		fault_register = no_os_field_get(LTC2672_FAULT_REG_MASK,
						 ltc2672_dev_desc->prev_command);

		return sprintf(buf, "%s",
			       ltc2672_fault_options[no_os_test_bit(LTC2672_INV_LENGTH, &fault_register)]);

	case DAC_REFERENCE:
		return sprintf(buf, "%.3f", ref_voltage);

	case DAC_RESISTOR:
		return sprintf(buf, "%.3f", resistor_fsadj);

	case DAC_NO_OP:
		return sprintf(buf, "%s", ltc2672_no_op_options[0]);

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Setter function for LTC2672 attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of expected bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int ltc2672_iio_attr_set(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv)
{
	int ret;
	uint8_t val;
	uint32_t write_val;
	uint32_t current_val_ua; //current value in uA
	char *end;
	uint32_t current_code;
	uint8_t gpio_state;
	uint32_t command;
	float requested_ref_voltage;
	float requested_resistor_value;
	uint32_t requested_sampling_rate;

	switch (priv) {
	case DAC_CH_SCALE:
	case DAC_CH_OFFSET:
	case DAC_CH_OPEN_CIRCUIT_FAULT:
	case DAC_READBACK:
	case DAC_FAULT:
	case DAC_OVER_TEMP_FAULT:
	case DAC_POWER_LIMIT_FAULT:
	case DAC_SPI_LENGTH_FAULT:
		//read-only
		break;

	case DAC_CH_RAW:
		if (ltc2672_dev_desc->out_spans[channel->ch_num] == LTC2672_VMINUS_VREF) {
			return -EINVAL;
		}

		write_val = no_os_str_to_uint32(buf);

		ret = ltc2672_set_code_channel(ltc2672_dev_desc, write_val, channel->ch_num);
		if (ret) {
			return ret;
		}

		/* Update ch-wise dac codes array */
		ch_dac_codes[channel->ch_num] = write_val;

		break;

	case DAC_SAMPLE_RATE:
		requested_sampling_rate = no_os_str_to_uint32(buf);
		if (requested_sampling_rate == 0) {
			return -EINVAL;
		}

		ltc2672_toggle_rate = requested_sampling_rate;

		/* Configure the DAC update rate supported by the selected platform */
		ret = ltc2672_set_toggling_rate(&ltc2672_toggle_rate);
		if (ret) {
			return ret;
		}

		break;

	case DAC_CH_CURRENT:
		if (ltc2672_dev_desc->out_spans[channel->ch_num] ==
		    LTC2672_VMINUS_VREF) {
			return -EINVAL;
		}

		current_val_ua = (uint32_t)(strtof(buf, &end) * 1000);

		/* Adjust for the reference value */
		current_val_ua *= (DAC_VREF / ref_voltage) * (resistor_fsadj /
				  DAC_FSADJ_RESISTOR);

		ret = ltc2672_set_current_channel(ltc2672_dev_desc, current_val_ua,
						  channel->ch_num);
		if (ret) {
			return ret;
		}

		/* Update ch-wise dac codes array */
		ch_dac_codes_reg_a[channel->ch_num] = ltc2672_current_to_code(device,
						      current_val_ua,
						      channel->ch_num);
		ch_dac_codes[channel->ch_num] = ch_dac_codes_reg_a[channel->ch_num];

		break;

	case DAC_CH_INPUT_A:
		if (ltc2672_dev_desc->out_spans[channel->ch_num] == LTC2672_VMINUS_VREF) {
			return -EINVAL;
		}

		current_val_ua = (uint32_t)(strtof(buf, &end) * 1000);

		/* Adjust for the reference value */
		current_val_ua *= (DAC_VREF / ref_voltage) * (resistor_fsadj /
				  DAC_FSADJ_RESISTOR);

		ret = ltc2672_write_input_register_channel(device, channel->ch_num,
				current_val_ua, true);
		if (ret) {
			return ret;
		}

		/* Update ch-wise dac codes array */
		ch_dac_codes_reg_a[channel->ch_num] = ltc2672_current_to_code(device,
						      current_val_ua,
						      channel->ch_num);

		break;

	case DAC_CH_INPUT_B:
		if (ltc2672_dev_desc->out_spans[channel->ch_num] == LTC2672_VMINUS_VREF) {
			return -EINVAL;
		}

		current_val_ua = (uint32_t)(strtof(buf, &end) * 1000);

		/* Adjust for the reference value */
		current_val_ua *= (DAC_VREF / ref_voltage) * (resistor_fsadj /
				  DAC_FSADJ_RESISTOR);

		ret = ltc2672_write_input_register_channel(device, channel->ch_num,
				current_val_ua, false);
		if (ret) {
			return ret;
		}

		/* Update ch-wise dac codes array */
		ch_dac_codes_reg_b[channel->ch_num] = ltc2672_current_to_code(device,
						      current_val_ua,
						      channel->ch_num);

		break;

	case DAC_CH_SPAN:
		for (val = 0; val < LTC2672_NUM_CURRENT_SPANS; val++) {
			if (!strcmp(buf, ltc2672_current_spans[val])) {
				break;
			}
		}

		if (val == LTC2672_NUM_CURRENT_SPANS - 1) {
			val = LTC2672_4800VREF;
		}

		ret = ltc2672_set_span_channel(ltc2672_dev_desc, val, channel->ch_num);
		if (ret) {
			return ret;
		}

		/* Calculate the scale from output span selected */
		ret = ltc2672_get_scale(channel->ch_num, &attr_scale_val[channel->ch_num]);
		if (ret) {
			return ret;
		}

		break;

	case DAC_CH_POWERDOWN:
		ret = ltc2672_power_down_channel(ltc2672_dev_desc, channel->ch_num);
		if (ret) {
			return ret;
		}

		/* Update ch-wise dac codes array */
		ch_dac_codes[channel->ch_num] = 0;

		break;

	case DAC_CH_SW_LDAC:
		ret = ltc2672_update_channel(ltc2672_dev_desc, channel->ch_num);
		if (ret) {
			return ret;
		}

		/* Update ch-wise dac codes array */
		ch_dac_codes[channel->ch_num] = ch_dac_codes_reg_a[channel->ch_num];

		break;

	case DAC_CH_WRITE_TO_N_UPDATE_ALL:
		if (ltc2672_dev_desc->out_spans[channel->ch_num] == LTC2672_VMINUS_VREF) {
			return -EINVAL;
		}

		current_val_ua = (uint32_t)(strtof(buf, &end) * 1000);

		/* Adjust for the reference value */
		current_val_ua *= (DAC_VREF / ref_voltage) * (resistor_fsadj /
				  DAC_FSADJ_RESISTOR);

		current_code = ltc2672_current_to_code(device, current_val_ua, channel->ch_num);

		if (ltc2672_dev_desc->id == LTC2672_12 || ltc2672_dev_desc->id == LTC2662_12)
			current_code <<= LTC2672_BIT_SHIFT_12BIT;

		command = LTC2672_COMMAND32_GENERATE(
				  LTC2672_CODE_TO_CHANNEL_X_PWRUP_UPD_CHANNEL_ALL, channel->ch_num, current_code);

		ret = ltc2672_transaction(ltc2672_dev_desc, command, true);
		if (ret) {
			return ret;
		}

		/* Update ch-wise dac codes array */
		ch_dac_codes_reg_a[channel->ch_num] = ltc2672_current_to_code(device,
						      current_val_ua,
						      channel->ch_num);

		for (val = 0; val < LTC2672_TOTAL_CHANNELS; val++) {
			ch_dac_codes[val] = ch_dac_codes_reg_a[val];
		}

		break;

	case DAC_CH_TOGGLE_SEL:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ltc2672_toggle_sel_options); val++) {
			if (!strcmp(buf, ltc2672_toggle_sel_options[val])) {
				break;
			}
		}

		toggle_sel_bits = val ? (toggle_sel_bits | NO_OS_BIT(channel->ch_num)) :
				  (toggle_sel_bits & ~NO_OS_BIT(channel->ch_num));

		break;

	case DAC_RAW:
		if (all_chs_span == LTC2672_VMINUS_VREF) {
			return -EINVAL;
		}

		write_val = no_os_str_to_uint32(buf);

		ret = ltc2672_set_code_all_channels(ltc2672_dev_desc, write_val);
		if (ret) {
			return ret;
		}

		/* Update ch-wise dac array */
		for (val = 0; val < LTC2672_TOTAL_CHANNELS; val++) {
			ch_dac_codes[val] = write_val;
		}

		/* Update all channels' cached value */
		all_chs_dac_code = write_val;

		break;

	case DAC_CURRENT:
		if (all_chs_span == LTC2672_VMINUS_VREF) {
			return -EINVAL;
		}

		current_val_ua = (uint32_t)(strtof(buf, &end) * 1000);

		/* Adjust for the reference value */
		current_val_ua *= (DAC_VREF / ref_voltage) * (resistor_fsadj /
				  DAC_FSADJ_RESISTOR);

		ret = ltc2672_set_current_all_channels(ltc2672_dev_desc, current_val_ua);
		if (ret) {
			return ret;
		}

		write_val = ltc2672_current_to_code(device, current_val_ua,
						    LTC2672_DAC0);

		/* Update ch-wise dac array */
		for (val = 0; val < LTC2672_TOTAL_CHANNELS; val++) {
			ch_dac_codes_reg_a[val] = write_val;
			ch_dac_codes[val] = ch_dac_codes_reg_a[val];
		}

		/* Update all channels' cached value */
		all_chs_dac_code_reg_a = write_val;
		all_chs_dac_code = all_chs_dac_code_reg_a;

		break;

	case DAC_SPAN:
		for (val = 0; val < LTC2672_NUM_CURRENT_SPANS; val++) {
			if (!strcmp(buf, ltc2672_current_spans[val])) {
				break;
			}
		}

		if (val == LTC2672_NUM_CURRENT_SPANS - 1) {
			val = LTC2672_4800VREF;
		}

		ret = ltc2672_set_span_all_channels(ltc2672_dev_desc, val);
		if (ret) {
			return ret;
		}

		/* Calculate the scale from output span selected */
		for (val = 0; val < LTC2672_TOTAL_CHANNELS; val++) {
			ret = ltc2672_get_scale(val, &attr_scale_val[val]);
			if (ret) {
				return ret;
			}
		}

		/* Update all channels' cached value */
		all_chs_span = ltc2672_dev_desc->out_spans[0];
		all_chs_scale = attr_scale_val[0];

		break;

	case DAC_MUX:
		for (val = 0; val < mux_count; val++) {
			if (!strcmp(buf, mux_select[val])) {
				break;
			}
		}

		if (val == mux_count) {
			return -EINVAL;
		}

		ret = ltc2672_monitor_mux(ltc2672_dev_desc, mux_map[val]);
		if (ret) {
			return ret;
		}
		mux_val = val;

		break;

	case DAC_RESET:
		ret = ltc2672_reset(ltc2672_dev_desc);
		if (ret) {
			return ret;
		}

		memset(ch_dac_codes, 0, sizeof(ch_dac_codes));
		memset(ch_dac_codes_reg_a, 0, sizeof(ch_dac_codes_reg_a));
		memset(ch_dac_codes_reg_b, 0, sizeof(ch_dac_codes_reg_b));
		memset(attr_scale_val, 0, sizeof(attr_scale_val));
		all_chs_dac_code = 0;
		all_chs_dac_code_reg_a = 0;
		all_chs_dac_code_reg_b = 0;
		all_chs_span = LTC2672_OFF;
		all_chs_scale = 0;
		mux_val = 0;
		toggle_sel_bits = 0;
		config_oc = false;
		config_pl = false;
		config_ts = false;
		config_rd = false;

		break;

	case DAC_HW_TOGGLE_STATE:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ltc2672_toggle_pins_states); val++) {
			if (!strcmp(buf, ltc2672_toggle_pins_states[val])) {
				break;
			}
		}

		/* Configure the output state of the GPIO based on the option chosen
		 * and set it to the respective value */
		if (val) {
			gpio_state = NO_OS_GPIO_HIGH;
			tgp_state = true;
		} else {
			gpio_state = NO_OS_GPIO_LOW;
			tgp_state = false;
		}

		/* Global Toggle bit need to be set */
		ret = ltc2672_global_toggle(ltc2672_dev_desc, 1);
		if (ret)
			return ret;

		/* Select the channels to togggle */
		ret = ltc2672_enable_toggle_channel(ltc2672_dev_desc, toggle_sel_bits);
		if (ret) {
			return ret;
		}

		ret = no_os_gpio_remove(ltc2672_dev_desc->gpio_tgp);
		if (ret) {
			return ret;
		}

		ret = no_os_gpio_get(&(ltc2672_dev_desc->gpio_tgp),
				     ltc2672_init_params.gpio_tgp);
		if (ret) {
			return ret;
		}

		ret = no_os_gpio_direction_output(ltc2672_dev_desc->gpio_tgp, gpio_state);
		if (ret) {
			return ret;
		}

		/* Reset the toggle select bits */
		ret = ltc2672_enable_toggle_channel(ltc2672_dev_desc, 0);
		if (ret) {
			return ret;
		}

		/* Update ch-wise dac codes array */
		for (val = 0; val < LTC2672_TOTAL_CHANNELS; val++) {
			if (no_os_test_bit(val, &toggle_sel_bits)) {
				ch_dac_codes[val] = gpio_state ? ch_dac_codes_reg_b[val] :
						    ch_dac_codes_reg_a[val];
			}
		}

		break;

	case DAC_TOGGLE_PWM:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ltc2672_toggle_pwm_options); val++) {
			if (!strcmp(buf, ltc2672_toggle_pwm_options[val])) {
				break;
			}
		}

		if (val) {
			/* Configure LDAC as a PWM GPIO */
			ret = no_os_gpio_get(&(ltc2672_dev_desc->gpio_tgp),
					     &toggle_pwm_gpio_params);
			if (ret) {
				return ret;
			}

			/* Enable PWM, if not already enabled */
			if (!ltc2672_tgp_pwm_enabled) {
				/* Global Toggle bit need to be set */
				ret = ltc2672_global_toggle(ltc2672_dev_desc, 1);
				if (ret)
					return ret;

				/* Select the channels to togggle */
				ret = ltc2672_enable_toggle_channel(ltc2672_dev_desc, toggle_sel_bits);
				if (ret) {
					return ret;
				}

				ret = no_os_pwm_enable(toggle_pwm_desc);
				if (ret) {
					return ret;
				}
			}

			ltc2672_tgp_pwm_enabled = true;
		} else {
			/* Disable PWM, if not already disabled */
			if (ltc2672_tgp_pwm_enabled) {
				ret = no_os_pwm_disable(toggle_pwm_desc);
				if (ret) {
					return ret;
				}

				/* Reset the toggle select bits */
				ret = ltc2672_enable_toggle_channel(ltc2672_dev_desc, 0);
				if (ret) {
					return ret;
				}

				tgp_state = true;

				/* Update ch-wise dac codes array */
				for (val = 0; val < LTC2672_TOTAL_CHANNELS; val++) {
					if (no_os_test_bit(val, &toggle_sel_bits)) {
						ch_dac_codes[val] = ch_dac_codes_reg_b[val];
					}
				}
			}

			ltc2672_tgp_pwm_enabled = false;
		}

		break;

	case DAC_CHIP_POWERDOWN:
		ret = ltc2672_chip_power_down(ltc2672_dev_desc);
		if (ret) {
			return ret;
		}

		/* Update ch-wise dac codes array */
		for (val = 0; val < LTC2672_TOTAL_CHANNELS; val++) {
			ch_dac_codes[val] = 0;
		}

		break;

	case DAC_INPUT_A:
		if (all_chs_span == LTC2672_VMINUS_VREF) {
			return -EINVAL;
		}

		current_val_ua = (uint32_t)(strtof(buf, &end) * 1000);

		/* Adjust for the reference value */
		current_val_ua *= (DAC_VREF / ref_voltage) * (resistor_fsadj /
				  DAC_FSADJ_RESISTOR);

		ret = ltc2672_write_input_register_all_channels(device, current_val_ua, true);
		if (ret) {
			return ret;
		}

		write_val = ltc2672_current_to_code(device,
						    current_val_ua,
						    LTC2672_DAC0);

		/* Update ch-wise dac array */
		for (val = 0; val < LTC2672_TOTAL_CHANNELS; val++) {
			ch_dac_codes_reg_a[val] = write_val;
		}

		/* Update all channels' cached value */
		all_chs_dac_code_reg_a = write_val;

		break;

	case DAC_INPUT_B:
		if (all_chs_span == LTC2672_VMINUS_VREF) {
			return -EINVAL;
		}

		current_val_ua = (uint32_t)(strtof(buf, &end) * 1000);

		/* Adjust for the reference value */
		current_val_ua *= (DAC_VREF / ref_voltage) * (resistor_fsadj /
				  DAC_FSADJ_RESISTOR);

		ret = ltc2672_write_input_register_all_channels(device, current_val_ua, false);
		if (ret) {
			return ret;
		}

		write_val = ltc2672_current_to_code(device,
						    current_val_ua,
						    LTC2672_DAC0);

		/* Update ch-wise dac array */
		for (val = 0; val < LTC2672_TOTAL_CHANNELS; val++) {
			ch_dac_codes_reg_b[val] = write_val;
		}

		/* Update all channels' cached value */
		all_chs_dac_code_reg_b = write_val;

		break;

	case DAC_HW_LDAC:
		ret = ltc2672_hw_ldac_update(ltc2672_dev_desc);

		/* Update ch-wise dac codes array */
		for (val = 0; val < LTC2672_TOTAL_CHANNELS; val++) {
			ch_dac_codes[val] = ch_dac_codes_reg_a[val];
		}

		break;

	case DAC_SW_LDAC:
		ret = ltc2672_update_all_channels(ltc2672_dev_desc);
		if (ret) {
			return ret;
		}

		/* Update ch-wise dac codes array */
		for (val = 0; val < LTC2672_TOTAL_CHANNELS; val++) {
			ch_dac_codes[val] = ch_dac_codes_reg_a[val];
		}

		break;

	case DAC_OPEN_CIRCUIT_CONFIG:
		for (val = 0; val < NO_OS_ARRAY_SIZE(fault_detection_options); val++) {
			if (!strcmp(buf, fault_detection_options[val])) {
				break;
			}
		}

		config_oc = val ? true : false;

		ret = ltc2672_config_command(ltc2672_dev_desc, CONFIG_COMMAND(config_oc,
					     config_pl, config_ts, config_rd));
		if (ret) {
			return ret;
		}

		break;

	case DAC_POWER_LIMIT_CONFIG:
		for (val = 0; val < NO_OS_ARRAY_SIZE(fault_detection_options); val++) {
			if (!strcmp(buf, fault_detection_options[val])) {
				break;
			}
		}

		config_pl = val ? true : false;

		ret = ltc2672_config_command(ltc2672_dev_desc, CONFIG_COMMAND(config_oc,
					     config_pl, config_ts, config_rd));
		if (ret) {
			return ret;
		}

		break;

	case DAC_THERMAL_SHUTDOWN_CONFIG:
		for (val = 0; val < NO_OS_ARRAY_SIZE(fault_detection_options);
		     val++) {
			if (!strcmp(buf, fault_detection_options[val])) {
				break;
			}
		}

		config_ts = val ? true : false;

		ret = ltc2672_config_command(ltc2672_dev_desc, CONFIG_COMMAND(config_oc,
					     config_pl, config_ts, config_rd));
		if (ret) {
			return ret;
		}

		break;

	case DAC_EXTERNAL_REFERENCE_CONFIG:
		for (val = 0; val < NO_OS_ARRAY_SIZE(external_reference_options); val++) {
			if (!strcmp(buf, external_reference_options[val])) {
				break;
			}
		}

		config_rd = val ? true : false;

		ret = ltc2672_config_command(ltc2672_dev_desc, CONFIG_COMMAND(config_oc,
					     config_pl, config_ts, config_rd));
		if (ret) {
			return ret;
		}

		break;

	case DAC_SW_TOGGLE_STATE:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ltc2672_gobal_toggle_options); val++) {
			if (!strcmp(buf, ltc2672_gobal_toggle_options[val])) {
				break;
			}
		}

		/* The toggle pin needs to be set high */
		tgp_state = true;

		ret = no_os_gpio_remove(ltc2672_dev_desc->gpio_tgp);
		if (ret) {
			return ret;
		}

		ret = no_os_gpio_get(&(ltc2672_dev_desc->gpio_tgp),
				     ltc2672_init_params.gpio_tgp);
		if (ret) {
			return ret;
		}

		ret = no_os_gpio_direction_output(ltc2672_dev_desc->gpio_tgp, NO_OS_GPIO_HIGH);
		if (ret) {
			return ret;
		}

		/* Select the channels to togggle */
		ret = ltc2672_enable_toggle_channel(ltc2672_dev_desc, toggle_sel_bits);
		if (ret) {
			return ret;
		}

		ret = ltc2672_global_toggle(ltc2672_dev_desc, val);
		if (ret)
			return ret;

		/* Reset the toggle select bits */
		ret = ltc2672_enable_toggle_channel(ltc2672_dev_desc, 0);
		if (ret) {
			return ret;
		}

		/* Update ch-wise dac codes array */
		for (val = 0; val < LTC2672_TOTAL_CHANNELS; val++) {
			if (no_os_test_bit(val, &toggle_sel_bits)) {
				ch_dac_codes[val] = ltc2672_dev_desc->global_toggle ? ch_dac_codes_reg_b[val] :
						    ch_dac_codes_reg_a[val];
			}
		}

		break;

	case DAC_REFERENCE:
		requested_ref_voltage = strtof(buf, &end);
		if (requested_ref_voltage < LTC2672_MIN_REF_VOLTAGE ||
		    requested_ref_voltage > LTC2672_MAX_REF_VOLTAGE) {
			return -EINVAL;
		}
		ref_voltage = requested_ref_voltage;

		break;

	case DAC_RESISTOR:
		requested_resistor_value = strtof(buf, &end);
		if (requested_resistor_value < LTC2672_MIN_FSADJ_RESISTOR ||
		    requested_resistor_value > LTC2672_MAX_FSADJ_RESISTOR) {
			return -EINVAL;
		}
		resistor_fsadj = requested_resistor_value;

		break;

	case DAC_NO_OP:
		ret = ltc2672_transaction(ltc2672_dev_desc,
					  LTC2672_COMMAND32_GENERATE(LTC2672_NO_OP, LTC2672_DAC0, LTC2672_DUMMY),
					  true);
		if (ret) {
			return ret;
		}

		break;

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Attribute available getter function for LTC2672 attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of SUCCESS, negative error code otherwise
 */
static int ltc2672_iio_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	buf[0] = '\0';

	switch (priv) {
	case DAC_CH_POWERDOWN:
		len = append_options_to_buffer(buf, ltc2672_powerdown_options,
					       NO_OS_ARRAY_SIZE(ltc2672_powerdown_options));
		break;

	case DAC_CH_SW_LDAC:
		len = append_options_to_buffer(buf, ltc2672_update_options,
					       NO_OS_ARRAY_SIZE(ltc2672_update_options));
		break;

	case DAC_CH_TOGGLE_SEL:
		len = append_options_to_buffer(buf, ltc2672_toggle_sel_options,
					       NO_OS_ARRAY_SIZE(ltc2672_toggle_sel_options));
		break;

	case DAC_CH_SPAN:
	case DAC_SPAN:
		len = append_options_to_buffer(buf, ltc2672_current_spans,
					       LTC2672_NUM_CURRENT_SPANS);
		break;

	case DAC_MUX:
		len = append_options_to_buffer(buf, mux_select,
					       mux_count);
		break;

	case DAC_RESET:
		len = append_options_to_buffer(buf, ltc2672_reset_options,
					       NO_OS_ARRAY_SIZE(ltc2672_reset_options));
		break;

	case DAC_HW_TOGGLE_STATE:
		len = append_options_to_buffer(buf, ltc2672_toggle_pins_states,
					       NO_OS_ARRAY_SIZE(ltc2672_toggle_pins_states));
		break;

	case DAC_TOGGLE_PWM:
		len = append_options_to_buffer(buf, ltc2672_toggle_pwm_options,
					       NO_OS_ARRAY_SIZE(ltc2672_toggle_pwm_options));
		break;

	case DAC_CHIP_POWERDOWN:
		len = append_options_to_buffer(buf, ltc2672_powerdown_options,
					       NO_OS_ARRAY_SIZE(ltc2672_powerdown_options));
		break;

	case DAC_HW_LDAC:
		len = append_options_to_buffer(buf, ltc2672_update_options,
					       NO_OS_ARRAY_SIZE(ltc2672_update_options));
		break;

	case DAC_SW_LDAC:
		len = append_options_to_buffer(buf, ltc2672_update_options,
					       NO_OS_ARRAY_SIZE(ltc2672_update_options));
		break;

	case DAC_CH_OPEN_CIRCUIT_FAULT:
	case DAC_OVER_TEMP_FAULT:
	case DAC_POWER_LIMIT_FAULT:
	case DAC_SPI_LENGTH_FAULT:
		len = append_options_to_buffer(buf, ltc2672_fault_options,
					       NO_OS_ARRAY_SIZE(ltc2672_fault_options));
		break;

	case DAC_FAULT:
		len = append_options_to_buffer(buf, ltc2672_fault_pins_states,
					       NO_OS_ARRAY_SIZE(ltc2672_fault_pins_states));
		break;

	case DAC_OPEN_CIRCUIT_CONFIG:
	case DAC_POWER_LIMIT_CONFIG:
	case DAC_THERMAL_SHUTDOWN_CONFIG:
		len = append_options_to_buffer(buf, fault_detection_options,
					       NO_OS_ARRAY_SIZE(fault_detection_options));
		break;

	case DAC_EXTERNAL_REFERENCE_CONFIG:
		len = append_options_to_buffer(buf, external_reference_options,
					       NO_OS_ARRAY_SIZE(external_reference_options));
		break;

	case DAC_SW_TOGGLE_STATE:
		len = append_options_to_buffer(buf, ltc2672_gobal_toggle_options,
					       NO_OS_ARRAY_SIZE(ltc2672_gobal_toggle_options));
		break;

	case DAC_NO_OP:
		len = append_options_to_buffer(buf, ltc2672_no_op_options,
					       NO_OS_ARRAY_SIZE(ltc2672_no_op_options));
		break;

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Attribute available setter function for LTC2672 attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of SUCCESS, negative error code otherwise
 */
static int ltc2672_iio_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	return len;
}

/**
 * @brief Assign device name and resolution
 * @param dev_id[in] - The device ID
 * @param dev_name[out] - The device name
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ltc2672_assign_device(enum ltc2672_device_id dev_id,
				     char** dev_name)
{
	switch (dev_id) {
	case LTC2662_16:
		ltc2672_init_params.id = LTC2662_16;
		*dev_name = DEVICE_LTC2662_16;
		mux_select = ltc2662_mux_select;
		mux_map = ltc2662_mux_map;
		mux_count = NO_OS_ARRAY_SIZE(ltc2662_mux_select);

		break;

	case LTC2672_16:
		ltc2672_init_params.id = LTC2672_16;
		*dev_name = DEVICE_LTC2672_16;
		mux_select = ltc2672_mux_select;
		mux_map = ltc2672_mux_map;
		mux_count = NO_OS_ARRAY_SIZE(ltc2672_mux_select);

		break;

	default:
		return -EINVAL;
	}

	return 0;
}

/**
* @brief	Init for reading/writing and parametrization of an
* 			LTC2672 IIO device.
* @param 	desc[in,out] - IIO device descriptor.
* @return	0 in case of success, negative error code otherwise.
*/
static int32_t ltc2672_iio_param_init(struct iio_device **desc)
{
	struct iio_device *ltc2672_iio_inst;

	if (!desc) {
		return -EINVAL;
	}

	ltc2672_iio_inst = calloc(1, sizeof(struct iio_device));
	if (!ltc2672_iio_inst) {
		return -ENOMEM;
	}

	ltc2672_iio_inst->num_ch = NO_OS_ARRAY_SIZE(ltc2672_iio_channels);
	ltc2672_iio_inst->channels = ltc2672_iio_channels;
	ltc2672_iio_inst->debug_attributes = NULL;
	if (ltc2672_dev_desc->id == LTC2672_12 || ltc2672_dev_desc->id == LTC2672_16) {
		ltc2672_iio_inst->attributes = ltc2672_iio_global_attributes;
	} else {
		ltc2672_iio_inst->attributes = ltc2662_iio_global_attributes;
	}

	ltc2672_iio_inst->submit = NULL;
	ltc2672_iio_inst->pre_enable = NULL;
	ltc2672_iio_inst->post_disable = NULL;
	ltc2672_iio_inst->read_dev = NULL;
	ltc2672_iio_inst->write_dev = NULL;
	ltc2672_iio_inst->debug_reg_read = NULL;
	ltc2672_iio_inst->debug_reg_write = NULL;
	ltc2672_iio_inst->trigger_handler = NULL;

	*desc = ltc2672_iio_inst;

	return 0;
}

/**
 * @brief	Initialize the IIO interface for LTC2672 IIO device.
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t ltc2672_iio_init()
{
	int32_t ret;
	enum ltc2672_device_id dev_id;
	uint8_t indx;

	/* IIO device descriptor */
	struct iio_device *ltc2672_iio_dev;

	/* IIOD init parameters */
	static struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES];

	/* IIO interface init parameters */
	struct iio_init_param iio_init_params = {
		.phy_type = USE_UART,
	};

	/* Add a fixed delay of 2 sec before system init for the PoR sequence to get completed */
	no_os_udelay(2000000);

	/* Initialize the system peripherals */
	ret = init_system();
	if (ret) {
		return ret;
	}

#if !defined(DC2903A)
	/* Read context attributes */
	static const char *mezzanine_names[] = {
		"EVAL-LTC2662-ARDZ",
		"EVAL-LTC2672-ARDZ"
	};

	/* Iterate twice to detect the correct attached board */
	for (indx = 0; indx < NO_OS_ARRAY_SIZE(mezzanine_names); indx++) {
		ret = get_iio_context_attributes_ex(&iio_init_params.ctx_attrs,
						    &iio_init_params.nb_ctx_attr,
						    eeprom_desc,
						    mezzanine_names[indx],
						    STR(HW_CARRIER_NAME),
						    &hw_mezzanine_is_valid,
						    GET_FIRMWARE_VERSION);
		if (ret) {
			return ret;
		}

		if (hw_mezzanine_is_valid) {
			dev_id = INDEX_TO_DEV_ID(indx);
			break;
		}

		if (indx != NO_OS_ARRAY_SIZE(mezzanine_names) - 1) {
			ret = remove_iio_context_attributes(iio_init_params.ctx_attrs);
			if (ret) {
				return ret;
			}
		}
	}
#else
	hw_mezzanine_is_valid = true;
#endif

	if (hw_mezzanine_is_valid) {
#if !defined(DC2903A)
		ret = ltc2672_assign_device(
			      dev_id,
			      &iio_device_init_params[0].name);
		if (ret) {
			return ret;
		}
#endif

		/* Initialize LTC2672 no-os device driver interface */
		ret = ltc2672_init(&ltc2672_dev_desc, &ltc2672_init_params);
		if (ret) {
			return ret;
		}

		/* Initialize the LTC2672 IIO app specific parameters */
		ret = ltc2672_iio_param_init(&ltc2672_iio_dev);
		if (ret) {
			return ret;
		}
		iio_init_params.nb_devs++;

		/* LTC2672 IIO device init parameters */
		iio_device_init_params[0].dev = ltc2672_dev_desc;
		iio_device_init_params[0].dev_descriptor = ltc2672_iio_dev;
	}

#if defined(DC2903A)
	iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
	iio_init_params.ctx_attrs = ctx_attrs;
	iio_init_params.nb_ctx_attr = NO_OS_ARRAY_SIZE(ctx_attrs);
#endif

	/* Initialize the IIO interface */
	iio_init_params.devs = iio_device_init_params;
	iio_init_params.uart_desc = uart_iio_com_desc;
	ret = iio_init(&ltc2672_iio_desc, &iio_init_params);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief 	Run the LTC2672 IIO event handler.
 * @return	none.
 * @details	This function monitors the new IIO client event.
 */
void ltc2672_iio_event_handler(void)
{
	iio_step(ltc2672_iio_desc);
}