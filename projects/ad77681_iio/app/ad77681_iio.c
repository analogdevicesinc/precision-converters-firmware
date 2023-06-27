/***************************************************************************//**
 *   @file    ad77681_iio.c
 *   @brief   Implementation of AD7768-1 IIO application interfaces
 *   @details This module acts as an interface for AD7768-1 IIO application
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
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "app_config.h"
#include "ad77681_iio.h"
#include "ad77681_user_config.h"
#include "ad77681_regs.h"
#include "ad77681_support.h"
#include "common.h"
#include "iio_trigger.h"
#include "no_os_error.h"
#include "no_os_util.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/*	Number of IIO devices */
#define NUM_OF_IIO_DEVICES	1

/* IIO trigger name */
#define IIO_TRIGGER_NAME		"ad77681_iio_trigger"

/* AD77681 Channel Number */
#define AD77681_NUM_CHANNELS			1
/* Bytes per sample (for ADC resolution of 24-bits)	*/
#define	BYTES_PER_SAMPLE				sizeof(uint32_t)
/* Number of data storage bits (needed for IIO client) */
#define CHN_STORAGE_BITS				(BYTES_PER_SAMPLE * 8)
/* AD77681 24 bits scale factor */
#define AD77681_SCALE_FACTOR			(1 << ADC_RESOLUTION)
/* AD77681 ADC data to Voltage conversion default scale factor for IIO client */
#define AD77681_DEFAULT_SCALE			((((float)(AD77681_VOLTAGE_REF / 1000.00) * 2) / AD77681_SCALE_FACTOR) * 1000)
/* Register Max Address	*/
#define AD77681_REG_MAX_ADDR			AD77681_REG_MCLK_COUNTER + 1
/* Conv Mode Value after setting a single conversion mode */
#define SINGLE_MODE_CONV_STANDBY		6
/* Conv Mode Value after setting a periodic conversion mode */
#define PERIODIC_MODE_CONV_STANDBY		7

/* ADC data buffer size */
#if defined(USE_SDRAM)
#define adc_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(32768)		// 32kbytes
static int8_t adc_data_buffer[DATA_BUFFER_SIZE] = { 0 };
#endif

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* IIO interface descriptor										*/
static struct iio_desc *p_ad77681_iio_desc;

/* Pointer to the struct representing the AD77681 IIO device	*/
struct ad77681_dev *p_ad77681_dev_inst = NULL;

/* IIO hw trigger descriptor */
static struct iio_hw_trig *ad77681_hw_trig_desc;

/* Pointer to the struct AD77681 status register				*/
struct ad77681_status_registers *p_ad77681_stat_reg = NULL;

/* Scale value per channel */
static float attr_scale_val[AD77681_NUM_CHANNELS] = {
	AD77681_DEFAULT_SCALE
};

/* Power mode values string representation */
static const char* power_mode_str[] = {
	"Eco-Mode",
	"Value-Not-Assigned",
	"Median-Mode",
	"Fast-Mode"
};

/* Conversion mode values string representation */
static const char* conv_mode_str[] = {
	"Continuous-Mode",
	"Continious-One-Shot-Mode",
	"Single-Mode",
	"Periodic-Mode",
	"Standby-Mode"
};

/* MCLK division values string representation */
static const char* mclk_division_str[] = {
	"AD77681_MCLK_DIV_16",
	"AD77681_MCLK_DIV_8",
	"AD77681_MCLK_DIV_4",
	"AD77681_MCLK_DIV_2"
};

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/******************************************************************************/
/************************ Functions Prototypes ********************************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/*!
 * @brief	Getter/Setter for the sampling frequency attribute value
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 * @Note	This attribute is used to define the timeout period in IIO
 *			client during data capture.
 *			Timeout = (number of requested samples * (1/sampling frequency)) + 1sec
 *			e.g. if sampling frequency = 64KSPS and requested samples = 400
 *			Timeout = (400 * (1/64000)) + 1 = 1.00625sec = ~1sec
 */
static int get_sampling_frequency(void *device,
				  char *buf,
				  uint32_t len,
				  const struct iio_ch_info *channel,
				  intptr_t id)
{
	return sprintf(buf, "%d", (int32_t)AD77681_DEFAULT_SAMPLING_FREQ);
}

static int set_sampling_frequency(void *device,
				  char *buf,
				  uint32_t len,
				  const struct iio_ch_info *channel,
				  intptr_t id)
{
	/* NA- Can't set sampling frequency value */
	return len;
}

/*!
 * @brief	Getter/Setter for the raw attribute value
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
int get_raw(void *device,
	    char *buf,
	    uint32_t len,
	    const struct iio_ch_info *channel,
	    intptr_t id)
{
	uint32_t adc_data_raw;

	/* Capture the raw adc data */
	if (ad77681_read_single_sample(&adc_data_raw) == 0) {
		return sprintf(buf, "%u", adc_data_raw);
	}

	return - EINVAL;
}

int set_raw(void *device,
	    char *buf,
	    uint32_t len,
	    const struct iio_ch_info *channel,
	    intptr_t id)
{
	/* NA- Can't set raw value */
	return len;
}

/*!
 * @brief	Getter/Setter for the scale attribute value
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
int get_scale(void* device,
	      char* buf,
	      uint32_t len,
	      const struct iio_ch_info* channel,
	      intptr_t id)
{
	return (int)sprintf(buf, "%f", attr_scale_val[channel->ch_num]);
}

int set_scale(void* device,
	      char* buf,
	      uint32_t len,
	      const struct iio_ch_info* channel,
	      intptr_t id)
{
	float scale;

	(void)sscanf(buf, "%f", &scale);

	if (scale > 0.0) {
		attr_scale_val[channel->ch_num] = scale;
		return len;
	}

	return -EINVAL;
}

/*!
 * @brief	Getter for the power mode available values
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int get_power_mode_available(void *device,
				    char *buf,
				    uint32_t len,
				    const struct iio_ch_info *channel,
				    intptr_t id)
{
	return sprintf(buf, "%s", "Eco-Mode Value-Not-Assigned Median-Mode Fast-Mode");
}

/*!
 * @brief	Setter for the power mode available values
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int set_power_mode_available(void *device,
				    char *buf,
				    uint32_t len,
				    const struct iio_ch_info *channel,
				    intptr_t id)
{
	/* NA- Can't set error value */
	return len;
}

/*!
 * @brief	Getter/Setter for the power mode attribute value
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
int get_power_mode(void *device,
		   char *buf,
		   uint32_t len,
		   const struct iio_ch_info *channel,
		   intptr_t id)
{
	uint8_t power_mode_value = 0;

	if (ad77681_spi_read_mask(device,
				  AD77681_REG_POWER_CLOCK,
				  AD77681_POWER_CLK_PWRMODE_MSK,
				  &power_mode_value) == 0) {
		return (int)sprintf(buf, "%s", power_mode_str[power_mode_value]);
	}

	return -EINVAL;
}

int set_power_mode(void *device,
		   char *buf,
		   uint32_t len,
		   const struct iio_ch_info *channel,
		   intptr_t id)
{
	uint8_t power_mode_value;

	for (power_mode_value = 0;
	     power_mode_value < (uint8_t)NO_OS_ARRAY_SIZE(power_mode_str);
	     power_mode_value++) {
		if (!strncmp(buf, power_mode_str[power_mode_value],
			     strlen(power_mode_str[power_mode_value]))) {
			break;
		}
	}

	if (power_mode_value < (uint8_t)NO_OS_ARRAY_SIZE(power_mode_str)) {
		if (ad77681_set_power_mode(device, power_mode_value) == 0) {
			return len;
		}
	}

	return -EINVAL;
}

/*!
 * @brief	Getter for the conv mode available values
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int get_conv_mode_available(void *device,
				   char *buf,
				   uint32_t len,
				   const struct iio_ch_info *channel,
				   intptr_t id)
{
	return sprintf(buf, "%s",
		       "Continuous-Mode Continious-One-Shot-Mode Single-Mode Periodic-Mode Standby-Mode");
}

/*!
 * @brief	Setter for the conv mode available values
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int set_conv_mode_available(void *device,
				   char *buf,
				   uint32_t len,
				   const struct iio_ch_info *channel,
				   intptr_t id)
{
	/* NA- Can't set error value */
	return len;
}

/*!
 * @brief	Getter/Setter for the conversion mode attribute value
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
int get_conv_mode(void* device,
		  char* buf,
		  uint32_t len,
		  const struct iio_ch_info* channel,
		  intptr_t id)
{
	uint8_t conv_mode_value = 0;

	if (ad77681_spi_read_mask(device,
				  AD77681_REG_CONVERSION,
				  AD77681_CONVERSION_MODE_MSK,
				  &conv_mode_value) == 0) {
		if (conv_mode_value <= (uint8_t)NO_OS_ARRAY_SIZE(conv_mode_str)) {
			return (int)sprintf(buf, "%s", conv_mode_str[conv_mode_value]);
		} else if (conv_mode_value == SINGLE_MODE_CONV_STANDBY) {
			return (int)sprintf(buf, "%s", conv_mode_str[2]);
		} else if (conv_mode_value == PERIODIC_MODE_CONV_STANDBY) {
			return (int)sprintf(buf, "%s", conv_mode_str[3]);
		}
	}

	return -EINVAL;
}

int set_conv_mode(void* device,
		  char* buf,
		  uint32_t len,
		  const struct iio_ch_info* channel,
		  intptr_t id)
{
	uint8_t conv_mode_value;

	for (conv_mode_value = 0;
	     conv_mode_value < (uint8_t)NO_OS_ARRAY_SIZE(conv_mode_str);
	     conv_mode_value++) {
		if (!strncmp(buf, conv_mode_str[conv_mode_value],
			     strlen(conv_mode_str[conv_mode_value]))) {
			break;
		}
	}

	if (conv_mode_value < (uint8_t)NO_OS_ARRAY_SIZE(conv_mode_str)) {
		if (ad77681_set_conv_mode(device, conv_mode_value, AD77681_AIN_SHORT,
					  false) == 0) {
			return len;
		}
	}

	return -EINVAL;
}

/*!
 * @brief	Getter for the mclk division available values
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */

static int get_mclk_division_available(void *device,
				       char *buf,
				       uint32_t len,
				       const struct iio_ch_info *channel,
				       intptr_t id)
{
	return sprintf(buf, "%s",
		       "AD77681_MCLK_DIV_16 AD77681_MCLK_DIV_8 AD77681_MCLK_DIV_4 AD77681_MCLK_DIV_2");
}

/*!
 * @brief	Setter for the mclk division available values
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int set_mclk_division_available(void *device,
				       char *buf,
				       uint32_t len,
				       const struct iio_ch_info *channel,
				       intptr_t id)
{
	/* NA- Can't set error value */
	return len;
}

/*!
 * @brief	Getter/Setter for the MCLK division attribute value
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
int get_mclk_division(void* device,
		      char* buf,
		      uint32_t len,
		      const struct iio_ch_info* channel,
		      intptr_t id)
{
	uint8_t mclk_division_value = 0;

	if (ad77681_spi_read_mask(device,
				  AD77681_REG_POWER_CLOCK,
				  AD77681_POWER_CLK_MCLK_DIV_MSK,
				  &mclk_division_value) == 0) {
		return (int)sprintf(buf, "%s", mclk_division_str[mclk_division_value >> 4]);
	}

	return -EINVAL;
}

int set_mclk_division(void* device,
		      char* buf,
		      uint32_t len,
		      const struct iio_ch_info* channel,
		      intptr_t id)
{
	uint8_t mclk_division_value = 0;
	uint8_t mclk_division_str_len = 0;

	mclk_division_str_len = (uint8_t)NO_OS_ARRAY_SIZE(mclk_division_str);

	for (uint8_t mclk_division_cntr = 0; mclk_division_cntr < mclk_division_str_len;
	     mclk_division_cntr++) {
		if (!strncmp(buf, mclk_division_str[mclk_division_cntr],
			     strlen(mclk_division_str[mclk_division_cntr]))) {
			mclk_division_value = mclk_division_cntr;
			break;
		}
	}

	if (mclk_division_value < mclk_division_str_len) {
		if (ad77681_set_mclk_div(device, mclk_division_value) == 0) {
			return len;
		}
	}

	return -EINVAL;
}

/*!
 * @brief	Get the actual register address value from the list
 * @param	reg- Register address to read from
 * @param	Reg_add - actual value of Register address
 * @return	true in case of success, false value otherwise
 */
bool debug_get_reg_value(uint8_t reg, uint8_t* Reg_add)
{
	bool	ad77681_regs_debug_flg = false;
	uint8_t ad77681_regs_arr_cntr;

	for (ad77681_regs_arr_cntr = 0;
	     ad77681_regs_arr_cntr < (uint8_t)AD77681_REG_MAX_ADDR;
	     ad77681_regs_arr_cntr++) {
		if (reg == ad77681_regs[ad77681_regs_arr_cntr]) {
			ad77681_regs_debug_flg = true;
			*Reg_add = reg;
			break;
		}
	}

	return ad77681_regs_debug_flg;
}

/*!
 * @brief	Read the debug register value
 * @param	dev- Pointer to IIO device instance
 * @param	reg- Register address to read from
 * @param	readval- Pointer to variable to read data into
 * @return	0 in case of success, negative value otherwise
 */
int32_t debug_reg_read(void *dev, uint32_t reg, uint32_t *readval)
{

	bool	ad77681_dev_debug_read_flg = false;
	uint8_t ad77681_dev_actual_reg_add = 0;

	ad77681_dev_debug_read_flg = debug_get_reg_value((uint8_t)reg,
				     &ad77681_dev_actual_reg_add);

	/* Read the data from device */
	if (ad77681_dev_debug_read_flg == true) {
		if ((ad77681_spi_reg_read(dev, (uint8_t)ad77681_dev_actual_reg_add,
					  (uint8_t *)readval) == 0)) {
			// Shift 8 bits to get the uint8 value from uint32
			*readval = *readval >> 8;
			return 0;
		}
	}

	return -EIO;
}

/*!
 * @brief	Write into the debug register
 * @param	dev- Pointer to IIO device instance
 * @param	reg- Register address to write into
 * @param	writeval- Register value to write
 * @return	0 in case of success, negative value otherwise
 */
int32_t debug_reg_write(void *dev, uint32_t reg, uint32_t writeval)
{
	bool	ad77681_dev_debug_write_flg = false;
	uint8_t ad77681_dev_actual_reg_add = 0;

	ad77681_dev_debug_write_flg = debug_get_reg_value((uint8_t)reg,
				      &ad77681_dev_actual_reg_add);

	if (ad77681_dev_debug_write_flg == true) {
		if (ad77681_spi_reg_write(dev, ad77681_dev_actual_reg_add,
					  (uint8_t) writeval) == 0) {
			return 0;
		}
	}

	return -EIO;
}

/**
 * @brief	Read buffered data corresponding to IIO device
 * @param	iio_dev_data[in] - IIO device data instance
 * @return	0 in case of success or negative value otherwise
 */
static int32_t iio_ad77681_submit_buffer(struct iio_device_data *iio_dev_data)
{
	uint32_t sample_index = 0;
	int32_t ret;
	uint32_t nb_of_samples;
	uint32_t adc_raw;

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	nb_of_samples = iio_dev_data->buffer->size / BYTES_PER_SAMPLE;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	while (sample_index < nb_of_samples) {
		ret = ad77681_read_single_sample(&adc_raw);
		if (ret) {
			return ret;
		}

		ret = no_os_cb_write(iio_dev_data->buffer->buf, &adc_raw, BYTES_PER_SAMPLE);
		if (ret) {
			return ret;
		}

		sample_index++;
	}
#endif

	return 0;
}

/**
 * @brief	Prepare for ADC data capture (transfer from device to memory)
 * @param	dev_instance[in] - IIO device instance
 * @param	chn_mask[in] - Channels select mask
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t iio_ad77681_prepare_transfer(void *dev,
		uint32_t chn_mask)
{
	int32_t ret;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = ad77681_set_conv_mode(
		      p_ad77681_dev_inst,
		      AD77681_CONV_CONTINUOUS,
		      AD77681_AIN_SHORT,
		      false);
	if (ret) {
		return ret;
	}

	ret = iio_trig_enable(ad77681_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief	Terminate current data transfer
 * @param	dev[in] - IIO device instance
 * @return	0 in case of success or negative value otherwise
 */
static int32_t iio_ad77681_end_transfer(void *dev)
{
	int32_t ret;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = iio_trig_disable(ad77681_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief Push data into IIO buffer when trigger handler IRQ is invoked
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
int32_t iio_ad77681_trigger_handler(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t adc_raw;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
						   iio_dev_data->buffer->bytes_per_scan)) * iio_dev_data->buffer->bytes_per_scan;
		buf_size_updated = true;
	}

	ret = ad77681_read_converted_sample(&adc_raw);
	if (ret) {
		return ret;
	}

	ret = no_os_cb_write(iio_dev_data->buffer->buf, &adc_raw, BYTES_PER_SAMPLE);
	if (ret) {
		return ret;
	}

	return 0;
}

/*********************************************************
 *               IIO Attributes and Structures
 ********************************************************/

/* IIOD channels attributes list */
struct iio_attribute channel_input_attributes[] = {
	{
		.name = "raw",
		.show = get_raw,
		.store = set_raw,
	},
	{
		.name = "scale",
		.show = get_scale,
		.store = set_scale,
	},

	END_ATTRIBUTES_ARRAY
};

/* IIOD device (global) attributes list */
static struct iio_attribute global_attributes[] = {
	{
		.name = "sampling_frequency",
		.show = get_sampling_frequency,
		.store = set_sampling_frequency,
	},
	{
		.name = "conv_mode_available",
		.show = get_conv_mode_available,
		.store = set_conv_mode_available,
	},
	{
		.name = "conv_mode",
		.show = get_conv_mode,
		.store = set_conv_mode,
	},
	{
		.name = "power_mode_available",
		.show = get_power_mode_available,
		.store = set_power_mode_available,
	},
	{
		.name = "power_mode",
		.show = get_power_mode,
		.store = set_power_mode,
	},
	{
		.name = "mclk_division_available",
		.show = get_mclk_division_available,
		.store = set_mclk_division_available,
	},
	{
		.name = "mclk_division",
		.show = get_mclk_division,
		.store = set_mclk_division,
	},

	END_ATTRIBUTES_ARRAY

};

/* IIOD channels configurations */
struct scan_type chn_scan = {
	.sign = 's',
	.realbits = ADC_RESOLUTION,
	.storagebits = CHN_STORAGE_BITS,
	.shift = 0,
	.is_big_endian = false
};

/* IIOD Channel list */
static struct iio_channel iio_ad77681_channels[] = {
	{
		.name = "voltage0",
		.ch_type = IIO_VOLTAGE,
		.channel = 0,
		.scan_index = 0,
		.scan_type = &chn_scan,
		.attributes = channel_input_attributes,
		.ch_out = false,
		.indexed = true,
	},
};

/**
 * @brief Initialization of AD7768-1 IIO hardware trigger specific parameters
 * @param desc[in,out] - IIO hardware trigger descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad77681_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	struct iio_hw_trig_init_param ad77681_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;
	int32_t ret;

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad77681_hw_trig_init_params.irq_id = TRIGGER_INT_ID;
	ad77681_hw_trig_init_params.name = IIO_TRIGGER_NAME;
	ad77681_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad77681_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad77681_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad77681_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad77681_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad77681_hw_trig_init_params.iio_desc = p_ad77681_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad77681_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/**
 * @brief	Init for reading/writing and parameterization of a
 * 			ad7768-1 IIO device
 * @param 	desc[in,out] - IIO device descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t iio_ad77681_init(struct iio_device **desc)
{
	struct iio_device *iio_ad77861_inst;

	iio_ad77861_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad77861_inst) {
		return -ENOMEM;
	}

	iio_ad77861_inst->num_ch = sizeof(iio_ad77681_channels) / sizeof(
					   iio_ad77681_channels[0]);
	iio_ad77861_inst->channels = iio_ad77681_channels;
	iio_ad77861_inst->attributes = global_attributes;

	iio_ad77861_inst->submit = iio_ad77681_submit_buffer;
	iio_ad77861_inst->pre_enable = iio_ad77681_prepare_transfer;
	iio_ad77861_inst->post_disable = iio_ad77681_end_transfer;
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_ad77861_inst->trigger_handler = iio_ad77681_trigger_handler;
#endif

	iio_ad77861_inst->debug_reg_read = debug_reg_read;
	iio_ad77861_inst->debug_reg_write = debug_reg_write;

	*desc = iio_ad77861_inst;

	return 0;
}

/**
 * @brief Release resources allocated for IIO device
 * @param desc[in] - IIO device descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad77681_iio_remove(struct iio_desc *desc)
{
	int32_t status;

	if (!desc) {
		return -ENOMEM;
	}

	status = iio_remove(desc);
	if (status) {
		return status;
	}

	return 0;
}

/**
 * @brief	Initialize the IIO interface for AD7768-1 IIO device
 * @return	none
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad77681_iio_initialize(void)
{
	int32_t init_status;

	/* IIO device descriptor */
	struct iio_device *p_iio_ad77681_dev;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	static struct iio_trigger ad77681_iio_trig_desc = {
		.is_synchronous = true,
	};

	/* IIO trigger init parameters */
	static struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad77681_iio_trig_desc,
		.name = IIO_TRIGGER_NAME,
	};
#endif

	/* IIO interface init parameters */
	static struct iio_init_param iio_init_params = {
		.phy_type = USE_UART,
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		.trigs = &iio_trigger_init_params,
#endif
	};

	/* IIOD init parameters */
	struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES] = {
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

	/* Initialize AD77681 device and peripheral interface */
	init_status = ad77681_setup(&p_ad77681_dev_inst, sad77681_init,
				    &p_ad77681_stat_reg);
	if (init_status != 0) {
		return init_status;
	}

	/* Initialize the AD77681 IIO application interface */
	init_status = iio_ad77681_init(&p_iio_ad77681_dev);
	if (init_status != 0) {
		return init_status;
	}

	iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
	iio_device_init_params[0].raw_buf = adc_data_buffer;
	iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;

	iio_device_init_params[0].dev = p_ad77681_dev_inst;
	iio_device_init_params[0].dev_descriptor = p_iio_ad77681_dev;

	iio_init_params.nb_devs++;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_init_params.nb_trigs++;
#endif

	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_desc;
	iio_init_params.devs = iio_device_init_params;
	init_status = iio_init(&p_ad77681_iio_desc, &iio_init_params);
	if (init_status) {
		ad77681_iio_remove(p_ad77681_iio_desc);
		return init_status;
	}
	
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Initialize the IIO trigger specific parameters */
	init_status = ad77681_iio_trigger_param_init(&ad77681_hw_trig_desc);
	if (init_status) {
		return init_status;
	}
#endif

	return 0;
}

/**
 * @brief 	Run the AD77681 IIO event handler
 * @return	none
 * @details	This function monitors the new IIO client event
 */
void ad77681_iio_event_handler(void)
{
	(void)iio_step(p_ad77681_iio_desc);
}
