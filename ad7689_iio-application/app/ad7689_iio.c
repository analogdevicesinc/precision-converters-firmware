/***************************************************************************//**
 *   @file    ad7689_iio.c
 *   @brief   Implementation of AD7689 IIO application interfaces
********************************************************************************
 * Copyright (c) 2021-22 Analog Devices, Inc.
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

#include "ad7689_iio.h"
#include "app_config.h"
#include "ad7689_user_config.h"
#include "ad7689_support.h"
#include "board_info.h"
#include "no_os_error.h"
#include "no_os_delay.h"
#include "iio_trigger.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/* ADC Raw to Voltage conversion default scale factor for IIO client */
#if defined(BIPOLAR)
/* Device supports only pseudo-bipolar mode. INX- = Vref / 2 */
#define ADC_DEFAULT_SCALE		(((ADC_DEFAULT_REF_VOLTAGE / 2) / ADC_MAX_COUNT_BIPOLAR) * 1000)
#else
#define ADC_DEFAULT_SCALE		((ADC_DEFAULT_REF_VOLTAGE / ADC_MAX_COUNT_UNIPOLAR) * 1000)
#endif

/* The output of temperature sensor is always unipolar (streight-binary) */
#define TEMPERATURE_SENSITIVITY		0.283	// 283mv
#define ROOM_TEMPERATURE			25.0
#define TEMPERATURE_CONV_SCALE		(ROOM_TEMPERATURE / TEMPERATURE_SENSITIVITY) * \
									((ADC_DEFAULT_REF_VOLTAGE / ADC_MAX_COUNT_UNIPOLAR) * 1000)

/*	Number of IIO devices */
#define NUM_OF_IIO_DEVICES	1

/* IIO trigger name */
#define AD7689_IIO_TRIGGER_NAME		"ad7689_iio_trigger"

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

/* Private IDs for IIO attributes */
#define	IIO_RAW_ATTR_ID			0
#define	IIO_SCALE_ATTR_ID		1
#define	IIO_OFFSET_ATTR_ID		2

/* Value indicating end of channels from active channels list */
#define END_OF_CHN		0xff

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

/* IIO interface descriptor */
static struct iio_desc *p_ad7689_iio_desc;

/* Pointer to the struct representing the AD7689 IIO device */
struct ad7689_dev *p_ad7689_dev_inst = NULL;

/* AD7689 IIO hw trigger descriptor */
static struct iio_hw_trig *ad7689_hw_trig_desc;

/* Number of active channels */
static volatile uint8_t num_of_active_channels;

/* Active channels list */
static uint8_t active_chns[ADC_CHN_COUNT + 1];

/* Index to next channel from active channels list */
static volatile uint8_t next_chn_indx;

/* Data buffer index */
static volatile uint8_t data_indx = 0;

static uint8_t first_active_chn;
static uint8_t second_active_chn;

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/* Device attributes with default values */

/* Scale attribute value per channel */
static float attr_scale_val[ADC_CHN_COUNT] = {
	ADC_DEFAULT_SCALE, ADC_DEFAULT_SCALE, ADC_DEFAULT_SCALE, ADC_DEFAULT_SCALE,
#if !defined(DEV_AD7682)
	ADC_DEFAULT_SCALE, ADC_DEFAULT_SCALE, ADC_DEFAULT_SCALE, ADC_DEFAULT_SCALE,
#endif
	TEMPERATURE_CONV_SCALE
};

/* AD7689 current configuration */
struct ad7689_config ad7689_current_config;

/* Context attributes ID */
enum context_attr_ids {
	HW_MEZZANINE_ID,
	HW_CARRIER_ID,
	HW_NAME_ID,
	DEF_NUM_OF_CONTXT_ATTRS
};

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

/* Hardware board information */
static struct board_info board_info;

/******************************************************************************/
/************************** Functions Declarations ****************************/
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
 *			e.g. if sampling frequency = 60KSPS and requested samples = 400
 *			Timeout = (400 * (1/60000)) + 1 = 1.0067sec = ~1sec
 */
static int get_sampling_frequency(void *device,
				  char *buf,
				  uint32_t len,
				  const struct iio_ch_info *channel,
				  intptr_t id)
{
	return sprintf(buf, "%d", SAMPLING_RATE);
}

static int set_sampling_frequency(void *device,
				  char *buf,
				  uint32_t len,
				  const struct iio_ch_info *channel,
				  intptr_t id)
{
	/* sampling frequency is read-only attribute */
	return len;
}


/*!
 * @brief	Getter/Setter for the raw, offset and scale attribute value
 * @param	device- pointer to IIO device structure
 * @param	buf- pointer to buffer holding attribute value
 * @param	len- length of buffer string data
 * @param	channel- pointer to IIO channel structure
 * @param	id- Attribute ID (optional)
 * @return	Number of characters read/written
 */
static int get_adc_raw(void *device,
		       char *buf,
		       uint32_t len,
		       const struct iio_ch_info *channel,
		       intptr_t id)
{
	static uint32_t adc_data_raw = 0;
	int32_t offset = 0;
	int32_t ret;

	switch (id) {
	case IIO_RAW_ATTR_ID:
		/* Capture the raw adc data */
		ret = ad7689_read_single_sample((uint8_t)channel->ch_num, &adc_data_raw);
		if (ret) {
			return sprintf(buf, " %s", "Error");
		}
		return sprintf(buf, "%d", adc_data_raw);

	case IIO_SCALE_ATTR_ID:
		return sprintf(buf, "%g", attr_scale_val[channel->ch_num]);

	case IIO_OFFSET_ATTR_ID:
#if defined(BIPOLAR)
		if (channel->ch_num == TEMPERATURE_CHN) {
			offset = 0;
		} else {
			if (adc_data_raw >= ADC_MAX_COUNT_BIPOLAR) {
				offset = -ADC_MAX_COUNT_UNIPOLAR;
			} else {
				offset = 0;
			}
		}
#endif
		return sprintf(buf, "%d", offset);

	default:
		break;
	}

	return len;
}

static int set_adc_raw(void *device,
		       char *buf,
		       uint32_t len,
		       const struct iio_ch_info *channel,
		       intptr_t id)
{
	/* ADC raw, offset and scale are read-only attributes */
	return len;
}

/**
 * @brief	Read buffer data corresponding to AD4170 IIO device
 * @param	iio_dev_data[in] - Pointer to IIO device data structure
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t iio_ad7689_submit_buffer(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t nb_of_samples;
	uint32_t sample_index = 0;
	uint8_t next_chn;
	uint8_t data_buf[BYTES_PER_SAMPLE] = { 0x0 };

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	nb_of_samples = iio_dev_data->buffer->size / BYTES_PER_SAMPLE;
	next_chn_indx = 0;
	data_indx = 0;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	ret = ad7689_perform_init_cnv(first_active_chn, second_active_chn,
				      num_of_active_channels);
	if (ret) {
		return ret;
	}

	while (sample_index < nb_of_samples) {
		/* The acquisition for 1st (n) and 2nd (n+1) active channels is started from
		* 'ad7689_enable_continuous_read_conversion' function. When chn_indx = 0,
		* (i.e. first entry to this function), the converion result for 1st active
		* channel (n) is read and is returned back. The next channel to be set for
		* acquisition therefore must be (n+2). This is done by adding +2 offset in
		* channel index recursively.
		**/
		if (active_chns[next_chn_indx] == END_OF_CHN) {
			next_chn_indx = 0;
		}
		next_chn = active_chns[next_chn_indx++];

		ret = ad7689_read_converted_sample(data_buf, next_chn);
		if (ret) {
			return ret;
		}

		ret = no_os_cb_write(iio_dev_data->buffer->buf, data_buf, BYTES_PER_SAMPLE);
		if (ret) {
			return ret;
		}

		/* Conversion delay = Acquisition time + Data read time
		 * Conv time = 4usec (min), Read time = ~2.1usec (@22.5Mhz SPI clock)
		 * Acq Time (req) = 4usec - 2.1usec =  1.9usec.
		 * Due to inaccuracy and overhead in the udelay() function,
		 * 1usec delay typically results into ~2.5usec time on SDP-K1 Mbed board.
		 * This delay is very critical in the conversion and may change
		 * from compiler to compiler and hardware to hardware. */
		if (next_chn == TEMPERATURE_CHN) {
			no_os_udelay(5);
		} else {
			no_os_udelay(1);
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
static int32_t iio_ad7689_prepare_transfer(void *dev_instance,
		uint32_t chn_mask)
{
	uint32_t mask = 0x1;
	uint8_t index = 0;
	int32_t ret;

	num_of_active_channels = 0;
	data_indx = 0;
	buf_size_updated = false;

	/* Get the active channels count based on the channel mask set in an IIO
	 * client application (channel mask starts from bit 0) */
	for (uint8_t chn = 0; chn < ADC_CHN_COUNT; chn++) {
		if (chn_mask & mask) {
			num_of_active_channels++;

			if (index == 0) {
				/* Get the n channel */
				first_active_chn = chn;
			} else if (index == 1) {
				/* Get the n+1 channel */
				second_active_chn = chn;
			} else {
				/* Store the list of n+2 and onward channels */
				active_chns[index - 2] = chn;
			}

			index++;
		}

		mask <<= 1;
	}

	if (index >= 2) {
		active_chns[index - 2] = first_active_chn;
		active_chns[index - 1] = second_active_chn;
	} else {
		active_chns[0] = first_active_chn;
	}

	next_chn_indx = 0;
	active_chns[index] = END_OF_CHN;	// end of channel list

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = ad7689_perform_init_cnv(first_active_chn, second_active_chn,
				      num_of_active_channels);
	if (ret) {
		return ret;
	}

	ret = iio_trig_enable(ad7689_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief	Perform tasks before end of current data transfer
 * @param	dev[in] - IIO device instance
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t iio_ad7689_end_transfer(void *dev)
{
	int32_t ret;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = iio_trig_disable(ad7689_hw_trig_desc);
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
int32_t iio_ad7689_trigger_handler(struct iio_device_data *iio_dev_data)
{
	uint8_t data_buf[BYTES_PER_SAMPLE] = { 0x0 };
	uint8_t next_chn;
	int32_t ret;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
						   iio_dev_data->buffer->bytes_per_scan)) * iio_dev_data->buffer->bytes_per_scan;
		buf_size_updated = true;
	}

	/* The acquisition for 1st (n) and 2nd (n+1) active channels is started from
	 * 'ad7689_enable_continuous_read_conversion' function. When chn_indx = 0,
	 * (i.e. first entry to this function), the converion result for 1st active
	 * channel (n) is read and is returned back. The next channel to be set for
	 * acquisition therefore must be (n+2). This is done by adding +2 offset in
	 * channel index recursively.
	 **/
	if (active_chns[next_chn_indx] == END_OF_CHN) {
		next_chn_indx = 0;
	}
	next_chn = active_chns[next_chn_indx++];

	ret = ad7689_read_converted_sample(data_buf, next_chn);
	if (ret) {
		return ret;
	}

	ret = no_os_cb_write(iio_dev_data->buffer->buf, data_buf, BYTES_PER_SAMPLE);
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
		.show = get_adc_raw,
		.store = set_adc_raw,
		.priv = IIO_RAW_ATTR_ID
	},
	{
		.name = "scale",
		.show = get_adc_raw,
		.store = set_adc_raw,
		.priv = IIO_SCALE_ATTR_ID
	},
	{
		.name = "offset",
		.show = get_adc_raw,
		.store = set_adc_raw,
		.priv = IIO_OFFSET_ATTR_ID
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
#if defined(BIPOLAR)
	.sign = 's',
#else
	.sign = 'u',
#endif
	.realbits = CHN_STORAGE_BITS,
	.storagebits = CHN_STORAGE_BITS,
	.shift = 0,
	.is_big_endian = false
};

static struct iio_channel iio_ad7689_channels[] = {
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
#if !defined(DEV_AD7682)
	{
		.name = "voltage4",
		.ch_type = IIO_VOLTAGE,
		.channel = 4,
		.scan_index = 4,
		.scan_type = &chn_scan,
		.attributes = channel_input_attributes,
		.ch_out = false,
		.indexed = true
	},
	{
		.name = "voltage5",
		.ch_type = IIO_VOLTAGE,
		.channel = 5,
		.scan_index = 5,
		.scan_type = &chn_scan,
		.attributes = channel_input_attributes,
		.ch_out = false,
		.indexed = true
	},
	{
		.name = "voltage6",
		.ch_type = IIO_VOLTAGE,
		.channel = 6,
		.scan_index = 6,
		.scan_type = &chn_scan,
		.attributes = channel_input_attributes,
		.ch_out = false,
		.indexed = true
	},
	{
		.name = "voltage7",
		.ch_type = IIO_VOLTAGE,
		.channel = 7,
		.scan_index = 7,
		.scan_type = &chn_scan,
		.attributes = channel_input_attributes,
		.ch_out = false,
		.indexed = true
	},
#endif
	{
		.name = "temperature",
		.ch_type = IIO_TEMP,
#if !defined(DEV_AD7682)
		.channel = 8,
		.scan_index = 8,
#else
		.channel = 4,
		.scan_index = 4,
#endif
		.scan_type = &chn_scan,
		.attributes = channel_input_attributes,
		.ch_out = false,
		.indexed = true
	},
};

/**
 * @brief	Read IIO context attributes
 * @param 	params[in,out] - Pointer to IIO context attributes init param
 * @param	attrs_cnt[in,out] - IIO contxt attributes count
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t get_iio_context_attributes(struct iio_cntx_attr_init *params,
		uint32_t *attrs_cnt)
{
	int32_t ret;
	struct iio_context_attribute *context_attributes;
	const char *board_status;
	uint8_t num_of_context_attributes = DEF_NUM_OF_CONTXT_ATTRS;
	uint8_t cnt = 0;

	if (!params || !attrs_cnt) {
		return -EINVAL;
	}

	if (is_eeprom_valid_dev_addr_detected()) {
		/* Read the board information from EEPROM */
		ret = read_board_info(eeprom_desc, &board_info);
		if (!ret) {
			if (!strcmp(board_info.board_id, HW_MEZZANINE_NAME)) {
				hw_mezzanine_is_valid = true;
			} else {
				hw_mezzanine_is_valid = false;
				board_status = "mismatch";
				num_of_context_attributes++;
			}
		} else {
			hw_mezzanine_is_valid = false;
			board_status = "not_detected";
			num_of_context_attributes++;
		}
	} else {
		hw_mezzanine_is_valid = false;
		board_status = "not_detected";
		num_of_context_attributes++;
	}

#if defined(FIRMWARE_VERSION)
	num_of_context_attributes++;
#endif

	/* Allocate dynamic memory for context attributes based on number of attributes
	 * detected/available */
	context_attributes = (struct iio_context_attribute *)calloc(
				     num_of_context_attributes,
				     sizeof(*context_attributes));
	if (!context_attributes) {
		return -ENOMEM;
	}

#if defined(FIRMWARE_VERSION)
	(context_attributes + cnt)->name = "fw_version";
	(context_attributes + cnt)->value = FIRMWARE_VERSION;
	cnt++;
#endif

	(context_attributes + cnt)->name = "hw_carrier";
	(context_attributes + cnt)->value = HW_CARRIER_NAME;
	cnt++;

	if (board_info.board_id[0] != '\0') {
		(context_attributes + cnt)->name = "hw_mezzanine";
		(context_attributes + cnt)->value = board_info.board_id;
		cnt++;
	}

	if (board_info.board_name[0] != '\0') {
		(context_attributes + cnt)->name = "hw_name";
		(context_attributes + cnt)->value = board_info.board_name;
		cnt++;
	}

	if (!hw_mezzanine_is_valid) {
		(context_attributes + cnt)->name = "hw_mezzanine_status";
		(context_attributes + cnt)->value = board_status;
		cnt++;
	}

	num_of_context_attributes = cnt;
	params->descriptor = context_attributes;
	*attrs_cnt = num_of_context_attributes;

	return 0;
}

/**
 * @brief	Init for reading/writing and parameterization of a
 * 			ad7689 IIO device
 * @param 	desc[in,out] - IIO device descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t iio_ad7689_init(struct iio_device **desc)
{
	struct iio_device *iio_ad7689_inst;

	iio_ad7689_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad7689_inst) {
		return -ENOMEM;
	}

	iio_ad7689_inst->num_ch = sizeof(iio_ad7689_channels) / sizeof(
					  iio_ad7689_channels[0]);
	iio_ad7689_inst->channels = iio_ad7689_channels;
	iio_ad7689_inst->attributes = global_attributes;

	iio_ad7689_inst->submit = iio_ad7689_submit_buffer;
	iio_ad7689_inst->pre_enable = iio_ad7689_prepare_transfer;
	iio_ad7689_inst->post_disable = iio_ad7689_end_transfer;
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_ad7689_inst->trigger_handler = iio_ad7689_trigger_handler;
#endif

	iio_ad7689_inst->debug_reg_read = NULL;
	iio_ad7689_inst->debug_reg_write = NULL;

	*desc = iio_ad7689_inst;

	return 0;
}

/**
 * @brief Initialization of AD7689 IIO hardware trigger specific parameters
 * @param desc[in,out] - IIO hardware trigger descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad7689_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	struct iio_hw_trig_init_param ad7689_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;
	int32_t ret;

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad7689_hw_trig_init_params.irq_id = IRQ_INT_ID;
	ad7689_hw_trig_init_params.name = AD7689_IIO_TRIGGER_NAME;
	ad7689_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad7689_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad7689_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad7689_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad7689_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad7689_hw_trig_init_params.iio_desc = &p_ad7689_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad7689_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/**
 * @brief	Initialize the IIO interface for AD7689 IIO device
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad7689_iio_initialize(void)
{
	int32_t init_status;

	/* IIO device descriptors */
	struct iio_device *p_iio_ad7689_dev;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	static struct iio_trigger ad7689_iio_trig_desc = {
		.is_synchronous = true,
	};

	/* IIO trigger init parameters */
	static struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad7689_iio_trig_desc,
		.name = AD7689_IIO_TRIGGER_NAME,
	};
#endif

	/* IIO context attributes */
	static struct iio_cntx_attr_init iio_cntx_attr_init_params;

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

	/* Initialize AD7689 device and peripheral interface */
	init_status = ad7689_init(&p_ad7689_dev_inst, &ad7689_init_params);
	if (init_status) {
		return init_status;
	}

	/* Read context attributes */
	init_status = get_iio_context_attributes(&iio_cntx_attr_init_params,
			&iio_init_params.nb_cntx_attrs);
	if (init_status) {
		return init_status;
	}

	if (hw_mezzanine_is_valid) {
		/* Initialize the device if HW mezzanine status is valid */
		init_status = iio_ad7689_init(&p_iio_ad7689_dev);
		if (init_status) {
			return init_status;
		}

		iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
		iio_device_init_params[0].raw_buf = adc_data_buffer;
		iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;

		iio_device_init_params[0].dev = p_ad7689_dev_inst;
		iio_device_init_params[0].dev_descriptor = p_iio_ad7689_dev;

		iio_init_params.nb_devs++;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		/* Initialize the AD7689 IIO trigger specific parameters */
		init_status = ad7689_iio_trigger_param_init(&ad7689_hw_trig_desc);
		if (init_status) {
			return init_status;
		}

		iio_init_params.nb_trigs++;
#endif
	}

	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_desc;
	iio_init_params.devs = iio_device_init_params;
	iio_init_params.cntx_attrs = &iio_cntx_attr_init_params;
	init_status = iio_init(&p_ad7689_iio_desc, &iio_init_params);
	if (init_status) {
		return init_status;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	init_pwm_trigger();
#endif

	/* Load the init config into current configuration */
	memcpy(&ad7689_current_config, &ad7689_init_params.config,
	       sizeof(ad7689_current_config));

	return 0;
}

/**
 * @brief 	Run the AD7689 IIO event handler
 * @return	none
 * @details	This function monitors the new IIO client event
 */
void ad7689_iio_event_handler(void)
{
	(void)iio_step(p_ad7689_iio_desc);
}
