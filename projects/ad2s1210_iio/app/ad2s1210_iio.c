/***************************************************************************//**
 *   @file    ad2s1210_iio.c
 *   @brief   Implementation of AD2S1210 IIO application interfaces
********************************************************************************
 * Copyright (c) 2023 Analog Devices, Inc.
 * Copyright (c) 2023 BayLibre, SAS.
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
#include <errno.h>
#include <limits.h>

#include "iio.h"
#include "iio_trigger.h"

#include "ad2s1210_iio.h"
#include "app_config.h"
#include "ad2s1210_user_config.h"
#include "common.h"
#include "no_os_error.h"
#include "no_os_util.h"

/******** Forward declaration of getter/setter functions ********/
static int iio_ad2s1210_attr_get(void *device, char *buf, uint32_t len,
				 const struct iio_ch_info *channel,
				 intptr_t priv);

static int iio_ad2s1210_attr_set(void *device, char *buf, uint32_t len,
				 const struct iio_ch_info *channel,
				 intptr_t priv);

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/
#define AD2S1210_CHN_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = iio_ad2s1210_attr_get,\
	.store = iio_ad2s1210_attr_set\
}

#define AD2S1210_CH(_name, _idx, _type, _ch_out) {\
	.name = _name, \
	.ch_type = _type,\
	.ch_out = _ch_out,\
	.indexed = true,\
	.channel = 0,\
	.scan_index = _idx,\
	.scan_type = &chn_scan[_idx],\
	.attributes = &ad2s1210_iio_ch_attributes[_idx][0]\
}

/* Number of IIO devices */
#define NUM_OF_IIO_DEVICES	1

#define	BYTES_PER_SAMPLE	2

/* Number of data storage bits (needed for IIO client to plot RESOLVER data) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

#define AD2S1210_IIO_TRIGGER_NAME         "ad2s1210_iio_trigger"

/* data buffer size */
#if defined(USE_SDRAM)
#define data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(32768)		/* 32kbytes */
static int8_t data_buffer[DATA_BUFFER_SIZE] = { 0 };
#endif

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* Pointer to the struct representing the AD2S1210 IIO device */
struct ad2s1210_dev *ad2s1210_dev_inst;

/* IIO interface descriptor */
static struct iio_desc *ad2s1210_iio_desc;

uint32_t active_chn_count;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
static struct iio_hw_trig *ad2s1210_hw_trig_desc;
#endif

enum ad2s1210_attribute_id {
	RAW_ATTR_ID,
	SCALE_ATTR_ID,
	SAMPLING_FREQ_ATTR_ID,
	LABEL_ATTR_ID,
	HYSTERESIS_ATTR_ID,
	HYSTERESIS_AVAILABLE_ATTR_ID,
	FREQ_ATTR_ID,
	FREQ_AVAIL_ATTR_ID,
};

struct scan_type chn_scan[RESOLVER_CHANNELS] = {
	{
		.sign = 'u',
		.realbits = CHN_STORAGE_BITS,
		.storagebits = CHN_STORAGE_BITS,
		.shift = 0,
		.is_big_endian = true,
	},
	{
		.sign = 's',
		.realbits = CHN_STORAGE_BITS,
		.storagebits = CHN_STORAGE_BITS,
		.shift = 0,
		.is_big_endian = true,
	},
	{
		.sign = 'u',
		.realbits = CHN_STORAGE_BITS,
		.storagebits = CHN_STORAGE_BITS,
		.shift = 0,
		.is_big_endian = false,
	},
};

/* IIO channels attributes list */
static struct iio_attribute
	ad2s1210_iio_ch_attributes[RESOLVER_CHANNELS][RESOLVER_MAX_ATTR] = {
	{
		AD2S1210_CHN_ATTR("hysteresis", HYSTERESIS_ATTR_ID),
		AD2S1210_CHN_ATTR("hysteresis_available", HYSTERESIS_AVAILABLE_ATTR_ID),
		AD2S1210_CHN_ATTR("label", LABEL_ATTR_ID),
		AD2S1210_CHN_ATTR("raw", RAW_ATTR_ID),
		AD2S1210_CHN_ATTR("scale", SCALE_ATTR_ID),
		END_ATTRIBUTES_ARRAY
	},
	{
		AD2S1210_CHN_ATTR("label", LABEL_ATTR_ID),
		AD2S1210_CHN_ATTR("raw", RAW_ATTR_ID),
		AD2S1210_CHN_ATTR("scale", SCALE_ATTR_ID),
		END_ATTRIBUTES_ARRAY
	},
	{
		AD2S1210_CHN_ATTR("frequency", FREQ_ATTR_ID),
		AD2S1210_CHN_ATTR("frequency_available", FREQ_AVAIL_ATTR_ID),
		AD2S1210_CHN_ATTR("label", LABEL_ATTR_ID),
		END_ATTRIBUTES_ARRAY
	},
};

/* IIO device (global) attributes list */
static struct iio_attribute ad2s1210_iio_global_attributes[] = {
	AD2S1210_CHN_ATTR("sampling_frequency", SAMPLING_FREQ_ATTR_ID),
	END_ATTRIBUTES_ARRAY
};

/* IIO channels info */
static struct iio_channel ad2s1210_iio_channels[RESOLVER_CHANNELS] = {
	AD2S1210_CH("position", 0, IIO_ANGL, false),
	AD2S1210_CH("velocity", 1, IIO_ANGL_VEL, false),
	AD2S1210_CH("altvoltage", 2, IIO_ALTVOLTAGE, true)
};

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment
 */
static bool buf_size_updated;

static const float ad2s1210_velocity_scale[] = {
	AD2S1210_TRACKING_RATE_10BIT,
	AD2S1210_TRACKING_RATE_12BIT,
	AD2S1210_TRACKING_RATE_14BIT,
	AD2S1210_TRACKING_RATE_16BIT,
};


/*!
 * @brief       Getter functions for AD2S1210 attributes
 * @param       device[in]- Pointer to IIO device instance
 * @param       buf[in,out]- IIO input data buffer
 * @param       len[in]- Number of input bytes
 * @param       channel[in] - Input channel
 * @param       priv[in] - Attribute private ID
 * @return      string length in case of success, negative error code otherwise
 */
static int iio_ad2s1210_attr_get(void *device, char *buf, uint32_t len,
				 const struct iio_ch_info *channel,
				 intptr_t priv)
{
	uint16_t data[2];
	uint16_t data_cpu = 0;
	struct ad2s1210_dev *dev = (struct ad2s1210_dev *) device;
	uint32_t active_mask = AD2S1210_POS_MASK;
	float rps_max;
	float scale = AD2S1210_POS_IIO_SCALE;
	int ret;
	uint16_t fexcit;

	if (channel->type == IIO_ANGL_VEL) {
		active_mask = AD2S1210_VEL_MASK;
	}

	switch (priv) {
	case RAW_ATTR_ID:
		ret = ad2s1210_spi_single_conversion(device, active_mask,
						     data, sizeof(data));
		if (ret) {
			return ret;
		}

		data_cpu = data[0];
		no_os_memswap64(&data_cpu, 2, 2);

		if (channel->type == IIO_ANGL_VEL) {
			return sprintf(buf, "%d", (int16_t)data_cpu);
		}

		return sprintf(buf, "%u", data_cpu);
	case SCALE_ATTR_ID:
		if (channel->type == IIO_ANGL_VEL) {
			rps_max = ad2s1210_velocity_scale[dev->resolution];
			scale = 2 * MATH_PI * rps_max / (SHRT_MAX + 1);
		}

		return snprintf(buf, len, "%10f", scale);

	case SAMPLING_FREQ_ATTR_ID:
		return snprintf(buf, len, "%d", SAMPLING_RATE);
	case LABEL_ATTR_ID:
		if (channel->type == IIO_ANGL_VEL) {
			return snprintf(buf, len, "velocity");
		} else if (channel->type == IIO_ALTVOLTAGE) {
			return snprintf(buf, len, "excitation");
		}
		return snprintf(buf, len, "position");

	case HYSTERESIS_ATTR_ID:
		ret = ad2s1210_hysteresis_is_enabled(dev);
		if (ret < 0) {
			return ret;
		}
		return snprintf(buf, len, "%d", ret);

	case HYSTERESIS_AVAILABLE_ATTR_ID:
		return snprintf(buf, len, "0 1");

	case FREQ_ATTR_ID:
		ret = ad2s1210_get_excitation_frequency(dev, &fexcit);
		if (ret) {
			return ret;
		}
		return snprintf(buf, len, "%hu", fexcit);

	case FREQ_AVAIL_ATTR_ID:
		return snprintf(buf, len, "[%d %d %d]", AD2S1210_MIN_EXCIT,
				AD2S1210_STEP_EXCIT, AD2S1210_MAX_EXCIT);
	default:

		break;
	}

	return len;
}

/*!
 * @brief       Setter functions for AD738x attributes
 * @param       device[in]- Pointer to IIO device instance
 * @param       buf[in]- IIO input data buffer
 * @param       len[in]- Number of input bytes
 * @param       channel[in] - Input channel
 * @param       priv[in] - Attribute private ID
 * @return      Positive len in case of success, negative error code otherwise
 */
static int iio_ad2s1210_attr_set(void *device, char *buf, uint32_t len,
				 const struct iio_ch_info *channel,
				 intptr_t priv)
{
	struct ad2s1210_dev *dev = (struct ad2s1210_dev *) device;
	uint16_t fexcit;
	uint8_t hysteresis;
	int ret;

	switch (priv) {
	case RAW_ATTR_ID:
	case SCALE_ATTR_ID:
	case LABEL_ATTR_ID:
	case HYSTERESIS_AVAILABLE_ATTR_ID:
	case FREQ_AVAIL_ATTR_ID:
		/* All read-only attributes */
		break;
	case FREQ_ATTR_ID:
		ret = sscanf(buf, "%hu", &fexcit);
		if (ret != 1) {
			return -EINVAL;
		}

		ret = ad2s1210_reinit_excitation_frequency(dev, fexcit);
		if (ret) {
			return ret;
		}
		break;

	case HYSTERESIS_ATTR_ID:
		ret = sscanf(buf, "%hhu", &hysteresis);
		if (ret != 1) {
			return -EINVAL;
		}

		ret = ad2s1210_set_hysteresis(dev, hysteresis ? true : false);
		if (ret) {
			return ret;
		}

		break;
	default:
		break;
	}

	return len;
}

/*!
 * @brief       Read the debug register value
 * @param       dev[in, out]- Pointer to IIO device instance
 * @param       reg[in]- Register address to read from
 * @param       readval[out]- Pointer to variable to read data into
 * @return      0 in case of success, negative value otherwise
 */
static int32_t iio_ad2s1210_debug_reg_read(void *dev, uint32_t reg,
		uint32_t *val)
{
	return ad2s1210_reg_read(dev,  (uint8_t) reg, (uint8_t *)val);
}

/*!
 * @brief       Write the debug register value
 * @param       dev[in, out]- Pointer to IIO device instance
 * @param       reg[in]- Register address to read from
 * @param       writeval[out]- Pointer to variable to write data into
 * @return      0 in case of success, negative value otherwise
 */
static int32_t iio_ad2s1210_debug_reg_write(void *dev, uint32_t reg,
		uint32_t val)
{
	return ad2s1210_reg_write(dev, (uint8_t)reg, (uint8_t)val);
}

/**
 * @brief	Read buffer data corresponding to AD2S1210 IIO device
 * @param	iio_dev_data[in] - Pointer to IIO device data structure
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t iio_ad2s1210_submit_buffer(struct iio_device_data *iio_dev_data)
{

	uint32_t sample_indx = 0;
	uint32_t nb_of_samples;
	int32_t ret;
	uint16_t data[2];
	uint8_t i;

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	nb_of_samples = iio_dev_data->buffer->size / BYTES_PER_SAMPLE;

	if (!buf_size_updated) {
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	/* Push into circular buffer */
	while (sample_indx < nb_of_samples) {
		ret = ad2s1210_spi_single_conversion(ad2s1210_dev_inst,
						     iio_dev_data->buffer->active_mask,
						     data, sizeof(data));
		if (ret) {
			return ret;
		}
		ret = no_os_cb_write(iio_dev_data->buffer->buf, data,
				     BYTES_PER_SAMPLE * active_chn_count);
		if (ret) {
			return ret;
		}
		sample_indx++;
	}
#endif
	return 0;
}

/**
 * @brief	Prepare for RESOLVER data capture (from device to memory)
 * @param	dev_instance[in] - IIO device instance
 * @param	chn_mask[in] - Channels select mask
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t iio_ad2s1210_prepare_transfer(void *dev_instance,
		uint32_t chn_mask)
{
	int32_t ret;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = iio_trig_enable(ad2s1210_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif
	active_chn_count = __builtin_popcount(chn_mask);

	return 0;
}

/**
 * @brief	Perform tasks before end of current data transfer
 * @param	dev[in] - IIO device instance
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t iio_ad2s1210_end_transfer(void *dev)
{
	int32_t ret;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = iio_trig_disable(ad2s1210_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif
	return 0;
}

/**
 * @brief       Push data into IIO buffer when trigger handler IRQ is invoked
 * @param       iio_dev_data[in] - IIO device data instance
 * @return      0 in case of success or negative value otherwise
 */
int32_t ad2s1210_trigger_handler(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint16_t data[2];

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for
		 * proper alignment of multi-channel IIO buffer data
		 */
		iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
						   iio_dev_data->buffer->bytes_per_scan))
						  * iio_dev_data->buffer->bytes_per_scan;
		buf_size_updated = true;
	}

	ret = ad2s1210_spi_single_conversion(ad2s1210_dev_inst,
					     iio_dev_data->buffer->active_mask,
					     data, sizeof(data));
	if (ret) {
		return ret;
	}

	ret = no_os_cb_write(iio_dev_data->buffer->buf, data,
			     BYTES_PER_SAMPLE * active_chn_count);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief	Init for reading/writing and parameterization of a
 *			ad2s1210 IIO device
 * @param	desc[in,out] - IIO device descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad2s1210_iio_param_init(struct iio_device **desc)
{
	struct iio_device *iio_ad2s1210_inst;

	iio_ad2s1210_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad2s1210_inst) {
		return -ENOMEM;
	}

	iio_ad2s1210_inst->num_ch = NO_OS_ARRAY_SIZE(ad2s1210_iio_channels);
	iio_ad2s1210_inst->channels = ad2s1210_iio_channels;
	iio_ad2s1210_inst->attributes = ad2s1210_iio_global_attributes;

	iio_ad2s1210_inst->submit = iio_ad2s1210_submit_buffer;
	iio_ad2s1210_inst->pre_enable = iio_ad2s1210_prepare_transfer;
	iio_ad2s1210_inst->post_disable = iio_ad2s1210_end_transfer;
	iio_ad2s1210_inst->debug_reg_read = iio_ad2s1210_debug_reg_read;
	iio_ad2s1210_inst->debug_reg_write = iio_ad2s1210_debug_reg_write;
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_ad2s1210_inst->trigger_handler = ad2s1210_trigger_handler;
#endif

	*desc = iio_ad2s1210_inst;

	return 0;
}

/**
 * @brief       Initialization of AD2S1210 IIO hardware trigger parameters
 * @param       desc[in,out] - IIO hardware trigger descriptor
 * @return      0 in case of success, negative error code otherwise
 */
static int32_t ad2s1210_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	int32_t ret;
	struct iio_hw_trig_init_param ad2s1210_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad2s1210_hw_trig_init_params.irq_id = TRIGGER_INT_ID;
	ad2s1210_hw_trig_init_params.name = AD2S1210_IIO_TRIGGER_NAME;
	ad2s1210_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad2s1210_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad2s1210_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad2s1210_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad2s1210_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad2s1210_hw_trig_init_params.iio_desc = ad2s1210_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad2s1210_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/**
 * @brief	Initialize the IIO interface for AD2S1210 IIO device
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad2s1210_iio_initialize(void)
{
	struct iio_ctx_attr *context_attributes;
	int32_t init_status;

	/* IIO device descriptors */
	struct iio_device *iio_ad2s1210_dev;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	static struct iio_trigger ad2s1210_iio_trig_desc = {
		.is_synchronous = true,
	};

	/* IIO trigger init parameters */
	static struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad2s1210_iio_trig_desc,
		.name = AD2S1210_IIO_TRIGGER_NAME,
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

	/* Initialize AD2S1210 device and peripheral interface */
	init_status = ad2s1210_init(&ad2s1210_dev_inst, &ad2s1210_init_params);
	if (init_status) {
		return init_status;
	}

	init_status = ad2s1210_iio_param_init(&iio_ad2s1210_dev);
	if (init_status) {
		return init_status;
	}

	/* The setup currently uses fly wires through the expanstion board
	 * that has its own eeprom. Becuase of the autodetect mechanism
	 * of eeprom we cannot choose which eeprom to read on the i2c bus.
	 * hardcode these context attributes until a proper board is
	 * available.
	 */
	context_attributes = (struct iio_ctx_attr *)calloc(
				     NUM_CTX_ATTR, sizeof(*context_attributes));
	context_attributes[0].name = "hw_carrier";
	context_attributes[0].value = STR(HW_CARRIER_NAME);
	context_attributes[1].name = "hw_mezzanine";
	context_attributes[1].value = HW_MEZZANINE_NAME;
	context_attributes[2].name = "hw_name";
	context_attributes[2].value = HW_NAME;
	context_attributes[3].name = "hw_vendor";
	context_attributes[3].value = HW_VENDOR;


	iio_init_params.ctx_attrs = context_attributes;
	iio_init_params.nb_ctx_attr = NUM_CTX_ATTR;

	iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
	iio_device_init_params[0].raw_buf = data_buffer;
	iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;
	iio_device_init_params[0].dev = ad2s1210_dev_inst;
	iio_device_init_params[0].dev_descriptor = iio_ad2s1210_dev;

	iio_init_params.nb_devs++;
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_init_params.nb_trigs++;
#endif

	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_desc;
	iio_init_params.devs = iio_device_init_params;
	init_status = iio_init(&ad2s1210_iio_desc, &iio_init_params);
	if (init_status) {
		return init_status;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Initialize the AD2S1210 IIO trigger specific parameters */
	init_status = ad2s1210_iio_trigger_param_init(&ad2s1210_hw_trig_desc);
	if (init_status) {
		return init_status;
	}

	/* Initialize the PWM trigger source for periodic RESOLVER sampling */
	init_status = init_pwm_trigger();
	if (init_status) {
		return init_status;
	}
#endif
	return 0;
}

/**
 * @brief	Run the AD2S1210 IIO event handler
 * @return	none
 * @details	This function monitors the new IIO client event
 */
void ad2s1210_iio_event_handler(void)
{
	(void)iio_step(ad2s1210_iio_desc);
}
