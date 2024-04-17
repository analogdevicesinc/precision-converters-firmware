/***************************************************************************//**
 *   @file    ad579x_iio.c
 *   @brief   AD579x IIO application interface module
********************************************************************************
 * Copyright (c) 2023-24 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <string.h>

#include "app_config.h"
#include "ad579x_iio.h"
#include "ad579x_support.h"
#include "ad5791.h"
#include "ad579x_user_config.h"
#include "common.h"
#include "no_os_error.h"
#include "no_os_util.h"
#include "no_os_gpio.h"
#include "no_os_pwm.h"
#include "iio_trigger.h"

/******** Forward declaration of getter/setter functions ********/
static int ad579x_iio_attr_get(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv);

static int ad579x_iio_attr_set(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv);

static int ad579x_iio_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int ad579x_iio_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

#define AD579X_CHN_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = ad579x_iio_attr_get,\
		.store = ad579x_iio_attr_set\
}

#define AD579X_CHN_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = ad579x_iio_attr_available_get,\
	.store = ad579x_iio_attr_available_set\
}

#define AD579X_CH(_name, _idx, _type) {\
	.name = _name, \
	.ch_type = _type,\
	.ch_out = true,\
	.indexed = true,\
	.channel = _idx,\
	.scan_index = _idx,\
	.scan_type = &ad579x_iio_scan_type,\
	.attributes = ad579x_iio_ch_attributes\
}

/*	Number of IIO devices */
#define NUM_OF_IIO_DEVICES	1

/* IIO trigger name */
#define AD579X_IIO_TRIGGER_NAME "ad579x_iio_trigger"

/* Conversion scale factor for IIO Client */
#define DEFAULT_SCALE		(((float)DAC_CH_SPAN/DAC_MAX_COUNT_BIN_OFFSET)*1000)

/* Bytes per sample */
#if defined(AD5760)
#define	BYTES_PER_SAMPLE	sizeof(uint16_t)	// For DAC resolution of 16-bits
#else
#define	BYTES_PER_SAMPLE	sizeof(uint32_t)	// For DAC resolution of 18, 20 bits
#endif

/* Number of data storage bits (needed for IIO client to send buffer of data) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

/* DAC data buffer size */
#if defined(USE_SDRAM)
#define dac_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(32768)
static int8_t dac_data_buffer[DATA_BUFFER_SIZE];
#endif

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* Pointer to the structure representing the AD579x IIO device */
struct ad5791_dev *ad579x_dev_desc = NULL;

/* IIO interface descriptor */
static struct iio_desc *ad579x_iio_desc;

/* AD579X IIO hw trigger descriptor */
static struct iio_hw_trig *ad579x_hw_trig_desc;

/* Descriptor to hold iio trigger details */
static struct iio_trigger ad579x_iio_trig_desc = {
	.is_synchronous = true,
	.enable = NULL,
	.disable = NULL
};

/* AD579X attribute unique IDs */
enum ad579x_attribute_ids {
	/* Channel Attr IDs */
	/* Channel raw value */
	DAC_RAW,
	/* Channel scale value */
	DAC_SCALE,
	/* Channel offset value */
	DAC_OFFSET,
	/* Channel powerdown */
	DAC_POWERDOWN,

	/* Global Attr IDs */
	/* Clear code register */
	DAC_CLR_CODE,
	/* Linearity error compensation */
	DAC_LIN_COMP,
	/* Output Amplifier */
	DAC_OUTPUT_AMPLIFIER,
	/* Powerdown mode */
	DAC_POWERDOWN_MODE,
	/* DAC Coding select */
	DAC_CODE_SELECT,
	/* Sampling frequency */
	DAC_SAMPLING_FREQUENCY,
	/* Load DAC Toggle */
	DAC_LDAC,
	/* Clear Async */
	DAC_CLR,
};

/* Code format selection options */
enum code_format_selection {
	TWOS_COMPLEMNT,
	BINARY_OFFSET
};

/* IIOD channels configurations */
struct scan_type ad579x_iio_scan_type = {
	.sign = 's',
	.realbits = DAC_RESOLUTION,
	.storagebits = CHN_STORAGE_BITS,
	.shift = 0,
	.is_big_endian = false
};

/* DAC voltage span for linearity compensation string representation */
static const char *ad579x_lin_comp_str [] = {
	"span_upto_10v",
	"span_10v_to_12v",
	"span_12v_to_16v",
	"span_16v_to_19v",
	"span_19v_to_20v",
	"span_10v_to_20v",
};

/* DAC output amplifier gain string representation */
static const char *ad579x_output_amplifier_gain [] = {
	"gain_of_two",
	"unity_gain_mode"
};

static const char *ad579x_powerdown_modes [] = {
	"6kohm_to_gnd",
	"three_state"
};

/* DAC code format select string representation */
static const char *ad579x_code_select [] = {
	"2s_complement",
	"offset_binary"
};

/* DAC channel powerdown string options */
static const char *ad579x_powerdown [] = {
	"0",
	"1"
};

/* IIO channels attributes list */
static struct iio_attribute ad579x_iio_ch_attributes [] = {
	AD579X_CHN_ATTR("raw", DAC_RAW),
	AD579X_CHN_ATTR("scale", DAC_SCALE),
	AD579X_CHN_ATTR("offset", DAC_OFFSET),
	AD579X_CHN_ATTR("powerdown", DAC_POWERDOWN),
	AD579X_CHN_AVAIL_ATTR("powerdown_available", DAC_POWERDOWN),
	END_ATTRIBUTES_ARRAY
};

/* IIO device (global) attributes list */
static struct iio_attribute ad579x_iio_global_attributes [] = {
	AD579X_CHN_ATTR("clear_code", DAC_CLR_CODE),
	AD579X_CHN_ATTR("clear", DAC_CLR),
	AD579X_CHN_AVAIL_ATTR("clear_available", DAC_CLR),
#if defined(DEV_AD5781) || defined(DEV_AD5791)
	AD579X_CHN_ATTR("linearity_comp", DAC_LIN_COMP),
	AD579X_CHN_AVAIL_ATTR("linearity_comp_available", DAC_LIN_COMP),
#elif (defined(DEV_AD5780) || defined(DEV_AD5790) || defined(DEV_AD5760)) && defined(INT_REF_0V_TO_10V)
	AD579X_CHN_ATTR("output_amplifier", DAC_OUTPUT_AMPLIFIER),
	AD579X_CHN_AVAIL_ATTR("output_amplifier_available", DAC_OUTPUT_AMPLIFIER),
#endif
	AD579X_CHN_ATTR("powerdown_mode", DAC_POWERDOWN_MODE),
	AD579X_CHN_AVAIL_ATTR("powerdown_mode_available", DAC_POWERDOWN_MODE),
	AD579X_CHN_ATTR("coding_select", DAC_CODE_SELECT),
	AD579X_CHN_AVAIL_ATTR("coding_select_available", DAC_CODE_SELECT),
	AD579X_CHN_ATTR("sampling_frequency", DAC_SAMPLING_FREQUENCY),
	AD579X_CHN_ATTR("hw_ldac_trigger", DAC_LDAC),
	AD579X_CHN_AVAIL_ATTR("hw_ldac_trigger_available", DAC_LDAC),
	END_ATTRIBUTES_ARRAY
};

/* IIO channels info */
static struct iio_channel ad579x_iio_channels[] = {
	AD579X_CH("Chn0", 0, IIO_VOLTAGE)
};

static float scale_val[AD579X_NUM_CHANNELS] = {
	DEFAULT_SCALE
};

static int32_t offset_val[AD579X_NUM_CHANNELS];

/* Variable to store linearity compensation value */
static enum ad5791_lin_comp_select lin_val = AD5781_SPAN_UPTO_10V;

/* Variable to store dac output state */
static volatile uint32_t dac_output_state = AD5791_OUT_TRISTATE;
static volatile uint32_t dac_pwd_mode = 1;

/* Variable to store dac amplifier gain value */
static volatile uint32_t dac_amp_gain = 1;

/* Shift value based on the resolution of the dac */
static uint8_t shift = MAX_RESOLUTION - DAC_RESOLUTION;

/* Variable to store the selected code format */
static uint8_t code_select_mode = TWOS_COMPLEMNT;

/* Variable to store DAC update rate */
static uint32_t ad579x_sampling_rate = MAX_SAMPLING_RATE;

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

/* Variable to store powered down status */
static bool dac_powered_down = true;

/* Variable to store channel span in volts */
static int8_t v_span = DAC_CH_SPAN;

/* Variable to store negative ref in volts */
static float v_neg = DAC_VREFN;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief	Set the sampling rate and get the updated value
 *			supported by MCU platform
 * @param 	sampling_rate[in,out] - Sampling rate value
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad579x_set_sampling_rate(uint32_t *sampling_rate)
{
	int32_t ret;
	uint32_t pwm_period_ns;

	if (!sampling_rate) {
		return -EINVAL;
	}

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	/* Enable PWM to get the PWM period (explicitly done for Mbed
	 * platform as it needs pwm to be enabled to update pwm period) */
	ret = no_os_pwm_enable(pwm_desc);
	if (ret) {
		return ret;
	}
#endif

	if (*sampling_rate > MAX_SAMPLING_RATE) {
		*sampling_rate = MAX_SAMPLING_RATE;
	}

	ret = no_os_pwm_set_period(pwm_desc, CONV_PERIOD_NSEC(*sampling_rate));
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_set_duty_cycle(pwm_desc,
				       CONV_DUTY_CYCLE_NSEC(*sampling_rate));
	if (ret) {
		return ret;
	}

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	ret = no_os_pwm_disable(pwm_desc);
	if (ret) {
		return ret;
	}

	/* Reconfigure the LDAC pin as GPIO output (non-PWM) */
	ret = ad579x_reconfig_ldac(ad579x_dev_desc);
	if (ret) {
		return ret;
	}
#endif

	/* Get the updated value supported by the platform */
	ret = no_os_pwm_get_period(pwm_desc, &pwm_period_ns);
	if (ret) {
		return ret;
	}

	/* Convert period (nsec) to frequency (in hertz) */
	*sampling_rate = (1.0 / pwm_period_ns) * 1000000000;

	return 0;
}

/**
 * @brief	Get the IIO scale factor
 * @param	scale[in,out] - IIO scale value
 * @return	0 in case of success, negative error code otherwise
 */
int ad579x_get_scale(float *scale)
{
	if (!scale) {
		return -EINVAL;
	}

	if (dac_amp_gain) {
		/* unity gain */
		v_span = DAC_CH_SPAN;
		v_neg = DAC_VREFN;
	} else {
		/* gain of two */
		v_span = DAC_CH_SPAN*2;
		v_neg = DAC_VREFN_GAIN_OF_TWO;
	}

	*scale = ((float)v_span/DAC_MAX_COUNT) * 1000;

	return 0;
}

/**
 * @brief	Get the IIO offset value
 * @param 	raw[in] - DAC raw data
 * @param	offset[in,out] - IIO offset value
 * @return	0 in case of success, negative error code otherwise
 */
int ad579x_get_offset(uint32_t raw, int32_t *offset)
{
	int32_t bin_code_offset;
	int32_t twosc_offset;

	if (!offset) {
		return -EINVAL;
	}

	/* Calculate offset values for both the coding selects*/
	bin_code_offset = (v_neg/v_span)*(1 << DAC_RESOLUTION);
	twosc_offset = ((v_span/2 + v_neg)/v_span)*(1 << DAC_RESOLUTION);

	if (!code_select_mode) {
		/* 2s complement code */
		if (raw >= DAC_MAX_COUNT_2S_COMPL) {
			*offset = -(DAC_MAX_COUNT_BIN_OFFSET-twosc_offset);
		} else {
			*offset = twosc_offset;
		}
	} else {
		/* Binary offset code */
		*offset = bin_code_offset;
	}

	return 0;
}

/*!
 * @brief	Getter function for AD579X attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of expected bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int ad579x_iio_attr_get(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	int32_t ret; // Variable to store return status
	uint32_t read_val; // Variable to store register data
	uint8_t val;

	switch (priv) {
	case DAC_RAW:
		ret = ad5791_get_register_value(device, AD5791_REG_DAC, &read_val);
		if (ret) {
			return ret;
		}

		/* Clear the address content from the register data */
		read_val &= ~AD579X_ADDRESS_MASK;
		read_val >>= shift;

		/* Caclulate the offset for IIO raw to voltage conversion */
		ret = ad579x_get_offset(read_val, &offset_val[channel->ch_num]);
		if (ret) {
			return ret;
		}

		return snprintf(buf, len, "%ld", read_val);

	case DAC_SCALE:
		return sprintf(buf, "%0.10f", scale_val[channel->ch_num]);

	case DAC_OFFSET:
		return sprintf(buf, "%ld", offset_val[channel->ch_num]);

	case DAC_POWERDOWN:
		ret = ad5791_get_register_value(ad579x_dev_desc, AD5791_REG_CTRL, &read_val);
		if (ret) {
			return ret;
		}

		read_val &= (AD5791_CTRL_DACTRI | AD5791_CTRL_OPGND);
		if (read_val) {
			val = 1;
		} else {
			val = 0;
		}

		return sprintf(buf, "%s", ad579x_powerdown[val]);

	case DAC_CLR_CODE:
		ret = ad5791_get_register_value(ad579x_dev_desc, AD5791_REG_CLR_CODE,
						&read_val);
		if (ret) {
			return ret;
		}

		/* Clear the address content from the register data */
		read_val &= ~AD579X_ADDRESS_MASK;
		read_val >>= shift;
		return sprintf(buf, "%ld", read_val);

	case DAC_LIN_COMP:
		return sprintf(buf, "%s", ad579x_lin_comp_str[lin_val]);

	case DAC_OUTPUT_AMPLIFIER:
		return sprintf(buf, "%s", ad579x_output_amplifier_gain[dac_amp_gain]);

	case DAC_POWERDOWN_MODE:
		return sprintf(buf, "%s", ad579x_powerdown_modes[dac_pwd_mode]);

	case DAC_CODE_SELECT:
		ret = ad5791_get_register_value(ad579x_dev_desc, AD5791_REG_CTRL, &read_val);
		if (ret) {
			return ret;
		}

		read_val &= AD5791_CTRL_BIN2SC_MASK;
		if (read_val) {
			val = BINARY_OFFSET;
		} else {
			val = TWOS_COMPLEMNT;
		}
		return sprintf(buf, "%s", ad579x_code_select[val]);

	case DAC_SAMPLING_FREQUENCY:
		return sprintf(buf, "%u", ad579x_sampling_rate);

	case DAC_LDAC:
		return sprintf(buf, "%s", "Trigger");

	case DAC_CLR:
		return sprintf(buf, "%s", "Clear");

	default:
		break;
	}

	return -EINVAL;
}

/*!
 * @brief	Setter function for AD579X attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of expected bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int ad579x_iio_attr_set(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	int32_t ret; // Variable to store return status
	uint8_t val;
	uint32_t write_val; // Variable to store the value to be written to register

	switch (priv) {
	case DAC_OFFSET:
	case DAC_SCALE:
		break;

	case DAC_RAW:
		write_val = no_os_str_to_uint32(buf);

		if (write_val > DAC_MAX_COUNT) {
			/* Write the max value if the value exceeds max count*/
			write_val = DAC_MAX_COUNT;
		}

		ret = ad5791_set_dac_value(ad579x_dev_desc, write_val);
		if (ret) {
			return ret;
		}

		break;

	case DAC_POWERDOWN:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad579x_powerdown); val++) {
			if (!strncmp(buf, ad579x_powerdown[val], strlen(buf)))
				break;
		}
		write_val = val;

		if (write_val) {
			/* Configure the DAC to powerdown mode */
			if (dac_pwd_mode) {
				val = AD5791_CTRL_DACTRI;
			} else {
				val = AD5791_CTRL_OPGND;
			}
			ret = ad5791_spi_write_mask(ad579x_dev_desc,
						    AD5791_REG_CTRL,
						    AD5791_CTRL_DACTRI | AD5791_CTRL_OPGND,
						    val);
			if (ret) {
				return ret;
			}
		} else {
			/* Configure the DAC to normal operating mode */
			ret = ad5791_dac_ouput_state(ad579x_dev_desc, AD5791_OUT_NORMAL);
			if (ret) {
				return ret;
			}
		}
		dac_powered_down = write_val;

		break;

	case DAC_CLR_CODE:
		write_val = no_os_str_to_uint32(buf);

		if (write_val > DAC_MAX_COUNT) {
			/* Write the max value if the value exceeds max count*/
			write_val = DAC_MAX_COUNT;
		}

		write_val <<= shift;
		ret = ad5791_set_register_value(ad579x_dev_desc, AD5791_REG_CLR_CODE,
						write_val);
		if (ret) {
			return ret;
		}

		break;

	case DAC_LIN_COMP:
		for (val = 0; val <= NUM_OF_V_SPANS; val++) {
			if (!strncmp(buf, ad579x_lin_comp_str[val], strlen(buf)))
				break;
		}

		lin_val = val;
		if (val == NUM_OF_V_SPANS) {
			val = AD5781_SPAN_10V_TO_20V;
		}

		ret = ad5791_set_lin_comp(ad579x_dev_desc, val);
		if (ret) {
			return ret;
		}

		break;

	case DAC_OUTPUT_AMPLIFIER:
		if (!strncmp(buf, ad579x_output_amplifier_gain[0], strlen(buf))) {
			val = 0;
		} else {
			val = 1;
		}

		ret = ad5791_spi_write_mask(ad579x_dev_desc,
					    AD5791_REG_CTRL,
					    AD5791_CTRL_RBUF_MASK,
					    AD5791_CTRL_RBUF(val));
		if (ret) {
			return ret;
		}
		dac_amp_gain = val;

		/* Recalculate scale based on gain selected */
		ret = ad579x_get_scale(&scale_val[channel->ch_num]);
		if (ret) {
			return ret;
		}

		break;

	case DAC_POWERDOWN_MODE:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad579x_powerdown_modes); val++) {
			if (!strncmp(buf, ad579x_powerdown_modes[val], strlen(buf)))
				break;
		}
		dac_pwd_mode = val;

		if (dac_powered_down) {
			/* Configure the DAC to powerdown mode */
			if (val) {
				val = AD5791_CTRL_DACTRI;
			} else {
				val = AD5791_CTRL_OPGND;
			}
			ret = ad5791_spi_write_mask(ad579x_dev_desc,
						    AD5791_REG_CTRL,
						    AD5791_CTRL_DACTRI | AD5791_CTRL_OPGND,
						    val);
			if (ret) {
				return ret;
			}
		}
		break;

	case DAC_CODE_SELECT:
		if (!strncmp(buf, ad579x_code_select[0], strlen(buf))) {
			val = 0;
		} else {
			val = 1;
		}

		ret = ad5791_spi_write_mask(ad579x_dev_desc, AD5791_REG_CTRL,
					    AD5791_CTRL_BIN2SC_MASK, AD5791_CTRL_BIN2SC(val));
		if (ret) {
			return ret;
		}
		code_select_mode = val;
		break;

	case DAC_SAMPLING_FREQUENCY:
		ad579x_sampling_rate = no_os_str_to_uint32(buf);

		ret = ad579x_set_sampling_rate(&ad579x_sampling_rate);
		if (ret) {
			return ret;
		}
		break;

	case DAC_LDAC:
		ret = ad5791_ldac_trigger(ad579x_dev_desc);
		if (ret) {
			return ret;
		}

		break;

	case DAC_CLR:
		ret = ad5791_clear_async(ad579x_dev_desc);
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
 * @brief	Attribute available getter function for AD579X attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of SUCCESS, negative error code otherwise
 */
static int ad579x_iio_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	switch (priv) {
	case DAC_POWERDOWN:
		return sprintf(buf, "%s %s", ad579x_powerdown[0], ad579x_powerdown[1]);

#if defined (DEV_AD5781)
	case DAC_LIN_COMP:
		return sprintf(buf, "%s %s", ad579x_lin_comp_str[0], ad579x_lin_comp_str[5]);
#elif defined (DEV_AD5791)
	case DAC_LIN_COMP:
		return sprintf(buf, "%s %s %s %s %s", ad579x_lin_comp_str[0],
			       ad579x_lin_comp_str[1], ad579x_lin_comp_str[2], ad579x_lin_comp_str[3],
			       ad579x_lin_comp_str[4]);
#endif

#if defined(INT_REF_0V_TO_10V)
	case DAC_OUTPUT_AMPLIFIER:
		return sprintf(buf, "%s %s", ad579x_output_amplifier_gain[0],
			       ad579x_output_amplifier_gain[1]);
#endif

	case DAC_POWERDOWN_MODE:
		return sprintf(buf, "%s %s", ad579x_powerdown_modes[0],
			       ad579x_powerdown_modes[1]);

	case DAC_CODE_SELECT:
		return sprintf(buf, "%s %s", ad579x_code_select[0], ad579x_code_select[1]);

	case DAC_LDAC:
		return sprintf(buf,
			       "%s",
			       "Trigger");

	case DAC_CLR:
		return sprintf(buf, "%s", "Clear");

	default:
		break;
	}
	return len;
}

/*!
 * @brief	Attribute available setter function for AD579X attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of SUCCESS, negative error code otherwise
 */
static int ad579x_iio_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	return len;
}

/*!
 * @brief	Read the debug register value
 * @param	dev[in, out]- Pointer to IIO device instance
 * @param	reg[in]- Register address to read from
 * @param	readval[out]- Pointer to variable to read data into
 * @return	0 in case of success, negative value otherwise
 */
static int32_t ad579x_iio_debug_reg_read(void *dev,
		uint32_t reg,
		uint32_t *readval)
{
	int32_t ret;

	if (!dev || !readval || (reg > AD5791_CMD_WR_SOFT_CTRL)) {
		return -EINVAL;
	}

	ret = ad5791_get_register_value(dev, reg, readval);
	if (ret) {
		return ret;
	}

	/* Clear the address content from the register data */
	*readval &= ~AD579X_ADDRESS_MASK;

	return 0;
}

/*!
 * @brief	Write the debug register value
 * @param	dev[in, out]- Pointer to IIO device instance
 * @param	reg[in]- Register address to write data into
 * @param	writeval[out]- Pointer to variable to write data into
 * @return	0 in case of success, negative value otherwise
 */
static int32_t ad579x_iio_debug_reg_write(void *dev,
		uint32_t reg,
		uint32_t writeval)
{
	int32_t ret;

	if (!dev  || (reg > AD5791_CMD_WR_SOFT_CTRL)) {
		return -EINVAL;
	}

	ret = ad5791_set_register_value(dev, reg, writeval);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief  Prepares the device for data transfer
 * @param  dev[in]- IIO device instance
 * @param  mask[in]- Channels select mask
 * @return 0 in case of success, error code otherwise
 */
static int32_t ad579x_iio_prepare_transfer(void *dev, uint32_t mask)
{
	int32_t ret;

	if (!dev) {
		return -EINVAL;
	}

	ret = iio_trig_enable(ad579x_hw_trig_desc);
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_enable(pwm_desc);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Terminate current data transfer
 * @param dev[in] - IIO device instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad579x_iio_end_transfer(void *dev)
{
	int32_t ret;

	if (!dev) {
		return -EINVAL;
	}

	ret = iio_trig_disable(ad579x_hw_trig_desc);
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_disable(pwm_desc);
	if (ret) {
		return ret;
	}

	/* Reconfigure the LDAC pin as GPIO output (non-PWM) */
	ret = ad579x_reconfig_ldac(ad579x_dev_desc);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Pops data from IIO buffer and writes into DAC when
 *		  trigger handler IRQ is invoked
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad579x_trigger_handler(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t dac_raw;	// Variable to store the raw dac code

	ret = iio_buffer_pop_scan(iio_dev_data->buffer, &dac_raw);
	if (ret) {
		return ret;
	}

	/* Write the value into the input register */
	dac_raw <<= shift;
	ret = ad5791_set_register_value(ad579x_dev_desc, AD5791_REG_DAC, dac_raw);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
* @brief	Init for reading/writing and parameterization of a
* 			AD579X IIO device
* @param 	desc[in,out] - IIO device descriptor
* @return	0 in case of success, negative error code otherwise
*/
static int32_t ad579x_iio_param_init(struct iio_device **desc)
{
	struct iio_device *iio_ad579x_inst;

	iio_ad579x_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad579x_inst) {
		return -EINVAL;
	}

	iio_ad579x_inst->num_ch = NO_OS_ARRAY_SIZE(ad579x_iio_channels);
	iio_ad579x_inst->channels = ad579x_iio_channels;
	iio_ad579x_inst->attributes = ad579x_iio_global_attributes;
	iio_ad579x_inst->debug_attributes = NULL;

	iio_ad579x_inst->submit = NULL; // TODO
	iio_ad579x_inst->pre_enable = ad579x_iio_prepare_transfer;
	iio_ad579x_inst->post_disable = ad579x_iio_end_transfer;
	iio_ad579x_inst->read_dev = NULL;
	iio_ad579x_inst->write_dev = NULL;
	iio_ad579x_inst->debug_reg_read = ad579x_iio_debug_reg_read;
	iio_ad579x_inst->debug_reg_write = ad579x_iio_debug_reg_write;
	iio_ad579x_inst->trigger_handler = ad579x_trigger_handler;

	*desc = iio_ad579x_inst;

	return 0;
}

/**
 * @brief Initialization of AD579x IIO hardware trigger specific parameters
 * @param desc[in,out] - IIO hardware trigger descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad579x_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	struct iio_hw_trig_init_param ad579x_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;
	int32_t ret;

	if (!desc) {
		return -EINVAL;
	}

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad579x_hw_trig_init_params.irq_id = TRIGGER_INT_ID;
	ad579x_hw_trig_init_params.name = AD579X_IIO_TRIGGER_NAME;
	ad579x_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_RISING;
	ad579x_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad579x_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad579x_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad579x_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad579x_hw_trig_init_params.iio_desc = ad579x_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad579x_hw_trig_init_params);
	if (ret) {
		return ret;
	}
	*desc = hw_trig_desc;

	return 0;
}

/**
 * @brief	Initialize the IIO interface for AD579x IIO device
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad579x_iio_init()
{
	int32_t ret;

	/* IIO device descriptor */
	struct iio_device *ad579x_iio_dev;

	/* IIO trigger init parameters */
	static struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad579x_iio_trig_desc,
		.name = AD579X_IIO_TRIGGER_NAME,
	};

	/* IIO interface init parameters */
	static struct iio_init_param iio_init_params = {
		.phy_type = USE_UART,
		.trigs = &iio_trigger_init_params,
	};

	/* IIOD init parameters */
	static struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES];

	/* Initialize the system peripherals */
	ret = init_system();
	if (ret) {
		return ret;
	}

	/* Initialize AD579x no-os device driver interface */
	ret = ad5791_init(&ad579x_dev_desc, ad579x_init_params);
	if (ret) {
		return ret;
	}

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	/* Read context attributes */
	ret = get_iio_context_attributes(&iio_init_params.ctx_attrs,
					 &iio_init_params.nb_ctx_attr,
					 eeprom_desc,
					 HW_MEZZANINE_NAME,
					 STR(HW_CARRIER_NAME),
					 &hw_mezzanine_is_valid);
	if (ret) {
		return ret;
	}

	if (hw_mezzanine_is_valid) {
#endif
		/* Initialize the AD579x IIO app specific parameters */
		ret = ad579x_iio_param_init(&ad579x_iio_dev);
		if (ret) {
			return ret;
		}
		iio_init_params.nb_devs++;

		/* AD579x IIO device init parameters */
		iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
		iio_device_init_params[0].raw_buf = dac_data_buffer;
		iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;
		iio_device_init_params[0].dev = ad579x_dev_desc;
		iio_device_init_params[0].dev_descriptor = ad579x_iio_dev;
		iio_device_init_params[0].trigger_id = "trigger0";
		iio_init_params.nb_trigs++;

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	}
#endif

	/* Initialize the IIO interface */
	iio_init_params.devs = iio_device_init_params;
	iio_init_params.uart_desc = uart_iio_com_desc;
	ret = iio_init(&ad579x_iio_desc, &iio_init_params);
	if (ret) {
		return ret;
	}

	/* Initialize the AD579x IIO trigger specific parameters */
	ret = ad579x_iio_trigger_param_init(&ad579x_hw_trig_desc);
	if (ret) {
		return ret;
	}

	/* Initialize the PWM trigger source */
	ret = init_pwm_trigger();
	if (ret) {
		return ret;
	}

	/* Reinitialize the LDAC pin as GPIO output (non-PWM) */
	ret = ad579x_reconfig_ldac(ad579x_dev_desc);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief 	Run the AD579x IIO event handler
 * @return	none
 * @details	This function monitors the new IIO client event
 */
void ad579x_iio_event_handler(void)
{
	iio_step(ad579x_iio_desc);
}
