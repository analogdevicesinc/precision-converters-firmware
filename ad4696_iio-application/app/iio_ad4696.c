/***************************************************************************//**
 *   @file    iio_ad4696.c
 *   @brief   Implementation of AD4696 IIO application interfaces
 *   @details This module acts as an interface for AD4696 IIO application
********************************************************************************
 * Copyright (c) 2021-22 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <inttypes.h>
#include <string.h>
#include <math.h>

#include "app_config.h"
#include "iio_ad4696.h"
#include "ad4696_support.h"
#include "ad4696_user_config.h"
#include "no_os_error.h"
#include "no_os_util.h"
#include "no_os_gpio.h"
#include "no_os_pwm.h"
#include "no_os_print_log.h"
#include "iio_trigger.h"

/******** Forward declaration of getter/setter functions ********/
static int iio_ad469x_attr_get(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv);

static int iio_ad469x_attr_set(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv);

static int iio_ad469x_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int iio_ad469x_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/
#define AD469X_CHN_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = iio_ad469x_attr_get,\
		.store = iio_ad469x_attr_set\
}

#define AD469X_CHN_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = iio_ad469x_attr_available_get,\
	.store = iio_ad469x_attr_available_set\
}

#define AD469X_IIO_CH(_name, _idx) {\
	.name = _name #_idx, \
	.ch_type = IIO_VOLTAGE,\
	.ch_out = false,\
	.indexed = true,\
	.channel = _idx,\
	.scan_index = _idx,\
	.scan_type = &ad469x_iio_scan_type,\
	.attributes = iio_ad469x_ch_attributes\
}

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
#define AD4696_IIO_TRIGGER_NAME		"ad469x_iio_trigger"

/* ADC Raw to Voltage conversion default scale factor for IIO client */
#if defined(PSEUDO_BIPOLAR_MODE)
/* Device supports pseudo-bipolar mode only with INX- = Vref / 2 */
#define DEFAULT_SCALE		(((DEFAULT_VREF  / 2) / ADC_MAX_COUNT_BIPOLAR) * 1000)
#else
#define DEFAULT_SCALE		((DEFAULT_VREF / ADC_MAX_COUNT_UNIPOLAR) * 1000)
#endif

/* Bytes per sample. This count should divide the total 256 bytes into 'n' equivalent
 * ADC samples as IIO library requests only 256bytes of data at a time in a given
 * data read query.
 * For 1 to 8-bit ADC, bytes per sample = 1 (2^0)
 * For 9 to 16-bit ADC, bytes per sample = 2 (2^1)
 * For 17 to 32-bit ADC, bytes per sample = 4 (2^2)
 **/
#define	BYTES_PER_SAMPLE	sizeof(uint16_t)	// For ADC resolution of 16-bits

/* Number of data storage bits (needed for IIO client to plot ADC data) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

#define REGISTER_MAX_VAL  	 0x017F

/* Timeout count to avoid stuck into potential infinite loop while checking
 * for new data into an acquisition buffer. The actual timeout factor is determined
 * through 'sampling_frequency' attribute of IIO app, but this period here makes sure
 * we are not stuck into a forever loop in case data capture is interrupted
 * or failed in between.
 * Note: This timeout factor is dependent upon the MCU clock frequency. Below timeout
 * is tested for SDP-K1 platform @180Mhz default core clock */
#define BUF_READ_TIMEOUT	0xffffffff

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* Pointer to the struct representing the AD4696 IIO device */
struct ad469x_dev *p_ad469x_dev = NULL;

/* Variable to store the sampling rate */
static uint32_t ad469x_sampling_frequency = DEFAULT_SAMPLING_RATE;

/* IIO interface descriptor */
static struct iio_desc *p_ad469x_iio_desc;

/* AD4696 IIO device descriptor */
static struct iio_device *p_iio_ad469x_dev;

/* AD4696 IIO hw trigger descriptor */
static struct iio_hw_trig *ad4696_hw_trig_desc;

/* Number of active channels in any data buffer read request */
static volatile uint8_t num_of_active_channels = 0;

/* Flag for checking the end of conversion in burst mode */
static volatile bool conversion_flag = false;

/* Flag to indicate data capture status */
static volatile bool start_data_capture = false;

/* Flag to indicate conversion mode status */
static volatile bool exit_conv_mode;

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/* AD469X attribute unique IDs */
enum ad469x_attribute_ids {
	ADC_RAW,
	ADC_SCALE,
	ADC_OFFSET,

	ADC_SAMPLING_FREQUENCY,
};

/* IIOD channels configurations */
struct scan_type ad469x_iio_scan_type = {
#if (DEFAULT_POLARITY_MODE == PSEUDO_BIPOLAR_MODE)
	.sign = 's',
#else
	.sign = 'u',
#endif
	.realbits = CHN_STORAGE_BITS,
	.storagebits = ADC_RESOLUTION,
	.shift = 0,
	.is_big_endian = false
};

/* AD469X device channel attributes list */
static struct iio_attribute iio_ad469x_ch_attributes[] = {
	AD469X_CHN_ATTR("raw", ADC_RAW),
	AD469X_CHN_ATTR("scale", ADC_SCALE),
	AD469X_CHN_ATTR("offset", ADC_OFFSET),
	END_ATTRIBUTES_ARRAY,
};

/* AD469X device (global) attributes list */
static struct iio_attribute iio_ad469x_global_attributes[] = {
	AD469X_CHN_ATTR("sampling_frequency", ADC_SAMPLING_FREQUENCY),
	END_ATTRIBUTES_ARRAY,
};

static struct iio_channel iio_ad469x_channels[] = {
	/* 24-bit ADC Pseudo Differential Input Channels (Count= 8) */
	AD469X_IIO_CH("Chn", 0),
	AD469X_IIO_CH("Chn", 1),
	AD469X_IIO_CH("Chn", 2),
	AD469X_IIO_CH("Chn", 3),
	AD469X_IIO_CH("Chn", 4),
	AD469X_IIO_CH("Chn", 5),
	AD469X_IIO_CH("Chn", 6),
	AD469X_IIO_CH("Chn", 7),
	AD469X_IIO_CH("Chn", 8),
	AD469X_IIO_CH("Chn", 9),
	AD469X_IIO_CH("Chn", 10),
	AD469X_IIO_CH("Chn", 11),
	AD469X_IIO_CH("Chn", 12),
	AD469X_IIO_CH("Chn", 13),
	AD469X_IIO_CH("Chn", 14),
	AD469X_IIO_CH("Chn", 15)
};

/* Scale value per channel */
static float ad469x_attr_scale_val[NO_OF_CHANNELS] = {
	DEFAULT_SCALE,
	DEFAULT_SCALE,
	DEFAULT_SCALE,
	DEFAULT_SCALE,
	DEFAULT_SCALE,
	DEFAULT_SCALE,
	DEFAULT_SCALE,
	DEFAULT_SCALE,
	DEFAULT_SCALE,
	DEFAULT_SCALE,
	DEFAULT_SCALE,
	DEFAULT_SCALE,
	DEFAULT_SCALE,
	DEFAULT_SCALE,
	DEFAULT_SCALE,
	DEFAULT_SCALE
};

/* AD4696 IIOD debug attributes list */
static struct iio_attribute ad469x_debug_attributes[] = {
	END_ATTRIBUTES_ARRAY
};

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/
/*!
 * @brief	Getter/Setter for the raw, offset and scale attribute value
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written
 */
static int iio_ad469x_attr_get(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	uint32_t adc_data_raw = 0;
	/* In pseudo-bipolar mode the offset is determined depending upon
	* the raw adc value. Hence it has been defined as static */
	static int32_t offset = 0;
	int32_t ret;

	if (buf == NULL) {
		return -ENOMEM;
	}

	switch (priv) {
	case ADC_RAW:
		ret = ad469x_read_single_sample(p_ad469x_dev, channel->ch_num, &adc_data_raw);
		if (ret) {
			return ret;
		}
#if (DEFAULT_POLARITY_MODE == PSEUDO_BIPOLAR_MODE)
		if (adc_data_raw >= ADC_MAX_COUNT_BIPOLAR) {
			offset = -ADC_MAX_COUNT_UNIPOLAR;
		} else {
			offset = 0;
		}
#endif
		return snprintf(buf, len, "%d", adc_data_raw);

	case ADC_SCALE:
		return (ssize_t) sprintf(buf, "%g", ad469x_attr_scale_val[channel->ch_num]);

	case ADC_OFFSET:
		return sprintf(buf, "%d", offset);

	case ADC_SAMPLING_FREQUENCY:
		return snprintf(buf, len, "%ld", ad469x_sampling_frequency);

	default:
		break;
	}

	return -EINVAL;
}

/*!
 * @brief	Setter function for AD469X attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of expected bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int iio_ad469x_attr_set(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	int ret;

	switch (priv) {
	/****************** ADC global setters ******************/

	/* These Attributes are only read only */
	case ADC_RAW:
	case ADC_OFFSET:
	case ADC_SCALE:
		break;

	case ADC_SAMPLING_FREQUENCY:
		ad469x_sampling_frequency = no_os_str_to_uint32(buf);

		ret = no_os_pwm_enable(pwm_desc);
		if (ret) {
			return ret;
		}

		ret = no_os_pwm_set_period(pwm_desc,
					   CONV_TRIGGER_PERIOD_NSEC(ad469x_sampling_frequency));
		if (ret) {
			return ret;
		}

		ret = no_os_pwm_set_duty_cycle(pwm_desc,
					       CONV_TRIGGER_DUTY_CYCLE_NSEC(ad469x_sampling_frequency));
		if (ret) {
			return ret;
		}

		ret = no_os_pwm_disable(pwm_desc);
		if (ret) {
			return ret;
		}

		return len;

	default:
		break;
	}

	return len;
}

/*!
 * @brief	Attribute available getter function for AD469X attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int iio_ad469x_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	return len;
}

/*!
 * @brief	Attribute available setter function for AD469X attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int iio_ad469x_attr_available_set(void *device,
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
static int32_t iio_ad469x_debug_reg_read(void *dev,
		uint32_t reg,
		uint32_t *readval)
{
	int32_t ret;

	if (!readval || (reg > REGISTER_MAX_VAL)) {
		return -EINVAL;
	}

	ret = ad469x_spi_reg_read(p_ad469x_dev, reg, (uint8_t *)readval);
	if (NO_OS_IS_ERR_VALUE(ret)) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Write the debug register value
 * @param	dev[in, out]- Pointer to IIO device instance
 * @param	reg[in]- Register address to write
 * @param	writeval[out]- Variable storing data to write
 * @return	0 in case of success, negative value otherwise
 */
static int32_t iio_ad469x_debug_reg_write(void *dev,
		uint32_t reg,
		uint32_t writeval)
{
	int32_t ret;

	if (reg > REGISTER_MAX_VAL) {
		return -EINVAL;
	}

	ret = ad469x_spi_reg_write(p_ad469x_dev, reg, (uint8_t)writeval);
	if (NO_OS_IS_ERR_VALUE(ret)) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Start a data capture in continuous/burst mode
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad4696_adc_start_data_capture(void)
{
	int32_t ret;
	start_data_capture = true;
	exit_conv_mode = false;

	ret = no_os_pwm_enable(pwm_desc);
	if (ret) {
		return ret;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = iio_trig_enable(ad4696_hw_trig_desc);
	if (ret) {
		return ret;
	}
#else
	ret = no_os_irq_enable(trigger_irq_desc, TRIGGER_INT_ID);
	if (ret) {
		return ret;
	}
#endif

	/* Enter into conversion mode */
	ret = ad469x_enter_conversion_mode(p_ad469x_dev);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Stop a data capture from continuous/burst mode
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad4696_adc_stop_data_capture(void)
{
	int32_t ret;
	uint32_t timeout = BUF_READ_TIMEOUT;
	start_data_capture = false;

	while (!exit_conv_mode && (timeout > 0)) {
		timeout--;
	};

	if (timeout == 0) {
		/* This returns the empty buffer */
		return -EIO;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Disable the interrupt */
	ret = iio_trig_enable(ad4696_hw_trig_desc);
	if (ret) {
		return ret;
	}
#else
	ret = no_os_irq_disable(trigger_irq_desc, TRIGGER_INT_ID);
	if (ret) {
		return ret;
	}
#endif

	/* Stop Generating PWM signal */
	ret = no_os_pwm_disable(pwm_desc);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief  Prepares the device for data transfer.
 * @param  dev[in, out]- Application descriptor.
 * @param  mask[in]- Number of bytes to transfer.
 * @return 0 in case of success, error code otherwise.
 */
static int32_t iio_ad469x_prepare_transfer(void *dev, uint32_t mask)
{
	uint32_t ch_mask = 0x1;
	uint8_t chn;
	int32_t ret;
	buf_size_updated = false;
	num_of_active_channels = 0;

	/* Reset the lower byte of the standard sequencer configuration register*/
	ret = ad469x_spi_reg_write(p_ad469x_dev,
				   AD469x_REG_SEQ_LB,
				   AD469x_SEQ_CHANNELS_RESET);
	if (ret) {
		return ret;
	}

	/* Reset the upper byte of the standard sequencer configuration register*/
	ret = ad469x_spi_reg_write(p_ad469x_dev,
				   AD469x_REG_SEQ_UB,
				   AD469x_SEQ_CHANNELS_RESET);
	if (ret) {
		return ret;
	}

	/* Write the lower byte of the channel mask to the lower byte
	* of the standard sequencer configuration register
	* */
	ret = ad469x_spi_reg_write(p_ad469x_dev,
				   AD469x_REG_SEQ_LB,
				   AD469x_SEQ_LB_CONFIG(mask));
	if (ret) {
		return ret;
	}

	/* Write the upper byte of the channel mask to the upper byte
	 * of the standard sequencer configuration register
	 * */
	ret = ad469x_spi_reg_write(p_ad469x_dev,
				   AD469x_REG_SEQ_UB,
				   AD469x_SEQ_UB_CONFIG(mask));
	if (ret) {
		return ret;
	}

	/* Updates the count of total number of active channels */
	for (chn = 0; chn < NO_OF_CHANNELS; chn++) {
		if (mask & ch_mask) {
			num_of_active_channels++;
		}
		ch_mask <<= 1;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = ad4696_adc_start_data_capture();
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief  Terminate current data transfer
 * @param  dev[in, out]- Application descriptor.
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t iio_ad469x_end_transfer(void *dev)
{
	int32_t ret;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)

	ret = ad4696_adc_stop_data_capture();
	if (ret) {
		return ret ;
	}

	return 0;
#endif
}

/**
 * @brief Push data into IIO buffer when trigger handler IRQ is invoked
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
int32_t ad469x_trigger_handler(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint8_t adc_data[2] = { 0 };

	if (start_data_capture) {
		if (!buf_size_updated) {
			/* Update total buffer size according to bytes per scan for proper
			 * alignment of multi-channel IIO buffer data */
			iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
							   iio_dev_data->buffer->bytes_per_scan)) * iio_dev_data->buffer->bytes_per_scan;
			buf_size_updated = true;
		}

		/* Read the sample for channel which has been sampled recently */
		ret = no_os_spi_write_and_read(p_ad469x_dev->spi_desc,
					       adc_data, BYTES_PER_SAMPLE);
		if (ret) {
			return -EIO;
		}

		no_os_swap(adc_data[0], adc_data[1]);

		return no_os_cb_write(iio_dev_data->buffer->buf,
				      adc_data,
				      BYTES_PER_SAMPLE);
	} else {
		/* Enter into register mode or exit from conversion mode */
		ad469x_exit_conversion_mode(p_ad469x_dev);
		exit_conv_mode = true;
	}

	return 0;
}

/*!
 * @brief Interrupt Service Routine to monitor end of conversion event.
 * @param context[in] - Callback context (unused)
 * @return none
 */
void burst_capture_callback(void *context)
{
	conversion_flag = true;
	
	if (!start_data_capture) {
		/* Enter into register mode or exit from conversion mode */
		ad469x_exit_conversion_mode(p_ad469x_dev);
		exit_conv_mode = true;
	}
}

/**
 * @brief Read buffer data corresponding to AD4696 IIO device.
 * @param [in, out] iio_dev_data - Device descriptor.
 * @return Number of samples read.
 */
static int32_t iio_ad469x_submit_samples(struct iio_device_data *iio_dev_data)
{
	int32_t ret = 0;

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	uint32_t timeout = BUF_READ_TIMEOUT;
	uint32_t sample_index = 0;
	uint8_t adc_sample[2] = { 0 };
	uint32_t nb_of_samples;

	if (!iio_dev_data) {
		return -EINVAL;
	}

	nb_of_samples = iio_dev_data->buffer->size / BYTES_PER_SAMPLE;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = nb_of_samples * BYTES_PER_SAMPLE *
						  num_of_active_channels;
		buf_size_updated = true;
	}

	/* Start data capture */
	ret = ad4696_adc_start_data_capture();
	if (ret) {
		return ret;
	}

	while (sample_index < nb_of_samples) {
		/* Check for status of conversion flag */
		while (!conversion_flag && timeout > 0) {
			timeout--;
		}

		if (timeout <= 0) {
			return -ETIMEDOUT;
		}

		conversion_flag = false;

		/* Read data over spi interface (in continuous read mode) */
		ret = no_os_spi_write_and_read(p_ad469x_dev->spi_desc,
					       adc_sample,
					       BYTES_PER_SAMPLE);
		if (ret) {
			return -EIO;
		}

		no_os_swap(adc_sample[0], adc_sample[1]);

		ret = no_os_cb_write(iio_dev_data->buffer->buf,
				     adc_sample,
				     BYTES_PER_SAMPLE);
		if (ret) {
			return -EIO;
		}

		sample_index++;
		memset(adc_sample, 0, BYTES_PER_SAMPLE);
	}

	/* Stop data capture */
	ret = ad4696_adc_stop_data_capture();
	if (ret) {
		return ret;
	}

#endif
	return 0;
}

/*********************************************************
 *               IIO Attributes and Structures
 ********************************************************/
/**
 * @brief	Init for reading/writing and parameterization of a
 * 			AD4696 IIO device
 * @param 	desc[in,out] - IIO device descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t iio_ad4696_init(struct iio_device **desc)
{
	struct iio_device *iio_ad469x_inst;

	iio_ad469x_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad469x_inst) {
		return -EINVAL;
	}

	iio_ad469x_inst->num_ch = NO_OS_ARRAY_SIZE(iio_ad469x_channels);
	iio_ad469x_inst->channels = iio_ad469x_channels;
	iio_ad469x_inst->attributes = iio_ad469x_global_attributes;
	iio_ad469x_inst->debug_attributes = ad469x_debug_attributes;

	iio_ad469x_inst->submit = iio_ad469x_submit_samples;
	iio_ad469x_inst->pre_enable = iio_ad469x_prepare_transfer;
	iio_ad469x_inst->post_disable = iio_ad469x_end_transfer;
	iio_ad469x_inst->read_dev = NULL;
	iio_ad469x_inst->write_dev = NULL;
	iio_ad469x_inst->debug_reg_read = iio_ad469x_debug_reg_read;
	iio_ad469x_inst->debug_reg_write = iio_ad469x_debug_reg_write;
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_ad469x_inst->trigger_handler = ad469x_trigger_handler;
#endif

	*desc = iio_ad469x_inst;

	return 0;
}

/**
 * @brief	Initialization of AD4696 IIO hardware trigger specific parameters
 * @param 	desc[in,out] - IIO hardware trigger descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad469x_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	int32_t ret;
	struct iio_hw_trig_init_param ad469x_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad469x_hw_trig_init_params.irq_id = TRIGGER_INT_ID;
	ad469x_hw_trig_init_params.name = AD4696_IIO_TRIGGER_NAME;
	ad469x_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad469x_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad469x_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad469x_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad469x_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad469x_hw_trig_init_params.iio_desc = &p_ad469x_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad469x_hw_trig_init_params);
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
static int32_t iio_ad469x_remove(struct iio_desc *desc)
{
	int32_t status;

	if (!desc) {
		return -EINVAL;
	}

	status = iio_remove(desc);
	if (status) {
		return status;
	}

	return 0;
}

/**
 * @brief Release resources allocated for IIO device
 * @param desc[in] - IIO device descriptor
 * @return 0 in case of success, negative value otherwise
 */
int32_t ad4696_iio_initialize(void)
{
	int32_t init_status;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	static struct iio_trigger ad469x_iio_trig_desc = {
		.is_synchronous = true,
		.enable = NULL,
		.disable = NULL
	};

	static struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad469x_iio_trig_desc,
		.name = AD4696_IIO_TRIGGER_NAME,
	};
#endif

	/* IIO interface init parameters */
	struct iio_init_param  iio_init_params = {
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

	/* Initialize AD4696 device and peripheral interface */
	init_status = ad469x_init(&p_ad469x_dev, &ad4696_init_str);
	if (init_status) {
		return init_status;
	}

	/* Configures the polarity mode */
#if (DEFAULT_POLARITY_MODE == PSEUDO_BIPOLAR_MODE)
	init_status = ad469x_polarity_mode_select(p_ad469x_dev,
			AD469x_PSEUDO_BIPOLAR_MODE);
#else
	init_status = ad469x_polarity_mode_select(p_ad469x_dev,
			AD469x_UNIPOLAR_MODE);
#endif
	if (init_status) {
		return init_status;
	}

	/* Configure reference control register */
	init_status = ad469x_reference_config(p_ad469x_dev);
	if (init_status) {
		return init_status;
	}

	/* Register and initialize the AD4696 device into IIO interface */
	init_status = iio_ad4696_init(&p_iio_ad469x_dev);
	if (init_status) {
		return init_status;
	}

	/* Register and initialize the AD4696 device into IIO interface */
	init_status = iio_ad4696_init(&p_iio_ad469x_dev);
	if (init_status) {
		return init_status;
	}

	/* Initialize the IIO interface */
	iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
	iio_device_init_params[0].raw_buf = adc_data_buffer;
	iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;

	iio_device_init_params[0].dev = p_ad469x_dev;
	iio_device_init_params[0].dev_descriptor = p_iio_ad469x_dev;

	iio_init_params.nb_devs++;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	init_status = ad469x_iio_trigger_param_init(&ad4696_hw_trig_desc);
	if (init_status) {
		return init_status;
	}

	iio_init_params.nb_trigs++;
#endif

	init_status = init_pwm();
	if (init_status) {
		return init_status;
	}

	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_desc;
	iio_init_params.devs = iio_device_init_params;
	init_status = iio_init(&p_ad469x_iio_desc, &iio_init_params);
	if (init_status) {
		pr_err("IIO Init Failed");
		iio_ad469x_remove(p_ad469x_iio_desc);
		return -ENOSYS;
	}

	return 0;
}

/**
 * @brief 	Run the ad469x IIO event handler
 * @return	None
 */
void ad4696_iio_event_handler(void)
{
	iio_step(p_ad469x_iio_desc);
}