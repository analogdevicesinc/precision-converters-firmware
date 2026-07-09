/***************************************************************************//**
 *   @file    ad559xr_iio.c
 *   @brief   Implementation of ad559xr IIO application interfaces
 *   @details This module acts as an interface for ad559xr IIO application
********************************************************************************
 * Copyright (c) 2026 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "ad559xr_user_config.h"
#include "ad559xr_iio.h"
#include "no_os_error.h"
#include "no_os_delay.h"
#include "no_os_util.h"
#include "app_config.h"
#include "iio_trigger.h"
#include "version.h"

/******** Forward declaration of getter/setter functions ********/
static int ad559xr_iio_attr_get(void* device,
				char* buf,
				uint32_t len,
				const struct iio_ch_info* channel,
				intptr_t priv);
static int ad559xr_iio_attr_set(void* device,
				char* buf,
				uint32_t len,
				const struct iio_ch_info* channel,
				intptr_t priv);

static int ad559xr_iio_attr_available_get(void* device,
		char* buf,
		uint32_t len,
		const struct iio_ch_info* channel,
		intptr_t priv);
static int ad559xr_iio_attr_available_set(void* device,
		char* buf,
		uint32_t len,
		const struct iio_ch_info* channel,
		intptr_t priv);

static int ad559xr_iio_gpio_attr_get(void* device,
				     char* buf,
				     uint32_t len,
				     const struct iio_ch_info* channel,
				     intptr_t id);
static int ad559xr_iio_gpio_attr_set(void* device,
				     char* buf,
				     uint32_t len,
				     const struct iio_ch_info* channel,
				     intptr_t priv);

int ad559xr_read_die_temp(float *result);

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/* IIO trigger name */
#define AD559XR_IIO_TRIGGER_NAME		"ad559xr_iio_trigger"

/* Number of IIO devices */
#define NUM_OF_IIO_DEVICES	         3

/* Number of Channels */
#define NO_OF_CHANNELS 8

/* Resolution of GPIO Subsystem */
#define GPIO_BIT 1

/* Sampling frequency for timeout purpose */
#define SAMPLING_FREQUENCY 1

/* ADC data buffer size */
#if defined(USE_SDRAM)
#define adc_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(32768)		// 32kbytes
static int8_t adc_data_buffer[DATA_BUFFER_SIZE];
#endif

/* IIO Channel attribute definition */
#define AD559XR_CHN_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = ad559xr_iio_attr_get,\
		.store = ad559xr_iio_attr_set\
}

/* IIO GPIO Channel attribute definition */
#define AD559XR_GPIO_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = ad559xr_iio_gpio_attr_get,\
		.store = ad559xr_iio_gpio_attr_set\
}

/* ad559xr Channel Scan structure */
#define AD559XR_DEFAULT_CHN_SCAN {\
	.sign = 'u',\
	.realbits = ADC_RESOLUTION,\
	.storagebits = STORAGE_BITS,\
	.shift = 0,\
	.is_big_endian = false\
}

/* ad559xr Channel Scan structure */
#define AD559XR_GPIO_CHN_SCAN {\
	.sign = 'u',\
	.realbits = GPIO_BIT,\
	.storagebits = GPIO_BIT,\
	.shift = 0,\
	.is_big_endian = false\
}

/* IIO Channel available attribute definition */
#define AD559XR_CHN_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = ad559xr_iio_attr_available_get,\
	.store = ad559xr_iio_attr_available_set\
}

/* IIOD channel scan configurations */
struct scan_type ad559xr_iio_scan_type[NUM_OF_IIO_DEVICES][NO_OF_CHANNELS] = {
	{ AD559XR_DEFAULT_CHN_SCAN }, { AD559XR_DEFAULT_CHN_SCAN }, {AD559XR_GPIO_CHN_SCAN}

};

/* AD559xr Channel*/
#define AD559XR_CH(_name, _dev, _idx, _type) {\
	.name = _name, \
	.ch_type = _type,\
	.ch_out = 0,\
	.indexed = true,\
	.channel = _idx,\
	.scan_index = _idx,\
	.scan_type = ad559xr_iio_scan_type[_dev],\
	.attributes = ad559xr_iio_ch_attributes[_dev]\
}

/* Address Mask */
#define AD5592R_REG_ADC_SEQ_ADDR_MSK(x)		    ((x >> 12) & 0x0F)

/* Macros needed for temperature calculation */
#define MAX_ADC_CODE				    4095.0
#define ADC_GAIN_LOW_CONVERSION_VALUE	2.654
#define ADC_GAIN_HIGH_CONVERSION_VALUE	1.327
#define TEMP_SAMPLE_SIZE			    5

/* Macros needed for number of characters */
#define GPIO_CHANNEL_NAME_SIZE 6
#define CHANNEL_NAME_SIZE 9

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* IIO interface descriptor */
static struct iio_desc* p_ad559xr_iio_desc = NULL;

/* ad559xr IIO device descriptor */
struct iio_device* p_iio_ad559xr_dev[NUM_OF_IIO_DEVICES];

/* ad559xr global device instance for accessing device specific APIs */
struct ad5592r_dev* ad559xr_dev_inst = NULL;

/* AD559xr IIO hw trigger descriptor */
static struct iio_hw_trig* ad559xr_hw_trig_desc;

/* ad559xr attribute unique IDs */
enum ad559xr_attribute_ids {
	IIO_RAW,
	IIO_SCALE,
	IIO_OFFSET,
	POWER_DOWN,
	CH_MODE,
	CH_OFFSTATE,
	NUM_OF_CHN_ATTR,

	ADC_SAMPLING_FREQUENCY,
	ADC_RANGE,
	DAC_RANGE,
	REF_SELECT,
	VREF_IN_V,
	TEMP,
	ADC_BUFFER,
	LDAC_MODE,
	REPETITION,
	DEVICE_NAME,
	RESTART_IIO_ATTR_ID,
	NUM_OF_DEV_ATTR = RESTART_IIO_ATTR_ID - NUM_OF_CHN_ATTR
};

/* Device type */
enum device_name {
	AD5592R,
	AD5593R,
};

/* ad559xr device channel attributes list */
static struct iio_attribute
	ad559xr_iio_ch_attributes[NUM_OF_IIO_DEVICES][NUM_OF_CHN_ATTR + 2] = {
	{
		AD559XR_CHN_ATTR("mode", CH_MODE),
		AD559XR_CHN_AVAIL_ATTR("mode_available", CH_MODE),
		AD559XR_CHN_ATTR("off_state", CH_OFFSTATE),
		AD559XR_CHN_AVAIL_ATTR("off_state_available", CH_OFFSTATE),

		END_ATTRIBUTES_ARRAY
	},
	/* DAC Channel attributes */
	{
		AD559XR_CHN_ATTR("raw", IIO_RAW),
		AD559XR_CHN_ATTR("scale", IIO_SCALE),
		AD559XR_CHN_ATTR("offset", IIO_OFFSET),
		AD559XR_CHN_ATTR("power_down", POWER_DOWN),
		AD559XR_CHN_AVAIL_ATTR("power_down_available", POWER_DOWN),

		END_ATTRIBUTES_ARRAY
	},
	{
		/* GPIO Subsystem */
		AD559XR_GPIO_ATTR("raw", IIO_RAW),

		END_ATTRIBUTES_ARRAY
	}
};

/* ad559xr adc device channel attributes list */
static struct iio_attribute
	ad559xr_iio_adc_ch_attributes[NUM_OF_IIO_DEVICES][NUM_OF_CHN_ATTR + 1] = {
	{
	},
	{
		AD559XR_CHN_ATTR("raw", IIO_RAW),
		AD559XR_CHN_ATTR("scale", IIO_SCALE),
		AD559XR_CHN_ATTR("offset", IIO_OFFSET),

		END_ATTRIBUTES_ARRAY
	}
};

/* IIOD device (global) attributes list */
static struct iio_attribute
	ad559xr_iio_global_attributes[NUM_OF_IIO_DEVICES][NUM_OF_DEV_ATTR + 9] = {
	{
		AD559XR_CHN_ATTR("device_generic", DEVICE_NAME),
		AD559XR_CHN_AVAIL_ATTR("device_generic_available", DEVICE_NAME),
		AD559XR_CHN_ATTR("adc_range", ADC_RANGE),
		AD559XR_CHN_AVAIL_ATTR("adc_range_available", ADC_RANGE),
		AD559XR_CHN_ATTR("dac_range", DAC_RANGE),
		AD559XR_CHN_AVAIL_ATTR("dac_range_available", DAC_RANGE),
		AD559XR_CHN_ATTR("ref_select", REF_SELECT),
		AD559XR_CHN_AVAIL_ATTR("ref_select_available", REF_SELECT),
		AD559XR_CHN_ATTR("v_ref_in_v", VREF_IN_V),
		AD559XR_CHN_ATTR("reconfigure_system", RESTART_IIO_ATTR_ID),
		AD559XR_CHN_AVAIL_ATTR("reconfigure_system_available", RESTART_IIO_ATTR_ID),

		END_ATTRIBUTES_ARRAY
	},
	{
		AD559XR_CHN_ATTR("sampling_frequency", ADC_SAMPLING_FREQUENCY),
		AD559XR_CHN_ATTR("die_temperature", TEMP),
		AD559XR_CHN_ATTR("adc_buffer", ADC_BUFFER),
		AD559XR_CHN_AVAIL_ATTR("adc_buffer_available", ADC_BUFFER),
		AD559XR_CHN_ATTR("ldac_mode", LDAC_MODE),
		AD559XR_CHN_AVAIL_ATTR("ldac_mode_available", LDAC_MODE),
		AD559XR_CHN_ATTR("set_repetition_bit", REPETITION),
		AD559XR_CHN_AVAIL_ATTR("set_repetition_bit_available", REPETITION),

		END_ATTRIBUTES_ARRAY
	},
	{
	}
};

static struct iio_channel
	ad559xr_iio_channels[NUM_OF_IIO_DEVICES][NO_OF_CHANNELS];

/* ad559xr IIO Channels init for IIO Device 0 */
static struct iio_channel
	ad559xr_iio_channels[NUM_OF_IIO_DEVICES][NO_OF_CHANNELS] = {
	{
		AD559XR_CH("voltage0", 0, 0, IIO_VOLTAGE),
		AD559XR_CH("voltage1", 0, 1, IIO_VOLTAGE),
		AD559XR_CH("voltage2", 0, 2, IIO_VOLTAGE),
		AD559XR_CH("voltage3", 0, 3, IIO_VOLTAGE),
		AD559XR_CH("voltage4", 0, 4, IIO_VOLTAGE),
		AD559XR_CH("voltage5", 0, 5, IIO_VOLTAGE),
		AD559XR_CH("voltage6", 0, 6, IIO_VOLTAGE),
		AD559XR_CH("voltage7", 0, 7, IIO_VOLTAGE)
	},
	{
	},
	{
	}
};

/* Global variable for scale attribute */
float attr_scale[NO_OF_CHANNELS];

/* Range modes available */
static char* ad559xr_range_modes[] = {
	"0_to_vref",
	"0_to_2vref",
};

/* Power modes available */
static char* ad559xr_power_down_modes[] = {
	"disabled",
	"enabled",
};

/* Reference Select */
static char* ad559xr_ref_sel[] = {
	"external",
	"internal"
};

/* Buffer options*/
static char* ad559xr_buf[] = {
	"disabled",
	"enabled"
};

/* LDAC Modes */
static char* ad559xr_ldac_modes[] = {
	"sync",
	"async",
	"trigger"
};

/* Repetition Modes */
static char* ad559xr_set_rep_mode[] = {
	"non-repeating",
	"repeating"
};

/* Channel Modes */
static char* ad559xr_ch_modes[] = {
	"gpio_unused",
	"adc",
	"dac",
	"adc_dac",
	"gpio_input",
	"gpio_output",
};

/* Channel Offstates */
static char* ad559xr_offstate[] = {
	"pull_down",
	"gpio_op_low",
	"gpio_op_high",
	"gpio_op_tristate"
};

/* Restart IIO option */
static const char* restart_iio_options[] = {
	"Enable",
};

/* Device Names */
static const char* device_name[] = {
	"ad5592r",
	"ad5593r",
};

/* Global variable for vref value*/
static float vref_value = DEFAULT_VREF;

/* Global Variable to hold repetition mode */
static int rep_mode = 1;

/* DAC Write Value*/
uint16_t dac_value;

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

/* Number of active channels requested by IIO Client */
uint8_t num_of_active_channels = 0;

/* Channel mask copy */
uint32_t channel_mask;

/* Number of valid ad559xr channels (ie non gpio) */
uint8_t ad559xr_channels = 0;

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/* Restart IIO flag */
static bool restart_iio_flag = false;

/* Device selection flag */
bool device_select = false;

/* GPIO option enabled */
bool gpio_enabled = false;

/* GPIO option enabled */
bool gpio_unused_enabled = false;

/* Cached GPIO Value */
bool gpio_cached_val[NO_OF_CHANNELS];

/* Active Device */
uint8_t active_device = AD5592R;

/* Offset needed for extra overhead based on active channels enabled */
uint16_t addl_offset[NO_OF_CHANNELS] = { 0, 3000, 3000, 5500, 8500, 10500, 11500, 13500 };

/* An array holding non gpio channel indices */
uint8_t non_gpio_channels[NO_OF_CHANNELS];

/* Number of non gpio channels variable */
uint8_t num_non_gpio_channels = 0;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief	Function to get scale value
 * @param	chn[in] - Channel number
 * @return  none
 */
void ad559xr_get_scale(uint8_t chn)
{
	if ((ad559xr_dev_inst->channel_modes[chn] == CH_MODE_ADC)
	    || (ad559xr_dev_inst->channel_modes[chn] == CH_MODE_DAC_AND_ADC)) {
		attr_scale[chn] = (1 << (ad559xr_dev_inst->adc_range)) * vref_value * 1000 /
				  ADC_MAX_COUNT;
	}

	if (ad559xr_dev_inst->channel_modes[chn] == CH_MODE_DAC) {
		attr_scale[chn] = (1 << (ad559xr_dev_inst->dac_range)) * vref_value * 1000 /
				  ADC_MAX_COUNT;
	}
}

/**
 * @brief	Function to set rep bit in adc sequencer
 * @param	rep_status[in] - Status of repeat bit
 * @return  none
 */
int ad559xr_set_rep_bit(bool rep_status)
{
	int ret;
	uint16_t temp_reg_val;

	ret = ad5592r_base_reg_read(ad559xr_dev_inst, AD5592R_REG_ADC_SEQ,
				    &temp_reg_val);
	if (ret) {
		return ret;
	}

	if (rep_status) {
		temp_reg_val |= AD5592R_REG_ADC_SEQ_REP;
	} else {
		temp_reg_val &= ~AD5592R_REG_ADC_SEQ_REP;
	}

	ret = ad5592r_base_reg_write(ad559xr_dev_inst, AD5592R_REG_ADC_SEQ,
				     temp_reg_val);
	if (ret) {
		return ret;
	}

	rep_mode = rep_status;

	return 0;
}

/*!
 * @brief	Getter/Setter for attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written
 */
static int ad559xr_iio_attr_get(void* device,
				char* buf,
				uint32_t len,
				const struct iio_ch_info* channel,
				intptr_t priv)
{
	int ret;
	static uint16_t adc_raw_data = 0;
	float temp = 0;
	uint8_t actual_ch_num = 0;

	if (gpio_enabled || gpio_unused_enabled) {
		/* Map the iio channel number to the actual channel number */
		if (channel->ch_num < ad559xr_channels) {
			actual_ch_num = non_gpio_channels[channel->ch_num];
		}
	} else {
		actual_ch_num = channel->ch_num;
	}

	switch (priv) {
	case IIO_RAW:
		switch (ad559xr_dev_inst->channel_modes[actual_ch_num]) {
		case CH_MODE_DAC:
			adc_raw_data = ad559xr_dev_inst->cached_dac[actual_ch_num];
			break;

		case CH_MODE_ADC:
		case CH_MODE_DAC_AND_ADC:
			ret = ad559xr_dev_inst->ops->read_adc(ad559xr_dev_inst, actual_ch_num,
							      &adc_raw_data);
			if (ret) {
				return ret;
			}
			adc_raw_data = AD5592R_REG_ADC_SEQ_CODE_MSK(adc_raw_data);

		}

		return sprintf(buf, "%d", adc_raw_data);

	case IIO_OFFSET:
		return sprintf(buf, "%d", 0);

	case DAC_RANGE:
		return sprintf(buf, "%s", ad559xr_range_modes[ad5592r_init_params.dac_range]);

	case ADC_RANGE:
		return sprintf(buf, "%s", ad559xr_range_modes[ad5592r_init_params.adc_range]);

	case POWER_DOWN:
		return sprintf(buf, "%s",
			       ad559xr_power_down_modes[ad559xr_dev_inst->power_down[channel->ch_num]]);

	case REF_SELECT:
		return sprintf(buf, "%s", ad559xr_ref_sel[ad5592r_init_params.int_ref]);

	case VREF_IN_V:
		return sprintf(buf, "%fV", vref_value);

	case DEVICE_NAME:
		return sprintf(buf, "%s", device_name[active_device]);

	case ADC_BUFFER:
		return sprintf(buf, "%s", ad559xr_buf[ad559xr_dev_inst->adc_buf]);

	case TEMP:
		ret = ad559xr_read_die_temp(&temp);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%f", temp);

	case IIO_SCALE:
		ad559xr_get_scale(actual_ch_num);

		return sprintf(buf, "%.8f", attr_scale[actual_ch_num]);

	case LDAC_MODE:
		return sprintf(buf, "%s", ad559xr_ldac_modes[ad559xr_dev_inst->ldac_mode]);

	case REPETITION:
		return sprintf(buf, "%s", ad559xr_set_rep_mode[rep_mode]);

	case CH_MODE:
		return sprintf(buf, "%s",
			       ad559xr_ch_modes[ad5592r_init_params.channel_modes[channel->ch_num]]);

	case CH_OFFSTATE:
		return sprintf(buf, "%s",
			       ad559xr_offstate[ad5592r_init_params.channel_offstate[channel->ch_num]]);

	case ADC_SAMPLING_FREQUENCY:
		return sprintf(buf, "%d", SAMPLING_FREQUENCY);

	case RESTART_IIO_ATTR_ID:
		return sprintf(buf, "%s", restart_iio_options[0]);

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Setter function for ad559xr attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of expected bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int ad559xr_iio_attr_set(void* device,
				char* buf,
				uint32_t len,
				const struct iio_ch_info* channel,
				intptr_t priv)
{
	int ret;
	uint8_t actual_ch_num = 0;
	uint8_t id;

	if (gpio_enabled || gpio_unused_enabled) {
		/* Map the iio channel number to the actual channel number */
		if (channel->ch_num < ad559xr_channels) {
			actual_ch_num = non_gpio_channels[channel->ch_num];
		}
	} else {
		actual_ch_num = channel->ch_num;
	}


	switch (priv) {
	case IIO_RAW:
		if (ad559xr_dev_inst->channel_modes[actual_ch_num] == CH_MODE_DAC) {
			dac_value = no_os_str_to_uint32(buf);
			if (dac_value > ADC_MAX_COUNT) {
				dac_value = ADC_MAX_COUNT;
			}

			/* DAC Write */
			ret = ad559xr_dev_inst->ops->write_dac(ad559xr_dev_inst, actual_ch_num,
							       dac_value);
			if (ret) {
				return ret;
			}

		}
		break;

	case IIO_SCALE:
	case IIO_OFFSET:
		break;

	case DAC_RANGE:
		for (id = ZERO_TO_VREF; id <= ZERO_TO_2VREF; id++) {
			if (!strcmp(buf, ad559xr_range_modes[id])) {
				break;
			}
		}

		ad5592r_init_params.dac_range = id;

		break;

	case ADC_RANGE:
		for (id = ZERO_TO_VREF; id <= ZERO_TO_2VREF; id++) {
			if (!strcmp(buf, ad559xr_range_modes[id])) {
				break;
			}
		}

		ad5592r_init_params.adc_range = id;

		break;

	case DEVICE_NAME:
		for (id = 0; id < NO_OS_ARRAY_SIZE(device_name); id++) {
			if (!strcmp(buf, device_name[id])) {
				break;
			}
		}

		active_device = id;

		break;

	case POWER_DOWN:
		for (id = 0; id < NO_OS_ARRAY_SIZE(ad559xr_power_down_modes); id++) {
			if (!strcmp(buf, ad559xr_power_down_modes[id])) {
				break;
			}
		}

		ret = ad5592r_power_down(ad559xr_dev_inst, actual_ch_num, id);
		if (ret) {
			return ret;
		}

		break;

	case REF_SELECT:
		for (id = 0; id < NO_OS_ARRAY_SIZE(ad559xr_ref_sel); id++) {
			if (!strcmp(buf, ad559xr_ref_sel[id])) {
				break;
			}
		}

		ad5592r_init_params.int_ref = id;

		break;

	case ADC_BUFFER:
		for (id = 0; id < NO_OS_ARRAY_SIZE(ad559xr_buf); id++) {
			if (!strcmp(buf, ad559xr_buf[id])) {
				break;
			}
		}

		ret = ad5592r_set_adc_buffer(ad559xr_dev_inst, id);
		if (ret) {
			return ret;
		}

		break;

	case VREF_IN_V:
		vref_value = strtof(buf, NULL);
		if (ad559xr_dev_inst->int_ref) {
			vref_value = DEFAULT_VREF;
		}

		break;

	case LDAC_MODE:
		for (id = 0; id < NO_OS_ARRAY_SIZE(ad559xr_ldac_modes); id++) {
			if (!strcmp(buf, ad559xr_ldac_modes[id])) {
				break;
			}
		}

		/* Write to LDAC Register */
		ret = ad5592r_base_reg_write(ad559xr_dev_inst, AD5592R_REG_LDAC, id);
		if (ret) {
			return ret;
		}

		ad559xr_dev_inst->ldac_mode = id;

		break;

	case REPETITION:
		for (id = 0; id < NO_OS_ARRAY_SIZE(ad559xr_set_rep_mode); id++) {
			if (!strcmp(buf, ad559xr_set_rep_mode[id])) {
				break;
			}
		}

		/* Set Repetition Bit in ADC Sequencer */
		ret = ad559xr_set_rep_bit(id);
		if (ret) {
			return ret;
		}

		break;

	case CH_MODE:
		for (id = 0; id < NO_OS_ARRAY_SIZE(ad559xr_ch_modes); id++) {
			if (!strcmp(buf, ad559xr_ch_modes[id])) {
				break;
			}
		}

		ad5592r_init_params.channel_modes[channel->ch_num] = id;

		break;

	case CH_OFFSTATE:
		for (id = 0; id < NO_OS_ARRAY_SIZE(ad559xr_offstate); id++) {
			if (!strcmp(buf, ad559xr_offstate[id])) {
				break;
			}
		}

		ad5592r_init_params.channel_offstate[channel->ch_num] = id;

		break;

	case RESTART_IIO_ATTR_ID:
		device_select = true;
		/* Set flag to true */
		restart_iio_flag = true;

		break;
	}

	return len;
}

/*!
 * @brief	Getter/Setter for GPIO attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written
 */
static int ad559xr_iio_gpio_attr_get(void* device,
				     char* buf,
				     uint32_t len,
				     const struct iio_ch_info* channel,
				     intptr_t priv)
{
	uint16_t gpio_val = 0;

	switch (priv) {
	case IIO_RAW:
		if (ad559xr_dev_inst->channel_modes[channel->ch_num] == CH_MODE_GPI) {
			ad559xr_dev_inst->gpio_out = 0;
			gpio_val = ad5592r_gpio_get(ad559xr_dev_inst, channel->ch_num);
		} else {
			gpio_val = gpio_cached_val[channel->ch_num];
		}
	}

	return sprintf(buf, "%d", gpio_val);
}

/*!
 * @brief	Setter function for ad559xr GPIO attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of expected bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int ad559xr_iio_gpio_attr_set(void* device,
				     char* buf,
				     uint32_t len,
				     const struct iio_ch_info* channel,
				     intptr_t priv)
{
	int ret;

	switch (priv) {
	case IIO_RAW:
		if (ad559xr_dev_inst->channel_modes[channel->ch_num] == CH_MODE_GPO) {
			dac_value = no_os_str_to_uint32(buf);
			if (dac_value > 1) {
				dac_value = 1;
			}

			gpio_cached_val[channel->ch_num] = dac_value;

			ret = ad5592r_gpio_set(ad559xr_dev_inst, channel->ch_num,
					       gpio_cached_val[channel->ch_num]);
			if (ret) {
				return ret;
			}
		}
	}

	return len;
}

/**
 * @brief Attribute available getter function for AD559xr attributes
 * @param device[in, out]- Pointer to IIO device instance
 * @param buf[in]- IIO input data buffer
 * @param len[in]- Number of input bytes
 * @param channel[in] - input channel
 * @param priv[in] - Attribute private ID
 * @return len in case of success, negative error code otherwise
 */
static int ad559xr_iio_attr_available_get(void* device,
		char* buf,
		uint32_t len,
		const struct iio_ch_info* channel,
		intptr_t priv)
{
	uint8_t val;
	buf[0] = '\0';

	switch (priv) {
	case ADC_RANGE:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad559xr_range_modes); val++) {
			strcat(buf, ad559xr_range_modes[val]);
			strcat(buf, " ");
		}

		break;

	case DAC_RANGE:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad559xr_range_modes); val++) {
			strcat(buf, ad559xr_range_modes[val]);
			strcat(buf, " ");
		}

		break;

	case POWER_DOWN:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad559xr_power_down_modes); val++) {
			strcat(buf, ad559xr_power_down_modes[val]);
			strcat(buf, " ");
		}

		break;

	case REF_SELECT:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad559xr_ref_sel); val++) {
			strcat(buf, ad559xr_ref_sel[val]);
			strcat(buf, " ");
		}

		break;

	case ADC_BUFFER:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad559xr_buf); val++) {
			strcat(buf, ad559xr_buf[val]);
			strcat(buf, " ");
		}

		break;

	case LDAC_MODE:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad559xr_ldac_modes); val++) {
			strcat(buf, ad559xr_ldac_modes[val]);
			strcat(buf, " ");
		}

		break;

	case REPETITION:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad559xr_set_rep_mode); val++) {
			strcat(buf, ad559xr_set_rep_mode[val]);
			strcat(buf, " ");
		}

		break;

	case CH_MODE:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad559xr_ch_modes); val++) {
			strcat(buf, ad559xr_ch_modes[val]);
			strcat(buf, " ");
		}

		break;

	case CH_OFFSTATE:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad559xr_offstate); val++) {
			strcat(buf, ad559xr_offstate[val]);
			strcat(buf, " ");
		}

		break;

	case RESTART_IIO_ATTR_ID:
		return sprintf(buf, "%s", restart_iio_options[0]);

	case DEVICE_NAME:
		for (val = 0; val < NO_OS_ARRAY_SIZE(device_name); val++) {
			strcat(buf, device_name[val]);
			strcat(buf, " ");
		}

		break;
	}

	/* Remove extra trailing space at the end of the buffer string */
	len = strlen(buf);
	buf[len - 1] = '\0';

	return len;
}

/*!
 * @brief Attribute available setter function for AD559xr attributes
 * @param device[in, out]- Pointer to IIO device instance
 * @param buf[in]- IIO input data buffer
 * @param len[in]- Number of input bytes
 * @param channel[in] - input channel
 * @param priv[in] - Attribute private ID
 * @return len in case of success, negative error code otherwise
 */
static int ad559xr_iio_attr_available_set(void* device,
		char* buf,
		uint32_t len,
		const struct iio_ch_info* channel,
		intptr_t priv)
{
	return len;
}

/**
 * @brief	Read the converted data for channels enabled
 * @param	dev[in] - IIO device instance
 * @param   values[in,out] - ADC raw values
 * @return	0 in case of success, negative error code otherwise
 */
int ad5592r_read_converted_data(struct ad5592r_dev* dev, uint16_t* values)
{
	int ret;
	uint8_t i;

	for (i = 0; i < num_of_active_channels; i++) {
		ret = ad5592r_spi_wnop_r16(dev, &dev->spi_msg);
		if (ret) {
			return ret;
		}
		values[i] = dev->spi_msg;
	}

	return 0;
}

/**
 * @brief	Read the converted data for channels enabled
 * @param	dev[in] - IIO device instance
 * @param   values[in,out] - ADC raw values
 * @return	0 in case of success, negative error code otherwise
 */
int ad5593r_read_converted_data(struct ad5592r_dev* dev, uint16_t* values)
{
	uint8_t data[AD5593R_ADC_VALUES_BUFF_SIZE];
	int i;
	int ret;

	data[0] = AD5593R_MODE_ADC_READBACK;
	ret = no_os_i2c_write(dev->i2c, data, 1, AD5593R_STOP_BIT);
	if (ret) {
		return ret;
	}

	ret = no_os_i2c_read(dev->i2c, data, (2 * num_of_active_channels),
			     AD5593R_STOP_BIT);
	if (ret) {
		return ret;
	}

	for (i = 0; i < num_of_active_channels; i++) {
		values[i] = ((uint16_t)(((data[2 * i] & 0xFF) << 8) + data[(2 * i) + 1]));
		values[i] = AD5592R_REG_ADC_SEQ_CODE_MSK(values[i]);
	}

	return 0;
}

/**
 * @brief	Calculates the die temperature
 * @details	Based on conversion equation, die temperature is estimated
 * @param	adc_temp_code[in] - data read from ADC readback frame
 * @param   adc_gain[in] - status of adc_gain
 * @return	result
 */
static float die_temp_calculation(uint16_t adc_temp_code, bool adc_gain)
{
	float result = 0;

	/* Use different equation depending on gain */
	if (adc_gain) {
		result = 25 + ((AD5592R_REG_ADC_SEQ_CODE_MSK(adc_temp_code) -
				((0.5 / (2 * (vref_value))) * MAX_ADC_CODE)) /
			       (ADC_GAIN_HIGH_CONVERSION_VALUE * (2.5 / vref_value)));
	} else {
		result = 25 + ((AD5592R_REG_ADC_SEQ_CODE_MSK(adc_temp_code) -
				((0.5 / (vref_value)) * MAX_ADC_CODE)) /
			       (ADC_GAIN_LOW_CONVERSION_VALUE * (2.5 / (vref_value))));
	}

	return result;
}

/**
 * @brief	Reads the temperature of the die
 * @details	Sets the devices to perform a temperature readback.
 *			Performs a number of samples based on TEMP_SAMPLE_SIZE
 * @param   result[out] - Result of the ADC Temp Channel
 * @return	0 in case of success, negative error code otherwise
 */
int ad559xr_read_die_temp(float *result)
{
	uint16_t readback_reg[2] = { 0 };
	int ret;
	uint8_t i;
	uint8_t j;
	int8_t adc_channel = -1;

	/* Find the user-configured ADC channel */
	for (i = 0; i < NO_OF_CHANNELS; i++) {
		if (ad559xr_dev_inst->channel_modes[i] == CH_MODE_ADC) {
			adc_channel = i;
			break;
		}
	}

	/* If no ADC channel is found, return an error */
	if (adc_channel == -1) {
		return -EINVAL;
	}

	/* Enable the temperature channel and the user-configured ADC channel */
	uint16_t ch_mask = NO_OS_BIT(adc_channel) | AD5592R_REG_ADC_SEQ_TEMP_READBACK;

	for (i = 0; i < TEMP_SAMPLE_SIZE; i++) {
		if (active_device == AD5592R) {
			ad559xr_dev_inst->spi_msg = swab16((uint16_t)(AD5592R_REG_ADC_SEQ << 11) |
							   ch_mask);

			ret = no_os_spi_write_and_read(ad559xr_dev_inst->spi,
						       (uint8_t *)&ad559xr_dev_inst->spi_msg,
						       sizeof(ad559xr_dev_inst->spi_msg));
			if (ret) {
				return ret;
			}

			/* Delays are added to compensate for the additional track time needed in case of temp channel */
			no_os_udelay(9);

			/* Read data from the ADC */
			ret = ad5592r_spi_wnop_r16(ad559xr_dev_inst, &ad559xr_dev_inst->spi_msg);
			if (ret) {
				return ret;
			}

			/* Read the temperature and ADC channel data */
			for (j = 0; j < 2; j++) {
				if (j == 1) {
					no_os_udelay(9);
				}
				ret = ad5592r_spi_wnop_r16(ad559xr_dev_inst, &ad559xr_dev_inst->spi_msg);
				if (ret) {
					return ret;
				}
				readback_reg[j] = ad559xr_dev_inst->spi_msg;
			}

			/* Check if the non-temperature channel repeats by comparing address bits */
			if (AD5592R_REG_ADC_SEQ_ADDR_MSK(readback_reg[0]) ==
			    AD5592R_REG_ADC_SEQ_ADDR_MSK(readback_reg[1])) {
				no_os_udelay(9);

				ret = ad5592r_spi_wnop_r16(ad559xr_dev_inst, &ad559xr_dev_inst->spi_msg);
				if (ret) {
					return ret;
				}

			}
			readback_reg[1] = AD5592R_REG_ADC_SEQ_CODE_MSK(ad559xr_dev_inst->spi_msg);

		} else {
			uint8_t data[18] = {0};

			data[0] = AD5592R_REG_ADC_SEQ;
			data[1] = ch_mask >> 8;
			data[2] = ch_mask & 0xFF;

			ret = no_os_i2c_write(ad559xr_dev_inst->i2c, data, 3, 1);
			if (ret) {
				return ret;
			}

			num_of_active_channels = 2;

			ret = ad5593r_read_converted_data(ad559xr_dev_inst, readback_reg);
			if (ret)
				return ret;
		}

		/* Calculate the temperature */
		*result += die_temp_calculation(readback_reg[1], ad559xr_dev_inst->adc_range);
	}

	*result /= TEMP_SAMPLE_SIZE;

	return 0;
}

/**
 * @brief Start the ADC data capture
 * @return 0 in case of success, negative error code otherwise
 */
int ad559xr_start_data_capture(void)
{
	int ret;
	uint8_t data[18] = { 0 };

	if (active_device == AD5592R) {
		/* Enable the selected channels */
		if (rep_mode) {
			ad559xr_dev_inst->spi_msg = swab16((uint16_t)(AD5592R_REG_ADC_SEQ << 11) |
							   channel_mask | AD5592R_REG_ADC_SEQ_REP);
		} else {
			ad559xr_dev_inst->spi_msg = swab16((uint16_t)(AD5592R_REG_ADC_SEQ << 11) |
							   channel_mask);
		}

		ret = no_os_spi_write_and_read(ad559xr_dev_inst->spi,
					       (uint8_t*)&ad559xr_dev_inst->spi_msg,
					       sizeof(ad559xr_dev_inst->spi_msg));
		if (ret) {
			return ret;
		}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		uint32_t period_ns;

		ret = ad5592r_spi_wnop_r16(ad559xr_dev_inst, &ad559xr_dev_inst->spi_msg);
		if (ret) {
			return ret;
		}
		if (num_of_active_channels > 1)
			/* Set PWM period based on number of channels enabled */
			period_ns = PERIOD_IN_NS + (PERIOD_FOR_ONE_READ) * num_of_active_channels +
				    addl_offset[num_of_active_channels];
		else {
			period_ns = PERIOD_IN_NS + addl_offset[num_of_active_channels];
		}

		ret = iio_trig_enable(ad559xr_hw_trig_desc);
		if (ret) {
			return ret;
		}

		ret = no_os_pwm_set_period(pwm_desc,
					   period_ns);
		if (ret) {
			return ret;
		}

		ret = no_os_pwm_enable(pwm_desc);
		if (ret) {
			return ret;
		}
#endif
	} else {
		/* Select channels in the sequencer */
		if (rep_mode) {
			channel_mask |= AD5592R_REG_ADC_SEQ_REP;
		}

		data[0] = AD5592R_REG_ADC_SEQ;
		data[1] = channel_mask >> 8;
		data[2] = channel_mask & 0xFF;

		ret = no_os_i2c_write(ad559xr_dev_inst->i2c, data, 3, 1);
		if (ret) {
			return ret;
		}
	}
	return 0;
}

/**
 * @brief End the ADC data capture
 * @return 0 in case of success, negative error code otherwise
 */
int ad559xr_end_data_capture(void)
{
	int ret;
	uint8_t data[18] = {0};

	if (active_device == AD5592R) {
		/* Disable the selected channels */
		if (rep_mode) {
			ad559xr_dev_inst->spi_msg = swab16((uint16_t)(AD5592R_REG_ADC_SEQ << 11) |
							   AD5592R_REG_ADC_SEQ_REP);
		} else {
			ad559xr_dev_inst->spi_msg = swab16((uint16_t)(AD5592R_REG_ADC_SEQ << 11));
		}

		ret = no_os_spi_write_and_read(ad559xr_dev_inst->spi,
					       (uint8_t*)&ad559xr_dev_inst->spi_msg,
					       sizeof(ad559xr_dev_inst->spi_msg));
		if (ret) {
			return ret;
		}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		ret = iio_trig_disable(ad559xr_hw_trig_desc);
		if (ret) {
			return ret;
		}

		ret = no_os_pwm_disable(pwm_desc);
		if (ret) {
			return ret;
		}
#endif
	} else {
		if (rep_mode) {
			data[0] = AD5592R_REG_ADC_SEQ;

			data[1] = AD5592R_REG_ADC_SEQ_REP >> 8;
		} else {
			data[0] = 0;
			data[1] = 0;
		}
		data[2] = 0;

		ret = no_os_i2c_write(ad559xr_dev_inst->i2c, data, 3, 1);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Prepare for ADC data capture (transfer from device to memory)
 * @param dev_instance[in] - IIO device instance
 * @param ch_mask[in] - Channels select mask
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad559xr_iio_prepare_transfer(void* dev_instance,
		uint32_t ch_mask)
{
	int ret;
	uint32_t new_ch_mask = 0;
	buf_size_updated = false;
	channel_mask = 0;
	uint8_t bit_index = 0;
	uint8_t i;

	/* Map ch_mask bits to non-GPIO channels */
	for (i = 0; i < num_non_gpio_channels; i++) {
		if (ch_mask & NO_OS_BIT(bit_index)) {
			new_ch_mask |= NO_OS_BIT(non_gpio_channels[i]);
		}
		bit_index++;
	}

	channel_mask = new_ch_mask;

	num_of_active_channels = no_os_hweight16(ch_mask);

	ret = ad559xr_start_data_capture();
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief	Perform tasks before end of current data transfer
 * @param	dev[in] - IIO device instance
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad559xr_iio_end_transfer(void* dev)
{
	int ret;

	ret = ad559xr_end_data_capture();
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Read requested number of ADC samples into IIO buffer for AD5592R
 * @param iio_dev_data[in] - Pointer to IIO device data structure
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad5592r_iio_submit_buffer(struct iio_device_data* iio_dev_data)
{
	int32_t ret;
	uint32_t sample_index = 0;
	uint32_t nb_of_samples;
	uint16_t adc_raw_data[NO_OF_CHANNELS] = { 0 };
	bool channels_received[NO_OF_CHANNELS] = { false };
	int8_t valid_samples = 0;
	uint16_t adc_raw_data_copy[NO_OF_CHANNELS] = { 0 };
	uint8_t buffer_index = 0;
	uint16_t data;
	uint8_t channel_address;
	int8_t id;
	uint16_t buffer_data[num_of_active_channels];
	bool all_channels_received;

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	nb_of_samples = (iio_dev_data->buffer->size / BYTES_PER_SAMPLE);
	if (buf_size_updated == 0) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	while (sample_index < nb_of_samples) {
		if (sample_index == 0) {
			/*
			* Invalid data:
			* Check Datasheet for First-Channel ADC Conversion Sequence timing diagram
			*/
			ret = ad5592r_spi_wnop_r16(ad559xr_dev_inst, &ad559xr_dev_inst->spi_msg);
			if (ret) {
				return ret;
			}
		}

		/* Reset Indices */
		memset(channels_received, 0, sizeof(channels_received));
		valid_samples = 0;
		memset(adc_raw_data_copy, 0, sizeof(adc_raw_data_copy));
		buffer_index = 0;

		while (valid_samples < num_of_active_channels) {
			/* Read converted samples */
			ret = ad5592r_read_converted_data(ad559xr_dev_inst, adc_raw_data);
			if (ret) {
				return ret;
			}

			/* Process the received data */
			for (id = 0; id < num_of_active_channels; id++) {
				data = adc_raw_data[id];
				channel_address = AD5592R_REG_ADC_SEQ_ADDR_MSK(
							  data); // Extract the 4 MSBs

				if (channel_address < NO_OF_CHANNELS && !channels_received[channel_address]) {
					// Mask the lower 12 bits and place in the appropriate index
					adc_raw_data_copy[channel_address] = AD5592R_REG_ADC_SEQ_CODE_MSK(data);
					channels_received[channel_address] = true;
					valid_samples++;
				}
			}

			/* Check if all channels are received */
			all_channels_received = true;
			for (id = 0; id < NO_OF_CHANNELS; id++) {
				if ((channel_mask & (NO_OS_BIT(id))) && !channels_received[id]) {
					all_channels_received = false;
					break;
				}
			}

			while (!all_channels_received) {
				// Perform extra NOP to get the missing channel data
				ret = ad5592r_spi_wnop_r16(ad559xr_dev_inst, &ad559xr_dev_inst->spi_msg);
				if (ret) {
					return ret;
				}

				// Process the new data
				for (id = 0; id < num_of_active_channels; id++) {
					data = ad559xr_dev_inst->spi_msg;
					channel_address = AD5592R_REG_ADC_SEQ_ADDR_MSK(
								  data); // Extract the 4 MSBs

					if (channel_address < NO_OF_CHANNELS && !channels_received[channel_address]) {
						// Mask the lower 12 bits and place in the appropriate index
						adc_raw_data_copy[channel_address] = AD5592R_REG_ADC_SEQ_CODE_MSK(data);
						channels_received[channel_address] = true;
						valid_samples++;
					}
				}
				// Check if all channels are received again
				all_channels_received = true;
				for (id = 0; id < NO_OF_CHANNELS; id++) {
					if ((channel_mask & NO_OS_BIT(id)) && !channels_received[id]) {
						all_channels_received = false;
						break;
					}
				}
			}
		}

		/* Prepare the data to be written to the buffer */
		for (id = 0; id < NO_OF_CHANNELS; id++) {
			if (channel_mask & NO_OS_BIT(id)) {
				buffer_data[buffer_index++] = adc_raw_data_copy[id];
			}
		}

		/* Push data into IIO circular buffer */
		ret = no_os_cb_write(iio_dev_data->buffer->buf,
				     buffer_data,
				     num_of_active_channels * BYTES_PER_SAMPLE);
		if (ret) {
			return ret;
		}

		sample_index += num_of_active_channels;
	}
#endif
	return 0;
}

/**
 * @brief Read requested number of ADC samples into IIO buffer for AD5593R
 * @param iio_dev_data[in] - Pointer to IIO device data structure
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad5593r_iio_submit_buffer(struct iio_device_data* iio_dev_data)
{

	int32_t ret;
	uint32_t sample_index = 0;
	uint32_t nb_of_samples;
	uint16_t adc_raw_data[NO_OF_CHANNELS] = { 0 };

	nb_of_samples = (iio_dev_data->buffer->size / BYTES_PER_SAMPLE);
	if (buf_size_updated == 0) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	while (sample_index < nb_of_samples) {
		ret = ad5593r_read_converted_data(ad559xr_dev_inst, adc_raw_data);
		if (ret) {
			return ret;
		}
		/* Push data into IIO circular buffer */
		ret = no_os_cb_write(iio_dev_data->buffer->buf,
				     adc_raw_data,
				     num_of_active_channels * BYTES_PER_SAMPLE);
		if (ret) {
			return ret;
		}
		sample_index += num_of_active_channels;
	}

	return 0;
}

/**
 * @brief Push data into IIO buffer when trigger handler IRQ is invoked
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
int32_t ad559xr_trigger_handler(struct iio_device_data* iio_dev_data)
{
	int ret;
	uint16_t adc_raw_data[NO_OF_CHANNELS] = { 0 };
	uint16_t adc_raw_data_copy[NO_OF_CHANNELS] = { 0 };
	bool channels_received[NO_OF_CHANNELS] = { false };
	int8_t valid_samples = 0;
	int8_t id;
	uint16_t buffer_data[num_of_active_channels];
	int8_t buffer_index = 0;
	uint16_t data;
	uint8_t channel_address;
	bool all_channels_received;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
						   iio_dev_data->buffer->bytes_per_scan)) * iio_dev_data->buffer->bytes_per_scan;
		buf_size_updated = true;
	}

	while (valid_samples < num_of_active_channels) {
		/* Read converted samples */
		ret = ad5592r_read_converted_data(ad559xr_dev_inst, adc_raw_data);
		if (ret) {
			return ret;
		}

		/* Process the received data */
		for (id = 0; id < num_of_active_channels; id++) {
			data = adc_raw_data[id];
			channel_address = AD5592R_REG_ADC_SEQ_ADDR_MSK(
						  data); // Extract the 4 MSBs

			if (channel_address < NO_OF_CHANNELS && !channels_received[channel_address]) {
				// Mask the lower 12 bits and place in the appropriate index
				adc_raw_data_copy[channel_address] = AD5592R_REG_ADC_SEQ_CODE_MSK(data);
				channels_received[channel_address] = true;
				valid_samples++;
			}
		}

		/* Check if all channels are received */
		all_channels_received = true;
		for (id = 0; id < NO_OF_CHANNELS; id++) {
			if ((channel_mask & (NO_OS_BIT(id))) && !channels_received[id]) {
				all_channels_received = false;
				break;
			}
		}

		while (!all_channels_received) {
			// Perform extra NOP to get the missing channel data
			ret = ad5592r_spi_wnop_r16(ad559xr_dev_inst, &ad559xr_dev_inst->spi_msg);
			if (ret) {
				return ret;
			}

			// Process the new data
			for (id = 0; id < num_of_active_channels; id++) {
				data = ad559xr_dev_inst->spi_msg;
				channel_address = AD5592R_REG_ADC_SEQ_ADDR_MSK(
							  data); // Extract the 4 MSBs

				if (channel_address < NO_OF_CHANNELS && !channels_received[channel_address]) {
					// Mask the lower 12 bits and place in the appropriate index
					adc_raw_data_copy[channel_address] = AD5592R_REG_ADC_SEQ_CODE_MSK(data);
					channels_received[channel_address] = true;
					valid_samples++;
				}
			}

			// Check if all channels are received again
			all_channels_received = true;
			for (id = 0; id < NO_OF_CHANNELS; id++) {
				if ((channel_mask & (NO_OS_BIT(id))) && !channels_received[id]) {
					all_channels_received = false;
					break;
				}
			}
		}
	}

	/* Prepare the data to be written to the buffer */
	for (id = 0; id < NO_OF_CHANNELS; id++) {
		if (channel_mask & (NO_OS_BIT(id))) {
			buffer_data[buffer_index++] = adc_raw_data_copy[id];
		}
	}

	/* Push data into IIO circular buffer */
	ret = no_os_cb_write(iio_dev_data->buffer->buf,
			     buffer_data,
			     num_of_active_channels * BYTES_PER_SAMPLE);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Read value of the debug register
 * @param dev[in]- Pointer to IIO device instance
 * @param reg[in]- Address of the register where the data is to be written
 * @param read_val[out]- Pointer to the register data variable
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad559xr_iio_debug_reg_read(void* dev,
		uint32_t reg,
		uint32_t* read_val)
{
	int ret;

	if (!dev || (!read_val) || (reg > AD5592R_REG_RESET)) {
		return -EINVAL;
	}

	ret = ad5592r_base_reg_read(dev, reg, read_val);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Write value to the debug register
 * @param dev[in]- Pointer to IIO device instance
 * @param reg[in]- Address of the register where the data is to be written
 * @param write_val[in]- Value of the data that is to be written
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad559xr_iio_debug_reg_write(void* dev,
		uint32_t reg,
		uint32_t write_val)
{
	int ret;

	if (!dev || (reg > AD5592R_REG_RESET)) {
		return -EINVAL;
	}

	ret = ad5592r_base_reg_write(dev, reg, write_val);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief	Init for reading/writing and parameterization of
 * 			ad559xr IIO device
 * @param 	desc[in,out] - IIO device descriptor
 * @param   dev_indx [in] - IIO device index
 * @return	0 in case of success, negative value otherwise
 */
static int ad559xr_iio_init(struct iio_device** desc, uint8_t dev_indx)
{
	struct iio_device* iio_ad559xr_inst;  // IIO Device Descriptor for ad559xr
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t indx;

	if (!desc) {
		return -EINVAL;
	}

	iio_ad559xr_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad559xr_inst) {
		return -ENOMEM;
	}

#if defined (SYSTEM_CONFIG_DISABLED)
	indx = dev_indx + 1;
#else
	indx = dev_indx;
#endif

	iio_ad559xr_inst->num_ch = ad559xr_channels;
	iio_ad559xr_inst->channels = ad559xr_iio_channels[dev_indx];
	iio_ad559xr_inst->attributes = ad559xr_iio_global_attributes[indx];
	iio_ad559xr_inst->debug_attributes = NULL;
	iio_ad559xr_inst->buffer_attributes = NULL;
	if (active_device == AD5592R) {
		iio_ad559xr_inst->submit = ad5592r_iio_submit_buffer;
	} else {
		iio_ad559xr_inst->submit = ad5593r_iio_submit_buffer;
	}
	iio_ad559xr_inst->pre_enable = ad559xr_iio_prepare_transfer;
	iio_ad559xr_inst->post_disable = ad559xr_iio_end_transfer;
	iio_ad559xr_inst->write_dev = NULL;
	iio_ad559xr_inst->debug_reg_read = ad559xr_iio_debug_reg_read;
	iio_ad559xr_inst->debug_reg_write = ad559xr_iio_debug_reg_write;
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_ad559xr_inst->trigger_handler = ad559xr_trigger_handler;
#endif
	for (j = 0; j < NO_OF_CHANNELS; j++) {
		if (i >= ad559xr_channels) {
			break;
		}
		if (ad559xr_dev_inst->channel_modes[j] == CH_MODE_ADC
		    || ad559xr_dev_inst->channel_modes[j] == CH_MODE_DAC
		    || ad559xr_dev_inst->channel_modes[j] == CH_MODE_DAC_AND_ADC) {
			ad559xr_iio_channels[dev_indx][i].name = malloc(CHANNEL_NAME_SIZE * sizeof(
						char)); // Allocate memory for the name
			if (!ad559xr_iio_channels[dev_indx][i].name) {
				free(iio_ad559xr_inst);
				return -ENOMEM;
			}
			sprintf(ad559xr_iio_channels[dev_indx][i].name, "voltage%d", j);
			ad559xr_iio_channels[dev_indx][i].channel = i;
			ad559xr_iio_channels[dev_indx][i].ch_type = IIO_VOLTAGE;
			ad559xr_iio_channels[dev_indx][i].indexed = true;
			ad559xr_iio_channels[dev_indx][i].scan_index = i;
			ad559xr_iio_channels[dev_indx][i].scan_type = ad559xr_iio_scan_type[dev_indx];
			if (ad559xr_dev_inst->channel_modes[j] == CH_MODE_DAC) {
				ad559xr_iio_channels[dev_indx][i].attributes =
					ad559xr_iio_ch_attributes[indx];
				ad559xr_iio_channels[dev_indx][i].ch_out = true;
			}

			if (ad559xr_dev_inst->channel_modes[j] == CH_MODE_ADC
			    || ad559xr_dev_inst->channel_modes[j] == CH_MODE_DAC_AND_ADC) {
				ad559xr_iio_channels[dev_indx][i].attributes =
					ad559xr_iio_adc_ch_attributes[indx];
				ad559xr_iio_channels[dev_indx][i].ch_out = false;
			}
			i++;

		}
	}

	*desc = iio_ad559xr_inst;

	return 0;
}

/**
 * @brief	Init for reading/writing and parameterization of
 * 			ad559xr GPIO Subsystem
 * @param 	desc[in,out] - IIO device descriptor
 * @param   indx [in] - IIO device index
 * @return	0 in case of success, negative value otherwise
 */
static int ad559xr_gpio_init(struct iio_device** desc, uint8_t indx)
{
	uint8_t i;
	int ret;
	struct iio_device* iio_ad559xr_inst;
	uint8_t ch_attr_index = 2;

#if defined (ONLY_GPIO_SUBSYSTEM)
	indx = 0;
#endif

	if (!desc) {
		return -EINVAL;
	}

	iio_ad559xr_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad559xr_inst) {
		return -ENOMEM;
	}

	iio_ad559xr_inst->num_ch = NO_OS_ARRAY_SIZE(ad559xr_iio_channels[indx]);
	iio_ad559xr_inst->channels = ad559xr_iio_channels[indx];

	for (i = 0; i < NO_OF_CHANNELS; i++) {
		if (ad559xr_dev_inst->channel_modes[i] == CH_MODE_GPI
		    || ad559xr_dev_inst->channel_modes[i] == CH_MODE_GPO) {
			ad559xr_iio_channels[indx][i].name = malloc(GPIO_CHANNEL_NAME_SIZE * sizeof(
					char)); // Allocate memory for the name
			if (!ad559xr_iio_channels[indx][i].name) {
				free(iio_ad559xr_inst);
				return -ENOMEM;
			}
			sprintf(ad559xr_iio_channels[indx][i].name, "gpio%d", i);
			ad559xr_iio_channels[indx][i].channel = i;
			ad559xr_iio_channels[indx][i].attributes =
				ad559xr_iio_ch_attributes[ch_attr_index];
			ad559xr_iio_channels[indx][i].ch_out = true;
			ad559xr_iio_channels[indx][i].ch_type = IIO_VOLTAGE;
			ad559xr_iio_channels[indx][i].indexed = true;
			ad559xr_iio_channels[indx][i].scan_index = i;
			ad559xr_iio_channels[indx][i].scan_type = ad559xr_iio_scan_type[indx];
		} else {
			ad559xr_iio_channels[indx][i].name = NULL;
			ad559xr_iio_channels[indx][i].channel = 0;
			ad559xr_iio_channels[indx][i].ch_type = 0;
			ad559xr_iio_channels[indx][i].indexed = false;
			ad559xr_iio_channels[indx][i].scan_index = 0;
			ad559xr_iio_channels[indx][i].scan_type = NULL;
			ad559xr_iio_channels[indx][i].attributes =
				0;
			ad559xr_iio_channels[indx][i].ch_out = 0;
		}

		/* Set up GPIO Direction */
		if (ad559xr_dev_inst->channel_modes[i] == CH_MODE_GPI) {
			ret = ad5592r_gpio_direction_input(ad559xr_dev_inst, i);
			if (ret) {
				free(iio_ad559xr_inst);
				return ret;
			}
		}

		if (ad559xr_dev_inst->channel_modes[i] == CH_MODE_GPO) {
			ret = ad5592r_gpio_direction_output(ad559xr_dev_inst, i, 1);
			if (ret) {
				free(iio_ad559xr_inst);
				return ret;
			}
		}
	}

	*desc = iio_ad559xr_inst;

	return 0;
}

/**
 * @brief	Initialization of ad559xr IIO hardware trigger specific parameters
 * @param 	desc[in,out] - IIO hardware trigger descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad559xr_iio_trigger_param_init(struct iio_hw_trig** desc)
{
	int32_t ret;
	struct iio_hw_trig_init_param ad559xr_hw_trig_init_params;
	struct iio_hw_trig* hw_trig_desc;

	if (!desc) {
		return -EINVAL;
	}

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad559xr_hw_trig_init_params.irq_id = TRIGGER_INT_ID;
	ad559xr_hw_trig_init_params.name = AD559XR_IIO_TRIGGER_NAME;
	pwm_extra_params = pwm_desc->extra;
	ad559xr_hw_trig_init_params.irq_ctrl = pwm_extra_params->nvic_tim;
	ad559xr_hw_trig_init_params.iio_desc = p_ad559xr_iio_desc;
	ad559xr_hw_trig_init_params.cb_info.event = INTR_CALLBACK_EVENT;
	ad559xr_hw_trig_init_params.cb_info.peripheral = INTR_CALLBACK_PERIPHERAL;
	ad559xr_hw_trig_init_params.cb_info.handle = trigger_handle;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad559xr_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/**
 * @brief Release resources allocated for IIO device
 * @param desc[in] - IIO device descriptor
 * @return 0 in case of success, negative value otherwise
 */
static int ad559xr_iio_remove(struct iio_desc* desc)
{
	int status;

	if (!desc) {
		return -EINVAL;
	}

	status = iio_remove(desc);
	if (status) {
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief 	Initialize the ad559xr Board Init Params
 * @param   desc[in,out] - IIO Device Descriptor
 * @param   dev_indx[in] - IIO Device Index
 * @return	0 in case of success, negative value otherwise
 */
static int board_iio_params_init(struct iio_device** desc,
				 uint8_t dev_indx)
{
	struct iio_device* iio_dev;

	if (!desc) {
		return -EINVAL;
	}

	iio_dev = calloc(1, sizeof(*iio_dev));
	if (!iio_dev) {
		return -ENOMEM;
	}

	iio_dev->num_ch = NO_OS_ARRAY_SIZE(ad559xr_iio_channels[dev_indx]);
	iio_dev->channels = ad559xr_iio_channels[dev_indx];
	iio_dev->attributes = ad559xr_iio_global_attributes[dev_indx];

	*desc = iio_dev;

	return 0;
}

/**
 * @brief 	Initialize the ad559xr IIO Interface
 * @return	0 in case of success, negative value otherwise
 */
int32_t iio_app_initialize(void)
{
	int32_t init_status;
	uint8_t i;
	ad559xr_channels = 0;
	num_non_gpio_channels = 0;

	memset(non_gpio_channels, 0, sizeof(non_gpio_channels));

	static struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES];
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	static struct iio_trigger ad559xr_iio_trig_desc = {
		.is_synchronous = true,
		.enable = NULL,
		.disable = NULL
	};

	/* IIO trigger init parameters */
	struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad559xr_iio_trig_desc,
		.name = AD559XR_IIO_TRIGGER_NAME,
	};
#endif

	/* IIO interface init parameters */
	struct iio_init_param iio_init_params = {
		.phy_type = USE_UART,
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		.trigs = &iio_trigger_init_params,
#endif
	};

	/* Delay needed after peripheral initialization before reading context attributes */
	no_os_mdelay(2000);

	/* Get Context Attributes */
	init_status = get_iio_context_attributes_ex(&iio_init_params.ctx_attrs,
			&iio_init_params.nb_ctx_attr,
			eeprom_desc,
			HW_MEZZANINE_NAME,
			STR(HW_CARRIER_NAME),
			&hw_mezzanine_is_valid,
			FIRMWARE_VERSION);
	if (init_status) {
		return init_status;
	}

#if defined (SYSTEM_CONFIG_ENABLED)
	/* Initialize board IIO paramaters */
	init_status = board_iio_params_init(&p_iio_ad559xr_dev[iio_init_params.nb_devs],
					    iio_init_params.nb_devs);
	if (init_status) {
		return init_status;
	}

	iio_device_init_params[iio_init_params.nb_devs].name = "system_config";
	iio_device_init_params[iio_init_params.nb_devs].dev_descriptor =
		p_iio_ad559xr_dev[iio_init_params.nb_devs];
	iio_init_params.nb_devs++;
#else
	active_device = ACTIVE_DEVICE_NAME;
	device_select = true;
#endif

	/* Active Device is selected */
	if (device_select) {
		if (hw_mezzanine_is_valid) {
			if (active_device == AD5592R) {
				init_status = ad5592r_init(&ad559xr_dev_inst, &ad5592r_init_params);
				if (init_status) {
					return init_status;
				}
			}

			else {
				/* Update the slave address */
				ad5592r_init_params.i2c_init->slave_address = AD5593R_I2C(AD5593R_A0_STATE);
				init_status = ad5593r_init(&ad559xr_dev_inst, &ad5592r_init_params);
				if (init_status) {
					return init_status;
				}
			}

			/* Keep repetition bit to true by default */
			init_status = ad559xr_set_rep_bit(true);
			if (init_status) {
				return init_status;
			}
		}

		/* Calculate the number of non-gpio Channels */
		for (i = 0; i < NO_OF_CHANNELS; i++) {
			if (!(ad559xr_dev_inst->channel_modes[i] == CH_MODE_UNUSED
			      || ad559xr_dev_inst->channel_modes[i] == CH_MODE_GPI
			      || ad559xr_dev_inst->channel_modes[i] == CH_MODE_GPO)) {
				ad559xr_channels++;
			}
		}

#if !defined(ONLY_GPIO_SUBSYSTEM)
		/* Initialize the ad559xr IIO application interface */
		init_status = ad559xr_iio_init(&p_iio_ad559xr_dev[iio_init_params.nb_devs],
					       iio_init_params.nb_devs);
		if (init_status) {
			return init_status;
		}

		/* Initialize the IIO interface */
		iio_device_init_params[iio_init_params.nb_devs].name =
			device_name[active_device];
		iio_device_init_params[iio_init_params.nb_devs].raw_buf = adc_data_buffer;
		iio_device_init_params[iio_init_params.nb_devs].raw_buf_len = DATA_BUFFER_SIZE;

		iio_device_init_params[iio_init_params.nb_devs].dev = ad559xr_dev_inst;
		iio_device_init_params[iio_init_params.nb_devs].dev_descriptor =
			p_iio_ad559xr_dev[iio_init_params.nb_devs];
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		iio_device_init_params[iio_init_params.nb_devs].trigger_id = "trigger0";
		iio_init_params.nb_trigs++;
#endif

		iio_init_params.nb_devs++;
#endif

		/* Check if any channel is configured as a GPIO Input or GPIO Output */
		for (i = 0; i < NO_OF_CHANNELS; i++) {
			if (ad559xr_dev_inst->channel_modes[i] == CH_MODE_GPI
			    || ad559xr_dev_inst->channel_modes[i] == CH_MODE_GPO) {
				gpio_enabled = true;
				break;
			} else {
				gpio_enabled = false;
			}
		}

		for (i = 0; i < NO_OF_CHANNELS; i++) {
			if (ad559xr_dev_inst->channel_modes[i] == CH_MODE_UNUSED) {
				gpio_unused_enabled = true;
				break;
			} else {
				gpio_unused_enabled = false;
			}
		}

		if (gpio_enabled) {
			init_status = ad559xr_gpio_init(&p_iio_ad559xr_dev[iio_init_params.nb_devs],
							iio_init_params.nb_devs);
			if (init_status) {
				return init_status;
			}

			/* Initialize the IIO interface */
			iio_device_init_params[iio_init_params.nb_devs].name =
				"one-bit-adc-dac";
			iio_device_init_params[iio_init_params.nb_devs].dev = ad559xr_dev_inst;
			iio_device_init_params[iio_init_params.nb_devs].dev_descriptor =
				p_iio_ad559xr_dev[iio_init_params.nb_devs];
			iio_init_params.nb_devs++;
		}

		/* Find the index of non-gpio channels */
		for (i = 0; i < NO_OF_CHANNELS; i++) {
			if (!(ad559xr_dev_inst->channel_modes[i] == CH_MODE_GPI ||
			      ad559xr_dev_inst->channel_modes[i] == CH_MODE_GPO
			      || ad559xr_dev_inst->channel_modes[i] == CH_MODE_UNUSED)) {
				non_gpio_channels[num_non_gpio_channels] = i;
				(num_non_gpio_channels)++;
			}
		}
	}
	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_desc;
	iio_init_params.devs = iio_device_init_params;
	init_status = iio_init(&p_ad559xr_iio_desc, &iio_init_params);
	if (init_status) {
		if (device_select) {
			if (active_device == AD5592R) {
				init_status = no_os_spi_remove(ad559xr_dev_inst->spi);
				if (init_status) {
					return init_status;
				}
			} else {
				init_status = no_os_i2c_remove(ad559xr_dev_inst->i2c);
				if (init_status) {
					return init_status;
				}
			}
			free(ad559xr_dev_inst);
		}
		ad559xr_iio_remove(p_ad559xr_iio_desc);
		return -ENOSYS;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Initialize the AD559xr IIO trigger specific parameters */
	init_status = ad559xr_iio_trigger_param_init(&ad559xr_hw_trig_desc);
	if (init_status) {
		return init_status;
	}

	/* The UART interrupt needs to be prioritized over the Timer Interrupt.
	 * If not, the Timer interrupt may occur during the period where there is a UART read happening
	 * for the READBUF command and would inturn lead to missing of characters in the IIO command
	 *  sent from the client. */
	init_status = no_os_irq_set_priority(pwm_extra_params->nvic_tim, PWM_TIM_IRQ_ID,
					     PWM_PRIORITY);
	if (init_status) {
		return init_status;
	}
#endif

	return 0;
}

/**
 * @brief 	Run the ad559xr IIO event handler
 * @return	None
 */
void iio_app_event_handler(void)
{
	if (restart_iio_flag) {
		/* Remove and free the pointers allocated during IIO init */
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		iio_hw_trig_remove(ad559xr_hw_trig_desc);
#endif

		iio_remove(p_ad559xr_iio_desc);

		/* Reset the restart_iio flag */
		restart_iio_flag = false;

		iio_app_initialize();
	}
	iio_step(p_ad559xr_iio_desc);
}
