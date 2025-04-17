/***************************************************************************//**
 * @file    ad717x_iio.c
 * @brief   Source file for the AD717x IIO Application
********************************************************************************
* Copyright (c) 2021-23,2025 Analog Devices, Inc.
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
#include "ad717x_user_config.h"
#include "app_config.h"
#include "ad717x_iio.h"
#include "ad717x.h"
#include "iio.h"
#include "no_os_util.h"
#include "no_os_error.h"
#include "common.h"
#include "iio_trigger.h"
#include "ad717x_support.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/* Define ADC Resolution in Bits */
#if defined (DEV_AD7177_2)
#define AD717x_RESOLUTION	24
#else
#define AD717x_RESOLUTION	24
#endif

/* ADC max count (full scale value) for unipolar inputs */
#define ADC_MAX_COUNT_UNIPOLAR	(uint32_t)((1 << AD717x_RESOLUTION) - 1)

/* ADC max count (full scale value) for bipolar inputs */
#define ADC_MAX_COUNT_BIPOLAR	(uint32_t)(1 << (AD717x_RESOLUTION-1))

/* Bytes per sample (*Note: 4 bytes needed per sample for data range
 * of 0 to 32-bit) */
#define	BYTES_PER_SAMPLE	sizeof(uint32_t)

/* Number of data storage bits (needed for IIO client) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

/* Private IDs for IIO attributes */
#define	AD717x_RAW_ATTR_ID		0
#define	AD717x_SCALE_ATTR_ID		1
#define	AD717x_OFFSET_ATTR_ID		2
#define AD717x_SAMPLING_FREQUENCY_ID	3

/* Data Buffer for burst mode data capture */
#define AD717x_DATA_BUFFER_SIZE		(8192)

/* Scan type definition */
#define AD717x_SCAN {\
	.sign = 'u',\
	.realbits = AD717x_RESOLUTION,\
	.storagebits = CHN_STORAGE_BITS,\
	.shift = 0,\
	.is_big_endian = false\
}

/* Channel attribute definition */
#define AD717x_CHANNEL(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = get_adc_attribute,\
	.store = set_adc_attribute\
}

/* AD717x Channel Definition */
#define IIO_AD717x_CHANNEL(_idx) {\
	.name = "ch" # _idx,\
	.ch_type = IIO_VOLTAGE,\
	.channel = _idx,\
	.scan_index = _idx,\
	.indexed = true,\
	.scan_type = &ad717x_scan_type[_idx],\
	.ch_out = false,\
	.attributes = ad717x_channel_attributes,\
}

/* ADC data buffer size */
#if defined(USE_SDRAM_CAPTURE_BUFFER)
#define adc_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(32768)		// 32kbytes
int8_t adc_data_buffer[DATA_BUFFER_SIZE];
#endif

/* IIO trigger name */
#define AD717X_IIO_TRIGGER_NAME		"ad717x_iio_trigger"

/* Number of IIO devices */
#define NUM_OF_IIO_DEVICES	1

/* Timeout count to avoid stuck into potential infinite loop while checking
 * for new data into an acquisition buffer. The actual timeout factor is determined
 * through 'sampling_frequency' attribute of IIO app, but this period here makes sure
 * we are not stuck into a forever loop in case data capture is interrupted
 * or failed in between.
 * Note: This timeout factor is dependent upon the MCU clock frequency. Below timeout
 * is tested for SDP-K1 platform @180Mhz default core clock */
#define AD717x_CONV_TIMEOUT	10000

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* IIO interface descriptor */
static struct iio_desc *p_ad717x_iio_desc;

/* Pointer to the struct representing the AD717x IIO device */
ad717x_dev *p_ad717x_dev_inst = NULL;

/* Device Name */
static const char dev_name[] = ACTIVE_DEVICE_NAME;

/* Channel scale values */
static float attr_scale_val[NUMBER_OF_CHANNELS];

/* Channel offset values */
static int attr_offset_val[NUMBER_OF_CHANNELS];

/* AD717x channel scan type */
static struct scan_type ad717x_scan_type[] = {
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
#if (NUMBER_OF_CHANNELS != 4)
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
#if (NUMBER_OF_CHANNELS != 4) && (NUMBER_OF_CHANNELS !=8 )
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN,
	AD717x_SCAN
#endif
#endif
};

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

/* AD717x IIO hw trigger descriptor */
static struct iio_hw_trig *ad717x_hw_trig_desc;

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/* Number of active channels requested by IIO Client */
static volatile uint8_t num_of_active_channels = 0;

/******************************************************************************/
/************************** Functions Declaration *****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definition ******************************/
/******************************************************************************/

/*!
 * @brief	Getter/Setter for the attribute value
 * @param	device[in]- pointer to IIO device structure
 * @param	buf[in]- pointer to buffer holding attribute value
 * @param	len[in]- length of buffer string data
 * @param	channel[in]- pointer to IIO channel structure
 * @param	id[in]- Attribute ID
 * @return	Number of characters read/written in case of success, negative error code otherwise
 */
static int32_t get_adc_attribute(void *device,
				 char *buf,
				 uint32_t len,
				 const struct iio_ch_info *channel,
				 intptr_t id)
{
	uint32_t adc_raw_data = 0;
	int32_t adc_offset = 0;

	switch (id) {
	case AD717x_RAW_ATTR_ID:
		if (ad717x_single_read(device, channel->ch_num, &adc_raw_data) < 0)
			return -EINVAL;

		return sprintf(buf, "%lu", adc_raw_data);

	case AD717x_SCALE_ATTR_ID:
		return sprintf(buf, "%f", attr_scale_val[channel->ch_num]);

	case AD717x_OFFSET_ATTR_ID:
		return sprintf(buf, "%d", attr_offset_val[channel->ch_num]);

	case AD717x_SAMPLING_FREQUENCY_ID:
		return sprintf(buf, "%d", AD717x_SAMPLING_RATE / NUMBER_OF_CHANNELS);

	default:
		break;
	}

	return -EINVAL;
}


static int32_t set_adc_attribute(void *device,
				 char *buf,
				 uint32_t len,
				 const struct iio_ch_info *channel,
				 intptr_t id)
{
	switch (id) {

	/* ADC Raw, Scale, Offset factors are constant for the firmware configuration */
	case AD717x_RAW_ATTR_ID:
	case AD717x_SCALE_ATTR_ID:
	case AD717x_OFFSET_ATTR_ID:
	case AD717x_SAMPLING_FREQUENCY_ID:
	default:
		break;
	}

	return len;
}

/* AD717X Channel Attributes */
static struct iio_attribute ad717x_channel_attributes[] = {
	AD717x_CHANNEL("raw", AD717x_RAW_ATTR_ID),
	AD717x_CHANNEL("scale", AD717x_SCALE_ATTR_ID),
	AD717x_CHANNEL("offset", AD717x_OFFSET_ATTR_ID),
	END_ATTRIBUTES_ARRAY
};

/* AD717x Global Attributes */
static struct iio_attribute iio_ad717x_global_attributes[] = {
	AD717x_CHANNEL("sampling_frequency", AD717x_SAMPLING_FREQUENCY_ID),
	END_ATTRIBUTES_ARRAY
};

/* IIO Attributes */
static struct iio_channel iio_adc_channels[] = {
	IIO_AD717x_CHANNEL(0),
	IIO_AD717x_CHANNEL(1),
	IIO_AD717x_CHANNEL(2),
	IIO_AD717x_CHANNEL(3),
#if (NUMBER_OF_CHANNELS != 4)
	IIO_AD717x_CHANNEL(4),
	IIO_AD717x_CHANNEL(5),
	IIO_AD717x_CHANNEL(6),
	IIO_AD717x_CHANNEL(7),
#if (NUMBER_OF_CHANNELS != 4) && (NUMBER_OF_CHANNELS != 8)
	IIO_AD717x_CHANNEL(8),
	IIO_AD717x_CHANNEL(9),
	IIO_AD717x_CHANNEL(10),
	IIO_AD717x_CHANNEL(11),
	IIO_AD717x_CHANNEL(12),
	IIO_AD717x_CHANNEL(13),
	IIO_AD717x_CHANNEL(14),
	IIO_AD717x_CHANNEL(15)
#endif
#endif
};

/**
 * @brief Read the debug register value
 * @param desc[in,out] - Pointer to IIO device descriptor
 * @param reg[in]- Address of the register to be read
 * @param readval[out]- Pointer to the register data variable
 * @return 0 in case of success, negative error code
 */
static int32_t iio_ad717x_debug_reg_read(void *dev,
		uint32_t reg,
		uint32_t *readval)
{
	int32_t debug_read_status; // Status check variable

	/* Retrieve the pointer to the requested register */
	ad717x_st_reg *read_register =  AD717X_GetReg(p_ad717x_dev_inst, reg);
	if (!read_register) {
		return -EINVAL;
	}

	/* Read the register data, extract the value */
	debug_read_status = AD717X_ReadRegister(p_ad717x_dev_inst, reg);
	if (debug_read_status) {
		return debug_read_status;
	}
	*readval = read_register->value;

	return 0;
}


/**
 * @brief Write value to the debug register
 * @param desc[in,out] Pointer to IIO device descriptor
 * @param reg[in] Address of the register where the data is to be written
 * @param write_val[out] Pointer to the register data variable
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t iio_ad717x_debug_reg_write(void *dev,
		uint32_t reg,
		uint32_t write_val)
{
	int32_t debug_write_status;	// Status check variable
	ad717x_st_reg *device_data_reg;  // Pointer to data register

	/* Retrieve the pointer to the requested registrer */
	device_data_reg = AD717X_GetReg(p_ad717x_dev_inst, reg);
	device_data_reg->value = write_val;

	/* Write value to the register */
	debug_write_status = AD717X_WriteRegister(p_ad717x_dev_inst, reg);
	if (debug_write_status) {
		return debug_write_status;
	}

	return 0;
}

/*!
 * @brief Function to prepare the ADC for continuous capture
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad717x_trigger_cont_data_capture(void)
{
	int32_t ret;

	/* Set ADC to Continuous conversion mode */
	ret = ad717x_set_adc_mode(p_ad717x_dev_inst, CONTINUOUS);
	if (ret) {
		return ret;
	}

	/* Enable Continuous read operation */
	ret = ad717x_enable_cont_read(p_ad717x_dev_inst, true);
	if (ret) {
		return ret;
	}

	/* Pull the cs line low to detect the EOC bit during data capture */
	ret = no_os_gpio_set_value(csb_gpio, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

	return ret;
}

/*!
 * @brief Function to stop continuous data capture
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad717x_stop_cont_data_capture(void)
{
	int32_t ret = 0;
	uint32_t adc_raw_data;
	uint8_t rdy_value = NO_OS_GPIO_HIGH;
	uint32_t timeout = AD717x_CONV_TIMEOUT;

	/* Read the data register when RDY is low to exit continuous read mode */
	do {
		ret = no_os_gpio_get_value(rdy_gpio, &rdy_value);
		if (ret) {
			return ret;
		}
		timeout--;
	} while ((rdy_value != NO_OS_GPIO_LOW) && (timeout > 0));

	if (timeout == 0) {
		return -ETIMEDOUT;
	}

	ret = AD717X_ReadData(p_ad717x_dev_inst, &adc_raw_data);
	if (ret) {
		return ret;
	}

	/* Disable continous read mode */
	ret = ad717x_enable_cont_read(p_ad717x_dev_inst, false);
	if (ret) {
		return ret;
	}

	return ret;
}

/**
 * @brief Prepare for ADC data capture (transfer from device to memory)
 * @param dev_instance[in] - IIO device instance
 * @param ch_mask[in] - Channels select mask
 * @return 0 in case of success or negative value otherwise
 */
static int32_t iio_ad717x_prepare_transfer(void *dev_instance,
		uint32_t chn_mask)
{
	int32_t ret;
	uint8_t ch_id;
	uint32_t mask = 0x1;
	uint8_t index = 0;
	num_of_active_channels = 0;
	buf_size_updated = false;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#if defined(USE_VIRTUAL_COM_PORT)
	/* TODO : spikes being observed in VCOM
	 * Added as temporary fix to the above issue.
	 * Makes the UART Interrupt low prior than the GPIO Interrupt
	 */
	ret = no_os_irq_set_priority(trigger_irq_desc, IRQ_INT_ID, 0);
	if (ret) {
		return ret;
	}

	HAL_NVIC_SetPriority(OTG_HS_IRQn, 1, 1);
#else
	ret = no_os_irq_set_priority(trigger_irq_desc, IRQ_INT_ID, RDY_GPIO_PRIORITY);
	if (ret) {
		return ret;
	}
#endif
#endif
#endif

	/* Enable requested channels and Disable the remaining */
	for (ch_id = 0;
	     ch_id < NUMBER_OF_CHANNELS; ch_id++) {
		if (chn_mask & mask) {
			ret = ad717x_set_channel_status(p_ad717x_dev_inst, ch_id, true);
			if (ret) {
				return ret;
			}
			num_of_active_channels++;
		} else {
			ret = ad717x_set_channel_status(p_ad717x_dev_inst, ch_id, false);
			if (ret) {
				return ret;
			}
		}
		mask <<= 1;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = ad717x_trigger_cont_data_capture();
	if (ret) {
		return ret;
	}

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	/* Clear pending Interrupt before enabling back the trigger.
	 * Else , a spurious interrupt is observed after a legitimate interrupt,
	 * as SPI SDO is on the same pin and is mistaken for an interrupt event */
	ret = no_os_irq_clear_pending(trigger_irq_desc, IRQ_INT_ID);
	if (ret) {
		return ret;
	}
#endif

	ret = iio_trig_enable(ad717x_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	return ret;
}


/**
 * @brief Read buffer data corresponding to AD4170 IIO device
 * @param iio_dev_data[in] - Pointer to IIO device data structure
 * @return 0 in case of success or negative value otherwise
 */
static int32_t iio_ad717x_submit_buffer(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t sample_index = 0;
	uint32_t nb_of_samples;
	uint32_t adc_raw_data = 0;

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	nb_of_samples = (iio_dev_data->buffer->size / BYTES_PER_SAMPLE);
	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	/* Set ADC to continuous conversion mode */
	ret = ad717x_set_adc_mode(p_ad717x_dev_inst, CONTINUOUS);
	if (ret) {
		return ret;
	}

	while (sample_index < nb_of_samples) {
		/* Wait for RDY Low */
		ret = AD717X_WaitForReady(p_ad717x_dev_inst, AD717x_CONV_TIMEOUT);
		if (ret) {
			return ret;
		}

		/* Read ADC Data */
		ret = AD717X_ReadData(p_ad717x_dev_inst, &adc_raw_data);
		if (ret) {
			return ret;
		}

		/* Write to CB */
		ret = no_os_cb_write(iio_dev_data->buffer->buf, &adc_raw_data,
				     BYTES_PER_SAMPLE);
		if (ret) {
			return ret;
		}
		sample_index++;
	}

	/* Set ADC to standby mode */
	ret = ad717x_set_adc_mode(p_ad717x_dev_inst, STANDBY);
	if (ret) {
		return ret;
	}
#endif
	return 0;
}


/**
 * @brief Perform tasks before end of current data transfer
 * @param dev[in] - IIO device instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t iio_ad717x_end_transfer(void *dev)
{
	int32_t ret;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = iio_trig_disable(ad717x_hw_trig_desc);
	if (ret) {
		return ret;
	}

	ret =  ad717x_stop_cont_data_capture();
	if (ret) {
		return ret;
	}
#endif
	/* Set ADC to Standby mode */
	return ad717x_set_adc_mode(p_ad717x_dev_inst, STANDBY);
}


/**
 * @brief Initialization of AD717x IIO hardware trigger specific parameters
 * @param desc[in,out] - IIO hardware trigger descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad717x_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	struct iio_hw_trig_init_param ad717x_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;
	int32_t ret;

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad717x_hw_trig_init_params.irq_id = IRQ_INT_ID;
	ad717x_hw_trig_init_params.name = AD717X_IIO_TRIGGER_NAME;
	ad717x_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad717x_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad717x_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad717x_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad717x_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad717x_hw_trig_init_params.iio_desc = p_ad717x_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad717x_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/*
 * @brief Push data into IIO buffer when trigger handler IRQ is invoked
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
int32_t ad717x_trigger_handler(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t adc_read_back = 0;

	ret = iio_trig_disable(ad717x_hw_trig_desc);
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

	ret = ad717x_adc_read_converted_sample(&adc_read_back);
	if (ret) {
		return ret;
	}

	ret = no_os_cb_write(iio_dev_data->buffer->buf,
			     &adc_read_back,
			     BYTES_PER_SAMPLE);
	if (ret) {
		return ret;
	}

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	/* Clear pending Interrupt before enabling back the trigger.
	 * Else , a spurious interrupt is observed after a legitimate interrupt,
	 * as SPI SDO is on the same pin and is mistaken for an interrupt event */
	ret = no_os_irq_clear_pending(trigger_irq_desc, IRQ_INT_ID);
	if (ret) {
		return ret;
	}
#endif

	/* Enable back the external interrupts */
	ret = iio_trig_enable(ad717x_hw_trig_desc);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Init for reading/writing and parameterization of a AD717x IIO device
 * @param desc[in,out] IIO device descriptor
 * @return 0 in case of success, negative error code otherwise
 */
int32_t iio_ad717x_init(struct iio_device **desc)
{
	struct iio_device *iio_ad717x_inst;  // IIO Device Descriptor for AD717x

	iio_ad717x_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad717x_inst) {
		return -EINVAL;
	}

	iio_ad717x_inst->num_ch = NO_OS_ARRAY_SIZE(iio_adc_channels);
	iio_ad717x_inst->channels = iio_adc_channels;
	iio_ad717x_inst->attributes = iio_ad717x_global_attributes;
	iio_ad717x_inst->buffer_attributes = NULL;
	iio_ad717x_inst->pre_enable = iio_ad717x_prepare_transfer;
	iio_ad717x_inst->post_disable = iio_ad717x_end_transfer;
	iio_ad717x_inst->submit = iio_ad717x_submit_buffer;
	iio_ad717x_inst->debug_reg_read = iio_ad717x_debug_reg_read;
	iio_ad717x_inst->debug_reg_write = iio_ad717x_debug_reg_write;
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_ad717x_inst->trigger_handler = ad717x_trigger_handler;
#endif

	*desc = iio_ad717x_inst;

	return 0;
}


/**
 * @brief Function to update scale and offset values based on user selection
 * @param device[in] AD717x device descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad717x_update_attr_parameters(ad717x_dev *device)
{
	float reference_value = 0; // Variable to hold the updated reference value
	uint8_t i;

	for (i = 0; i < NUMBER_OF_CHANNELS; i++) {
		/* Update reference value */
		switch (device->setups[device->chan_map[i].setup_sel].ref_source) {
		case INTERNAL_REF:
			reference_value = AD717X_INTERNAL_REFERENCE;
			break;
		case EXTERNAL_REF:
			reference_value = AD717x_EXTERNAL_REFERENCE;
			break;
		case AVDD_AVSS:
			reference_value = AD717X_AVDD_AVSS_REFERENCE;
			break;
		default:
			return -EINVAL;
		}

		/* Update channel attribute parameters */
		if (!(device->setups[device->chan_map[i].setup_sel].bi_unipolar)) {
			/* Settings for Unipolar mode */
			attr_scale_val[i] = ((reference_value / ADC_MAX_COUNT_UNIPOLAR) * 1000) /
					    SCALE_FACTOR_DR;
			attr_offset_val[i] = 0;
			ad717x_scan_type[i].sign = 'u';
			ad717x_scan_type[i].realbits = AD717x_RESOLUTION;
		} else {
			/* Settings for Bipolar mode */
			attr_scale_val[i] = ((reference_value / (ADC_MAX_COUNT_BIPOLAR)) * 1000) /
					    SCALE_FACTOR_DR;
			attr_offset_val[i] = -(1 << (AD717x_RESOLUTION - 1));
			ad717x_scan_type[i].sign = 's';
			ad717x_scan_type[i].realbits = CHN_STORAGE_BITS;
		}
	}

	return 0;
}


/**
 * @brief 	Initialize the AD717x IIO Interface
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad717x_iio_initialize(void)
{
	int32_t iio_init_status;		    // Status check variable
	struct iio_device *ad717x_iio_desc;	    //  aD717x IIO Descriptor

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	static struct iio_trigger ad717x_iio_trig_desc = {
		.is_synchronous = true,
	};

	/* IIO trigger init parameters */
	struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad717x_iio_trig_desc,
		.name = AD717X_IIO_TRIGGER_NAME,
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
	struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES] = {
		{
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
			.trigger_id = "trigger0"
#endif
		}
	};

	/* Init the system peripheral- UART */
	iio_init_status = init_system();
	if (iio_init_status) {
		return iio_init_status;
	}

	/* Initialize AD717x device */
	iio_init_status = AD717X_Init(&p_ad717x_dev_inst, ad717x_init_params);
	if (iio_init_status) {
		return iio_init_status;
	}

	/* Read context attributes */
	iio_init_status = get_iio_context_attributes(&iio_init_params.ctx_attrs,
			  &iio_init_params.nb_ctx_attr,
			  eeprom_desc,
			  HW_MEZZANINE_NAME,
			  STR(HW_CARRIER_NAME),
			  &hw_mezzanine_is_valid);
	if (iio_init_status) {
		return iio_init_status;
	}

	if (hw_mezzanine_is_valid) {
		/* Initialize the AD717x IIO Interface */
		iio_init_status = iio_ad717x_init(&ad717x_iio_desc);
		if (iio_init_status) {
			return iio_init_status;
		}

		iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
		iio_device_init_params[0].raw_buf = adc_data_buffer;
		iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;

		iio_device_init_params[0].dev = p_ad717x_dev_inst;
		iio_device_init_params[0].dev_descriptor = ad717x_iio_desc;

		iio_init_params.nb_devs++;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		iio_init_params.nb_trigs++;
#endif
	}

	/* Update the ADC scale respective to the device settings */
	iio_init_status = ad717x_update_attr_parameters(p_ad717x_dev_inst);
	if (iio_init_status) {
		return iio_init_status;
	}

	/* Initialize the IIO Interface */
	iio_init_params.uart_desc = uart_desc;
	iio_init_params.devs = iio_device_init_params;
	iio_init_status = iio_init(&p_ad717x_iio_desc, &iio_init_params);
	if (iio_init_status) {
		return iio_init_status;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Initialize the AD717x IIO trigger specific parameters */
	iio_init_status = ad717x_iio_trigger_param_init(&ad717x_hw_trig_desc);
	if (iio_init_status) {
		return iio_init_status;
	}
#endif

	return 0;
}


/**
 * @brief 	Run the AD717x IIO event handler
 * @return	None
 */
void ad717x_iio_event_handler(void)
{
	(void)iio_step(p_ad717x_iio_desc);
}
