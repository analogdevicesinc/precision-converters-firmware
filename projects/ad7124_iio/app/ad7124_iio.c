/**************************************************************************//**
 * @file    ad7124_iio.c
 * @brief   Main interface for AD7124 IIO Application firmware example program.
 * @details This module acts as an interface for the AD7124 IIO Application.
*******************************************************************************
* Copyright (c) 2023-25 Analog Devices, Inc.

* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
******************************************************************************/

/*****************************************************************************/
/***************************** Include Files *********************************/
/*****************************************************************************/

#include <stdio.h>
#include <string.h>

#include "ad7124_user_config.h"
#include "ad7124_iio.h"
#include "no_os_util.h"
#include "no_os_error.h"
#include "common.h"
#include "no_os_irq.h"
#include "ad7124_support.h"
#include "iio_trigger.h"
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
#include "pl_gui_views.h"
#include "pl_gui_events.h"
#include "adi_fft.h"
#endif

/******** Forward declaration of getter/setter functions ********/
static int32_t ad7124_iio_attr_get(void *device,
				   char *buf,
				   uint32_t len,
				   const struct iio_ch_info *channel,
				   intptr_t id);

static int32_t ad7124_iio_attr_set(void *device,
				   char *buf,
				   uint32_t len,
				   const struct iio_ch_info *channel,
				   intptr_t id);

static int ad7124_iio_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int ad7124_iio_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int iio_ad7124_local_backend_event_read(void *conn,
		uint8_t *buf,
		uint32_t len);
static int iio_ad7124_local_backend_event_write(void *conn,
		uint8_t *buf,
		uint32_t len);

static float ad7124_data_to_voltage_without_vref(int32_t data, uint8_t chn);
static float ad7124_data_to_voltage_wrt_vref(int32_t data, uint8_t chn);
static int32_t ad7124_code_to_straight_binary(uint32_t code, uint8_t chn);

/*****************************************************************************/
/********************* Macros and Constants Definition ***********************/
/*****************************************************************************/

/* For ADC resolution of 24-bits */
#define	BYTES_PER_SAMPLE	sizeof(uint32_t)

/* Number of data storage bits (needed for IIO client to plot ADC data) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

/* ADC data buffer size */
#if defined(USE_SDRAM)
#define adc_data_buffer			SDRAM_START_ADDRESS
#define ADC_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
/* Note: Setting lower size due to memory constraints on MCU */
#define ADC_BUFFER_SIZE			(16000)
#else
#define ADC_BUFFER_SIZE			(32768)		// 32kbytes
#endif
static int8_t adc_data_buffer[ADC_BUFFER_SIZE] = { 0 };
#endif

/* Default ADC Vref voltage */
#define AD7124_DEFAULT_REF_VOLTAGE	(2.5)

#define NUM_OF_IIO_DEVICES 1

#define AD7124_CHN_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = ad7124_iio_attr_get,\
	.store = ad7124_iio_attr_set\
}

#define AD7124_CHN_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = ad7124_iio_attr_available_get,\
	.store = ad7124_iio_attr_available_set\
}

#define AD7124_CH(_name, _dev, _idx, _type) {\
	.name = _name, \
	.ch_type = _type,\
	.ch_out = 0,\
	.indexed = true,\
	.channel = _idx,\
	.scan_index = _idx,\
	.scan_type = chn_scan[_dev],\
	.attributes = chn_attr[_dev]\
}

#define AD7124_DEFAULT_CHN_SCAN {\
	.sign = 's',\
	.realbits = ADC_RESOLUTION,\
	.storagebits = CHN_STORAGE_BITS,\
	.shift = 0,\
	.is_big_endian = false\
}

/* NOTE: Maximum sampling rates for each mode.
 * 'Low power mode' where maximum sampling rate is in the range 0 to 2400SPS
 * 'Mid Power Mode' where maximum sampling rate is in the range 2400 to 4800SPS
 * 'High Power Mode' where the maximum sampling rate is in the range 4800 to 19200SPS */
#define SAMPLING_RATE_LOW_POWER 2400
#define SAMPLING_RATE_MID_POWER 4800
#define SAMPLING_RATE_HIGH_POWER 19200

/* Sampling rate macro for fft calculation for pocket lab */
#define SAMPLING_RATE 19200

#define AD7124_MAX_REG AD7124_GAIN7_REG

/* Local backend buffer (for storing IIO commands and responses) */
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
#define APP_LOCAL_BACKEND_BUF_SIZE	0x1000	// min 4096 bytes required
static char app_local_backend_buff[APP_LOCAL_BACKEND_BUF_SIZE];
#endif

/*****************************************************************************/
/******************** Variables and User Defined Data Types ******************/
/*****************************************************************************/

/* IIO interface descriptor */
static struct iio_desc *ad7124_iio_desc;

/* Pointer to the struct representing the AD7124 IIO device */
struct ad7124_dev *ad7124_dev_inst = NULL;

/* AD7124 IIO hw trigger descriptor */
static struct iio_hw_trig *ad7124_hw_trig_desc;

/* Scale attribute value per channel */
static float attr_scale_val[NUM_OF_IIO_DEVICES][AD7124_MAX_CHANNELS];

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

struct scan_type chn_scan[NUM_OF_IIO_DEVICES][AD7124_MAX_CHANNELS] = {
	{ AD7124_DEFAULT_CHN_SCAN }
};

/* Sampling Frequency */
static uint32_t ad7124_sampling_frequency;

/* Number of active channels requested by IIO Client */
static uint8_t num_of_active_channels;

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/* Flag to denote that sample has been captured */
volatile bool data_capture_done = false;

/* IIO trigger name */
#define AD7124_IIO_TRIGGER_NAME		"ad7124_iio_trigger"

static struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES] = {
	{
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		.trigger_id = "trigger0",
#endif
	}
};

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
static struct iio_trigger ad7124_iio_trig_desc = {
	.is_synchronous = true,
};

/* IIO trigger init parameters */
struct iio_trigger_init iio_trigger_init_params = {
	.descriptor = &ad7124_iio_trig_desc,
	.name = AD7124_IIO_TRIGGER_NAME,
};
#endif

#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
/* IIO local backend init parameters */
static struct iio_local_backend local_backend_init_params = {
	.local_backend_event_read = iio_ad7124_local_backend_event_read,
	.local_backend_event_write = iio_ad7124_local_backend_event_write,
	.local_backend_buff = app_local_backend_buff,
	.local_backend_buff_len = APP_LOCAL_BACKEND_BUF_SIZE,
};
#endif

/* IIO interface init parameters */
static struct iio_init_param iio_init_params = {
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_REMOTE)
	.phy_type = USE_UART,
#else
	.phy_type = USE_LOCAL_BACKEND,
	.local_backend = &local_backend_init_params,
#endif
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	.trigs = &iio_trigger_init_params,
#endif
};

enum ad7124_iio_attr_id {
	// Channels attributes
	IIO_RAW_ATTR_ID,
	IIO_SCALE_ATTR_ID,
	IIO_OFFSET_ATTR_ID,
	NUM_OF_CHN_ATTR,

	IIO_3DB_FREQUENCY_ID,
	IIO_SAMPLING_FREQUENCY_ID,
	IIO_POWER_MODE_ID
};

/* IIOD channels attributes list */
static struct iio_attribute chn_attr[NUM_OF_IIO_DEVICES][NUM_OF_CHN_ATTR + 1] =
{
	{
		AD7124_CHN_ATTR("raw", IIO_RAW_ATTR_ID),
		AD7124_CHN_ATTR("scale", IIO_SCALE_ATTR_ID),
		AD7124_CHN_ATTR("offset", IIO_OFFSET_ATTR_ID),

		END_ATTRIBUTES_ARRAY
	}
};

/* IIOD device (global) attributes list */
static struct iio_attribute dev_attr[NUM_OF_IIO_DEVICES][NUM_OF_CHN_ATTR + 4] =
{
	{
		AD7124_CHN_ATTR("filter_low_pass_3db_frequency", IIO_3DB_FREQUENCY_ID),
		AD7124_CHN_ATTR("sampling_frequency", IIO_SAMPLING_FREQUENCY_ID),
		AD7124_CHN_ATTR("power_mode", IIO_POWER_MODE_ID),
		AD7124_CHN_AVAIL_ATTR("power_mode_available", IIO_POWER_MODE_ID),

		END_ATTRIBUTES_ARRAY
	}
};

/* IIOD channels structure */
static struct iio_channel iio_chans[NUM_OF_IIO_DEVICES][NUM_OF_CHANNELS] = {
	{
		AD7124_CH("voltage0", 0, 0, IIO_VOLTAGE),
		AD7124_CH("voltage1", 0, 1, IIO_VOLTAGE),
		AD7124_CH("voltage2", 0, 2, IIO_VOLTAGE),
		AD7124_CH("voltage3", 0, 3, IIO_VOLTAGE),
#if ((INPUT_MODE == PSUEDO_DIFFERENTIAL_MODE) && defined(DEV_AD7124_4)) || defined(DEV_AD7124_8)
		AD7124_CH("voltage4", 0, 4, IIO_VOLTAGE),
		AD7124_CH("voltage5", 0, 5, IIO_VOLTAGE),
		AD7124_CH("voltage6", 0, 6, IIO_VOLTAGE),
		AD7124_CH("voltage7", 0, 7, IIO_VOLTAGE),
#endif
#if defined(DEV_AD7124_8) && (INPUT_MODE == PSUEDO_DIFFERENTIAL_MODE)
		AD7124_CH("voltage8", 0, 8, IIO_VOLTAGE),
		AD7124_CH("voltage9", 0, 9, IIO_VOLTAGE),
		AD7124_CH("voltage10", 0, 10, IIO_VOLTAGE),
		AD7124_CH("voltage11", 0, 11, IIO_VOLTAGE),
		AD7124_CH("voltage12", 0, 12, IIO_VOLTAGE),
		AD7124_CH("voltage13", 0, 13, IIO_VOLTAGE),
		AD7124_CH("voltage14", 0, 14, IIO_VOLTAGE),
		AD7124_CH("voltage15", 0, 15, IIO_VOLTAGE)
#endif
	}
};

#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
/* Pocket lab GUI views init parameters */
struct pl_gui_views pocket_lab_gui_views[] = {
	PL_GUI_ADD_POWER_UP_DEF_VIEW,
	PL_GUI_ADD_ATTR_EDIT_DEF_VIEW,
	PL_GUI_ADD_REG_DEBUG_DEF_VIEW,
	PL_GUI_ADD_DMM_DEF_VIEW,
	PL_GUI_ADD_CAPTURE_DEF_VIEW,
	PL_GUI_ADD_ANALYSIS_DEF_VIEW,
	PL_GUI_ADD_ABOUT_DEF_VIEW,
	{ NULL }
};

/* FFT init parameters */
struct adi_fft_init_params fft_init_params = {
	.vref = AD7124_DEFAULT_REF_VOLTAGE,
	.sample_rate = SAMPLING_RATE,
	.samples_count = ADI_FFT_MAX_SAMPLES,
	.input_data_zero_scale = ADC_MAX_COUNT_BIPOLAR,
	.input_data_full_scale = ADC_MAX_COUNT_UNIPOLAR,
	.convert_data_to_volt_without_vref = &ad7124_data_to_voltage_without_vref,
	.convert_data_to_volt_wrt_vref = &ad7124_data_to_voltage_wrt_vref,
	.convert_code_to_straight_binary = &ad7124_code_to_straight_binary
};

/* Pocket lab GUI device init parameters */
struct pl_gui_device_param pl_gui_device_params = {
	.fft_params = &fft_init_params
};

/* Pocket lab GUI init parameters */
static struct pl_gui_init_param pocket_lab_gui_init_params = {
	.views = pocket_lab_gui_views,
	.device_params = &pl_gui_device_params
};

struct pl_gui_desc* pocket_lab_gui_desc;
#endif

/* Power modes available */
static char *ad7124_power_mode[] = {
	"low_power_mode",
	"mid_power_mode",
	"high_power_mode"
};

/*****************************************************************************/
/************************** Functions Declaration ****************************/
/*****************************************************************************/

/*****************************************************************************/
/************************** Functions Definition *****************************/
/*****************************************************************************/
/**
 * @brief Get the IIO scale for input channel
 * @param chn[in] - Input channel
 * @param scale[in,out] - Channel IIO scale value
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad7124_get_scale(uint8_t chn, float *scale)
{
	enum  ad7124_input_polarity polarity;
	int32_t ret;

	if (!scale) {
		return -EINVAL;
	}

	ret = ad7124_get_polarity(ad7124_dev_inst, chn, &polarity);
	if (ret) {
		return ret;
	}

	if (polarity == AD7124_BIPOLAR) {
		*scale = (AD7124_DEFAULT_REF_VOLTAGE / ADC_MAX_COUNT_BIPOLAR) * 1000;
		chn_scan[0][chn].sign = 's';
		chn_scan[0][chn].realbits = CHN_STORAGE_BITS;
	} else {
		*scale = (AD7124_DEFAULT_REF_VOLTAGE / ADC_MAX_COUNT_UNIPOLAR) * 1000;
		chn_scan[0][chn].sign = 'u';
		chn_scan[0][chn].realbits = ADC_RESOLUTION;
	}

	return 0;
}

/**
 * @brief Set sampling rate
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad7124_set_sampling_rate(void)
{
	switch (ad7124_dev_inst->power_mode) {
	case AD7124_LOW_POWER:
		ad7124_sampling_frequency = SAMPLING_RATE_LOW_POWER;
		break;

	case AD7124_MID_POWER:
		ad7124_sampling_frequency = SAMPLING_RATE_MID_POWER;
		break;

	case AD7124_HIGH_POWER:
		ad7124_sampling_frequency = SAMPLING_RATE_HIGH_POWER;
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief Getter for the raw attribute value
 * @param device[in]- pointer to IIO device instance
 * @param buf[in]- pointer to buffer holding attribute value
 * @param len[in]- length of buffer string data
 * @param channel[in]- pointer to IIO channel structure
 * @param id[in]- Attribute ID
 * @return len in case of success, negative error code otherwise.
 */
static int32_t ad7124_iio_attr_get(void *device,
				   char *buf,
				   uint32_t len,
				   const struct iio_ch_info *channel,
				   intptr_t id)
{
	static int32_t adc_data_raw = 0;
	int32_t offset = 0;
	int ret;
	enum ad7124_input_polarity polarity;
	uint16_t frequency;

	switch (id) {
	case IIO_RAW_ATTR_ID:
		ret = ad7124_single_read(device, channel->ch_num, &adc_data_raw);
		if (ret) {
			return ret;
		}
		return sprintf(buf, "%d", adc_data_raw);

	case IIO_SCALE_ATTR_ID:
		return sprintf(buf, "%f", attr_scale_val[0][channel->ch_num]);

	case IIO_OFFSET_ATTR_ID:
		ret = ad7124_get_polarity(ad7124_dev_inst, channel->ch_num, &polarity);
		if (ret) {
			return ret;
		}

		if (polarity == AD7124_BIPOLAR) {
			offset = -ADC_MAX_COUNT_BIPOLAR;
		} else {
			offset = 0;
		}

		return sprintf(buf, "%d", offset);

	case IIO_3DB_FREQUENCY_ID:
		ret = ad7124_get_3db_frequency(ad7124_dev_inst, 0, &frequency);
		if (ret) {
			return ret;
		}
		return sprintf(buf, "%d", frequency);


	case IIO_SAMPLING_FREQUENCY_ID:
		return sprintf(buf,
			       "%d",
			       ad7124_sampling_frequency);

	case IIO_POWER_MODE_ID:
		return sprintf(buf, "%s", ad7124_power_mode[ad7124_dev_inst->power_mode]);

	default:
		return -EINVAL;
	}

	return len;
}

/**
 * @brief Setter for the raw attribute value
 * @param device[in]- pointer to IIO device instance
 * @param buf[in]- pointer to buffer holding attribute value
 * @param len[in]- length of buffer string data
 * @param channel[in]- pointer to IIO channel structure
 * @param id[in]- Attribute ID
 * @return len in case of success, negative error code otherwise.
 */
static int32_t ad7124_iio_attr_set(void *device,
				   char *buf,
				   uint32_t len,
				   const struct iio_ch_info *channel,
				   intptr_t id)
{
	uint16_t frequency;
	int ret;
	int ch;

	switch (id) {
	/* Read-only attributes */
	case IIO_RAW_ATTR_ID:
	case IIO_SCALE_ATTR_ID:
	case IIO_OFFSET_ATTR_ID:
		break;

	case IIO_3DB_FREQUENCY_ID:
		frequency = no_os_str_to_uint32(buf);

		for (ch = 0; ch < NUM_OF_CHANNELS; ch++) {
			ret = ad7124_set_3db_frequency(ad7124_dev_inst, ch, frequency);
			if (ret) {
				return ret;
			}
		}

		ad7124_sampling_frequency = ad7124_get_odr(ad7124_dev_inst, 0);

		break;

	case IIO_SAMPLING_FREQUENCY_ID:
		frequency = no_os_str_to_uint32(buf);

		for (ch = 0; ch < NUM_OF_CHANNELS; ch++) {
			ret = ad7124_set_odr(ad7124_dev_inst, (float)frequency, ch);
			if (ret) {
				return ret;
			}
		}
		ad7124_sampling_frequency = ad7124_get_odr(ad7124_dev_inst, 0);

		break;

	case IIO_POWER_MODE_ID:
		for (id = AD7124_LOW_POWER; id <= AD7124_HIGH_POWER; id++) {
			if (!strcmp(buf, ad7124_power_mode[id])) {
				break;
			}
		}

		ret = ad7124_set_power_mode(ad7124_dev_inst, id);
		if (ret) {
			return ret;
		}

		ad7124_sampling_frequency = ad7124_get_odr(ad7124_dev_inst, channel->ch_num);

		break;

	default:
		return -EINVAL;
	}

	return len;
}

/**
 * @brief Attribute available getter function for AD7124 attributes
 * @param device[in, out]- Pointer to IIO device instance
 * @param buf[in]- IIO input data buffer
 * @param len[in]- Number of input bytes
 * @param channel[in] - input channel
 * @param priv[in] - Attribute private ID
 * @return len in case of success, negative error code otherwise
 */
static int ad7124_iio_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	switch (priv) {
	case IIO_POWER_MODE_ID :
		return sprintf(buf,
			       "%s %s %s",
			       ad7124_power_mode[0],
			       ad7124_power_mode[1],
			       ad7124_power_mode[2]);
	}

	return len;
}

/*!
 * @brief Attribute available setter function for AD7124 attributes
 * @param device[in, out]- Pointer to IIO device instance
 * @param buf[in]- IIO input data buffer
 * @param len[in]- Number of input bytes
 * @param channel[in] - input channel
 * @param priv[in] - Attribute private ID
 * @return len in case of success, negative error code otherwise
 */
static int ad7124_iio_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	return len;
}

/**
 * @brief Read value of the debug register
 * @param dev[in]- Pointer to IIO device instance
 * @param reg[in]- Address of the register where the data is to be written
 * @param read_val[out]- Pointer to the register data variable
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad7124_iio_debug_reg_read(void *dev,
		uint32_t reg,
		uint32_t *read_val)
{
	int32_t ret;

	if (!dev || !read_val || (reg > AD7124_MAX_REG)) {
		return -EINVAL;
	}

	/* Read the register data, extract the value */
	ret = ad7124_read_register2(dev, reg, read_val);
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
static int32_t ad7124_iio_debug_reg_write(void *dev,
		uint32_t reg,
		uint32_t write_val)
{
	int32_t ret;

	if (!dev || (reg > AD7124_MAX_REG)) {
		return -EINVAL;
	}

	/* Write value to the register */
	ret = ad7124_write_register2(dev, reg, write_val);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief	Read the IIO local backend event data
 * @param	conn[in] - connection descriptor
 * @param	buf[in] - local backend data handling buffer
 * @param	len[in] - Number of bytes to read
 * @return	0 in case of success, negative error code otherwise
 */
static int iio_ad7124_local_backend_event_read(void *conn,
		uint8_t *buf,
		uint32_t len)
{
	int ret = 0;
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
	ret = pl_gui_event_read(buf, len);
#endif
	return ret;
}

/**
 * @brief Write the IIO local backend event data
 * @param conn[in] - connection descriptor
 * @param buf[in] - local backend data handling buffer
 * @param len[in] - Number of bytes to read
 * @return 0 in case of success, negative error code otherwise
 */
static int iio_ad7124_local_backend_event_write(void *conn,
		uint8_t *buf,
		uint32_t len)
{
	int ret = 0;
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
	ret = pl_gui_event_write(buf, len);
#endif
	return ret;
}

/**
 * @brief Prepare for ADC data capture (transfer from device to memory)
 * @param dev_instance[in] - IIO device instance
 * @param chn_mask[in] - Channels select mask
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad7124_iio_prepare_transfer(void* dev_instance, uint32_t ch_mask)
{
	int32_t ret;
	uint8_t ch_id;
	uint32_t mask = 0x1;
	bool ch_status;
	num_of_active_channels = 0;
	buf_size_updated = false;
	uint16_t updated_frequency;

	/* Enable requested channels and Disable the remaining */
	for (ch_id = 0;
	     ch_id < NUM_OF_CHANNELS; ch_id++) {
		if (ch_mask & mask) {
			ch_status = true;
			num_of_active_channels++;
		} else {
			ch_status = false;
		}
		ret = ad7124_set_channel_status(ad7124_dev_inst, ch_id, ch_status);
		if (ret) {
			return ret;
		}
		mask <<= 1;
	}

	/* Update sampling frequency based on num_of_active_channels */
	if (num_of_active_channels > 1) {
		ret = ad7124_update_sampling_rate(ad7124_dev_inst, &updated_frequency);
		if (ret) {
			return ret;
		}

		ad7124_sampling_frequency = updated_frequency;
	}

	/* The UART interrupt needs to be prioritized over the GPIO (end of conversion) interrupt.
	 * If not, the GPIO interrupt may occur during the period where there is a UART read happening
	 * for the READBUF command. If UART interrupts are not prioritized, then it would lead to missing of
	 * characters in the IIO command sent from the client. */
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)  && (ACTIVE_IIO_CLIENT == IIO_CLIENT_REMOTE)
	ret = no_os_irq_set_priority(trigger_irq_desc, IRQ_INT_ID, RDY_GPIO_PRIORITY);
	if (ret) {
		return ret;
	}
#endif

	ret = ad7124_trigger_data_capture(ad7124_dev_inst);
	if (ret) {
		return ret;
	}

	/* Clear pending interrupt to ensure first sample is valid data */
	ret = no_os_irq_clear_pending(trigger_irq_desc, IRQ_INT_ID);
	if (ret) {
		return ret;
	}

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	ret = no_os_irq_enable(trigger_irq_desc, IRQ_INT_ID);
	if (ret) {
		return ret;
	}
#else /* Continuous Capture Mode */

	/* Clear pending Interrupt before enabling back the trigger.
	 * Else , a spurious interrupt is observed after a legitimate interrupt,
	 * as SPI SDO is on the same pin and is mistaken for an interrupt event */
	ret = no_os_irq_clear_pending(trigger_irq_desc, IRQ_INT_ID);
	if (ret) {
		return ret;
	}

	ret = iio_trig_enable(ad7124_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}


/**
 * @brief Perform tasks before end of current data transfer
 * @param dev_instance[in] - IIO device instance
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad7124_iio_end_transfer(void *dev)
{
	int32_t ret;

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	ret = no_os_irq_disable(trigger_irq_desc, IRQ_INT_ID);
	if (ret) {
		return ret;
	}
#else /* Continuous Capture Mode */
	ret = iio_trig_disable(ad7124_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	ret =  ad7124_stop_data_capture(ad7124_dev_inst);
	if (ret) {
		return ret;
	}

	data_capture_done = false;

	/* Put ADC to Standby mode */
	return ad7124_set_adc_mode(ad7124_dev_inst, AD7124_STANDBY);
}

/*
 * @brief Push data into IIO buffer when trigger handler IRQ is invoked
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success, negative value otherwise
 */
int32_t ad7124_trigger_handler(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	int32_t adc_read_back;

	/* As RDY pin is shared with SPI SDO pin, the interrupts are disabled
	 * to not misinterpret any activity on SDO pin as end of conversion
	 * (RDY interrupt) event */
	ret = iio_trig_disable(ad7124_hw_trig_desc);
	if (ret) {
		return ret;
	}

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = ((uint32_t)(ADC_BUFFER_SIZE /
						   iio_dev_data->buffer->bytes_per_scan)) * iio_dev_data->buffer->bytes_per_scan;
		buf_size_updated = true;
	}

	/* Read the converted data */
	ret = ad7124_read_converted_data(ad7124_dev_inst, &adc_read_back);
	if (ret) {
		return ret;
	}

	ret = no_os_cb_write(iio_dev_data->buffer->buf,
			     &adc_read_back,
			     BYTES_PER_SAMPLE);
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

	/* Enable back the interrupts to use RDY / SDO shared pin as end of
	 * conversion interrupt event monitor pin */
	ret = iio_trig_enable(ad7124_hw_trig_desc);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Interrupt Service Routine to monitor end of conversion event.
 * @param ctx[in] - Callback context (unused)
 * @return none
 * @note Callback registered for the the RDY interrupt to indicate
 * end of conversion in case of burst data capturing.
 */
void data_capture_callback(void *ctx)
{
	data_capture_done = true;
}

/**
 * @brief Read requested number of ADC samples into IIO buffer
 * @param iio_dev_data[in] - Pointer to IIO device data structure
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad7124_iio_submit_buffer(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t sample_index = 0;
	uint32_t nb_of_samples;
	uint32_t adc_raw_data = 0;
	uint32_t timeout;

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	nb_of_samples = (iio_dev_data->buffer->size / BYTES_PER_SAMPLE);
	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	while (sample_index < nb_of_samples) {
		timeout = AD7124_CONV_TIMEOUT;

		/* Check for data capture completion */
		while (!data_capture_done && timeout) {
			timeout--;
		}
		if (timeout == 0) {
			return -ETIMEDOUT;
		}
		data_capture_done = false;

		/* As DOUT/RDY uses a shared pin, interrupt is disabled to not misinterpret data low as rdy low */
		ret = no_os_irq_disable(trigger_irq_desc, IRQ_INT_ID);
		if (ret) {
			return ret;
		}

		/* Read converted samples */
		ret = ad7124_read_converted_data(ad7124_dev_inst, &adc_raw_data);
		if (ret) {
			return ret;
		}

		/* Push data into IIO circular buffer */
		ret = no_os_cb_write(iio_dev_data->buffer->buf,
				     &adc_raw_data,
				     BYTES_PER_SAMPLE);
		if (ret) {
			return ret;
		}

		/* Clear pending Interrupt before enabling back the trigger.
		 * Else, a spurious interrupt is observed after a legitimate interrupt,
		 * as SPI SDO is on the same pin and is mistaken for an interrupt event */
		ret = no_os_irq_clear_pending(trigger_irq_desc, IRQ_INT_ID);
		if (ret) {
			return ret;
		}

		/* Interrupt is enabled back after data is pushed into buffer */
		ret = no_os_irq_enable(trigger_irq_desc, IRQ_INT_ID);
		if (ret) {
			return ret;
		}

		sample_index++;
	}
#endif

	return 0;
}

/**
 * @brief Convert ADC data to voltage without Vref
 * @param data[in] - ADC data in straight binary format (signed)
 * @param chn[in] - ADC channel
 * @return voltage
 */
static float ad7124_data_to_voltage_without_vref(int32_t data, uint8_t chn)
{
	enum ad7124_input_polarity polarity;
	bool bipolar = ad7124_get_polarity(ad7124_dev_inst, chn, &polarity);

	if (bipolar) {
		return ((float)data / ADC_MAX_COUNT_BIPOLAR);
	} else {
		return ((float)data / ADC_MAX_COUNT_UNIPOLAR);
	}
}

/**
 * @brief Convert ADC data to voltage with respect to Vref
 * @param data[in] - ADC data in straight binary format (signed)
 * @param chn[in] - ADC channel
 * @return voltage
 */
static float ad7124_data_to_voltage_wrt_vref(int32_t data, uint8_t chn)
{
	enum ad7124_input_polarity polarity;
	bool bipolar = ad7124_get_polarity(ad7124_dev_inst, chn, &polarity);

	if (bipolar) {
		return ((float)data * (AD7124_DEFAULT_REF_VOLTAGE / ADC_MAX_COUNT_BIPOLAR));
	} else {
		return ((float)data * (AD7124_DEFAULT_REF_VOLTAGE / ADC_MAX_COUNT_UNIPOLAR));
	}
}

/**
 * @brief Convert ADC code to straight binary data
 * @param code[in] - ADC code (unsigned)
 * @param chn[in] - ADC channel
 * @return ADC straight binary data (signed)
 */
static int32_t ad7124_code_to_straight_binary(uint32_t code, uint8_t chn)
{
	int32_t adc_data;
	enum ad7124_input_polarity polarity;
	bool bipolar = ad7124_get_polarity(ad7124_dev_inst, chn, &polarity);

	/* Bipolar ADC Range:  (-FS) <-> 0 <-> (+FS) : 0 <-> 2^(ADC_RES-1)-1 <-> 2^(ADC_RES)-1
	   Unipolar ADC Range: 0 <-> (+FS) : 0 <-> 2^(ADC_RES)-1
	 **/
	if (bipolar) {
		/* Data output format is offset binary for bipolar mode */
		adc_data = code - ADC_MAX_COUNT_BIPOLAR;
	} else {
		/* Data output format is straight binary for unipolar mode */
		adc_data = code;
	}

	return adc_data;
}

/**
 * @brief Release resources allocated for IIO device
 * @param desc[in] - IIO device descriptor
 * @return 0 in case of success, negative error code otherwise
 */
int ad7124_iio_remove(struct iio_desc *desc)
{
	int ret;

	if (!desc) {
		return -EINVAL;
	}

	ret = iio_remove(desc);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Init for reading/writing and parameterization of a
 * 			AD7124 IIO device
 * @param desc[in,out] - IIO device descriptor
 * @param dev_indx[in] - IIO device number
 * @return 0 in case of success, negative error code otherwise
 */
int ad7124_iio_init(struct iio_device **desc, uint8_t dev_indx)
{
	struct iio_device *iio_ad7124_inst;  // IIO Device Descriptor for AD7124
	int ret;
	uint8_t chn;

	if (!desc) {
		return -EINVAL;
	}

	iio_ad7124_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad7124_inst) {
		return -ENOMEM;
	}

	iio_ad7124_inst->num_ch = NO_OS_ARRAY_SIZE(iio_chans[dev_indx]);
	iio_ad7124_inst->channels = iio_chans[dev_indx];
	iio_ad7124_inst->attributes = dev_attr[dev_indx];
	iio_ad7124_inst->debug_reg_read = ad7124_iio_debug_reg_read;
	iio_ad7124_inst->debug_reg_write = ad7124_iio_debug_reg_write;
	iio_ad7124_inst->submit = ad7124_iio_submit_buffer;
	iio_ad7124_inst->pre_enable = ad7124_iio_prepare_transfer;
	iio_ad7124_inst->post_disable = ad7124_iio_end_transfer;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_ad7124_inst->trigger_handler = ad7124_trigger_handler;
#endif

	ret = ad7124_set_sampling_rate();
	if (ret) {
		return ret;
	}

	for (chn = 0; chn < AD7124_MAX_CHANNELS; chn++) {
		ret = ad7124_set_odr(ad7124_dev_inst,
				     ad7124_sampling_frequency,
				     chn);
		if (ret) {
			free(iio_ad7124_inst);
			return ret;
		}
	}

	ad7124_sampling_frequency = ad7124_get_odr(ad7124_dev_inst, 0);

	for (chn = 0; chn < AD7124_MAX_CHANNELS; chn++) {
		ret = ad7124_get_scale(chn, &attr_scale_val[0][chn]);
		if (ret) {
			free(iio_ad7124_inst);
			return ret;
		}
	}

	*desc = iio_ad7124_inst;

	return 0;
}

/**
 * @brief Initialization of AD7124 IIO hardware trigger specific parameters
 * @param desc[in,out] - IIO hardware trigger descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad7124_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	struct iio_hw_trig_init_param ad7124_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;
	int32_t ret;

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad7124_hw_trig_init_params.irq_id = IRQ_INT_ID;
	ad7124_hw_trig_init_params.name = AD7124_IIO_TRIGGER_NAME;
	ad7124_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad7124_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad7124_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad7124_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad7124_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad7124_hw_trig_init_params.iio_desc = ad7124_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad7124_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/**
 * @brief Initialize the AD7124 IIO Interface
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad7124_iio_initialize(void)
{
	int32_t ret;
	/* AD7124 IIO Descriptor */
	static struct iio_device *ad7124_iio_dev[NUM_OF_IIO_DEVICES];

	/* Init the application specific system peripherals */
	ret = init_system();
	if (ret) {
		return (ret);
	}

	ret = get_iio_context_attributes(&iio_init_params.ctx_attrs,
					 &iio_init_params.nb_ctx_attr,
					 eeprom_desc,
					 HW_MEZZANINE_NAME,
					 STR(HW_CARRIER_NAME),
					 &hw_mezzanine_is_valid);
	if (ret) {
		return ret;
	}

	if (hw_mezzanine_is_valid) {
		/* Initialize no-os device */
		ret = ad7124_setup(&ad7124_dev_inst, &ad7124_init_params);
		if (ret) {
			return ret;
		}

		/* Initialize the IIO devices */
		ret = ad7124_iio_init(&ad7124_iio_dev[0], 0);
		if (ret) {
			return ret;
		}

		iio_device_init_params[0].dev = ad7124_dev_inst;
		iio_device_init_params[0].dev_descriptor = ad7124_iio_dev[0];

		iio_init_params.nb_devs++;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		iio_init_params.nb_trigs++;
#endif
	}

	/* AD7124 IIO device init parameters */
	iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
	iio_device_init_params[0].raw_buf = adc_data_buffer;
	iio_device_init_params[0].raw_buf_len = ADC_BUFFER_SIZE;

	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_desc;
	iio_init_params.devs = iio_device_init_params;

	ret = iio_init(&ad7124_iio_desc, &iio_init_params);
	if (ret) {
		ad7124_iio_remove(ad7124_iio_desc);
		return ret;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Initialize the AD7124 IIO trigger specific parameters */
	ret = ad7124_iio_trigger_param_init(&ad7124_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
	pocket_lab_gui_init_params.extra = &iio_init_params;
	ret = pl_gui_init(&pocket_lab_gui_desc, &pocket_lab_gui_init_params);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief Run the AD7124 IIO event handler
 * @return None
 */
void ad7124_iio_event_handler(void)
{
	(void)iio_step(ad7124_iio_desc);

#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
	pl_gui_event_handle(LVGL_TICK_TIME_MS);
#endif
}
