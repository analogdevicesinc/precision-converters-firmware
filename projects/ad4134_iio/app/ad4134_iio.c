/***************************************************************************//**
 *   @file    ad7134_iio.c
 *   @brief   Implementation of AD7134 IIO apllication interfaces
********************************************************************************
 * Copyright (c) 2020-21, 2023-24 Analog Devices, Inc.
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

#include "ad7134_iio.h"
#include "app_config.h"
#include "ad7134_support.h"
#include "ad7134_user_config.h"
#include "common.h"
#include "no_os_error.h"
#include "iio_trigger.h"
#if (INTERFACE_MODE == TDM_MODE)
#include "stm32_tdm_support.h"
#endif

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/* Default ADC Vref voltage */
#define ADC_DEFAULT_REF_VOLTAGE		(4.096)

/* ADC data to Voltage conversion default scale factor for IIO client */
#define ADC_DEFAULT_SCALE	((ADC_DEFAULT_REF_VOLTAGE / (ADC_MAX_COUNT_BIPOLAR)) * 1000)

/* IIO trigger name */
#define AD7134_IIO_TRIGGER_NAME		"ad4134_iio_trigger"

/* Number of IIO devices */
#define NUM_OF_IIO_DEVICES	1

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* IIO interface descriptor */
static struct iio_desc *p_ad7134_iio_desc;

/* AD7134 IIO hw trigger descriptor */
static struct iio_hw_trig *ad7134_hw_trig_desc;

/* Pointer to the struct representing the AD7134 IIO device */
struct ad713x_dev *p_ad7134_dev_inst = NULL;

/* Device attributes with default values */

/* Scale attribute value per channel. The scale has been negated because the analog inputs
 * to the ADC are inverted via the LTC6373 on board */
static float attr_scale_val[] = {
	-ADC_DEFAULT_SCALE, -ADC_DEFAULT_SCALE, -ADC_DEFAULT_SCALE, -ADC_DEFAULT_SCALE
	};

/* Offset attribute value per channel */
static int attr_offset_val[AD7134_NUM_CHANNELS] = { 0x0 };

/* ADC data buffer size */
#if defined(USE_SDRAM)
#define adc_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(128000)
static int8_t adc_data_buffer[DATA_BUFFER_SIZE];
#endif

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static bool buf_size_updated = false;

/* Global pointer to copy the private iio_device_data
 * structure from ad7134_trigger_handler() */
struct iio_device_data *ad7134_iio_dev_data;

/* Pointer to the ADC data buffer */
static uint8_t *ad7134_dma_buff;

/* Flag to indicate if data read request is for raw read Operation
 * or data capture operation */
bool data_capture_operation = false;

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/*!
 * @brief	Getter/Setter for the scale attribute value
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int32_t get_scale(void *device,
			 char *buf,
			 uint32_t len,
			 const struct iio_ch_info *channel,
			 intptr_t id)
{
	return sprintf(buf, "%f", attr_scale_val[channel->ch_num]);
}

static int32_t set_scale(void *device,
			 char *buf,
			 uint32_t len,
			 const struct iio_ch_info *channel,
			 intptr_t id)
{
	float scale;

	(void)sscanf(buf, "%f", &scale);

	if (scale > 0) {
		attr_scale_val[channel->ch_num] = scale;
		return len;
	}

	return -EINVAL;
}

/*!
 * @brief	Getter/Setter for the offset attribute value
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int32_t get_offset(void *device,
			  char *buf,
			  uint32_t len,
			  const struct iio_ch_info *channel,
			  intptr_t id)
{
	return sprintf(buf, "%d", attr_offset_val[channel->ch_num]);
}

static int32_t set_offset(void *device,
			  char *buf,
			  uint32_t len,
			  const struct iio_ch_info *channel,
			  intptr_t id)
{
	// Offset value is read-only
	return 0;
}

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
 *			e.g. if sampling frequency = 60KSPS and requested samples = 400
 *			Timeout = (400 * (1/60000)) + 1 = 1.0067sec = ~1sec
 */
static int32_t get_sampling_frequency(void *device,
				      char *buf,
				      uint32_t len,
				      const struct iio_ch_info *channel,
				      intptr_t id)
{
	return sprintf(buf, "%d", SAMPLING_RATE);
}

static int32_t set_sampling_frequency(void *device,
				      char *buf,
				      uint32_t len,
				      const struct iio_ch_info *channel,
				      intptr_t id)
{
	/* Sampling frequency determines the IIO client timeout. It is defined in the
	 * software and not allowed to change externally */
	return -EINVAL;
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
static int32_t get_raw(void *device,
		       char *buf,
		       uint32_t len,
		       const struct iio_ch_info *channel,
		       intptr_t id)
{
	uint16_t adc_data_raw = 0;

	/* Capture the raw adc data */
	if (ad7134_perform_conv_and_read_sample((uint32_t)channel->ch_num,
						&adc_data_raw) == 0) {
		/* Update the scale and offset values */
		if (adc_data_raw >= ADC_MAX_COUNT_BIPOLAR) {
			/* Take the 2s complement for the negative counts (>full scale value) */
			attr_offset_val[channel->ch_num] = -ADC_MAX_COUNT_UNIPOLAR;
		} else {
			/* Negate the results as analog inputs are inverted */
			attr_offset_val[channel->ch_num] = 0;
		}

		return sprintf(buf, "%d", adc_data_raw);
	}

	return -EINVAL;
}

static int32_t set_raw(void *device,
		       char *buf,
		       uint32_t len,
		       const struct iio_ch_info *channel,
		       intptr_t id)
{
	/* NA- Can't set raw value */
	return len;
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
	if ((reg > AD713X_REG_TEMPERATURE_DATA) ||
	    (ad713x_spi_reg_read(dev, reg, (uint8_t *)readval) != 0)) {
		return -EINVAL;
	}

	return 0;
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
	if ((reg > AD713X_REG_TEMPERATURE_DATA) ||
	    (ad713x_spi_reg_write(dev, reg, writeval) != 0)) {
		return -EINVAL;
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
	{
		.name = "offset",
		.show = get_offset,
		.store = set_offset,
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

	END_ATTRIBUTES_ARRAY
};

/* IIOD channels configurations */
struct scan_type chn_scan = {
	.sign = 'u',
	.realbits = ADC_RESOLUTION,
	.storagebits = ADC_RESOLUTION,
	.shift = 0,
	.is_big_endian = false
};

static struct iio_channel iio_ad7134_channels[] = {
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
	{
		.name = "voltage1",
		.ch_type = IIO_VOLTAGE,
		.channel = 1,
		.scan_index = 1,
		.scan_type = &chn_scan,
		.attributes = channel_input_attributes,
		.ch_out = false,
		.indexed = true
	},
	{
		.name = "voltage2",
		.ch_type = IIO_VOLTAGE,
		.channel = 2,
		.scan_index = 2,
		.scan_type = &chn_scan,
		.attributes = channel_input_attributes,
		.ch_out = false,
		.indexed = true
	},
	{
		.name = "voltage3",
		.ch_type = IIO_VOLTAGE,
		.channel = 3,
		.scan_index = 3,
		.scan_type = &chn_scan,
		.attributes = channel_input_attributes,
		.ch_out = false,
		.indexed = true
	},
};

/**
 * @brief Prepare for ADC data capture (transfer from device to memory)
 * @param dev_instance[in] - IIO device instance
 * @param ch_mask[in] - Channels select mask
 * @return 0 in case of success or negative value otherwise
 */
static int32_t iio_ad7134_prepare_transfer(void *dev_instance,
		uint32_t chn_mask)
{
	int32_t ret;

	data_capture_operation = true;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
#if (INTERFACE_MODE != TDM_MODE)
	/* The UART interrupt needs to be prioritized over the GPIO (end of conversion) interrupt.
	 * If not, the GPIO interrupt may occur during the period where there is a UART read happening
	 * for the READBUF command. If UART interrupts are not prioritized, then it would lead to missing of
	 * characters in the IIO command sent from the client. */
	ad7134_configure_intr_priority();
#endif // INTERFACE_MODE
	ret = iio_trig_enable(ad7134_hw_trig_desc);
	if (ret) {
		return ret;
	}
#if (INTERFACE_MODE == TDM_MODE)
	ret = start_tdm_dma_to_cb_transfer(ad7134_tdm_desc, ad7134_iio_dev_data,
					   TDM_DMA_READ_SIZE, BYTES_PER_SAMPLE, TDM_DMA_READ_SIZE);
	if (ret) {
		return ret;
	}
#endif
#endif // DATA_CAPTURE_MODE

	return 0;
}

/**
 * @brief Read data in burst mode via TDM-DMA
 * @param nb_of_bytes[in] - Number of bytes requested by IIO
 * @param iio_dev_data[in] - IIO Device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad7134_read_burst_data_tdm(uint32_t nb_of_bytes,
		struct iio_device_data *iio_dev_data)
{
	uint32_t ad7134_buff_available_size;
	uint32_t timeout;
	uint32_t remaining_bytes = nb_of_bytes;
	int32_t ret;

#if (INTERFACE_MODE == TDM_MODE)
	do {
		if (remaining_bytes > DATA_BUFFER_SIZE) {
			nb_of_bytes = DATA_BUFFER_SIZE;
			remaining_bytes -= nb_of_bytes;
		} else {
			nb_of_bytes = remaining_bytes;
			remaining_bytes = 0;
		}

		/* Retrieve the address of data buffer from which DMA data write needs to start */
		ret = no_os_cb_prepare_async_write(iio_dev_data->buffer->buf, nb_of_bytes,
						   &ad7134_dma_buff, &ad7134_buff_available_size);
		if (ret) {
			return ret;
		}

		/* Trigger TDM-DMA read to capture data into buffer in the background */
		ret = no_os_tdm_read(ad7134_tdm_desc, ad7134_dma_buff,
				     nb_of_bytes / BYTES_PER_SAMPLE);
		if (ret) {
			return ret;
		}

		/* Wait until DMA buffer is full */
		timeout = AD7134_CONV_TIMEOUT;
		while (!dma_buffer_full) {
			timeout--;
		}

		/* Update the data buffer pointer to a new index post DMA write operation */
		ret = no_os_cb_end_async_write(iio_dev_data->buffer->buf);
		if (ret) {
			return ret;
		}

		ret = no_os_tdm_stop(ad7134_tdm_desc);
		if (ret) {
			return ret;
		}

		if (timeout == 0) {
			return -ETIMEDOUT;
		}
		dma_buffer_full = false;
	} while (remaining_bytes > 0);
#endif

	return 0;
}

/**
 * @brief Read data in burst mode via Bit Banging Method
 * @param nb_of_samples[in] - Number of samples requested by IIO
 * @param iio_dev_data[in] - IIO Device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad7134_read_burst_data_bit_bang(uint32_t nb_of_samples,
		struct iio_device_data *iio_dev_data)
{
	uint32_t sample_index = 0;
	int32_t ret;
	int32_t adc_data;
	uint8_t ch_id;

	while (sample_index < nb_of_samples) {
		for (ch_id = 0; ch_id < AD7134_NUM_CHANNELS; ch_id++) {
			/* Read ADC data using GPIO Bit banging method for
			 * detecting a level change in ODR and DCLK signals */
			ret = ad7134_perform_conv_and_read_sample(ch_id, &adc_data);
			if (ret) {
				return ret;
			}

			/* Write the ADC Data to circular buffer */
			ret = no_os_cb_write(iio_dev_data->buffer->buf, &adc_data, BYTES_PER_SAMPLE);
			if (ret) {
				return ret;
			}
		}
		sample_index++;
	}

	return 0;
}

/**
 * @brief Read buffer data corresponding to AD7134 IIO device
 * @param iio_dev_data[in] - Pointer to IIO device data structure
 * @return 0 in case of success or negative value otherwise
 */
static int32_t iio_ad7134_submit_buffer(struct iio_device_data *iio_dev_data)
{
	uint32_t nb_of_samples;
	uint32_t timeout = AD7134_CONV_TIMEOUT;
	int32_t ret;
	nb_of_samples = iio_dev_data->buffer->size / BYTES_PER_SAMPLE;

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
#if (INTERFACE_MODE == TDM_MODE)
	ret = ad7134_read_burst_data_tdm(iio_dev_data->buffer->size, iio_dev_data);
	if (ret) {
		return ret;
	}
#else
	if (!buf_size_updated) {
		/* Update total buffer size according to number of samples requested
		 * for proper alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	ret = ad7134_read_burst_data_bit_bang(nb_of_samples, iio_dev_data);
	if (ret) {
		return ret;
	}
#endif // INTERFACE_MODE
#endif // DATA_CAPTURE_MODE

	return 0;
}

/**
 * @brief Perform tasks before end of current data transfer
 * @param dev[in] - IIO device instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t iio_ad7134_end_transfer(void *dev)
{
	int32_t ret;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
#if (INTERFACE_MODE != TDM_MODE)
	ret = iio_trig_disable(ad7134_hw_trig_desc);
	if (ret) {
		return ret;
	}
#else // TDM_MODE
	/* Stop TDM DMA data capture */
	ret = no_os_tdm_stop(ad7134_tdm_desc);
	if (ret) {
		return ret;
	}
#endif // INTERFACE_MODE
#endif // DATA_CAPTURE_MODE

	data_capture_operation = false;

	return 0;
}

/*
 * @brief Push data into IIO buffer when trigger handler IRQ is invoked
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 * @note This function is executed only in case of continuous capture
		 in SPI Mode.
 */
int32_t ad7134_trigger_handler(struct iio_device_data *iio_dev_data)
{
	uint16_t adc_data[AD7134_NUM_CHANNELS] = { 0x0 };
	int32_t ret;

#if (INTERFACE_MODE == TDM_MODE)
	/* Disable IIO trigger after first occurrence to the trigger handler.
	 * The handler is enabled only once to point the private iio_dev_data to a
	 * global ad7134_iio_dev_data structure variable for future IIO CB operations */
	ret = iio_trig_disable(ad7134_hw_trig_desc);
	if (ret) {
		return ret;
	}

	ad7134_iio_dev_data = iio_dev_data;
#else
	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
						   iio_dev_data->buffer->bytes_per_scan)) * iio_dev_data->buffer->bytes_per_scan;
		buf_size_updated = true;
	}

	/* Disable interrupt */
	ret = iio_trig_disable(ad7134_hw_trig_desc);
	if (ret) {
		return ret;
	}

	/* Read all channels using GPIO Bit banging method for
	 * delecting a level change in DCLK signal */
	ret = ad7134_read_all_channels(adc_data);
	if (ret) {
		return ret;
	}

	/* Write the ADC Data to Circular Buffer */
	ret = no_os_cb_write(iio_dev_data->buffer->buf, &adc_data,
			     BYTES_PER_SAMPLE * AD7134_NUM_CHANNELS);
	if (ret) {
		return ret;
	}

	/* Enable back the interrupt */
	iio_trig_enable(ad7134_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief	Init for reading/writing and parameterization of a
 * 			ad7134 IIO device
 * @param 	desc[in,out] - IIO device descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t iio_ad7134_init(struct iio_device **desc)
{
	struct iio_device *iio_ad7134_inst;

	iio_ad7134_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad7134_inst) {
		return -ENOMEM;
	}

	iio_ad7134_inst->num_ch = sizeof(iio_ad7134_channels) / sizeof(
					  iio_ad7134_channels[0]);
	iio_ad7134_inst->channels = iio_ad7134_channels;
	iio_ad7134_inst->attributes = global_attributes;
	iio_ad7134_inst->debug_reg_read = debug_reg_read;
	iio_ad7134_inst->debug_reg_write = debug_reg_write;
	iio_ad7134_inst->pre_enable = iio_ad7134_prepare_transfer;
	iio_ad7134_inst->submit = iio_ad7134_submit_buffer;
	iio_ad7134_inst->post_disable = iio_ad7134_end_transfer;
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_ad7134_inst->trigger_handler = ad7134_trigger_handler;
#endif

	*desc = iio_ad7134_inst;

	return 0;
}

/**
 * @brief Initialization of AD7134 IIO hardware trigger specific parameters
 * @param desc[in,out] - IIO hardware trigger descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad7134_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	struct iio_hw_trig_init_param ad7134_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;
	int32_t ret;

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad7134_hw_trig_init_params.irq_id = IRQ_INT_ID;
	ad7134_hw_trig_init_params.name = AD7134_IIO_TRIGGER_NAME;
	ad7134_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad7134_hw_trig_init_params.irq_ctrl = external_int_desc;
	ad7134_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad7134_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad7134_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad7134_hw_trig_init_params.iio_desc = p_ad7134_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad7134_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/**
 * @brief Initialize the AD7134 device for iio interface.
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad7134_iio_initialize(void)
{
	int32_t init_status;

	/* IIO device descriptor */
	struct iio_device *p_iio_ad7134_dev;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	static struct iio_trigger ad7134_iio_trig_desc = {
		.is_synchronous = true,
	};

	/* IIO trigger init parameters */
	static struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad7134_iio_trig_desc,
		.name = AD7134_IIO_TRIGGER_NAME,
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

	/* Initialize AD7134 device and peripheral interface */
	init_status = ad713x_init(&p_ad7134_dev_inst, &ad713x_init_params);
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
		/* Initialize the AD7134 IIO Interface */
		init_status = iio_ad7134_init(&p_iio_ad7134_dev);
		if (init_status) {
			return init_status;
		}

		iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
		iio_device_init_params[0].raw_buf = adc_data_buffer;
		iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;

		iio_device_init_params[0].dev = p_ad7134_dev_inst;
		iio_device_init_params[0].dev_descriptor = p_iio_ad7134_dev;

		iio_init_params.nb_devs++;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		iio_init_params.nb_trigs++;
#endif
	}

	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_iio_com_desc;
	iio_init_params.devs = iio_device_init_params;
	init_status = iio_init(&p_ad7134_iio_desc, &iio_init_params);
	if (init_status) {
		return init_status;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Initialize the IIO trigger specific parameters */
	init_status = ad7134_iio_trigger_param_init(&ad7134_hw_trig_desc);
	if (init_status) {
		return init_status;
	}
#endif

	/* Enable PWM in case of operation in Target Mode */
#if (AD7134_ASRC_MODE == TARGET_MODE)
	init_status = init_pwm();
	if (init_status) {
		return init_status;
	}
#endif

	/* Initialize the data capture interface for AD7134 */
	init_status = ad7134_data_capture_init(p_ad7134_dev_inst);
	if (init_status) {
		return init_status;
	}

	return 0;
}


/**
 * @brief 	Run the AD7134 IIO event handler
 * @return	none
 * @details	This function monitors the new IIO client event
 */
void ad7134_iio_event_handler(void)
{
	(void)iio_step(p_ad7134_iio_desc);
}
