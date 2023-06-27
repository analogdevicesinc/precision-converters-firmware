/***************************************************************************//**
 *   @file    ad738x_iio.c
 *   @brief   AD738x IIO application interface module
********************************************************************************
 * Copyright (c) 2022-23 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "app_config.h"
#include "ad738x_iio.h"
#include "ad738x_user_config.h"
#include "common.h"
#include "iio.h"
#include "iio_types.h"
#include "iio_trigger.h"
#include "no_os_error.h"
#include "no_os_delay.h"

/******** Forward declaration of getter/setter functions ********/
static int iio_ad738x_attr_get(void *device, char *buf, uint32_t len,
			       const struct iio_ch_info *channel, intptr_t priv);

static int iio_ad738x_attr_set(void *device, char *buf, uint32_t len,
			       const struct iio_ch_info *channel, intptr_t priv);

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

#define AD738X_CHN_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = iio_ad738x_attr_get,\
	.store = iio_ad738x_attr_set\
}

#define AD738X_CH(_name, _idx, _type) {\
	.name = _name, \
	.ch_type = _type,\
	.ch_out = false,\
	.indexed = true,\
	.channel = _idx,\
	.scan_index = _idx,\
	.scan_type = &chn_scan,\
	.attributes = ad738x_iio_ch_attributes\
}

/*	Number of IIO devices */
#define NUM_OF_IIO_DEVICES	1

/* Bytes per sample for IIO channel scan structure
 * For 1 to 8-bit ADC, bytes per sample = 1 (2^0)
 * For 9 to 16-bit ADC, bytes per sample = 2 (2^1)
 * For 17 to 32-bit ADC, bytes per sample = 4 (2^2)
 **/
#define	BYTES_PER_SAMPLE	2

/* Number of actual data storage bits required for IIO client */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

/* Minimum sampling frequency supported/configured in the firmware.
 * Note: This is not an actual device sampling frequency.
 * It is just used for IIO oscilloscope timeout calculations. */
#define AD738X_MIN_SAMPLING_FREQ	(100 / ADC_CHANNELS)

/* Default IIO scale factor for raw to voltage conversion */
#define	AD738X_DEF_IIO_SCALE	(ADC_REF_VOLTAGE / ADC_MAX_COUNT_BIPOLAR) * 1000

/* IIO trigger name */
#define AD738X_IIO_TRIGGER_NAME		"ad738x_iio_trigger"

/* ADC data buffer size */
#if defined(USE_SDRAM)
#define adc_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(32768)		// 32kbytes
static int8_t adc_data_buffer[DATA_BUFFER_SIZE] = { 0 };
#endif

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* Pointer to the structure representing the AD738x IIO device */
struct ad738x_dev *ad738x_dev_inst = NULL;

/* AD738x IIO interface descriptor */
static struct iio_desc *p_ad738x_iio_desc;

/* AD738x IIO hw trigger descriptor */
static struct iio_hw_trig *ad738x_hw_trig_desc;

/* IIO scale attribute value per channel */
static float attr_scale_val[ADC_CHANNELS] = {
	AD738X_DEF_IIO_SCALE, AD738X_DEF_IIO_SCALE
};

/* IIO channels scan structure */
static struct scan_type chn_scan = {
	.sign = 's',
	.realbits = ADC_RESOLUTION,
	.storagebits = CHN_STORAGE_BITS,
};

/* IIO attributes ID */
enum ad738x_attribute_id {
	RAW_ATTR_ID,
	SCALE_ATTR_ID,
	OFFSET_ATTR_ID,
	SAMPLING_FREQ_ATTR_ID,
};

/* IIO channels attributes list */
static struct iio_attribute ad738x_iio_ch_attributes[] = {
	AD738X_CHN_ATTR("raw", RAW_ATTR_ID),
	AD738X_CHN_ATTR("scale", SCALE_ATTR_ID),
	AD738X_CHN_ATTR("offset", OFFSET_ATTR_ID),
	END_ATTRIBUTES_ARRAY
};

/* IIO device (global) attributes list */
static struct iio_attribute ad738x_iio_global_attributes[] = {
	AD738X_CHN_ATTR("sampling_frequency", SAMPLING_FREQ_ATTR_ID),
	END_ATTRIBUTES_ARRAY
};

/* IIO channels info */
static struct iio_channel ad738x_iio_channels[ADC_CHANNELS] = {
	AD738X_CH("Chn0", 0, IIO_VOLTAGE),
	AD738X_CH("Chn1", 1, IIO_VOLTAGE)
};

/* Current channel index */
static volatile uint8_t chn_index;

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
 * @brief	Getter functions for AD738x attributes
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in,out]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	0 in case of success, negative error code otherwise
 * @Note	This sampling_frequency attribute is used to define the
 *			timeout period in IIO client during data capture.
 *			Timeout (1 chn) = (requested samples * sampling frequency) + 1sec
 *			Timeout (n chns) = ((requested samples * n) / sampling frequency) + 1sec
 *			e.g. If sampling frequency = 31.5KSPS, requested samples = 4000, n=1min or 8max
 *			Timeout (1 chn) = (4000 / 315000) + 1 = ~1.13sec
 *			Timeout (8 chns) = ((4000 * 8) / 315000) + 1 = ~2.01sec
 */
static int iio_ad738x_attr_get(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	static uint16_t adc_data_raw[ADC_CHANNELS];
	int32_t offset;
	int32_t ret;

	switch (priv) {
	case RAW_ATTR_ID:
		ret = ad738x_spi_single_conversion(device, adc_data_raw);
		if (ret) {
			break;
		}
		return sprintf(buf, "%d", adc_data_raw[channel->ch_num]);

	case SCALE_ATTR_ID:
		return snprintf(buf, len, "%10f", attr_scale_val[channel->ch_num]);

	case OFFSET_ATTR_ID:
		/* For IIO clients, the raw to voltage conversion happens
		 * using formula: voltage = (adc_raw + offset) * scale
		 * Offset is determined based on the coding scheme of device.
		 * AD738x uses fixed 2's' complement data format.
		 **/
		if (adc_data_raw[channel->ch_num] >= ADC_MAX_COUNT_BIPOLAR) {
			offset = -ADC_MAX_COUNT_UNIPOLAR;
		} else {
			offset = 0;
		}
		return sprintf(buf, "%d", offset);

	case SAMPLING_FREQ_ATTR_ID:
		/* Sampling frequency for IIO oscilloscope timeout purpose.
		 * Does not indicate an actual sampling rate of device.
		 * Refer the 'note' in function description above for timeout calculations */
		return sprintf(buf, "%d", AD738X_MIN_SAMPLING_FREQ);

	default:
		break;
	}

	return len;
}

/*!
 * @brief	Setter functions for AD738x attributes
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	0 in case of success, negative error code otherwise
 */
static int iio_ad738x_attr_set(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	switch (priv) {
	case RAW_ATTR_ID:
	case SCALE_ATTR_ID:
	case OFFSET_ATTR_ID:
	case SAMPLING_FREQ_ATTR_ID:
		/* All read-only attributes */
		break;

	default:
		break;
	}

	return len;
}

/**
 * @brief	Read buffer data corresponding to AD738x ADC IIO device
 * @param	iio_dev_data[in] - IIO device data instance
 * @return	0 in case of success or negative value otherwise
 */
static int32_t iio_ad738x_submit_buffer(struct iio_device_data *iio_dev_data)
{
	uint32_t sample_indx = 0;
	uint32_t nb_of_samples;
	uint16_t adc_raw[ADC_CHANNELS];
	int32_t ret;
	uint8_t chn;
	uint8_t mask;

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	nb_of_samples = iio_dev_data->buffer->size / BYTES_PER_SAMPLE;
	chn_index = 0;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	while (sample_indx < nb_of_samples) {
		ret = ad738x_spi_single_conversion(ad738x_dev_inst, adc_raw);
		if (ret) {
			return ret;
		}

		/* Push into circular buffer */
		mask = 0x1;
		for (chn = 0; chn < ADC_CHANNELS; chn++) {
			if (iio_dev_data->buffer->active_mask & mask) {
				ret = no_os_cb_write(iio_dev_data->buffer->buf, &adc_raw[chn],
						     BYTES_PER_SAMPLE);
				if (ret) {
					return ret;
				}
			}

			mask <<= 1;
		}

		sample_indx++;
	}
#endif

	return 0;
}

/**
 * @brief	Prepare for data transfer
 * @param	dev[in] - IIO device instance
 * @param	ch_mask[in] - Channels select mask
 * @return	0 in case of success or negative value otherwise
 */
static int32_t iio_ad738x_prepare_transfer(void *dev,
		uint32_t ch_mask)
{
	int32_t ret;
	chn_index = 0;

	printf("open device\r\n");

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = iio_trig_enable(ad738x_hw_trig_desc);
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
static int32_t iio_ad738x_end_transfer(void *dev)
{
	int32_t ret;

	printf("close device\r\n");

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = iio_trig_disable(ad738x_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief	Push data into IIO buffer when trigger handler IRQ is invoked
 * @param	iio_dev_data[in] - IIO device data instance
 * @return	0 in case of success or negative value otherwise
 */
int32_t ad738x_trigger_handler(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint16_t adc_raw[ADC_CHANNELS];
	uint8_t mask = 0x1;
	uint8_t chn;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
						   iio_dev_data->buffer->bytes_per_scan)) * iio_dev_data->buffer->bytes_per_scan;
		buf_size_updated = true;
	}

	ret = ad738x_spi_single_conversion(ad738x_dev_inst, adc_raw);
	if (ret) {
		return ret;
	}

	/* Push into circular buffer */
	for (chn = 0; chn < ADC_CHANNELS; chn++) {
		if (iio_dev_data->buffer->active_mask & mask) {
			ret = no_os_cb_write(iio_dev_data->buffer->buf, &adc_raw[chn],
					     BYTES_PER_SAMPLE);
			if (ret) {
				return ret;
			}
		}

		mask <<= 1;
	}

	return 0;
}

/**
 * @brief	Initialization of AD738x specific IIO parameters
 * @param 	desc[in,out] - IIO device descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad738x_iio_param_init(struct iio_device **desc)
{
	struct iio_device *iio_ad738x_inst;

	iio_ad738x_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad738x_inst) {
		return -ENOMEM;
	}

	iio_ad738x_inst->num_ch = NO_OS_ARRAY_SIZE(ad738x_iio_channels);
	iio_ad738x_inst->channels = ad738x_iio_channels;
	iio_ad738x_inst->attributes = ad738x_iio_global_attributes;

	iio_ad738x_inst->submit = iio_ad738x_submit_buffer;
	iio_ad738x_inst->pre_enable = iio_ad738x_prepare_transfer;
	iio_ad738x_inst->post_disable = iio_ad738x_end_transfer;
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_ad738x_inst->trigger_handler = ad738x_trigger_handler;
#endif

	*desc = iio_ad738x_inst;

	return 0;
}

/**
 * @brief	Initialization of AD738x IIO hardware trigger specific parameters
 * @param 	desc[in,out] - IIO hardware trigger descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad738x_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	int32_t ret;
	struct iio_hw_trig_init_param ad738x_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad738x_hw_trig_init_params.irq_id = TRIGGER_INT_ID;
	ad738x_hw_trig_init_params.name = AD738X_IIO_TRIGGER_NAME;
	ad738x_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad738x_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad738x_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad738x_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad738x_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad738x_hw_trig_init_params.iio_desc = p_ad738x_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad738x_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/**
 * @brief	Initialize the IIO interface for AD738x IIO device
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad738x_iio_initialize(void)
{
	int32_t init_status;

	/* IIO device descriptor */
	struct iio_device *iio_ad738x_dev;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	static struct iio_trigger ad738x_iio_trig_desc = {
		.is_synchronous = true,
	};

	/* IIO trigger init parameters */
	static struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad738x_iio_trig_desc,
		.name = AD738X_IIO_TRIGGER_NAME,
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
	static struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES] = {
		{
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
			.trigger_id = "trigger0",
#endif
		}
	};

	/* Initialize the system peripherals */
	init_status = init_system();
	if (init_status) {
		return init_status;
	}

	/* Initialize AD738x no-os device driver interface */
	init_status = ad738x_init(&ad738x_dev_inst, &ad738x_init_params);
	if (init_status) {
		return init_status;
	}

	/* Initialize the AD738x IIO app specific parameters */
	init_status = ad738x_iio_param_init(&iio_ad738x_dev);
	if (init_status) {
		return init_status;
	}

	iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
	iio_device_init_params[0].raw_buf = adc_data_buffer;
	iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;

	iio_device_init_params[0].dev = ad738x_dev_inst;
	iio_device_init_params[0].dev_descriptor = iio_ad738x_dev;

	iio_init_params.nb_devs++;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_init_params.nb_trigs++;
#endif

	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_iio_com_desc;
	iio_init_params.devs = iio_device_init_params;
	init_status = iio_init(&p_ad738x_iio_desc, &iio_init_params);
	if (init_status) {
		return init_status;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Initialize the AD738x IIO trigger specific parameters */
	init_status = ad738x_iio_trigger_param_init(&ad738x_hw_trig_desc);
	if (init_status) {
		return init_status;
	}

	/* Initialize the PWM trigger source for periodic ADC sampling */
	init_status = init_pwm_trigger();
	if (init_status) {
		return init_status;
	}
#endif

	return 0;
}

/**
 * @brief 	Run the AD738x IIO event handler
 * @return	none
 * @details	This function monitors the new IIO client event
 */
void ad738x_iio_event_handler(void)
{
	(void)iio_step(p_ad738x_iio_desc);
}
