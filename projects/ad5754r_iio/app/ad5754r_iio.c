/***************************************************************************//**
 *   @file    ad5754r_iio.c
 *   @brief   Implementation of AD5754R IIO application interfaces
********************************************************************************
 * Copyright (c) 2024 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdlib.h>
#include <string.h>

#include "app_config.h"
#include "ad5754r_iio.h"
#include "ad5754r_user_config.h"
#include "cn0586_support.h"
#include "common.h"
#include "no_os_error.h"
#include "no_os_util.h"
#include "no_os_delay.h"
#include "no_os_gpio.h"
#include "no_os_pwm.h"
#include "no_os_alloc.h"
#include "iio_trigger.h"

/******** Forward declaration of getter/setter functions ********/
static int ad5754r_iio_attr_get(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv);

static int ad5754r_iio_attr_set(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv);

static int ad5754r_iio_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int ad5754r_iio_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

/******************************************************************************/
/************************ Macros and Constants ********************************/
/******************************************************************************/

#define AD5754R_IIO_TRIGGER_NAME "ad5754r_iio_trigger"

#define AD5754R_CHN_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = ad5754r_iio_attr_get,\
	.store = ad5754r_iio_attr_set\
}

#define AD5754R_CHN_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = ad5754r_iio_attr_available_get,\
	.store = ad5754r_iio_attr_available_set\
}

#define CN0586_CHN_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = ad5754r_iio_attr_get,\
	.store = ad5754r_iio_attr_set\
}

#define CN0586_CHN_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = ad5754r_iio_attr_available_get,\
	.store = ad5754r_iio_attr_available_set\
}

#define AD5754R_DAC_CH(_name, _idx) {\
	.name = _name # _idx, \
	.ch_type = IIO_VOLTAGE,\
	.ch_out = 1,\
	.indexed = true,\
	.channel = _idx,\
	.scan_index = _idx,\
	.scan_type = &iio_ad5754r_scan_type,\
	.attributes = iio_ad5754r_dac_ch_attributes\
}

/*	Number of IIO devices */
#ifdef DEV_CN0586
#define NUM_OF_IIO_DEVICES	2
#else
#define NUM_OF_IIO_DEVICES	1
#endif

#define	BYTES_PER_SAMPLE	sizeof(uint16_t)	// For DAC resolution of 16-bits

/* Number of data storage bits (needed for IIO client to send buffer of data) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

#define		BYTE_SIZE		(uint32_t)8
#define		BYTE_MASK		(uint32_t)0xff

#define DATA_BUFFER_SIZE  (32768)

static int8_t dac_data_buffer[DATA_BUFFER_SIZE];

#ifdef DEV_CN0586
#define AD5754R_ATTRS_OFFSET	6
#else
#define AD5754R_ATTRS_OFFSET	0
#endif

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

#ifdef DEV_CN0586
struct cn0586_dev *cn0586_dev_inst = NULL;
#endif

/* IIO interface descriptor */
static struct iio_desc *ad5754r_iio_desc;

/* Pointer to the struct representing the AD5754R IIO device */
struct ad5754r_dev *ad5754r_dev_inst = NULL;

/* AD5754R IIO hw trigger descriptor */
static struct iio_hw_trig *ad5754r_hw_trig_desc;

/* Active channel sequence */
static uint8_t ad5754r_active_chns[AD5754R_NUM_CHANNELS];

/* Number of active channels */
static uint8_t num_of_active_channels = 0;

/* Descriptor to hold iio trigger details */
static struct iio_trigger ad5754r_iio_trig_desc = {
	.is_synchronous = true,
	.enable = NULL,
	.disable = NULL
};

/* AD5754R attribute unique IDs */
enum ad5754r_attribute_ids {
#ifdef DEV_CN0586
	/* CFTL Global Attr IDs */
	HVOUT_STATE,
	HVOUT_RANGE,
	HVOUT_VOLTS,
#endif
	/* AD5754R Channel Attr IDs */
	DAC_CH_RAW,
	DAC_CH_REG,
	DAC_CH_SCALE,
	DAC_CH_OFFSET,
	DAC_CH_POWERUP,
	DAC_CH_RANGE,

	/* AD5754R Global Attribute IDs */
	DAC_INT_REF_POWERUP,
	DAC_CLEAR_SETTING,
	DAC_SDO_DIS,
	DAC_UPDATE_RATE,
	DAC_CLAMP_EN,
	DAC_TSD_EN,
	DAC_OC_TSD,
	DAC_ALL_CH_CLR,
	DAC_SW_LDAC,
	DAC_HW_LDAC,
};

/* IIOD channels configurations */
struct scan_type iio_ad5754r_scan_type = {
	.sign = 'u',
	.realbits = AD5754R_MAX_RESOLUTION,
	.storagebits = CHN_STORAGE_BITS,
	.shift = 0,
	.is_big_endian = false
};

/* DAC channel attributes structure */
static struct iio_attribute iio_ad5754r_dac_ch_attributes[] = {
	AD5754R_CHN_ATTR("dac_register", DAC_CH_REG),
	AD5754R_CHN_ATTR("raw", DAC_CH_RAW),
	AD5754R_CHN_ATTR("scale", DAC_CH_SCALE),
	AD5754R_CHN_ATTR("offset", DAC_CH_OFFSET),
	AD5754R_CHN_ATTR("powerup", DAC_CH_POWERUP),
	AD5754R_CHN_AVAIL_ATTR("powerup_available", DAC_CH_POWERUP),
	AD5754R_CHN_ATTR("range", DAC_CH_RANGE),
	AD5754R_CHN_AVAIL_ATTR("range_available", DAC_CH_RANGE),
	END_ATTRIBUTES_ARRAY,
};

/* DAC device (global) attributes list */
static struct iio_attribute ad5754r_iio_global_attributes[] = {
#ifdef DEV_CN0586
	/* CFTL Global Attributes */
	CN0586_CHN_ATTR("hvout_state", HVOUT_STATE),
	CN0586_CHN_AVAIL_ATTR("hvout_state_available", HVOUT_STATE),
	CN0586_CHN_ATTR("hvout_range", HVOUT_RANGE),
	CN0586_CHN_AVAIL_ATTR("hvout_range_available", HVOUT_RANGE),
	CN0586_CHN_ATTR("hvout_volts", HVOUT_VOLTS),
	END_ATTRIBUTES_ARRAY,
#endif
	/* AD5754R Global Attributes */
	AD5754R_CHN_ATTR("int_ref_powerup", DAC_INT_REF_POWERUP),
	AD5754R_CHN_AVAIL_ATTR("int_ref_powerup_available", DAC_INT_REF_POWERUP),
	AD5754R_CHN_ATTR("clear_setting", DAC_CLEAR_SETTING),
	AD5754R_CHN_AVAIL_ATTR("clear_setting_available", DAC_CLEAR_SETTING),
	AD5754R_CHN_ATTR("sdo_disable", DAC_SDO_DIS),
	AD5754R_CHN_AVAIL_ATTR("sdo_disable_available", DAC_SDO_DIS),
	AD5754R_CHN_ATTR("sampling_frequency", DAC_UPDATE_RATE),
	AD5754R_CHN_ATTR("clamp_enable", DAC_CLAMP_EN),
	AD5754R_CHN_AVAIL_ATTR("clamp_enable_available", DAC_CLAMP_EN),
	AD5754R_CHN_ATTR("tsd_enable", DAC_TSD_EN),
	AD5754R_CHN_AVAIL_ATTR("tsd_enable_available", DAC_TSD_EN),
	AD5754R_CHN_ATTR("oc_tsd", DAC_OC_TSD),
	AD5754R_CHN_AVAIL_ATTR("oc_tsd_available", DAC_OC_TSD),
	AD5754R_CHN_ATTR("all_chns_clear", DAC_ALL_CH_CLR),
	AD5754R_CHN_AVAIL_ATTR("all_chns_clear_available", DAC_ALL_CH_CLR),
	AD5754R_CHN_ATTR("sw_ldac_trigger", DAC_SW_LDAC),
	AD5754R_CHN_AVAIL_ATTR("sw_ldac_trigger_available", DAC_SW_LDAC),
	AD5754R_CHN_ATTR("hw_ldac_trigger", DAC_HW_LDAC),
	AD5754R_CHN_AVAIL_ATTR("hw_ldac_trigger_available", DAC_HW_LDAC),
	END_ATTRIBUTES_ARRAY,
};

/* IIOD channels configurations */
static struct iio_channel ad5754r_iio_channels[AD5754R_NUM_CHANNELS] = {
	AD5754R_DAC_CH("Chn", 0),
	AD5754R_DAC_CH("Chn", 1),
	AD5754R_DAC_CH("Chn", 2),
	AD5754R_DAC_CH("Chn", 3)
};

/* Offset for DAC channels */
static int32_t offset[AD5754R_NUM_CHANNELS];

/* Loop count for Cyclic Mode Waveform Generation. */
static int32_t loop_count = -1;

/* DAC output voltage range */
static const char *ad5754r_output_ranges[] = {
	"0v_to_5v",
	"0v_to_10v",
	"0v_to_10v8",
	"neg5v_to_5v",
	"neg10v_to_10v",
	"neg10v8_to_10v8",
};

/* DAC channel powerup state */
static const char *ad5754r_dac_ch_pwr_state[] = {
	"powerdown",
	"powerup"
};

/* DAC channel clear settings */
static const char *ad5754r_clear_settings[] = {
	"0v",
	"midscale_code"
};

/* DAC channel sdo enable/disable */
static const char *ad5754r_sdo_state[] = {
	"enable",
	"disable"
};

/* DAC clamp/tsd disable/enable */
static const char *ad5754r_clamp_tsd_state[] = {
	"disable",
	"enable"
};

/* DAC oc/tsd alert states */
static const char *ad5754r_oc_tsd_alert_state[] = {
	"None",
	"OC",
	"TSD",
	"OC_and_TSD",
};

/* CFTL HVOUT range options */
static const char *cn0586_hvout_range[] = {
	"0V_to_100V",
	"M100V_to_100V",
	"M50V_to_50V",
	"0V_to_200V"
};

/* CFTL HVOUT state options */
static const char *cn0586_hvout_state[] = {
	"Disabled",
	"Enabled"
};

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

/* Sampling rate/frequency value */
static uint32_t sampling_rate = MAX_SAMPLING_RATE;

/* Use scale factor of 1000 so that (raw + offset) * scale yields
 * millivolts. */
static float scale_factor = 1000;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief	Reconfigure LDAC pin as GPIO output
 * @param 	device[in] - AD5754R device instance
 * @return	0 in case of success, negative error code otherwise
 */
int ad5754r_reconfig_ldac(struct ad5754r_dev *device)
{
	int ret;

	if (!device) {
		return -EINVAL;
	}

	ret = no_os_gpio_remove(device->gpio_ldac);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_get(&device->gpio_ldac, ad5754r_init_params.gpio_ldac_init);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(device->gpio_ldac, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	return 0;
}


/**
 * @brief	Get the sampling rate supported by MCU platform
 * @param 	sampling_rate[in,out] - sampling rate value
 * @return	0 in case of success, negative error code otherwise
 */
static int ad5754r_get_sampling_rate(uint32_t *sampling_rate)
{
	uint32_t pwm_period_ns;
	int ret;

	if (!sampling_rate) {
		return -EINVAL;
	}

	ret = no_os_pwm_get_period(pwm_desc, &pwm_period_ns);
	if (ret) {
		return ret;
	}

	/* Convert period (nsec) to frequency (in hertz) */
	*sampling_rate = CONV_TRIGGER_PERIOD_NSEC(pwm_period_ns);

	return 0;
}

/**
 * @brief	Set the sampling rate supported by MCU platform
 * @param 	sampling_rate[in] - sampling rate value
 * @return	0 in case of success, negative error code otherwise
 */
static int ad5754r_set_sampling_rate(uint32_t sampling_rate)
{
	int ret;

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	/* Enable PWM to get the PWM period (explicitly done for Mbed
	 * platform as it needs pwm to be enabled to update pwm period) */
	ret = no_os_pwm_enable(pwm_desc);
	if (ret) {
		return ret;
	}
#endif

	if (sampling_rate > MAX_SAMPLING_RATE) {
		sampling_rate = MAX_SAMPLING_RATE;
	}

	ret = no_os_pwm_set_period(pwm_desc, CONV_TRIGGER_PERIOD_NSEC(sampling_rate));
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_set_duty_cycle(pwm_desc,
				       CONV_TRIGGER_DUTY_CYCLE_NSEC(sampling_rate));
	if (ret) {
		return ret;
	}

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	ret = no_os_pwm_disable(pwm_desc);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/*!
 * @brief	Getter function for AD5754R attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of expected bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int ad5754r_iio_attr_get(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv)
{
	int ret; // Variable to store return status
	uint16_t read_val; // Variable to store register data
	float scale;
	uint8_t chn = channel->ch_num;

	switch (priv) {
#ifdef DEV_CN0586
	case HVOUT_STATE:
		return sprintf(buf,
			       "%s",
			       cn0586_hvout_state[cn0586_dev_inst->state]);

	case HVOUT_RANGE:
		return sprintf(buf,
			       "%s",
			       cn0586_hvout_range[cn0586_dev_inst->range]);

	case HVOUT_VOLTS:
		return sprintf(buf, "%0.10f", cn0586_dev_inst->hvout_volts);
#endif
	case DAC_CH_RAW:
		ret = ad5754r_read_dac_ch_register(ad5754r_dev_inst, chn, &read_val);
		if (ret) {
			return ret;
		}

		/* Check range of channel */
		if (ad5754r_dev_inst->dac_ch_range[chn] > AD5754R_SPAN_0V_TO_10V8) {
#if !defined(USE_BINARY_CODING)
			if (read_val >= DAC_MAX_COUNT_2S_COMPL) {
				offset[chn] = -(DAC_MAX_COUNT_BIN_OFFSET + 1);
			} else {
				offset[chn] = 0;
			}
#else
			offset[chn] = - DAC_MAX_COUNT_2S_COMPL;
#endif
		} else {
			offset[chn] = 0;
		}

		return sprintf(buf, "%u", read_val);

	case DAC_CH_REG:
		return ad5754r_iio_attr_get(device, buf, len, channel, DAC_CH_RAW);

	case DAC_CH_SCALE:
		scale = (ad5754r_gain_values_scaled[ad5754r_dev_inst->dac_ch_range[chn]]
			 * AD5754R_VREF) /
			(AD5754R_GAIN_SCALE * (1 << AD5754R_MAX_RESOLUTION));
		return sprintf(buf, "%0.10f", scale * scale_factor);

	case DAC_CH_OFFSET:
		return sprintf(buf, "%d", offset[chn]);

	case DAC_CH_POWERUP:
		return sprintf(buf, "%s",
			       ad5754r_dac_ch_pwr_state[ad5754r_dev_inst->dac_ch_pwr_states[chn]]);

	case DAC_CH_RANGE:
		return sprintf(buf, "%s",
			       ad5754r_output_ranges[ad5754r_dev_inst->dac_ch_range[chn]]);

	case DAC_INT_REF_POWERUP:
		return sprintf(buf, "%s",
			       ad5754r_dac_ch_pwr_state[ad5754r_dev_inst->int_ref_pwrup]);

	case DAC_CLEAR_SETTING:
		return sprintf(buf, "%s", ad5754r_clear_settings[ad5754r_dev_inst->clear_sel]);

	case DAC_SDO_DIS:
		return sprintf(buf, "%s", ad5754r_sdo_state[ad5754r_dev_inst->sdo_dis]);

	case DAC_UPDATE_RATE:
		ret = ad5754r_get_sampling_rate(&sampling_rate);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%lu", sampling_rate);

	case DAC_CLAMP_EN:
		return sprintf(buf, "%s", ad5754r_clamp_tsd_state[ad5754r_dev_inst->clamp_en]);

	case DAC_TSD_EN:
		return sprintf(buf, "%s", ad5754r_clamp_tsd_state[ad5754r_dev_inst->tsd_en]);

	case DAC_OC_TSD:
		ret = ad5754r_read(ad5754r_dev_inst,
				   AD5754R_PREP_INSTR_ADDR(AD5754R_REG_PWR_CTRL, 0),
				   &read_val);
		if (ret) {
			return ret;
		}

		read_val &= AD5754R_PWR_OC_ALERT_MASK |
			    AD5754R_PWR_TSD_ALERT_MASK;
		if (read_val == 0) {
			return sprintf(buf, "%s", ad5754r_oc_tsd_alert_state[0]);
		} else if (read_val == AD5754R_PWR_OC_ALERT_MASK) {
			return sprintf(buf, "%s", ad5754r_oc_tsd_alert_state[1]);
		} else if (read_val == AD5754R_PWR_TSD_ALERT_MASK) {
			return sprintf(buf, "%s", ad5754r_oc_tsd_alert_state[2]);
		} else if (read_val == (AD5754R_PWR_OC_ALERT_MASK |
					AD5754R_PWR_TSD_ALERT_MASK)) {
			return sprintf(buf, "%s", ad5754r_oc_tsd_alert_state[3]);
		} else {
			return -EINVAL;
		}

	case DAC_ALL_CH_CLR:
		return sprintf(buf, "%s", "Clear");

	case DAC_SW_LDAC:
		return sprintf(buf, "%s", "Trigger");

	case DAC_HW_LDAC:
		return sprintf(buf, "%s", "Trigger");

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Setter function for AD5754R attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of expected bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int ad5754r_iio_attr_set(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv)
{
	uint16_t write_val;
	int ret;
	uint8_t val;
	uint8_t chn = channel->ch_num;
	float volts;
	char *end;
	int32_t cyc_count;

	switch (priv) {
#ifdef DEV_CN0586
	case HVOUT_STATE:
		for (val = HVOUT_DISABLED; val < NUM_OF_HVOUT_STATES; val++) {
			if (!strncmp(buf, cn0586_hvout_state[val], strlen(buf))) {
				break;
			}
		}

		if (val == HVOUT_ENABLED) {
			/* Write full-scale code to DAC Channel D */
			ret = ad5754r_write(ad5754r_dev_inst,
					    AD5754R_PREP_INSTR_ADDR(AD5754R_REG_DAC, AD5754R_DAC_CH_D),
					    AD5754R_BYTE_H | AD5754R_BYTE_L);
			if (ret) {
				return ret;
			}

			cn0586_dev_inst->state = HVOUT_ENABLED;
		} else {
			/* Write 0 to DAC Channel D */
			ret = ad5754r_write(ad5754r_dev_inst,
					    AD5754R_PREP_INSTR_ADDR(AD5754R_REG_DAC, AD5754R_DAC_CH_D),
					    0);
			if (ret) {
				return ret;
			}

			cn0586_dev_inst->state = HVOUT_DISABLED;
		}

		/* Update AD5754R outputs using SW LDAC */
		ret = ad5754r_write(ad5754r_dev_inst, AD5754R_INSTR_LOAD, 0x0000);
		if (ret) {
			return ret;
		}

		break;

	case HVOUT_RANGE:
		for (val = HVOUT_0V_100V; val < NUM_OF_HVOUT_RANGES; val++) {
			if (!strncmp(buf, cn0586_hvout_range[val], strlen(buf))) {
				break;
			}
		}

		/* Disable hvout */
		ret = ad5754r_write(ad5754r_dev_inst,
				    AD5754R_PREP_INSTR_ADDR(AD5754R_REG_DAC, AD5754R_DAC_CH_D),
				    0);
		if (ret) {
			return ret;
		}
		cn0586_dev_inst->state = HVOUT_DISABLED;

		ret = cn0586_set_hvout_range(cn0586_dev_inst, val);
		if (ret) {
			return ret;
		}

		break;

	case HVOUT_VOLTS:
		volts = strtof(buf, &end);
		ret = cn0586_set_hvout_volts(cn0586_dev_inst, volts);
		if (ret) {
			return ret;
		}

		break;
#endif

	case DAC_CH_REG:
		write_val = no_os_str_to_uint32(buf);
		ret = ad5754r_write(ad5754r_dev_inst,
				    AD5754R_PREP_INSTR_ADDR(AD5754R_REG_DAC, chn),
				    write_val);
		if (ret) {
			return ret;
		}

		break;

	case DAC_CH_RAW:
		write_val = no_os_str_to_uint32(buf);
		ret = ad5754r_update_dac_ch_register(ad5754r_dev_inst, chn, write_val);
		if (ret) {
			return ret;
		}

		break;

	case DAC_CH_SCALE:
	case DAC_CH_OFFSET:
		break;

	case DAC_CH_POWERUP:
		for (val = 0; val <= AD5754R_PWR_DAC_CH_POWERUP; val++) {
			if (!strncmp(buf, ad5754r_dac_ch_pwr_state[val], strlen(buf)))
				break;
		}

		ret = ad5754r_set_ch_pwrup(ad5754r_dev_inst, chn, val);
		if (ret) {
			return ret;
		}

		break;

	case DAC_CH_RANGE:
		for (val = 0; val <= AD5754R_SPAN_M10V8_TO_10V8; val++) {
			if (!strncmp(buf, ad5754r_output_ranges[val], strlen(buf)))
				break;
		}

		ret = ad5754r_set_ch_range(ad5754r_dev_inst, chn, val);
		if (ret) {
			return ret;
		}

		break;

	case DAC_INT_REF_POWERUP:
		for (val = 0; val <= AD5754R_PWR_INT_REF_POWERUP; val++) {
			if (!strncmp(buf, ad5754r_dac_ch_pwr_state[val], strlen(buf)))
				break;
		}

		ret = ad5754r_set_int_ref_pwrup(ad5754r_dev_inst, val);
		if (ret) {
			return ret;
		}

		break;

	case DAC_CLEAR_SETTING:
		for (val = 0; val <= AD5754R_CTRL_CLEAR_MIDSCALE_CODE; val++) {
			if (!strncmp(buf, ad5754r_clear_settings[val], strlen(buf)))
				break;
		}

		ret = ad5754r_set_clear_mode(ad5754r_dev_inst, val);
		if (ret) {
			return ret;
		}
		break;

	case DAC_SDO_DIS:
		for (val = 0; val <= AD5754R_CTRL_SDO_DIS; val++) {
			if (!strncmp(buf, ad5754r_sdo_state[val], strlen(buf)))
				break;
		}

		ret = ad5754r_set_sdo_disable(ad5754r_dev_inst, val);
		if (ret) {
			return ret;
		}

		break;

	case DAC_UPDATE_RATE:
		sampling_rate = no_os_str_to_uint32(buf);
		ret = ad5754r_set_sampling_rate(sampling_rate);
		if (ret) {
			return ret;
		}
		break;

	case DAC_CLAMP_EN:
		for (val = 0; val <= AD5754R_CTRL_CLAMP_EN; val++) {
			if (!strncmp(buf, ad5754r_clamp_tsd_state[val], strlen(buf)))
				break;
		}

		ret = ad5754r_set_current_clamp_en(ad5754r_dev_inst, val);
		if (ret) {
			return ret;
		}

		break;

	case DAC_TSD_EN:
		for (val = 0; val <= AD5754R_CTRL_TSD_EN; val++) {
			if (!strncmp(buf, ad5754r_clamp_tsd_state[val], strlen(buf)))
				break;
		}

		ret = ad5754r_set_tsd_en(ad5754r_dev_inst, val);
		if (ret) {
			return ret;
		}

		break;

	case DAC_ALL_CH_CLR:
#if defined(DEV_AD5754R)
		ret = ad5754r_clear_async(ad5754r_dev_inst);
		if (ret) {
			return ret;
		}
#else
		if (ad5754r_dev_inst->gpio_clear) {
			ret = no_os_gpio_set_value(ad5754r_dev_inst->gpio_clear,
						   NO_OS_GPIO_LOW);
			if (ret) {
				return ret;
			}

			/* Minimum pulse width for EVAL-CN0586-ARDZ is 4us */
			no_os_udelay(4);

			ret = no_os_gpio_set_value(ad5754r_dev_inst->gpio_clear,
						   NO_OS_GPIO_HIGH);
			if (ret) {
				return ret;
			}
		}

		/* If no gpio is assigned use SW CLEAR */
		ret = ad5754r_write(ad5754r_dev_inst, AD5754R_INSTR_CLEAR, 0x0000);
		if (ret) {
			return ret;
		}
#endif
		break;

	case DAC_HW_LDAC:
		if (!ad5754r_dev_inst->gpio_ldac) {
			return -ENOSYS;
		}

#if defined(DEV_AD5754R)
		ret = ad5754r_ldac_trigger(ad5754r_dev_inst);
		if (ret) {
			return ret;
		}
#else
		ret = no_os_gpio_set_value(ad5754r_dev_inst->gpio_ldac,
					   NO_OS_GPIO_LOW);
		if (ret) {
			return ret;
		}

		/* Minimum pulse width for EVAL-CN0586-ARDZ is 4us */
		no_os_udelay(4);

		ret = no_os_gpio_set_value(ad5754r_dev_inst->gpio_ldac,
					   NO_OS_GPIO_HIGH);
		if (ret) {
			return ret;
		}
#endif

		break;

	case DAC_SW_LDAC:
		return ad5754r_write(ad5754r_dev_inst, AD5754R_INSTR_LOAD, 0x0000);

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Attribute available getter function for AD5754R attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of input bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int ad5754r_iio_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	switch (priv) {
#ifdef DEV_CN0586
	case HVOUT_STATE:
		return sprintf(buf,
			       "%s %s",
			       cn0586_hvout_state[0],
			       cn0586_hvout_state[1]);

	case HVOUT_RANGE:
		return sprintf(buf,
			       "%s %s %s %s",
			       cn0586_hvout_range[0],
			       cn0586_hvout_range[1],
			       cn0586_hvout_range[2],
			       cn0586_hvout_range[3]);
#endif

	case DAC_CLEAR_SETTING:
		return sprintf(buf, "%s %s", ad5754r_clear_settings[0],
			       ad5754r_clear_settings[1]);

	case DAC_CH_RANGE:
		return sprintf(buf,
			       "%s %s %s %s %s %s",
			       ad5754r_output_ranges[0],
			       ad5754r_output_ranges[1],
			       ad5754r_output_ranges[2],
			       ad5754r_output_ranges[3],
			       ad5754r_output_ranges[4],
			       ad5754r_output_ranges[5]);

	case DAC_CH_POWERUP:
		return sprintf(buf,
			       "%s %s",
			       ad5754r_dac_ch_pwr_state[0],
			       ad5754r_dac_ch_pwr_state[1]);

	case DAC_INT_REF_POWERUP:
		return sprintf(buf,
			       "%s %s",
			       ad5754r_dac_ch_pwr_state[0],
			       ad5754r_dac_ch_pwr_state[1]);

	case DAC_SDO_DIS:
		return sprintf(buf,
			       "%s %s",
			       ad5754r_sdo_state[0],
			       ad5754r_sdo_state[1]);

	case DAC_CLAMP_EN:
		return sprintf(buf,
			       "%s %s",
			       ad5754r_clamp_tsd_state[0],
			       ad5754r_clamp_tsd_state[1]);

	case DAC_TSD_EN:
		return sprintf(buf,
			       "%s %s",
			       ad5754r_clamp_tsd_state[0],
			       ad5754r_clamp_tsd_state[1]);

	case DAC_OC_TSD:
		return sprintf(buf,
			       "%s %s %s %s",
			       ad5754r_oc_tsd_alert_state[0],
			       ad5754r_oc_tsd_alert_state[1],
			       ad5754r_oc_tsd_alert_state[2],
			       ad5754r_oc_tsd_alert_state[3]);

	case DAC_ALL_CH_CLR:
		return sprintf(buf,
			       "%s",
			       "Clear");

	case DAC_HW_LDAC:
		return sprintf(buf,
			       "%s",
			       "Trigger");

	case DAC_SW_LDAC:
		return sprintf(buf,
			       "%s",
			       "Trigger");

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Attribute available setter function for AD5754R attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of input bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int ad5754r_iio_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	// TODO Implement afterwards
	return len;
}

/*!
 * @brief	Read the debug register value.
 * @param	dev[in, out]- Pointer to IIO device instance.
 * @param	reg[in]- Register address to read from.
 * @param	readval[out]- Pointer to variable to read data into.
 * @return	0 in case of success, negative value otherwise.
 */
static int32_t ad5754r_iio_debug_reg_read(void *dev,
		uint32_t reg,
		uint32_t *readval)
{
	if (!dev || !readval || (reg > AD5754R_INSTR_LOAD)) {
		return -EINVAL;
	}

	return ad5754r_read(dev, (uint8_t)reg, (uint16_t *)readval);
}

/*!
 * @brief	Write the debug register value.
 * @param	dev[in, out]- Pointer to IIO device instance.
 * @param	reg[in]- Register address to write data into.
 * @param	writeval[out]- Pointer to variable to write data into.
 * @return	0 in case of success, negative value otherwise.
 */
static int32_t ad5754r_iio_debug_reg_write(void *dev,
		uint32_t reg,
		uint32_t writeval)
{
	if (!dev  || (reg > AD5754R_INSTR_LOAD)) {
		return -EINVAL;
	}

	return ad5754r_write(dev, (uint8_t)reg, (uint16_t)writeval);
}

/**
 * @brief  Prepares the device for data transfer
 * @param  dev[in]- IIO device instance
 * @param  mask[in]- Channels select mask
 * @return 0 in case of success, error code otherwise
 */
static int32_t ad5754r_iio_prepare_transfer(void *dev, uint32_t mask)
{
	int32_t ret;
	uint8_t ch_mask = 0x1;
	uint8_t index = 0;
	uint8_t chn;

	/* Power up the internal reference */
	ret = ad5754r_set_int_ref_pwrup(ad5754r_dev_inst, AD5754R_PWR_INT_REF_POWERUP);
	if (ret) {
		return ret;
	}

	/* Store active channels based on channel mask set in the
	 * IIO client, and also power up the enabled channels */
	for (chn = 0; chn < AD5754R_NUM_CHANNELS; chn++) {
		if (ch_mask & mask) {
			ad5754r_active_chns[index++] = chn;
#if defined(DEV_CN0586)
			if (chn > AD5754R_DAC_CH_A) {
				return -EINVAL;
			}
#endif
			ret = ad5754r_set_ch_pwrup(ad5754r_dev_inst, chn, AD5754R_PWR_DAC_CH_POWERUP);
			if (ret) {
				return ret;
			}
		}
		ch_mask <<= 1;
	}
	num_of_active_channels = index;

	ret = iio_trig_enable(ad5754r_hw_trig_desc);
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
 * @brief Performs tasks needed to end data transfer
 * @param dev[in] - Device instance.
 * @return Number of samples read.
 */
static int32_t ad5754r_iio_end_transfer(void *dev)
{
	int32_t ret;

	if (!dev) {
		return -EINVAL;
	}

	ret = iio_trig_disable(ad5754r_hw_trig_desc);
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_disable(pwm_desc);
	if (ret) {
		return ret;
	}

	/* Reconfigure the LDAC pin as GPIO output (non-PWM) */
	ret = ad5754r_reconfig_ldac(ad5754r_dev_inst);
	if (ret) {
		return ret;
	}

#if defined(DEV_CN0586)
	/* Only DAC A voltage changes, so we need to update the hvout_volts */
	ret = cn0586_get_hvout_volts(cn0586_dev_inst);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief Pops data from IIO buffer and writes into DAC when
 *		  trigger handler IRQ is invoked
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad5754r_trigger_handler(struct iio_device_data *iio_dev_data)
{
	volatile int32_t ret;
	uint16_t dac_raw;	// Variable to store the raw dac code
	static uint8_t chn = 0;

	ret = iio_buffer_pop_scan(iio_dev_data->buffer, &dac_raw);
	if (ret) {
		return ret;
	}

	/* Write the value into the input register */
	ad5754r_update_dac_ch_register(ad5754r_dev_inst, ad5754r_active_chns[chn],
				       dac_raw);

	if (chn != (num_of_active_channels - 1)) {
		chn += 1;
	} else {
		chn = 0;
	}

	return 0;
}

/**
* @brief	Init for reading/writing and parametrization of an
* 			AD5754R IIO device.
* @param 	desc[in,out] - IIO device descriptor.
* @return	0 in case of success, negative error code otherwise.
*/
static int32_t ad5754r_iio_param_init(struct iio_device **desc)
{
	struct iio_device *iio_ad5754r_inst;

	iio_ad5754r_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad5754r_inst) {
		return -ENOMEM;
	}

	iio_ad5754r_inst->num_ch = NO_OS_ARRAY_SIZE(ad5754r_iio_channels);
	iio_ad5754r_inst->channels = ad5754r_iio_channels;
	iio_ad5754r_inst->attributes = ad5754r_iio_global_attributes +
				       AD5754R_ATTRS_OFFSET;
	iio_ad5754r_inst->debug_attributes = NULL;

	iio_ad5754r_inst->submit = NULL;
	iio_ad5754r_inst->pre_enable = ad5754r_iio_prepare_transfer;
	iio_ad5754r_inst->post_disable = ad5754r_iio_end_transfer;
	iio_ad5754r_inst->read_dev = NULL;
	iio_ad5754r_inst->write_dev = NULL;
	iio_ad5754r_inst->debug_reg_read = ad5754r_iio_debug_reg_read;
	iio_ad5754r_inst->debug_reg_write = ad5754r_iio_debug_reg_write;
	iio_ad5754r_inst->trigger_handler = ad5754r_trigger_handler;

	*desc = iio_ad5754r_inst;

	return 0;
}

/**
* @brief	Init for reading/writing and parametrization of an
* 			AD5754R IIO device.
* @param 	desc[in,out] - IIO device descriptor.
* @return	0 in case of success, negative error code otherwise.
*/
static int32_t cn0586_iio_param_init(struct iio_device **desc)
{
	struct iio_device *iio_cn0586_inst;

	iio_cn0586_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_cn0586_inst) {
		return -ENOMEM;
	}

	iio_cn0586_inst->num_ch = 0;
	iio_cn0586_inst->channels = NULL;
	iio_cn0586_inst->attributes = ad5754r_iio_global_attributes;
	iio_cn0586_inst->debug_attributes = NULL;

	iio_cn0586_inst->submit = NULL;
	iio_cn0586_inst->pre_enable = NULL;
	iio_cn0586_inst->post_disable = NULL;
	iio_cn0586_inst->read_dev = NULL;
	iio_cn0586_inst->write_dev = NULL;
	iio_cn0586_inst->debug_reg_read = NULL;
	iio_cn0586_inst->debug_reg_write = NULL;
	iio_cn0586_inst->trigger_handler = NULL;

	*desc = iio_cn0586_inst;

	return 0;
}

/**
 * @brief	Initialization of AD5754R IIO hardware trigger specific parameters
 * @param 	desc[in,out] - IIO hardware trigger descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad5754r_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	int32_t ret;
	struct iio_hw_trig_init_param ad5754r_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;

	if (!desc) {
		return -EINVAL;
	}

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad5754r_hw_trig_init_params.irq_id = TRIGGER_INT_ID;
	ad5754r_hw_trig_init_params.name = AD5754R_IIO_TRIGGER_NAME;
	ad5754r_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_RISING;
	ad5754r_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad5754r_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad5754r_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad5754r_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad5754r_hw_trig_init_params.iio_desc = ad5754r_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad5754r_hw_trig_init_params);
	if (ret) {
		no_os_free(hw_trig_desc);
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/**
 * @brief	Initialize the IIO interface for AD5754R IIO device.
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t ad5754r_iio_init(void)
{
	int32_t ret;

	/* IIO device descriptor */
	struct iio_device *ad5754r_iio_dev;
#if defined(DEV_CN0586)
	struct iio_device *cn0586_iio_dev = NULL;
#endif

	/* IIO trigger init parameters */
	static struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad5754r_iio_trig_desc,
		.name = AD5754R_IIO_TRIGGER_NAME,
	};

	/* IIO interface init parameters */
	static struct iio_init_param iio_init_params = {
		.phy_type = USE_UART,
		.trigs = &iio_trigger_init_params,
		.nb_trigs = 1
	};

	/* IIOD init parameters */
	static struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES];

	/* Initialize the system peripherals */
	ret = init_system();
	if (ret) {
		return ret;
	}

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

	/* Ignore EEPROM check result for EVAL-AD5754-REBZ as there are no I2C pins
	 * accessible on the Evaluation Board*/
#if defined(DEV_CN0586)
	if (hw_mezzanine_is_valid) {
#endif
	/* Initialize AD5754R no-os device driver interface */
	ret = ad5754r_init(&ad5754r_dev_inst, &ad5754r_init_params);
	if (ret) {
		return ret;
	}

	/* Check if descriptors have been assigned for GPIOs */
	if (ad5754r_init_params.gpio_clear_init) {
		if (!ad5754r_dev_inst->gpio_clear) {
			return -ENOSYS;
		}
	}

	if (ad5754r_init_params.gpio_ldac_init) {
		if (!ad5754r_dev_inst->gpio_ldac) {
			return -ENOSYS;
		}
	}

	/* Initialize the AD5754R IIO app specific parameters */
	ret = ad5754r_iio_param_init(&ad5754r_iio_dev);
	if (ret) {
		return ret;
	}
	iio_init_params.nb_devs++;

#ifdef DEV_CN0586
	if (ad5754r_dev_inst) {
		ret = cn0586_init(&cn0586_dev_inst, ad5754r_dev_inst);
		if (ret) {
			return ret;
		}

		ret = cn0586_iio_param_init(&cn0586_iio_dev);
		if (ret) {
			return ret;
		}
		iio_init_params.nb_devs++;
	}
#endif

	/* AD5754R IIO device init parameters */
	iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
	iio_device_init_params[0].raw_buf = dac_data_buffer;
	iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;
	iio_device_init_params[0].dev = ad5754r_dev_inst;
	iio_device_init_params[0].dev_descriptor = ad5754r_iio_dev;
	iio_device_init_params[0].trigger_id = "trigger0";

#ifdef DEV_CN0586
	/* CFTL IIO device init parameters */
	iio_device_init_params[1].name = "cn0586";
	iio_device_init_params[1].raw_buf = dac_data_buffer;
	iio_device_init_params[1].raw_buf_len = DATA_BUFFER_SIZE;
	iio_device_init_params[1].dev = cn0586_dev_inst;
	iio_device_init_params[1].dev_descriptor = cn0586_iio_dev;
	iio_device_init_params[1].trigger_id = NULL; // TODO
#endif

#if defined(DEV_CN0586)
	}
#endif

	/* Initialize the IIO interface */
	iio_init_params.devs = iio_device_init_params;
	iio_init_params.uart_desc = uart_iio_com_desc;
	ret = iio_init(&ad5754r_iio_desc, &iio_init_params);
	if (ret) {
		return ret;
	}

	ret = ad5754r_iio_trigger_param_init(&ad5754r_hw_trig_desc);
	if (ret) {
		return ret;
	}

	ret = init_pwm();
	if (ret) {
		return ret;
	}

	/* Reconfigure the LDAC pin as GPIO output (non-PWM) */
	ret = ad5754r_reconfig_ldac(ad5754r_dev_inst);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief 	Run the AD5754r IIO event handler.
 * @return	none.
 * @details	This function monitors the new IIO client event.
 */
void ad5754r_iio_event_handler(void)
{
	iio_step(ad5754r_iio_desc);
}