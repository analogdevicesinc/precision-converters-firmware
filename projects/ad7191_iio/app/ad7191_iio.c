/***************************************************************************//**
 *   @file    ad7191_iio.c
 *   @brief   Implementation of AD7191 IIO application interfaces
 *   @details This module acts as an interface for AD7191 IIO application
********************************************************************************
 * Copyright (c) 2024 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <string.h>
#include <stdbool.h>

#include "ad7191_user_config.h"
#include "ad7191_iio.h"
#include "iio_trigger.h"
#include "no_os_error.h"
#include "no_os_delay.h"
#include "no_os_util.h"
#include "no_os_gpio.h"
#include "app_config.h"
#include "ad7191_support.h"
#include "common.h"

/******** Forward declaration of getter/setter functions ********/
static int ad7191_iio_attr_get(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv);

static int ad7191_iio_attr_set(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv);

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/* IIO trigger name */
#define AD7191_IIO_TRIGGER_NAME		"ad7191_iio_trigger"

/* Number of IIO devices */
#define NUM_OF_IIO_DEVICES	         1

#define NO_OF_CHANNELS 1

/* ADC data buffer size */
#if defined(USE_SDRAM)
#define adc_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(32768)		// 32kbytes
static int8_t adc_data_buffer[DATA_BUFFER_SIZE];
#endif

/* Timeout count to avoid stuck into potential infinite loop while checking
 * for new data into an acquisition buffer. The actual timeout factor is determined
 * through 'sampling_frequency' attribute of IIO app, but this period here makes sure
 * we are not stuck into a forever loop in case data capture is interrupted
 * or failed in between.
 * Note: This timeout factor is dependent upon the MCU clock frequency. Below timeout
 * is tested for SDP-K1 platform @180Mhz default core clock */
#define BUF_READ_TIMEOUT	0xffffffff

/* IIO Channel attribute definition */
#define AD7191_CHN_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = ad7191_iio_attr_get,\
		.store = ad7191_iio_attr_set\
}

/* AD7191 Channel Scan structure */
#define AD7191_DEFAULT_CHN_SCAN {\
	.sign = 'u',\
	.realbits = ADC_RESOLUTION,\
	.storagebits = STORAGE_BITS,\
	.shift = 0,\
	.is_big_endian = false\
}

/* IIOD channel scan configurations */
struct scan_type ad7191_iio_scan_type[NUM_OF_IIO_DEVICES][NO_OF_CHANNELS] = {
	{ AD7191_DEFAULT_CHN_SCAN }
};

/* IIO Channel Definition */
#define AD7191_IIO_CH(_name, _dev, _idx) {\
	.name = _name #_idx, \
	.ch_type = IIO_VOLTAGE,\
	.ch_out = false,\
	.indexed = true,\
	.channel = _idx,\
	.scan_index = _idx,\
	.scan_type = ad7191_iio_scan_type[_dev],\
	.attributes = ad7191_iio_ch_attributes[_dev]\
}

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* IIO interface descriptor */
static struct iio_desc *p_ad7191_iio_desc = NULL;

/* AD7191 IIO device descriptor */
struct iio_device *p_iio_ad7191_dev;

/* AD7191 global device instance for accessing device specific APIs */
struct ad7191_dev *ad7191_dev_inst;

/* AD7191 IIO hw trigger descriptor */
static struct iio_hw_trig *ad7191_hw_trig_desc;

/* Channel range values string representation
 * (possible values specified in datasheet) */
static char *ad7191_range_str[] = {
	"+/-2.5V",
	"+/-312.5mV",
	"+/-39.06mV",
	"+/-19.53mV",
};

/* AD7191 attribute unique IDs */
enum ad7191_attribute_ids {
	ADC_RAW,
	ADC_SCALE,
	ADC_OFFSET,
	NUM_OF_CHN_ATTR,

	ADC_RANGE,
	ADC_SAMPLING_FREQUENCY,
	NUM_OF_DEV_ATTR = ADC_SAMPLING_FREQUENCY - NUM_OF_CHN_ATTR
};

/* AD7191 device channel attributes list */
static struct iio_attribute
	ad7191_iio_ch_attributes[NUM_OF_IIO_DEVICES][NUM_OF_CHN_ATTR + 1] = {
	{
		AD7191_CHN_ATTR("raw", ADC_RAW),
		AD7191_CHN_ATTR("scale", ADC_SCALE),
		AD7191_CHN_ATTR("offset", ADC_OFFSET),

		END_ATTRIBUTES_ARRAY
	}
};

/* IIOD device (global) attributes list */
static struct iio_attribute
	ad7191_iio_global_attributes[NUM_OF_IIO_DEVICES][NUM_OF_DEV_ATTR + 1] = {
	{
		AD7191_CHN_ATTR("range", ADC_RANGE),
		AD7191_CHN_ATTR("sampling_frequency", ADC_SAMPLING_FREQUENCY),

		END_ATTRIBUTES_ARRAY
	}
};

/* AD7191 IIO Channels */
static struct iio_channel
	ad7191_iio_channels[NUM_OF_IIO_DEVICES][NO_OF_CHANNELS] = {
	{
		AD7191_IIO_CH("channel", 0, 0),
	}
};

/* List of channels to be captured */
static uint8_t ad7191_active_channels[NO_OF_CHANNELS];

/* Channel ID during data capture */
volatile uint8_t chan_id = 0;

/* Number of channels enabled by the IIO Client */
uint8_t num_of_active_channels = 0;

/* AD7191 Sampling Frequency */
uint8_t ad7191_sampling_frequency = 0;

/* Flag to indicate data capture status */
volatile static bool data_capture_done = false;

/* Flag to indicate data capture operation */
volatile static bool data_capture_operation = false;

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/* Scale attribute value per channel */
static float attr_scale_val;

/* AD7191 Gain attribute value */
static uint8_t ad7191_gain = 1;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/*!
 * @brief	Getter function for the AD7191 Attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written
 */
static int ad7191_iio_attr_get(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	int ret;
	/* AD7191 ODR1 value */
	int odr1 = NO_OS_GPIO_LOW;
	/* AD7191 ODR2 value */
	int odr2 = NO_OS_GPIO_LOW;
	/* AD7191 ODR value */
	int odr_val;
	uint8_t pga_val;
	uint32_t adc_raw_data = 0;

	switch (priv) {
	case ADC_RAW:
		ret = ad7191_get_raw_data(&adc_raw_data);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%ld", adc_raw_data);

	case ADC_OFFSET:
		// Offset for bipolar
		return sprintf(buf, "%ld", -ADC_MAX_COUNT_BIPOLAR);

	case ADC_SCALE:
		ret = ad7191_get_pga_val(&pga_val);
		if (ret) {
			return ret;
		}

		switch (pga_val) {
		case 0:
			ad7191_gain = 1;
			break;

		case 1:
			ad7191_gain = 8;
			break;

		case 2:
			ad7191_gain = 64;
			break;

		case 3:
			ad7191_gain = 128;
			break;

		default:
			return -EINVAL;
		}

		attr_scale_val = (AD7191_DEFAULT_REF_VOLTAGE / (ADC_MAX_COUNT_BIPOLAR *
				  ad7191_gain)) * 1000;

		return sprintf(buf, "%.10f", attr_scale_val);

	case ADC_RANGE:
		ret = ad7191_get_pga_val(&pga_val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%s", ad7191_range_str[pga_val]);

	case ADC_SAMPLING_FREQUENCY:
		/* Getting the odr1 value */
		ret = no_os_gpio_get_value(ad7191_dev_inst->odr1_gpio, &odr1);
		if (ret) {
			return ret;
		}

		/* Getting the odr2 value */
		ret = no_os_gpio_get_value(ad7191_dev_inst->odr2_gpio, &odr2);
		if (ret) {
			return ret;
		}

		if (odr2) {
			odr_val = (1 << odr2) + odr1;
		} else {
			odr_val = odr1;
		}

		switch (odr_val) {
		case 0:
			ad7191_sampling_frequency = 120;
			break;

		case 1 :
			ad7191_sampling_frequency = 60;
			break;

		case 2 :
			ad7191_sampling_frequency = 50;
			break;

		case 3:
			ad7191_sampling_frequency = 10;
			break;

		default:
			return -EINVAL;
		}

		return sprintf(buf,
			       "%d",
			       ad7191_sampling_frequency);
	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Setter function for AD7191 attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of expected bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int ad7191_iio_attr_set(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	// TODO Fill up the function
	return len;
}

/*!
 * @brief Interrupt Service Routine to monitor end of conversion event.
 * @param ctx[in] - Callback context (unused)
 * @return none
 * @note Callback registered for the the RDY interrupt to indicate
 * 		 end of conversion in case of burst data capturing.
 */
void data_capture_callback(void *ctx)
{
	data_capture_done = true;
}

/*!
 * @brief	Getting the pga_val from the ADC.
 * @param	pga_val[in,out] - pointer to the pga_val variable.
 * @return	0 in case of success, negative error code otherwise
 */
int ad7191_get_pga_val(uint8_t *pga_val)
{
	int ret;
	/* AD7191 PGA1 value */
	int pga1 = NO_OS_GPIO_LOW;
	/* AD7191 PGA2 value */
	int pga2 = NO_OS_GPIO_LOW;

	if (!pga_val) {
		return -EINVAL;
	}

	/* Getting the pga1 value */
	ret = no_os_gpio_get_value(ad7191_dev_inst->pga1_gpio, &pga1);
	if (ret) {
		return ret;
	}

	/* Getting the pga2 value */
	ret = no_os_gpio_get_value(ad7191_dev_inst->pga2_gpio, &pga2);
	if (ret) {
		return ret;
	}

	if (pga2) {
		*pga_val = (1 << pga2) + pga1;
	} else {
		*pga_val = pga1;
	}

	return 0;
}

/*!
 * @brief	Read raw data from the ADC.
 * @param	adc_raw_data[in,out] - pointer to the adc data variable.
 * @return	0 in case of success, negative error code otherwise.
 */
int ad7191_get_raw_data(uint32_t *adc_raw_data)
{
	uint32_t timeout = BUF_READ_TIMEOUT;
	int ret;
	uint8_t buff[3] = { 0 };

	ret = no_os_gpio_set_value(ad7191_dev_inst->csb_gpio, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

	/* Clear pending Interrupt before enabling back the trigger.
	* Else , a spurious interrupt is observed after a legitimate interrupt,
	* as SPI SDO is on the same pin and is mistaken for an interrupt event */
	ret = no_os_irq_clear_pending(trigger_irq_desc, IRQ_INT_ID);
	if (ret) {
		return ret;
	}

	/* Enable interrupts to detect End of Conversion */
#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	ret = no_os_irq_enable(trigger_irq_desc, IRQ_INT_ID);
	if (ret) {
		return ret;
	}
#else
	ret = iio_trig_enable(ad7191_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	/* Wait for end of conversion */
	while ((!data_capture_done) && (timeout > 0)) {
		timeout--;
	}

	if (timeout == 0) {
		return -ETIMEDOUT;
	}

	data_capture_done = false;

	/* Disable end of conversion interrupts */
#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	ret = no_os_irq_disable(trigger_irq_desc, IRQ_INT_ID);
	if (ret) {
		return ret;
	}
#else
	ret = iio_trig_disable(ad7191_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	/* Read data over spi interface */
	ret = no_os_spi_write_and_read(ad7191_dev_inst->spi_desc, buff, sizeof(buff));
	if (ret) {
		return ret;
	}

	/* Converting the 3 bytes of buff data into the 24 bit adc raw data value */
	*adc_raw_data = no_os_get_unaligned_be24(buff);

	return 0;
}

/*!
 * @brief Prepare for ADC data capture (transfer from device to memory)
 * @param dev_instance[in] - IIO device instance
 * @param chn_mask[in] - Channels select mask
 * @return 0 in case of success, negative error code otherwise
 */
static int ad7191_iio_prepare_transfer(void* dev_instance, uint32_t ch_mask)
{
	int ret;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	data_capture_operation = true;
	ret = iio_trig_enable(ad7191_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/*!
 * @brief	Perform tasks before end of current data transfer
 * @param	dev[in] - IIO device instance
 * @return	0 in case of success, negative error code otherwise
 */
static int ad7191_iio_end_transfer(void *dev)
{
	int ret;

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	ret = no_os_irq_disable(trigger_irq_desc, IRQ_INT_ID);
	if (ret) {
		return ret;
	}
#else
	data_capture_operation = false;
	ret = iio_trig_disable(ad7191_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/*!
 * @brief Get a number of samples from all the active channels
 * @param iio_device_data[in, out] - IIO device data instance
 * @return	0 in case of success, negative error code otherwise
 */
static int iio_ad7191_submit_samples(struct iio_device_data *iio_dev_data)
{
	uint32_t ret;
	uint32_t count = 0;
	uint8_t buff[3] = { 0 };
	uint32_t nb_of_samples;
	uint32_t data_read = 0 ;

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

	while (nb_of_samples--) {
		/* Read the raw data value */
		ret = ad7191_get_raw_data(&data_read);
		if (ret) {
			return ret;
		}

		ret = no_os_cb_write(iio_dev_data->buffer->buf,
				     &data_read,
				     BYTES_PER_SAMPLE);
		if (ret) {
			return ret;
		}

	}

	return 0;
}

/*!
 * @brief Push data into IIO buffer when trigger handler IRQ is invoked
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
int ad7191_trigger_handler(struct iio_device_data *iio_dev_data)
{
	if (!data_capture_operation) {
		data_capture_done = true;
	} else {
		int32_t ret;
		uint8_t buff[3] = { 0 };
		uint32_t adc_raw_data = 0;

		if (!buf_size_updated) {
			/* Update total buffer size according to bytes per scan for proper
			 * alignment of multi-channel IIO buffer data */
			iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
							   iio_dev_data->buffer->bytes_per_scan)) * iio_dev_data->buffer->bytes_per_scan;
			buf_size_updated = true;
		}

		ret = iio_trig_disable(ad7191_hw_trig_desc);
		if (ret) {
			return ret;
		}

		ret = no_os_spi_write_and_read(ad7191_dev_inst->spi_desc, buff, sizeof(buff));
		if (ret) {
			return ret;
		}

		/* Converting the 3 bytes of buff data into the 24 bit adc raw data value */
		adc_raw_data = no_os_get_unaligned_be24(buff);

		ret = no_os_gpio_set_value(ad7191_dev_inst->csb_gpio, NO_OS_GPIO_LOW);
		if (ret) {
			return ret;
		}

		ret = no_os_cb_write(iio_dev_data->buffer->buf,
				     &adc_raw_data, BYTES_PER_SAMPLE);
		if (ret) {
			return ret;
		}

		/* Clear pending Interrupt before enabling back the trigger.
		* Else , a spurious interrupt is observed after a legitimate interrupt,
		* as SPI SDO is on the same pin and is mistaken for an interrupt event */
		ret = no_os_irq_clear_pending(trigger_irq_desc, IRQ_INT_ID);
		if (ret) {
			return ret;
		}

		ret = iio_trig_enable(ad7191_hw_trig_desc);
		if (ret) {
			return ret;
		}
	}
	return 0;
}

/*!
 * @brief	Init for reading/writing and parameterization of
 * 			ad7191 IIO device
 * @param 	desc[in,out] - IIO device descriptor
 * @return	0 in case of success, negative value otherwise
 */
static int ad7191_iio_init(struct iio_device **desc)
{
	struct iio_device *iio_ad7191_inst;  // IIO Device Descriptor for ad7191

	if (!desc) {
		return -EINVAL;
	}

	iio_ad7191_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad7191_inst) {
		return -ENOMEM;
	}

	iio_ad7191_inst->num_ch = NO_OS_ARRAY_SIZE(ad7191_iio_channels);
	iio_ad7191_inst->channels = ad7191_iio_channels;
	iio_ad7191_inst->attributes = ad7191_iio_global_attributes;
	iio_ad7191_inst->debug_attributes = NULL;
	iio_ad7191_inst->buffer_attributes = NULL;
	iio_ad7191_inst->submit = iio_ad7191_submit_samples;
	iio_ad7191_inst->pre_enable = ad7191_iio_prepare_transfer;
	iio_ad7191_inst->post_disable = ad7191_iio_end_transfer;
	iio_ad7191_inst->write_dev = NULL;
	iio_ad7191_inst->debug_reg_read = NULL;
	iio_ad7191_inst->debug_reg_write = NULL;
	iio_ad7191_inst->trigger_handler = ad7191_trigger_handler;

	*desc = iio_ad7191_inst;

	return 0;
}

/*!
 * @brief	Initialization of AD7191 IIO hardware trigger specific parameters
 * @param 	desc[in,out] - IIO hardware trigger descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int ad7191_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	int32_t ret;
	struct iio_hw_trig_init_param ad7191_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;

	if (!desc) {
		return -EINVAL;
	}

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad7191_hw_trig_init_params.irq_id = IRQ_INT_ID;
	ad7191_hw_trig_init_params.name = AD7191_IIO_TRIGGER_NAME;
	ad7191_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad7191_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad7191_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad7191_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad7191_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad7191_hw_trig_init_params.iio_desc = p_ad7191_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad7191_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/*!
 * @brief Release resources allocated for IIO device
 * @param desc[in] - IIO device descriptor
 * @return 0 in case of success, negative value otherwise
 */
static int ad7191_iio_remove(struct iio_desc *desc)
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
 * @brief 	Initialize the ad7191 IIO Interface
 * @return	0 in case of success, negative value otherwise
 */
int ad7191_iio_initialize(void)
{
	int32_t init_status;
	uint8_t ret;

	ret = ad7191_init_gpio(&ad7191_dev_inst, ad7191_init_params);
	if (ret) {
		return ret;
	}

	/* IIO interface init parameters */
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	static struct iio_trigger ad7191_iio_trig_desc = {
		.is_synchronous = true,
		.enable = NULL,
		.disable = NULL
	};

	/* IIOD init parameters */
	static struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad7191_iio_trig_desc,
		.name = AD7191_IIO_TRIGGER_NAME,
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
	
	init_status = no_os_spi_init(&ad7191_dev_inst->spi_desc,
				     ad7191_init_params.spi_init);
	if (init_status) {
		no_os_free(ad7191_dev_inst->spi_desc);
	}

	/* TODO Implement Read context attributes */

	/* Initialize the AD7191 IIO application interface */
	init_status = ad7191_iio_init(&p_iio_ad7191_dev);
	if (init_status) {
		return init_status;
	}

	/* Initialize the IIO interface */
	iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
	iio_device_init_params[0].raw_buf = adc_data_buffer;
	iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;

	iio_device_init_params[0].dev = ad7191_dev_inst;
	iio_device_init_params[0].dev_descriptor = p_iio_ad7191_dev;

	iio_init_params.nb_devs++;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_init_params.nb_trigs++;
#endif

	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_desc;
	iio_init_params.devs = iio_device_init_params;
	init_status = iio_init(&p_ad7191_iio_desc, &iio_init_params);
	if (init_status) {
		ad7191_iio_remove(p_ad7191_iio_desc);
		return -ENOSYS;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	init_status = ad7191_iio_trigger_param_init(&ad7191_hw_trig_desc);
	if (init_status) {
		return init_status;
	}
#endif

	return 0;
}

/*!
 * @brief 	Run the ad7191 IIO event handler
 * @return	None
 */
void ad7191_iio_event_handler(void)
{
	iio_step(p_ad7191_iio_desc);
}
