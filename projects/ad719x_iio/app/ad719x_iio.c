/***************************************************************************//**
 *   @file    ad719x_iio.c
 *   @brief   Implementation of AD719X IIO application interfaces
********************************************************************************
 * Copyright (c) 2021-23 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <errno.h>

#include "ad719x_iio.h"
#include "ad719x.h"
#include "ad719x_user_config.h"
#include "ad719x_support.h"
#include "app_config.h"
#include "no_os_error.h"
#include "no_os_util.h"
#include "no_os_print_log.h"
#include "iio_trigger.h"
#include "common.h"

/*****************************************************************************/
/********************* Macros and Constants Definition ***********************/
/*****************************************************************************/
/******** Forward declaration of getter/setter functions ********/
static int iio_ad719x_attr_get(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv);

static int iio_ad719x_attr_set(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv);

static int iio_ad719x_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int iio_ad719x_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

#define AD719X_CHN_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = iio_ad719x_attr_get,\
	.store = iio_ad719x_attr_set\
}

#define AD719X_CHN_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = iio_ad719x_attr_available_get,\
	.store = iio_ad719x_attr_available_set\
}

#if (INPUT_CONFIG == DIFFERENTIAL_INPUT)
#define AD719X_IIO_CHANN_DEF(nm, ch1, ch2) \
	{ \
		.name = nm, \
		.ch_type = IIO_VOLTAGE, \
		.channel = ch1, \
		.channel2 = ch2, \
		.scan_type = &ad719x_iio_scan_type, \
		.attributes = iio_ad719x_ch_attributes, \
	    .scan_index = ch1/2, \
		.ch_out = false, \
		.indexed = true, \
		.diferential = true, \
	}
#else
#define AD719X_IIO_CH(_idx, chn_no) {\
	.name = _idx, \
	.ch_type = IIO_VOLTAGE,\
	.ch_out = false,\
	.indexed = true,\
	.channel = chn_no,\
	.scan_index = chn_no,\
	.scan_type = &ad719x_iio_scan_type,\
	.attributes = iio_ad719x_ch_attributes\
}
#endif

/* ADC data buffer size */
#if defined(USE_SDRAM)
#define adc_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(32768)		// 32kbytes
static int8_t adc_data_buffer[DATA_BUFFER_SIZE];
#endif

/*	Number of IIO devices */
#define NUM_OF_IIO_DEVICES	         1

/* IIO trigger name */
#define ad719x_IIO_TRIGGER_NAME		"ad719x_iio_trigger"

/* Number of bytes for storage of one sample */
#define BYTES_PER_SAMPLE             (STORAGE_BITS/8)

/* ADC data to Voltage conversion default scale factor for IIO client */
#if (POLARITY_CONFIG == BIPOLAR_MODE)
#define AD719X_DEFAULT_SCALE(x)		((AD719X_DEFAULT_REF_VOLTAGE / (ADC_MAX_COUNT_BIPOLAR * x)) * 1000)
#else
#define AD719X_DEFAULT_SCALE(x)		((AD719X_DEFAULT_REF_VOLTAGE / (ADC_MAX_COUNT_UNIPOLAR * x)) * 1000)
#endif

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/
/* IIO interface descriptor */
static struct iio_desc *p_ad719x_iio_desc = NULL;

/* Pointer to the struct representing the AD719X IIO device */
struct ad719x_dev *p_ad719x_dev_inst = NULL;

/* ad719x IIO device descriptor */
struct iio_device *p_iio_ad719x_dev;

/* ad719x IIO hw trigger descriptor */
static struct iio_hw_trig *ad719x_hw_trig_desc;

/* Variable to detect if the data is ready*/
volatile static bool data_ready = false;

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/* Timeout count to avoid stuck into potential infinite loop while checking
 * for new data into an acquisition buffer. The actual timeout factor is determined
 * through 'sampling_frequency' attribute of IIO app, but this period here makes sure
 * we are not stuck into a forever loop in case data capture is interrupted
 * or failed in between.
 * Note: This timeout factor is dependent upon the MCU clock frequency. Below timeout
 * is tested for SDP-K1 platform @180Mhz default core clock */
#define BUF_READ_TIMEOUT	0xffffffff

/*****************************************************************************/
/********************** IIO Attributes and Channels **************************/
/*****************************************************************************/
/* IIOD channels configurations */
struct scan_type ad719x_iio_scan_type = {
	.sign = 'u',
	.realbits = REAL_BITS,
	.storagebits = STORAGE_BITS,
	.shift = false,
	.is_big_endian = false
};

/* AD719X attribute unique IDs */
enum ad719x_attribute_ids {
	ADC_RANGE,
	ADC_BRIDGE_SWITCH,
	ADC_OPERATING_MODE,
	ADC_SAMPLING_FREQUENCY,

	ADC_RAW,
	ADC_SCALE,
	ADC_OFFSET,
};

/* Channel range values string representation
 * (possible values specified in datasheet) */
static char *ad719x_range_str[] = {
	"+/-2.5V",
	"+/-312.5mV",
	"+/-156.2mV",
	"+/-78.125mV",
	"+/-39.06mV",
	"+/-19.53mV",
};

/* Bridge Switch value string representation */
static char *ad719x_bridge_switch_str[] = {
	"Bridge_Switch_Closed",
	"Bridge_Switch_Opened",
};

/* Operating Mode string representation */
static char *ad719x_operating_mode_str[] = {
	"Continuous_Conversion_Mode",
	"Single_Conversion_Mode",
	"Ideal_Mode",
	"Power_Down_Mode",
	"Internal_Zero-Scale_Calibration",
	"Internal_Full-Scale_Calibration",
	"System_Zero-Scale_Calibration",
	"System_Full-Scale_Calibration"
};

/* AD719X device channel attributes list */
static struct iio_attribute iio_ad719x_ch_attributes[] = {
	AD719X_CHN_ATTR("raw", ADC_RAW),
	AD719X_CHN_ATTR("scale", ADC_SCALE),
	AD719X_CHN_ATTR("offset", ADC_OFFSET),
	END_ATTRIBUTES_ARRAY,
};

/* AD719X device (global) attributes list */
static struct iio_attribute iio_ad719x_global_attributes[] = {
	AD719X_CHN_ATTR("range", ADC_RANGE),
	AD719X_CHN_AVAIL_ATTR("range_available", ADC_RANGE),
	AD719X_CHN_ATTR("bridge_switch", ADC_BRIDGE_SWITCH),
	AD719X_CHN_AVAIL_ATTR("bridge_switch_available", ADC_BRIDGE_SWITCH),
	AD719X_CHN_ATTR("operating_mode", ADC_OPERATING_MODE),
	AD719X_CHN_AVAIL_ATTR("operating_mode_available", ADC_OPERATING_MODE),
	AD719X_CHN_ATTR("sampling_frequency", ADC_SAMPLING_FREQUENCY),
	END_ATTRIBUTES_ARRAY,
};

#if (INPUT_CONFIG == DIFFERENTIAL_INPUT)
static struct iio_channel iio_ad719x_channels[] = {
#if defined(DEV_AD7190) || defined(DEV_AD7192) || defined(DEV_AD7195)
	AD719X_IIO_CHANN_DEF("AIN1-AIN2", AD719X_CH_0, AD719X_CH_1),
	AD719X_IIO_CHANN_DEF("AIN2-AIN3", AD719X_CH_2, AD719X_CH_3)
#elif defined(DEV_AD7193)
	/* 24-bit ADC Differential Input Channels (Count= 4) */
	AD719X_IIO_CHANN_DEF("AIN1-AIN2", AD719X_CH_0, AD719X_CH_1),
	AD719X_IIO_CHANN_DEF("AIN3-AIN4", AD719X_CH_2, AD719X_CH_3),
	AD719X_IIO_CHANN_DEF("AIN5-AIN6", AD719X_CH_4, AD719X_CH_5),
	AD719X_IIO_CHANN_DEF("AIN7-AIN8", AD719X_CH_6, AD719X_CH_7)
#endif // Device Channel Select
};
#else
static struct iio_channel iio_ad719x_channels[] = {
	/* 24-bit ADC Pseudo Differential Input Channels (Count= 8) */
#if defined(DEV_AD7190) || defined(DEV_AD7192) || defined(DEV_AD7195)
	AD719X_IIO_CH("AIN1", AD719X_CH_0),
	AD719X_IIO_CH("AIN2", AD719X_CH_1),
	AD719X_IIO_CH("AIN3", AD719X_CH_2),
	AD719X_IIO_CH("AIN4", AD719X_CH_3)
#elif defined(DEV_AD7193)
	AD719X_IIO_CH("AIN1", AD719X_CH_0),
	AD719X_IIO_CH("AIN2", AD719X_CH_1),
	AD719X_IIO_CH("AIN3", AD719X_CH_2),
	AD719X_IIO_CH("AIN4", AD719X_CH_3),
	AD719X_IIO_CH("AIN5", AD719X_CH_4),
	AD719X_IIO_CH("AIN6", AD719X_CH_5),
	AD719X_IIO_CH("AIN7", AD719X_CH_6),
	AD719X_IIO_CH("AIN8", AD719X_CH_7)
#endif // Device Channel Select
};
#endif // Differential Input Select

/* IIOD debug attributes list */
static struct iio_attribute ad719x_debug_attributes[] = {
	END_ATTRIBUTES_ARRAY
};

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/
/*!
 * @brief	Getter function for AD719X attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int iio_ad719x_attr_get(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	int32_t ret;
	uint32_t value;
	struct ad719x_dev *desc = (struct ad719x_dev *)device;

	switch (priv) {
	/****************** ADC global getters ******************/
	case ADC_RANGE:
		ret = ad719x_get_register_value(desc, AD719X_REG_CONF, BYTES_TRANSFER_THREE,
						&value);
		if (NO_OS_IS_ERR_VALUE(ret)) {
			return ret;
		}
		return sprintf(buf, "%s", ad719x_range_str[AD719X_CONF_GAIN(value)]);

	case ADC_BRIDGE_SWITCH:
		ret = ad719x_get_register_value(desc, AD719X_REG_GPOCON, sizeof(uint8_t),
						&value);
		if (NO_OS_IS_ERR_VALUE(ret)) {
			return ret;
		}
		value = (value & AD719X_GPOCON_BPDSW) >> BPDSW_BIT_POSTION;
		return sprintf(buf, "%s", ad719x_bridge_switch_str[value]);

	case ADC_OPERATING_MODE:
		ret = ad719x_get_register_value(desc, AD719X_REG_MODE, BYTES_TRANSFER_THREE,
						&value);
		if (NO_OS_IS_ERR_VALUE(ret)) {
			return ret;
		}
		// Shifting the value by 21 bits to get the mode bits.
		value = value >> MODE_BIT_POSTION;
		return sprintf(buf, "%s", ad719x_operating_mode_str[value]);

	case ADC_SAMPLING_FREQUENCY:
		return snprintf(buf, len, "%d", SAMPLING_RATE_HZ);

	/****************** ADC channel getters ******************/
	case ADC_RAW:
		if (channel->differential != true) {
			switch (desc->chip_id) {
			case AD7190:
			case AD7192:
			case AD7195:
				ret = ad719x_channels_select(desc,
							     AD719X_CH_MASK(channel->ch_num) << AD7190_2_5_CHN_SHIFT);
				break;
			case AD7193:
				ret = ad719x_channels_select(desc,AD719X_CH_MASK(channel->ch_num));
				break;
			default:
				return -ENODEV;
			}
		} else {
			/* In differential mode, the IIO backend supplies channel
			        * number as multiple of 2 */
			ret = ad719x_channels_select(desc,
						     AD719X_CH_MASK(channel->ch_num / AD719X_CH_2));
		}

		if (NO_OS_IS_ERR_VALUE(ret)) {
			return ret;
		}

		ret = ad719x_single_conversion(desc, &value);
		if (NO_OS_IS_ERR_VALUE(ret)) {
			return ret;
		}
		return snprintf(buf, len, "%"PRId32"", value);

	case ADC_SCALE:
		return sprintf(buf, "%f", AD719X_DEFAULT_SCALE(desc->current_gain));

	case ADC_OFFSET:
		if (desc->current_polarity != 0) { // Offset for unipolar
			return snprintf(buf, len, "%d", 0);
		} else { // Offset for bipolar
			return snprintf(buf, len, "%ld", -ADC_MAX_COUNT_BIPOLAR);
		}

	default:
		break;
	}

	return len;
}

/*!
 * @brief	Setter function for AD719X attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of expected bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int iio_ad719x_attr_set(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	uint32_t value;
	int32_t ret;
	uint8_t i;
	struct ad719x_dev *desc = (struct ad719x_dev *)device;

	switch (priv) {
	/****************** ADC global setters ******************/
	case ADC_RANGE:
		for (i = AD719X_ADC_GAIN_1; i <= AD719X_ADC_GAIN_64; i++) {
			if (!strncmp(buf, ad719x_range_str[i], strlen(buf))) {
				value = i;
				break;
			}
		}

		if (i > AD719X_ADC_GAIN_64) {
			return -EINVAL;
		}

		ret = ad719x_range_setup(desc, true, value);
		if (NO_OS_IS_ERR_VALUE(ret)) {
			return ret;
		}

		return len;

	case ADC_BRIDGE_SWITCH:
		if (!strncmp(buf, ad719x_bridge_switch_str[0], strlen(buf))) {
			value = false;
		} else if (!strncmp(buf, ad719x_bridge_switch_str[1], strlen(buf))) {
			value = true;
		}

		ret = ad719x_set_bridge_switch(desc, value);
		if (NO_OS_IS_ERR_VALUE(ret)) {
			return ret;
		}

		return len;

	case ADC_OPERATING_MODE:
		for (i = AD719X_MODE_CONT; i <= AD719X_MODE_CAL_SYS_FULL; i++) {
			if (!strncmp(buf, ad719x_operating_mode_str[i], strlen(buf))) {
				value = i;
				break;
			}
		}

		if (i > AD719X_MODE_CAL_SYS_FULL) {
			return -EINVAL;
		}

		ret = ad719x_set_operating_mode(desc, value);
		if (NO_OS_IS_ERR_VALUE(ret)) {
			return ret;
		}

		return len;

	/* These Attributes are only read only */
	case ADC_RAW:
	case ADC_OFFSET:
	case ADC_SCALE:
	case ADC_SAMPLING_FREQUENCY:

	default:
		break;
	}

	return len;
}

/*!
 * @brief	Attribute available getter function for AD719X attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int iio_ad719x_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	switch (priv) {
	case ADC_RANGE:
		return sprintf(buf, "%s %s %s %s %s %s",
			       ad719x_range_str[0], ad719x_range_str[1],
			       ad719x_range_str[2], ad719x_range_str[3],
			       ad719x_range_str[4], ad719x_range_str[5]);

	case ADC_BRIDGE_SWITCH:
		return sprintf(buf, "%s %s",
			       ad719x_bridge_switch_str[0], ad719x_bridge_switch_str[1]);

	case ADC_OPERATING_MODE:
		return sprintf(buf,
			       "%s %s %s %s %s %s %s %s",
			       ad719x_operating_mode_str[0],
			       ad719x_operating_mode_str[1],
			       ad719x_operating_mode_str[2],
			       ad719x_operating_mode_str[3],
			       ad719x_operating_mode_str[4],
			       ad719x_operating_mode_str[5],
			       ad719x_operating_mode_str[6],
			       ad719x_operating_mode_str[7]);

	default:
		break;
	}

	return len;
}

/*!
 * @brief	Attribute available setter function for AD719X attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int iio_ad719x_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	return len;
}

/**
 * @brief  Prepares the device for data transfer.
 * @param  dev[in, out]- Application descriptor.
 * @param  mask[in]- Number of bytes to transfer.
 * @return 0 in case of success, negative error code otherwise.
 */
static int iio_ad719x_prepare_transfer(void *dev, uint32_t mask)
{
	/* Command Word to start continuous conversion */
	uint8_t cmd_wrd = CNV_START_CMD;
	int32_t ret;

	/* Updates the channel select bits with user selected channels */
#if (INPUT_CONFIG != DIFFERENTIAL_INPUT) && (defined(DEV_AD7190) || \
	defined(DEV_AD7192) || defined(DEV_AD7195))
	ret = ad719x_channels_select(p_ad719x_dev_inst,
				     mask << AD7190_2_5_CHN_SHIFT);
#else
	ret = ad719x_channels_select(p_ad719x_dev_inst, mask);
#endif
	if (ret) {
		return ret;
	}

	/* Reads the Mode register and updates the register with
	 * following configuration:
	 * -> Internal Clock Enabled
	 * -> Data Rate Bits: Selected rate in the config file
	 * -> Enable the status bits to get channel number
	 *    corresponding to each conversion */
	ret = ad719x_clock_select(p_ad719x_dev_inst, AD719X_INT_CLK_4_92_MHZ_TRIST);
	if (ret) {
		return ret;
	}

	ret = ad719x_output_rate_select(p_ad719x_dev_inst, DATA_OUTPUT_RATE_BITS);
	if (ret) {
		return ret;
	}

	/* Puts the device into continuous conversion mode */
	ret = ad719x_set_operating_mode(p_ad719x_dev_inst, AD719X_MODE_CONT);
	if (ret) {
		return ret;
	}

	/* Writes the conversion start command word */
	ret = no_os_spi_write_and_read(p_ad719x_dev_inst->spi_desc,
				       &cmd_wrd, sizeof(uint8_t));
	if (ret) {
		return ret;
	}

	/* Pull the cs line low to detect the EOC bit during data capture */
	ret = no_os_gpio_set_value(gpio_cs, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = iio_trig_enable(ad719x_hw_trig_desc);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_set_value(p_ad719x_dev_inst->sync_pin, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}
#else
	ret = no_os_irq_enable(trigger_irq_desc, TRIGGER_INT_ID);
	if (ret) {
		return ret;
	}
	/* It take around 2ms for IIO back-end to switch
	 * from prepare_tranfer function to read_samples function.
	 * Hence pulling the SYNC pin low will hault the device into reset
	 * state and avoid any delay between starting conversion and data capture.
	 */
	ret = no_os_gpio_set_value(p_ad719x_dev_inst->sync_pin, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief  Close active channels.
 * @param  dev[in, out]- Application descriptor.
 * @return 0 in case of success, error code otherwise.
 */
static int iio_ad719x_close_channels(void *dev)
{
	uint32_t timeout = BUF_READ_TIMEOUT;
	uint8_t stop_cmd = CNV_STOP_CMD;
	uint8_t eoc_pin_status = NO_OS_GPIO_HIGH;
	int32_t ret;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = no_os_gpio_set_value(p_ad719x_dev_inst->sync_pin, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}
	ret = no_os_irq_disable(trigger_irq_desc, TRIGGER_INT_ID);
	if (ret) {
		return ret;
	}
#endif

	ret = no_os_gpio_set_value(p_ad719x_dev_inst->sync_pin, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	/* Wait until the end of Conversion */
	while ((eoc_pin_status != NO_OS_GPIO_LOW) && (timeout > 0)) {
		ret = no_os_gpio_get_value(p_ad719x_dev_inst->gpio_miso, &eoc_pin_status);
		if (ret) {
			return ret;
		}
		timeout--;
	}

	if (!timeout) {
		return -EIO;
	}

	ret = no_os_spi_write_and_read(p_ad719x_dev_inst->spi_desc,
				       &stop_cmd, sizeof(uint8_t));
	if (ret) {
		return ret;
	}

	/* Configures the device into single conversion mode */
	ret = ad719x_set_operating_mode(p_ad719x_dev_inst, AD719X_MODE_IDLE);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Get a number of samples from all the active channels.
 * @param [in, out] dev - Device descriptor.
 * @param [out] buff - Sample buffer.
 * @param [in] nb_samples - Number of samples to get.
 * @return Number of samples read.
 */
static int iio_ad719x_submit_samples(struct iio_device_data *iio_dev_data)
{
	uint32_t ret;
	uint32_t count = 0;
	uint8_t data_read[4] = { 0 };
	volatile uint32_t timeout = BUF_READ_TIMEOUT;
	uint32_t nb_of_samples;

	if (!iio_dev_data) {
		return -EINVAL;
	}

	nb_of_samples = iio_dev_data->buffer->size / BYTES_PER_SAMPLE;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	ret = no_os_gpio_set_value(p_ad719x_dev_inst->sync_pin, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	while (count < (nb_of_samples)) {
		/* Wait until the end of conversion */
		while (data_ready != true && timeout > 0) {
			timeout--;
		}

		if (!timeout) {
			return -EIO;
		}

		ret = no_os_irq_disable(trigger_irq_desc, TRIGGER_INT_ID);
		if (ret) {
			return ret;
		}

		/* Read data over spi interface (in continuous read mode) */
		ret = no_os_spi_write_and_read(p_ad719x_dev_inst->spi_desc, data_read, 3);
		if (ret) {
			return ret;
		}

		/* Interchanging the MSB and LSB positions in the sample buffer */
		no_os_swap(data_read[0], data_read[2]);
		data_read[3] = 0;

		ret = no_os_irq_enable(trigger_irq_desc, TRIGGER_INT_ID);
		if (ret) {
			return ret;
		}

		ret = no_os_gpio_set_value(gpio_cs, NO_OS_GPIO_LOW);
		if (ret) {
			return ret;
		}

		ret = no_os_cb_write(iio_dev_data->buffer->buf,
				     data_read, BYTES_PER_SAMPLE);
		if (ret) {
			return ret;
		}

		count++;
		data_ready = false;
		timeout = BUF_READ_TIMEOUT;
		memset(data_read, 0, sizeof(data_read));
	}

	/* Halt the data conversion on the device */
	ret = no_os_gpio_set_value(p_ad719x_dev_inst->sync_pin, NO_OS_GPIO_LOW);
	if (NO_OS_IS_ERR_VALUE(ret)) {
		return ret;
	}

	return count;
}

/**
 * @brief	Reads data from the ADC and pushes it into IIO buffer when the
			IRQ is triggered.
 * @param	iio_dev_data[in] - IIO device data instance.
 * @return	0 in case of success or negative value otherwise.
 */
static int32_t ad719x_trigger_handler(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint8_t data_read[4] = { 0 };

	ret = iio_trig_disable(ad719x_hw_trig_desc);
	if (ret) {
		return ret;
	}

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
						   iio_dev_data->buffer->bytes_per_scan)) * iio_dev_data->buffer->bytes_per_scan;
		buf_size_updated = true;
	}

	/* Read data over spi interface (in continuous read mode) */
	ret = no_os_spi_write_and_read(p_ad719x_dev_inst->spi_desc, data_read, 3) ;
	if (ret) {
		return ret;
	}

	/* Interchanging the MSB and LSB positions in the sample buffer */
	no_os_swap(data_read[0], data_read[2]);
	data_read[3] = 0;

	ret = no_os_gpio_set_value(gpio_cs, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

	ret = iio_trig_enable(ad719x_hw_trig_desc);
	if (ret) {
		return ret;
	}

	ret = no_os_cb_write(iio_dev_data->buffer->buf,
			     data_read, BYTES_PER_SAMPLE);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief Interrupt Service Routine to monitor data ready event.
 * @param context[in] - Callback context (unused)
 * @return none
 */
void burst_capture_callback(void *context)
{
	data_ready = true;
}

/*!
 * @brief	Read the debug register value
 * @param	dev[in, out]- Pointer to IIO device instance
 * @param	reg[in]- Register address to read from
 * @param	readval[out]- Pointer to variable to read data into
 * @return	0 in case of success, negative value otherwise
 */
static int iio_ad719x_debug_reg_read(void *dev, uint32_t reg,
				     uint32_t *readval)
{
	struct ad719x_dev *desc = (struct ad719x_dev *)dev;
	int32_t ret;

	ret = ad719x_get_register_value(desc, reg, BYTES_TRANSFER_THREE, readval);
	if (NO_OS_IS_ERR_VALUE(ret)) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Write the debug register value
 * @param	dev[in, out]- Pointer to IIO device instance
 * @param	reg[in]- Register address to read from
 * @param	writeval[out]- Pointer to variable to write data into
 * @return	0 in case of success, negative value otherwise
 */
static int iio_ad719x_debug_reg_write(void *dev, uint32_t reg,
				      uint32_t writeval)
{
	struct ad719x_dev *desc = (struct ad719x_dev *)dev;
	int32_t ret;

	ret = ad719x_set_register_value(desc, reg, writeval, BYTES_TRANSFER_THREE);
	if (NO_OS_IS_ERR_VALUE(ret)) {
		return ret;
	}

	return 0;
}

/*****************************************************************************/
/******************************* IIO Initialization **************************/
/*****************************************************************************/
/**
 * @brief	Init for reading/writing and parameterization of
 * 			ad719x IIO device
 * @param 	desc[in,out] - IIO device descriptor
 * @return	0 in case of success, negative value otherwise
 */
static int iio_ad719x_init(struct iio_device **desc)
{
	struct iio_device *iio_ad719x_inst;  // IIO Device Descriptor for ad719x

	iio_ad719x_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad719x_inst) {
		return -ENOMEM;
	}

	iio_ad719x_inst->num_ch = NO_OS_ARRAY_SIZE(iio_ad719x_channels);
	iio_ad719x_inst->channels = iio_ad719x_channels;
	iio_ad719x_inst->attributes = iio_ad719x_global_attributes;
	iio_ad719x_inst->debug_attributes = ad719x_debug_attributes;
	iio_ad719x_inst->buffer_attributes = NULL;
	iio_ad719x_inst->submit = iio_ad719x_submit_samples;
	iio_ad719x_inst->pre_enable = iio_ad719x_prepare_transfer;
	iio_ad719x_inst->post_disable = iio_ad719x_close_channels;
	iio_ad719x_inst->write_dev = NULL;
	iio_ad719x_inst->debug_reg_read = iio_ad719x_debug_reg_read;
	iio_ad719x_inst->debug_reg_write = iio_ad719x_debug_reg_write;
	iio_ad719x_inst->trigger_handler = ad719x_trigger_handler;

	*desc = iio_ad719x_inst;

	return 0;
}

/**
 * @brief	Initialization of AD719X IIO hardware trigger specific parameters
 * @param 	desc[in,out] - IIO hardware trigger descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad719x_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	int32_t ret;
	struct iio_hw_trig_init_param ad719x_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad719x_hw_trig_init_params.irq_id = TRIGGER_INT_ID;
	ad719x_hw_trig_init_params.name = ad719x_IIO_TRIGGER_NAME;
	ad719x_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad719x_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad719x_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad719x_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad719x_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad719x_hw_trig_init_params.iio_desc = p_ad719x_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad719x_hw_trig_init_params);
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
static int iio_ad719x_remove(struct iio_desc *desc)
{
	int32_t status;

	if (!desc) {
		return -EINVAL;
	}

	status = iio_remove(desc);
	if (status) {
		return -1;
	}

	return 0;
}

/**
 * @brief 	Initialize the ad719x IIO Interface
 * @return	0 in case of success, negative value otherwise
 */
int32_t ad719x_iio_initialize(void)
{
	int32_t init_status;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	static struct iio_trigger ad719x_iio_trig_desc = {
		.is_synchronous = true,
		.enable = NULL,
		.disable = NULL
	};

	static struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad719x_iio_trig_desc,
		.name = ad719x_IIO_TRIGGER_NAME,
	};
#endif

	/* IIO interface init parameters */
	struct iio_init_param iio_init_params = {
		.phy_type = USE_UART,
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		.trigs = &iio_trigger_init_params,
#endif
	};

	/* IIOD init parameters */
	static struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES] = {
		{
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
			.trigger_id = "trigger0",
#endif
		}
	};

	/* Init the system peripherals */
	init_status = init_system();
	if (init_status) {
		return init_status;
	}

	/* Read context attributes */
	init_status = get_iio_context_attributes(&iio_init_params.ctx_attrs,
			&iio_init_params.nb_ctx_attr,
			eeprom_desc,
			HW_MEZZANINE_NAME,
			STR(HW_CARRIER_NAME),
			&hw_mezzanine_is_valid);
	if (init_status) {
		return init_status;
	}

	if (hw_mezzanine_is_valid) {
		/* Initialize AD719X device and peripheral interface */
		init_status = ad719x_init(&p_ad719x_dev_inst, ad719x_init_params);
		if (init_status) {
			return init_status;
		}

#if !(ACTIVE_MODE == NORMAL_MODE)
		init_status = ad719x_noise_config();
		if (init_status) {
			return init_status;
		}
#endif
		/* Initialize the CS gpio pin */
		init_status = ad719x_gpio_cs_init();
		if (init_status) {
			return init_status;
		}

		/* Initialize the AD719X IIO application interface */
		init_status = iio_ad719x_init(&p_iio_ad719x_dev);
		if (init_status) {
			return init_status;
		}

		/* Initialize the IIO interface */
		iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
		iio_device_init_params[0].raw_buf = adc_data_buffer;
		iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;

		iio_device_init_params[0].dev = p_ad719x_dev_inst;
		iio_device_init_params[0].dev_descriptor = p_iio_ad719x_dev;

		iio_init_params.nb_devs++;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		iio_init_params.nb_trigs++;
#endif
	}

	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_desc;
	iio_init_params.devs = iio_device_init_params;
	init_status = iio_init(&p_ad719x_iio_desc, &iio_init_params);
	if (init_status) {
		pr_err("IIO Init Failed");
		iio_ad719x_remove(p_ad719x_iio_desc);
		return -ENOSYS;
	}
	
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	init_status = ad719x_iio_trigger_param_init(&ad719x_hw_trig_desc);
	if (init_status) {
		return init_status;
	}
#endif

	return 0;
}

/**
 * @brief 	Run the ad719x IIO event handler
 * @return	None
 */
void ad719x_iio_event_handler(void)
{
	iio_step(p_ad719x_iio_desc);
}
