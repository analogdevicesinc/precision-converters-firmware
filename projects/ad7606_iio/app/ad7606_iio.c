/***************************************************************************//**
 *   @file    ad7606_iio.c
 *   @brief   Implementation of AD7606 IIO application interfaces
 *   @details This module acts as an interface for AD7606 IIO application
********************************************************************************
 * Copyright (c) 2020-2023, 2025 Analog Devices, Inc.
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
#include <errno.h>
#include <math.h>

#include "app_config.h"
#include "ad7606_iio.h"
#include "ad7606.h"
#include "ad7606_support.h"
#include "ad7606_user_config.h"
#include "common.h"
#include "no_os_error.h"
#include "no_os_gpio.h"
#include "iio_trigger.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/* ADC data to Voltage conversion scale factor for IIO client */
#define DEFAULT_SCALE		((DEFAULT_CHN_RANGE / ADC_MAX_COUNT_BIPOLAR) * 1000)

/* LSB Threshold to entry into open circuit detection as per datasheet */
#define MANUAL_OPEN_DETECT_ENTRY_TRHLD			350

/* Manual open circuit detect LSB threshold @50K Rpd as per datasheet */
#define MANUAL_OPEN_DETECT_THRESHOLD_RPD50K		20

/* Number of consecutive conversions (N) in manual open circuit detection */
#define MANUAL_OPEN_DETECT_CONV_CNTS			10

/* LSB Threshold b/w consecutive N conversions */
#define MANUAL_OPEN_DETECT_CONV_TRSHLD			10

/* Number of common mode conversions in manual open circuit detect */
#define MANUAL_OPEN_DETECT_CM_CNV_CNT			3

/* Max number of queue counts for auto mode open circuit detection */
#define AUTO_OPEN_DETECT_QUEUE_MAX_CNT			128
#define AUTO_OPEN_DETECT_QUEUE_EXTRA_CONV_CNT	15

/* Maximum ADC calibration gain value */
#define ADC_CALIBRATION_GAIN_MAX		64.0

#if defined(DEV_AD7606C_18)
#define	OFFSET_REG_RESOLUTION		4
#else
#define	OFFSET_REG_RESOLUTION		1
#endif

/* Number of IIO devices */
#define NUM_OF_IIO_DEVICES	1

/* IIO trigger name */
#define IIO_TRIGGER_NAME		"ad7606_iio_trigger"

/* Number of data storage bits (needed for IIO client to plot ADC data) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

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

/* IIO interface descriptor */
static struct iio_desc *p_ad7606_iio_desc;

/**
 * Pointer to the struct representing the AD7606 IIO device
 */
struct ad7606_dev *p_ad7606_dev_inst = NULL;

/* IIO hw trigger descriptor */
static struct iio_hw_trig *ad7606_hw_trig_desc;

/* Number of active channels */
static volatile uint8_t num_of_active_channels;

/* Active channels list */
static volatile uint8_t active_chns[AD7606X_ADC_CHANNELS];

static volatile uint8_t chn_indx = 0;

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/* Device attributes with default values */

/* AD7606 Attribute IDs */
enum ad7606_attribute_id {
	RAW_ATTR_ID,
	SCALE_ATTR_ID,
	OFFSET_ATTR_ID,
	SAMPLING_FREQ_ATTR_ID
};

/* Power down mode values string representation (possible values specified in datasheet) */
static char *operating_mode_str[] = {
	"0  (Normal Mode)",
	"1  (Standby Mode)",
	"2  (Auto Standby Mode)",
	"3  (Shutdown Mode)"
};

/* Bandwidth values string */
static char *bandwidth_str[] = {
	"0  (Low)",
	"1  (High)"
};

/* Channel range values string representation (possible values specified in datasheet) */
static char *chn_range_str[] = {
#if defined(DEV_AD7606B)
	"0  (+/-2.5V SE)", "1  (+/-5.0V SE)", "2  (+/-10.0V SE)", "3  (+/-10.0V SE)",
	"4  (+/-10.0V SE)", "5  (+/-10.0V SE)", "6  (+/-10.0V SE)", "7  (+/-10.0V SE)",
	"8  (+/-10.0V SE)", "9  (+/-10.0V SE)", "10  (+/-10.0V SE)", "11  (+/-10.0V SE)",
#elif defined(DEV_AD7606C_18) || defined(DEV_AD7606C_16)
	"0  (+/-2.5V SE)", "1  (+/-5.0V SE)", "2  (+/-6.25V SE)", "3  (+/-10.0V SE)",
	"4  (+/-12.5V SE)", "5  (0 to 5V SE)", "6  (0 to 10V SE)", "7  (0 to 12.5V SE)",
	"8  (+/-5.0V Diff)", "9  (+/-10.0V Diff)", "10  (+/-12.5V Diff)", "11  (+/-20.0V Diff)"
#elif defined(DEV_AD7609)
	"0  (+/-10.0V SE)", "1  (+/-20.0V SE)"
#else
	"0  (+/-5.0V SE)", "1  (+/-10.0V SE)"
#endif
};

/* Oversampling values string representation (possible values specified in datasheet) */
static char *oversampling_val_str[] = {
	"0 (no oversampling)", "1 (oversampling by 2)", "2 (oversampling by 4)",
	"3 (oversampling by 8)", "4 (oversampling by 16)", "5 (oversampling by 32)",
	"6 (oversampling by 64)", "7 (oversampling by 128)", "8 (oversampling by 256)"
};


/* Channel range values string representation (possible values specified in datasheet) */
static float chn_range_val[] = {
#if defined(DEV_AD7606B)
	2.5, 5.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0, 10.0
#elif defined(DEV_AD7606C_18) || defined(DEV_AD7606C_16)
	2.5, 5.0, 6.25, 10.0, 12.5, 5.0, 10.0, 12.5, 5.0, 10.0, 12.5, 20.0
#elif defined(DEV_AD7609)
	10.0, 20.0
#else
	5.0, 10.0
#endif
};

/* Range value per channel */
static float attr_chn_range[AD7606X_ADC_CHANNELS];
/* Scale value per channel */
static float attr_scale_val[AD7606X_ADC_CHANNELS];

/* Scale value per channel */
static polarity_e attr_polarity_val[AD7606X_ADC_CHANNELS];

/* IIOD channels scan structure */
static struct scan_type chn_scan[AD7606X_ADC_CHANNELS];

/* Channel range */
typedef enum {
	LOW,
	HIGH
} range_e;

/* Open detect auto mode QUEUE register count */
static uint8_t open_detect_queue_cnts[AD7606X_ADC_CHANNELS] = {
	0
};

/* ADC gain calibration Rfilter value (in Kohms) */
static uint8_t gain_calibration_reg_val[AD7606X_ADC_CHANNELS] = {
	0
};

/* Gain calibration status */
static bool gain_calibration_done = false;

/* Open circuit mode detection flags */
static bool open_circuit_detection_done = false;
static bool open_circuit_detection_error = false;
static bool open_circuit_detect_read_done = false;

/******************************************************************************/
/************************ Functions Prototypes ********************************/
/******************************************************************************/

static int32_t reformat_adc_raw_data(uint32_t adc_raw_data, uint8_t chn);
static float convert_adc_data_to_voltage(int32_t adc_data, float scale);
static void update_vltg_conv_scale_factor(float chn_range, polarity_e polarity,
		uint8_t chn);
static void save_local_attributes(void);

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/*!
 * @brief	Getter functions for AD7606 attributes
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 * @Note	This sampling_frequency attribute is used to define the
 *			timeout period in IIO client during data capture.
 *			Timeout (1 chn) = (requested samples * sampling frequency) + 1sec
 *			Timeout (n chns) = ((requested samples * n) / sampling frequency) + 1sec
 *			e.g. If sampling frequency = 31.5KSPS, requested samples = 4000, n=1min or 8max
 *			Timeout (1 chn) = (4000 / 315000) + 1 = ~1.13sec
 *			Timeout (8 chns) = ((4000 * 8) / 315000) + 1 = ~2.01sec
 */
static int iio_ad7606_attr_get(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	static uint32_t adc_data_raw = 0;
	int32_t offset = 0;
	uint32_t val;
	int32_t	 ret;

	val = no_os_str_to_uint32(buf);

	switch (priv) {
	case RAW_ATTR_ID:
		/* Capture the raw adc data */
		ret = ad7606_read_single_sample(device, &adc_data_raw,
						(uint8_t)channel->ch_num);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%d", adc_data_raw);

	case SCALE_ATTR_ID:
		return snprintf(buf, len, "%.10f", attr_scale_val[channel->ch_num]);

	case OFFSET_ATTR_ID:
		if (attr_polarity_val[channel->ch_num] == BIPOLAR) {
			if (adc_data_raw >= ADC_MAX_COUNT_BIPOLAR) {
				offset = -ADC_MAX_COUNT_UNIPOLAR;
			}
		}

		return sprintf(buf, "%d", offset);

	case SAMPLING_FREQ_ATTR_ID:
		/* Sampling frequency for IIO oscilloscope timeout purpose.
		 * Does not indicate an actual sampling rate of device.
		 * Refer the 'note' in function description above for timeout calculations */
		return sprintf(buf, "%d", SAMPLING_RATE);

	default:
		break;
	}

	return len;
}

/*!
 * @brief	Setter functions for AD7606 attributes
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 */
static int iio_ad7606_attr_set(void *device,
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
		/* All are read-only attributes */
		break;

	default:
		break;
	}

	return len;
}

/*!
 * @brief	Getter/Setter for the operating mode attribute value
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 * @note	Available only for AD7606B and AD7606C
 */
static int get_operating_mode(void *device,
			      char *buf,
			      uint32_t len,
			      const struct iio_ch_info *channel,
			      intptr_t id)
{
	uint8_t read_val;
	uint8_t operating_mode_value;

	if (ad7606_spi_reg_read(device, AD7606_REG_CONFIG, &read_val) == 0) {
		operating_mode_value = (read_val & AD7606_CONFIG_OPERATION_MODE_MSK);

		if (operating_mode_value < sizeof(operating_mode_str) / sizeof(
			    operating_mode_str[0])) {
			return sprintf(buf, "%s", operating_mode_str[operating_mode_value]);
		}
	}

	return -EINVAL;
}

static int set_operating_mode(void *device,
			      char *buf,
			      uint32_t len,
			      const struct iio_ch_info *channel,
			      intptr_t id)
{
	uint8_t operating_mode_value;

	(void)sscanf(buf, "%d", &operating_mode_value);

	if (operating_mode_value < sizeof(operating_mode_str) / sizeof(
		    operating_mode_str[0])) {
		if (ad7606_reg_write_mask(device,
					  AD7606_REG_CONFIG,
					  AD7606_CONFIG_OPERATION_MODE_MSK,
					  operating_mode_value) == 0) {
			return len;
		}
	}

	return -EINVAL;
}


/*!
 * @brief	Getter/Setter for the power down mode attribute value
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 * @note	Available for all devices except AD7606B and AD7606C
 */
static int get_power_down_mode(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t id)
{
	uint8_t gpio_stby_val;
	uint8_t gpio_range_val;

	if (no_os_gpio_get_value(((struct ad7606_dev *)device)->gpio_stby_n,
				 &gpio_stby_val) == 0) {
		if (no_os_gpio_get_value(((struct ad7606_dev *)device)->gpio_range,
					 &gpio_range_val) == 0) {

			if (gpio_stby_val) {
				return sprintf(buf, "%s", operating_mode_str[AD7606_NORMAL]);
			} else {
				if (gpio_range_val) {
					return sprintf(buf, "%s", operating_mode_str[AD7606_STANDBY]);
				} else {
					return sprintf(buf, "%s", operating_mode_str[AD7606_SHUTDOWN]);
				}
			}
		}
	}

	return -EINVAL;
}

static int set_power_down_mode(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t id)
{
	uint8_t power_down_mode_value;
	static enum ad7606_op_mode prev_power_down_mode = AD7606_NORMAL;
	struct ad7606_config dev_config = {0};

	sscanf(buf, "%d", &power_down_mode_value);

	if (power_down_mode_value < (sizeof(operating_mode_str) / sizeof(
					     operating_mode_str[0]))) {

		dev_config.op_mode = power_down_mode_value;

		switch (power_down_mode_value) {
		case AD7606_NORMAL:
			if (ad7606_set_config(device, dev_config) == 0) {
				/* Reset the device if previous power down mode was either standby
				 * or shutdown */
				if (prev_power_down_mode != AD7606_NORMAL) {

					/* Power-up wait time */
					no_os_mdelay(1);

					/* Toggle reset pin */
					if (no_os_gpio_set_value(((struct ad7606_dev *)device)->gpio_reset,
								 NO_OS_GPIO_HIGH) == 0) {
						no_os_mdelay(1);

						if (no_os_gpio_set_value(((struct ad7606_dev *)device)->gpio_reset,
									 NO_OS_GPIO_LOW) == 0) {
							prev_power_down_mode = AD7606_NORMAL;
							return len;
						}
					}
				}
			}
			break;

		case AD7606_STANDBY:
			if (ad7606_set_config(device, dev_config) == 0) {
				prev_power_down_mode = AD7606_STANDBY;
				return len;
			}
			break;

		case AD7606_SHUTDOWN:
			if (ad7606_set_config(device, dev_config) == 0) {
				prev_power_down_mode = AD7606_SHUTDOWN;
				return len;
			}
			break;

		default:
			break;
		}
	}

	return -EINVAL;
}


/*!
 * @brief	Getter/Setter for the range attribute value
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 * @note	Available for all devices except AD7606B and AD7606C
 */
static int get_range(void *device,
		     char *buf,
		     uint32_t len,
		     const struct iio_ch_info *channel,
		     intptr_t id)
{
	uint8_t gpio_range_val;
	struct ad7606_dev *dev = device;

	if (no_os_gpio_get_value(dev->gpio_range, &gpio_range_val) == 0) {
		if (gpio_range_val) {
			return sprintf(buf, "%s", chn_range_str[HIGH]);
		} else {
			return sprintf(buf, "%s", chn_range_str[LOW]);
		}
	}

	return -EINVAL;
}

static int set_range(void *device,
		     char *buf,
		     uint32_t len,
		     const struct iio_ch_info *channel,
		     intptr_t id)
{
	uint8_t range_value;
	struct ad7606_dev *dev = device;

	(void)sscanf(buf, "%d", &range_value);

	if (range_value < (sizeof(chn_range_str) / sizeof(chn_range_str[0]))) {
		if (range_value == LOW) {
			if (no_os_gpio_set_value(dev->gpio_range, NO_OS_GPIO_LOW) == 0) {
				return len;
			}
		} else {
			if (no_os_gpio_set_value(dev->gpio_range, NO_OS_GPIO_HIGH) == 0) {
				return len;
			}
		}
	}

	return -EINVAL;
}


/*!
 * @brief	Getter/Setter for the oversampling attribute value
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 * @note	Available for all devices except AD7606B and AD7606C
 */
static int get_oversampling(void *device,
			    char *buf,
			    uint32_t len,
			    const struct iio_ch_info *channel,
			    intptr_t id)
{
	uint8_t oversampling_value;
	uint8_t read_val;
	uint8_t gpio_osr0_val;
	uint8_t gpio_osr1_val;
	uint8_t gpio_osr2_val;
	struct ad7606_dev *dev = device;

#if defined(DEV_AD7606B) || defined(DEV_AD7606C_18) || defined(DEV_AD7606C_16)
	if (ad7606_spi_reg_read(device,
				AD7606_REG_OVERSAMPLING,
				&read_val) == 0) {
		oversampling_value = (read_val & AD7606_OVERSAMPLING_MSK);

		if (oversampling_value < sizeof(oversampling_val_str) / sizeof(
			    oversampling_val_str[0])) {
			return sprintf(buf, "%s", oversampling_val_str[oversampling_value]);
		}
	}
#else
	if (no_os_gpio_get_value(dev->gpio_os0, &gpio_osr0_val) == 0) {
		if (no_os_gpio_get_value(dev->gpio_os1, &gpio_osr1_val) == 0) {
			if (no_os_gpio_get_value(dev->gpio_os2, &gpio_osr2_val) == 0) {
				oversampling_value = (gpio_osr2_val << 2) | (gpio_osr1_val << 1) |
						     gpio_osr0_val;

				if (oversampling_value < (sizeof(oversampling_val_str) / sizeof(
								  oversampling_val_str[0]))) {
					return sprintf(buf, "%s", oversampling_val_str[oversampling_value]);
				}
			}
		}
	}
#endif

	return -EINVAL;
}

static int set_oversampling(void *device,
			    char *buf,
			    uint32_t len,
			    const struct iio_ch_info *channel,
			    intptr_t id)
{
	uint8_t oversampling_value;
	struct ad7606_oversampling oversampling_cfg;

	(void)sscanf(buf, "%d", &oversampling_value);

	if (oversampling_value < (sizeof(oversampling_val_str) / sizeof(
					  oversampling_val_str[0]))) {

		oversampling_cfg.os_pad = 0;
		oversampling_cfg.os_ratio = oversampling_value;

		ad7606_set_oversampling(device, oversampling_cfg);

		return len;
	}

	return -EINVAL;
}


/*!
 * @brief	Getter/Setter for the bandwidth attribute value
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 * @note	Available for only AD7606C
 */
static int get_bandwidth(void *device,
			 char *buf,
			 uint32_t len,
			 const struct iio_ch_info *channel,
			 intptr_t id)
{
	uint8_t bw_value;
	uint8_t read_val;

	if (ad7606_spi_reg_read(device,
				AD7606_REG_BANDWIDTH,
				&read_val) == 0) {
		bw_value = (read_val >> (channel->ch_num)) & 0x1;

		if (bw_value < sizeof(bandwidth_str) / sizeof(
			    bandwidth_str[0])) {
			return sprintf(buf, "%s", bandwidth_str[bw_value]);
		}
	}

	return -EINVAL;
}

static int set_bandwidth(void *device,
			 char *buf,
			 uint32_t len,
			 const struct iio_ch_info *channel,
			 intptr_t id)
{
	uint8_t bw_value;
	uint8_t read_val;

	(void)sscanf(buf, "%d", &bw_value);

	if (bw_value < sizeof(bandwidth_str) / sizeof(
		    bandwidth_str[0])) {
		if (ad7606_spi_reg_read(device,
					AD7606_REG_BANDWIDTH,
					&read_val) == 0) {
			if (bw_value) {
				read_val |= (1 << (channel->ch_num));
			} else {
				read_val &= (~(1 << (channel->ch_num)));
			}

			if (ad7606_spi_reg_write(device,
						 AD7606_REG_BANDWIDTH,
						 read_val) == 0) {
				return len;
			}
		}
	}

	return -EINVAL;
}


/*!
 * @brief	Getter/Setter for the channel range attribute value
 * @@param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 * @note	Available only for AD7606B and AD7606C
 */
static int get_chn_range(void *device,
			 char *buf,
			 uint32_t len,
			 const struct iio_ch_info *channel,
			 intptr_t id)
{
	uint8_t read_val;
	uint8_t chn_range;

	if (ad7606_spi_reg_read(device, AD7606_REG_RANGE_CH_ADDR(channel->ch_num),
				&read_val) == 0) {
		if (((channel->ch_num) % 2) != 0) {
			read_val >>= CHANNEL_RANGE_MSK_OFFSET;
			chn_range = read_val;
		} else {
			chn_range = (read_val & AD7606_RANGE_CH_MSK(channel->ch_num));
		}

		if (chn_range < sizeof(chn_range_str) / sizeof(chn_range_str[0])) {
			attr_chn_range[channel->ch_num] = chn_range_val[chn_range];
			attr_polarity_val[channel->ch_num] = ad7606_get_input_polarity(chn_range);

			return sprintf(buf, "%s", chn_range_str[chn_range]);
		}
	}

	return -EINVAL;
}

static int set_chn_range(void *device,
			 char *buf,
			 uint32_t len,
			 const struct iio_ch_info *channel,
			 intptr_t id)
{
	uint8_t chn_range;

	(void)sscanf(buf, "%d", &chn_range);

	if (chn_range < sizeof(chn_range_val) / sizeof(chn_range_val[0])) {
		/* Update scale factor based on channel range and polarity  */
		attr_polarity_val[channel->ch_num] = ad7606_get_input_polarity(chn_range);
		attr_chn_range[channel->ch_num] = chn_range_val[chn_range];
		update_vltg_conv_scale_factor(attr_chn_range[channel->ch_num],
					      attr_polarity_val[channel->ch_num], channel->ch_num);

		if (((channel->ch_num) % 2) != 0) {
			chn_range <<= CHANNEL_RANGE_MSK_OFFSET;
		}

		if (ad7606_reg_write_mask(device,
					  AD7606_REG_RANGE_CH_ADDR(channel->ch_num),
					  AD7606_RANGE_CH_MSK(channel->ch_num),
					  chn_range) == 0) {
			return len;
		}
	}

	return -EINVAL;
}


/*!
 * @brief	Getter/Setter for the channel offset attribute value
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 * @note	Available only for AD7606B and AD7606C
 */
static int get_chn_offset(void *device,
			  char *buf,
			  uint32_t len,
			  const struct iio_ch_info *channel,
			  intptr_t id)
{
	uint8_t chn_offset_value;

	if (ad7606_spi_reg_read(device, AD7606_REG_OFFSET_CH(channel->ch_num),
				&chn_offset_value) == 0) {
		return sprintf(buf, "%d", chn_offset_value);
	}

	return -EINVAL;
}

static int set_chn_offset(void *device,
			  char *buf,
			  uint32_t len,
			  const struct iio_ch_info *channel,
			  intptr_t id)
{
	uint8_t chn_offset_value = 0;

	(void)sscanf(buf, "%d", &chn_offset_value);

	if (ad7606_set_ch_offset(device, channel->ch_num,
				 chn_offset_value) == 0) {
		return len;
	}

	return -EINVAL;
}


/*!
 * @brief	Getter/Setter for the channel pahse offset attribute value
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 * @note	Available only for AD7606B and AD7606C
 */
static int get_chn_phase_offset(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t id)
{
	uint8_t chn_phase_offset_value;

	if (ad7606_spi_reg_read(device,
				AD7606_REG_PHASE_CH(channel->ch_num),
				&chn_phase_offset_value) == 0) {
		return sprintf(buf, "%d", chn_phase_offset_value);
	}

	return -EINVAL;
}

static int set_chn_phase_offset(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t id)
{
	uint8_t chn_phase_offset_value = 0;

	(void)sscanf(buf, "%d", &chn_phase_offset_value);

	if (ad7606_set_ch_phase(device, channel->ch_num,
				chn_phase_offset_value) == 0) {
		return len;
	}

	return -EINVAL;
}


/*!
 * @brief	Getter/Setter for the channel temperature attribute value
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 * @note	Available only for AD7606B and AD7606C
 */
static int get_chn_temperature(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t id)
{
	uint32_t adc_data_raw;
	int32_t adc_data;
	float temperature;
	float voltage;
	int32_t ret;

	/* Configure the channel multiplexer to select temperature read */
	if (ad7606_reg_write_mask(device,
				  AD7606_REG_DIAGNOSTIC_MUX_CH(channel->ch_num),
				  AD7606_DIAGN_MUX_CH_MSK(channel->ch_num),
				  AD7606_DIAGN_MUX_CH_VAL((channel->ch_num),
						  TEMPERATURE_MUX)) == 0) {

		/* Allow to settle Mux channel */
		no_os_udelay(100);

		/* Sample the channel and read conversion result */
		ret = ad7606_read_single_sample(device, &adc_data_raw,
						channel->ch_num);
		if (ret) {
			return ret;
		}

		adc_data = reformat_adc_raw_data(adc_data_raw, channel->ch_num);

		/* Convert ADC data into equivalent voltage */
		voltage = convert_adc_data_to_voltage(adc_data,
						      attr_scale_val[channel->ch_num]);

		/* Obtain the temperature using equation specified in device datasheet */
		temperature = ((voltage - 0.69068) / 0.019328) + 25.0;

		/* Change channel mux back to analog input */
		(void)ad7606_reg_write_mask(device,
					    AD7606_REG_DIAGNOSTIC_MUX_CH(channel->ch_num),
					    AD7606_DIAGN_MUX_CH_MSK(channel->ch_num),
					    AD7606_DIAGN_MUX_CH_VAL((channel->ch_num),
							    ANALOG_INPUT_MUX));

		return sprintf(buf, "%f", temperature);
	}

	return -EINVAL;
}

static int set_chn_temperature(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t id)
{
	// NA- Can't set temperature
	return -EINVAL;
}


/*!
 * @brief	Getter/Setter for the channel Vref attribute value
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 * @note	Available only for AD7606B and AD7606C
 */
static int get_chn_vref(void *device,
			char *buf,
			uint32_t len,
			const struct iio_ch_info *channel,
			intptr_t id)
{
	float vref_voltge;
	uint32_t adc_data_raw;
	int32_t adc_data;
	int32_t ret;

	/* Configure the channel multiplexer to select Vref read */
	if (ad7606_reg_write_mask(device,
				  AD7606_REG_DIAGNOSTIC_MUX_CH(channel->ch_num),
				  AD7606_DIAGN_MUX_CH_MSK(channel->ch_num),
				  AD7606_DIAGN_MUX_CH_VAL((channel->ch_num),
						  VREF_MUX)) == 0) {

		/* Allow to settle Mux channel */
		no_os_udelay(100);

		/* Sample the channel and read conversion result */
		ret = ad7606_read_single_sample(device, &adc_data_raw,
						channel->ch_num);
		if (ret) {
			return ret;
		}

		adc_data = reformat_adc_raw_data(adc_data_raw, channel->ch_num);

		/* Convert ADC data into equivalent voltage */
		vref_voltge = convert_adc_data_to_voltage(adc_data,
				attr_scale_val[channel->ch_num]);

		/* Divide by 4 since Vref Mux has 4x multiplier on it */
		vref_voltge /= VREF_MUX_MULTIPLIER;

		/* Change channel mux back to analog input */
		(void)ad7606_reg_write_mask(device,
					    AD7606_REG_DIAGNOSTIC_MUX_CH(channel->ch_num),
					    AD7606_DIAGN_MUX_CH_MSK(channel->ch_num),
					    AD7606_DIAGN_MUX_CH_VAL((channel->ch_num),
							    ANALOG_INPUT_MUX));

		return sprintf(buf, "%f", vref_voltge);
	}

	return -EINVAL;
}

static int set_chn_vref(void *device,
			char *buf,
			uint32_t len,
			const struct iio_ch_info *channel,
			intptr_t id)
{
	// NA- Can't set Vref
	return - EINVAL;
}


/*!
 * @brief	Getter/Setter for the channel Vdrive attribute value
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 * @note	Available only for AD7606B and AD7606C
 */
static int get_chn_vdrive(void *device,
			  char *buf,
			  uint32_t len,
			  const struct iio_ch_info *channel,
			  intptr_t id)
{
	float vdrive_voltge;
	uint32_t adc_data_raw;
	int32_t adc_data;
	int32_t ret;

	/* Configure the channel multiplexer to select Vdrive read */
	if (ad7606_reg_write_mask(device,
				  AD7606_REG_DIAGNOSTIC_MUX_CH(channel->ch_num),
				  AD7606_DIAGN_MUX_CH_MSK(channel->ch_num),
				  AD7606_DIAGN_MUX_CH_VAL((channel->ch_num),
						  VDRIVE_MUX)) == 0) {

		/* Allow to settle Mux channel */
		no_os_udelay(100);

		/* Sample the channel and read conversion result */
		ret = ad7606_read_single_sample(device, &adc_data_raw,
						channel->ch_num);
		if (ret) {
			return ret;
		}

		adc_data = reformat_adc_raw_data(adc_data_raw, channel->ch_num);

		/* Convert ADC data into equivalent voltage */
		vdrive_voltge = convert_adc_data_to_voltage(adc_data,
				attr_scale_val[channel->ch_num]);

		/* Change channel mux back to analog input */
		(void)ad7606_reg_write_mask(device,
					    AD7606_REG_DIAGNOSTIC_MUX_CH(channel->ch_num),
					    AD7606_DIAGN_MUX_CH_MSK(channel->ch_num),
					    AD7606_DIAGN_MUX_CH_VAL((channel->ch_num),
							    ANALOG_INPUT_MUX));

		return sprintf(buf, "%f", vdrive_voltge);
	}

	return -EINVAL;
}

static int set_chn_vdrive(void *device,
			  char *buf,
			  uint32_t len,
			  const struct iio_ch_info *channel,
			  intptr_t id)
{
	// NA- Can't set Vdrive
	return - EINVAL;
}


/*!
 * @brief	Getter/Setter for the channel ALDO attribute value
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 * @note	Available only for AD7606B and AD7606C
 */
static int get_chn_aldo(void *device,
			char *buf,
			uint32_t len,
			const struct iio_ch_info *channel,
			intptr_t id)
{
	float aldo_voltge;
	uint32_t adc_data_raw;
	int32_t adc_data;
	int32_t ret;

	/* Configure the channel multiplexer to select ALDO read */
	if (ad7606_reg_write_mask(device,
				  AD7606_REG_DIAGNOSTIC_MUX_CH(channel->ch_num),
				  AD7606_DIAGN_MUX_CH_MSK(channel->ch_num),
				  AD7606_DIAGN_MUX_CH_VAL((channel->ch_num),
						  ALDO_MUX)) == 0) {

		/* Allow to settle Mux channel */
		no_os_udelay(100);

		/* Sample the channel and read conversion result */
		ret = ad7606_read_single_sample(device, &adc_data_raw,
						channel->ch_num);
		if (ret) {
			return ret;
		}

		adc_data = reformat_adc_raw_data(adc_data_raw, channel->ch_num);

		/* Convert ADC data into equivalent voltage */
		aldo_voltge = convert_adc_data_to_voltage(adc_data,
				attr_scale_val[channel->ch_num]);

		/* Divide by 4 since ALDO Mux has 4x multiplier on it */
		aldo_voltge /= VREF_MUX_MULTIPLIER;

		/* Change channel mux back to analog input */
		(void)ad7606_reg_write_mask(device,
					    AD7606_REG_DIAGNOSTIC_MUX_CH(channel->ch_num),
					    AD7606_DIAGN_MUX_CH_MSK(channel->ch_num),
					    AD7606_DIAGN_MUX_CH_VAL((channel->ch_num),
							    ANALOG_INPUT_MUX));

		return sprintf(buf, "%f", aldo_voltge);
	}

	return -EINVAL;
}

static int set_chn_aldo(void *device,
			char *buf,
			uint32_t len,
			const struct iio_ch_info *channel,
			intptr_t id)
{
	// NA- Can't set ALDO
	return - EINVAL;
}


/*!
 * @brief	Getter/Setter for the channel DLDO attribute value
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 * @note	Available only for AD7606B and AD7606C
 */
static int get_chn_dldo(void *device,
			char *buf,
			uint32_t len,
			const struct iio_ch_info *channel,
			intptr_t id)
{
	float dldo_voltge;
	uint32_t adc_data_raw;
	int32_t adc_data;
	int32_t ret;

	/* Configure the channel multiplexer to select DLDO read */
	if (ad7606_reg_write_mask(device,
				  AD7606_REG_DIAGNOSTIC_MUX_CH(channel->ch_num),
				  AD7606_DIAGN_MUX_CH_MSK(channel->ch_num),
				  AD7606_DIAGN_MUX_CH_VAL((channel->ch_num),
						  DLDO_MUX)) == 0) {

		/* Allow to settle Mux channel */
		no_os_udelay(100);

		/* Sample the channel and read conversion result */
		ret = ad7606_read_single_sample(device, &adc_data_raw,
						channel->ch_num);
		if (ret) {
			return ret;
		}

		adc_data = reformat_adc_raw_data(adc_data_raw, channel->ch_num);

		/* Convert ADC data into equivalent voltage */
		dldo_voltge = convert_adc_data_to_voltage(adc_data,
				attr_scale_val[channel->ch_num]);

		/* Divide by 4 since ALDO Mux has 4x multiplier on it */
		dldo_voltge /= VREF_MUX_MULTIPLIER;

		/* Change channel mux back to analog input */
		(void)ad7606_reg_write_mask(device,
					    AD7606_REG_DIAGNOSTIC_MUX_CH(channel->ch_num),
					    AD7606_DIAGN_MUX_CH_MSK(channel->ch_num),
					    AD7606_DIAGN_MUX_CH_VAL((channel->ch_num),
							    ANALOG_INPUT_MUX));

		return sprintf(buf, "%f", dldo_voltge);
	}

	return -EINVAL;
}

static int set_chn_dldo(void *device,
			char *buf,
			uint32_t len,
			const struct iio_ch_info *channel,
			intptr_t id)
{
	// NA- Can't set DLDO
	return - EINVAL;
}


/*!
 * @brief	Getter/Setter for the channel open circuit detect manual attribute value
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 */
static int get_chn_open_circuit_detect_manual(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t id)
{
	int32_t prev_adc_code, curr_adc_code;
	uint32_t adc_data_raw;
	bool open_detect_flag = false;
	bool open_detect_done = false;
	uint8_t cnt;
	int32_t ret;

	/* Enter into manual open circuit detection mode */
	do {
		if (ad7606_spi_reg_write(device, AD7606_REG_OPEN_DETECT_QUEUE, 1) == 0) {
			/* Read the ADC on selected chnnel (first reading post open circuit detection start) */
			ret = ad7606_read_single_sample(device, &adc_data_raw,
							channel->ch_num);
			if (ret) {
				return ret;
			}

			prev_adc_code = reformat_adc_raw_data(adc_data_raw, channel->ch_num);

			/* Perform N conversions and monitor the code delta */
			for (cnt = 0; cnt < MANUAL_OPEN_DETECT_CONV_CNTS; cnt++) {
				/* Check if code is within 350LSB (nearest ZS code) */
				if (prev_adc_code >= 0 && prev_adc_code < MANUAL_OPEN_DETECT_ENTRY_TRHLD) {
					/* Perform next conversion and read the result */
					ret = ad7606_read_single_sample(device, &adc_data_raw,
									channel->ch_num);
					if (ret) {
						return ret;
					}

					curr_adc_code = reformat_adc_raw_data(adc_data_raw, channel->ch_num);

					/* Check if delta b/w current and previus reading is within 10 LSB code */
					if (abs(curr_adc_code - prev_adc_code) > MANUAL_OPEN_DETECT_CONV_TRSHLD) {
						open_detect_done = true;
						break;
					}

					/* Get the previous code */
					prev_adc_code = curr_adc_code;
				} else {
					open_detect_done = true;
					break;
				}
			}

			/* Break if open circuit detection aborted (in case above conditions not met) */
			if (open_detect_done)
				break;

			/* Set common mode high (enabling open circuit detect on selected channel) */
			if (ad7606_spi_reg_write(device,
						 AD7606_REG_OPEN_DETECT_ENABLE,
						 (1 << (channel->ch_num))) == 0) {

				/* Perform next conversions (~2-3) and read the result (with common mode set high) */
				for (cnt = 0; cnt < MANUAL_OPEN_DETECT_CM_CNV_CNT; cnt++) {
					no_os_udelay(100);
					ret = ad7606_read_single_sample(device, &adc_data_raw,
									channel->ch_num);
					if (ret) {
						return ret;
					}

					curr_adc_code = reformat_adc_raw_data(adc_data_raw, channel->ch_num);
				}

				/* Check if delta b/w common mode high code and previous N conversion code is > threshold */
				if ((curr_adc_code - prev_adc_code) < MANUAL_OPEN_DETECT_THRESHOLD_RPD50K) {
					open_detect_done = true;
					break;
				}
			} else {
				return -EINVAL;
			}

			/* Set common mode low (disabling open circuit detect on channel) */
			if (ad7606_spi_reg_write(device,
						 AD7606_REG_OPEN_DETECT_ENABLE,
						 0) == 0) {
				/* Perform next conversion and read the result (with common mode set low) */
				ret = ad7606_read_single_sample(device, &adc_data_raw,
								channel->ch_num);
				if (ret) {
					return ret;
				}

				curr_adc_code = reformat_adc_raw_data(adc_data_raw, channel->ch_num);

				/* Check if delta b/w common mode low code and previous N conversion code is < threshold */
				if (abs(curr_adc_code - prev_adc_code) < MANUAL_OPEN_DETECT_THRESHOLD_RPD50K) {
					open_detect_flag = true;
					open_detect_done = true;
				}
			} else {
				return -EINVAL;
			}
		} else {
			return -EINVAL;
		}
	} while (0);

	/* Disable open detect mode */
	(void)ad7606_spi_reg_write(device, AD7606_REG_OPEN_DETECT_QUEUE, 0);

	if (open_detect_done) {
		if (open_detect_flag) {
			strcpy(buf, "Open Circuit Detected");
		} else {
			strcpy(buf, "Open Circuit Not Detected");
		}

		return len;
	}

	return -EINVAL;
}

static int set_chn_open_circuit_detect_manual(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t id)
{
	// NA- Can't set open circuit detect
	return - EINVAL;
}


/*!
 * @brief	Getter/Setter for the channel open circuit detect auto attribute value
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 */
static int get_chn_open_circuit_detect_auto(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t id)
{
	if (open_circuit_detect_read_done) {
		open_circuit_detect_read_done = false;

		if (open_circuit_detection_error) {
			strcpy(buf, "Error!!");
		}

		if (open_circuit_detection_done) {
			strcpy(buf, "Open Circuit Detected");
		} else {
			strcpy(buf, "Open Circuit Not Detected");
		}

		return len;
	}

	return sprintf(buf, "OPEN_DETECT_QUEUE: %d",
		       open_detect_queue_cnts[channel->ch_num]);
}

static int set_chn_open_circuit_detect_auto(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t id)
{
	uint8_t data;
	uint8_t open_detect_flag = false;
	int32_t rw_status = -EINVAL;
	uint16_t conv_cnts;

	(void)sscanf(buf, "%d", &data);
	open_circuit_detection_error = false;

	if ((data > 1 && data <= AUTO_OPEN_DETECT_QUEUE_MAX_CNT) && (buf[0] >= '0'
			&& buf[0] <= '9')) {
		open_detect_queue_cnts[channel->ch_num] = data;

		/* Enter into open circuit auto open detect mode */
		if (ad7606_spi_reg_write(device,
					 AD7606_REG_OPEN_DETECT_QUEUE,
					 open_detect_queue_cnts[channel->ch_num]) == 0) {
			/* Enable open circuit detection on selected channel */
			if (ad7606_spi_reg_write(device,
						 AD7606_REG_OPEN_DETECT_ENABLE,
						 (1 << (channel->ch_num))) == 0) {
				/* Monitor the open detect flag for max N+15 (open detect queue count) conversions.
				 * Note: In ideal scenario, the open detect flash should be monitored continuously while
				 * background N conversions are in progress */
				for (conv_cnts = 0;
				     conv_cnts < (open_detect_queue_cnts[channel->ch_num] +
						  AUTO_OPEN_DETECT_QUEUE_EXTRA_CONV_CNT);
				     conv_cnts++) {
					if (ad7606_convst(device) == 0) {
						no_os_udelay(100);

						/* Monitor the open detect flag */
						if (ad7606_spi_reg_read(device,
									AD7606_REG_OPEN_DETECTED,
									&open_detect_flag) == 0) {
							open_detect_flag >>= (channel->ch_num);
							open_detect_flag &= 0x1;

							rw_status = 0;
							if (open_detect_flag) {
								break;
							}
						} else {
							rw_status = -EINVAL;
							break;
						}
					} else {
						rw_status = -EINVAL;
						break;
					}
				}
			}
		}

		/* Disable open detect mode and clear open detect flag */
		(void)ad7606_spi_reg_write(device, AD7606_REG_OPEN_DETECT_QUEUE, 0);
		(void)ad7606_spi_reg_write(device, AD7606_REG_OPEN_DETECTED, 0xFF);

		open_detect_queue_cnts[channel->ch_num] = 0;

		if (rw_status == 0) {
			if (open_detect_flag) {
				open_circuit_detection_done = true;
			} else {
				open_circuit_detection_done = false;
			}

			open_circuit_detect_read_done = true;
			return len;
		}
	}

	open_circuit_detection_error = true;
	return -EINVAL;
}


/*!
 * @brief	Getter/Setter for the adc offset calibration
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 */
static int get_chn_calibrate_adc_offset(void *device,
					char *buf,
					uint32_t len,
					const struct iio_ch_info *channel,
					intptr_t id)
{
	float lsb_voltage;
	float adc_voltage;
	polarity_e polarity = attr_polarity_val[channel->ch_num];
	uint32_t adc_raw_data;
	int32_t adc_data;
	int8_t chn_offset;
	int32_t ret;

	/* Perform the system offset calibration */

	if (polarity == UNIPOLAR) {
		lsb_voltage = attr_chn_range[channel->ch_num] / ADC_MAX_COUNT_UNIPOLAR;
	} else {
		lsb_voltage = attr_chn_range[channel->ch_num] / ADC_MAX_COUNT_BIPOLAR;
	}

	/* Sample and read the ADC channel */
	ret = ad7606_read_single_sample(device, &adc_raw_data,
					channel->ch_num);
	if (ret) {
		return ret;
	}

	adc_data = reformat_adc_raw_data(adc_raw_data, channel->ch_num);

	/* Get an equivalent ADC voltage */
	adc_voltage = convert_adc_data_to_voltage(adc_data,
			attr_scale_val[channel->ch_num]);

	/* Calculate the channel offset and write it to offset register */
	chn_offset = -(adc_voltage / lsb_voltage / OFFSET_REG_RESOLUTION);

	if (ad7606_set_ch_offset(device, channel->ch_num,
				 chn_offset) == 0) {
		return sprintf(buf, "%s", "ADC Calibration Done");
	}

	return -EINVAL;
}

static int set_chn_calibrate_adc_offset(void *device,
					char *buf,
					uint32_t len,
					const struct iio_ch_info *channel,
					intptr_t id)
{
	// NA- Can't set open circuit detect
	return - EINVAL;
}


/*!
 * @brief	Getter/Setter for the adc gain calibration
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written in case of success,
 *			negative error code otherwise
 */
static int get_chn_calibrate_adc_gain(void *device,
				      char *buf,
				      uint32_t len,
				      const struct iio_ch_info *channel,
				      intptr_t id)
{
	uint8_t read_val;

	if (gain_calibration_done) {
		/* Get calibration status for previous gain value write event */
		gain_calibration_done = false;
		return sprintf(buf, "Calibration Done (Rfilter=%d K)",
			       gain_calibration_reg_val[channel->ch_num]);
	}

	/* Return gain value when normal read event is triggered */
	if (ad7606_spi_reg_read(device,
				AD7606_REG_GAIN_CH(channel->ch_num),
				&read_val) == 0) {
		gain_calibration_reg_val[channel->ch_num] = (read_val & AD7606_GAIN_MSK);
		return sprintf(buf, "Rfilter= %d K",
			       gain_calibration_reg_val[channel->ch_num]);
	}

	return -EINVAL;
}

static int set_chn_calibrate_adc_gain(void *device,
				      char *buf,
				      uint32_t len,
				      const struct iio_ch_info *channel,
				      intptr_t id)
{
	float data;

	if (buf[0] >= '0' && buf[0] <= '9') {
		(void)sscanf(buf, "%f", &data);

		if (data >= 0 && data < ADC_CALIBRATION_GAIN_MAX) {
			/* Get the nearest value of unsigned integer */
			gain_calibration_reg_val[channel->ch_num] = (uint8_t)(round(data));

			/* Perform the gain calibration by writing gain value into gain register */
			if (ad7606_set_ch_gain(device,
					       channel->ch_num,
					       gain_calibration_reg_val[channel->ch_num]) == 0) {
				gain_calibration_done = true;
				return len;
			}
		}
	}

	return -EINVAL;
}


/*!
 * @brief	Read the debug register value
 * @param	dev[in]- Pointer to IIO device instance
 * @param	reg[in]- Register address to read from
 * @param	readval[out]- Pointer to variable to read data into
 * @return	0 in case of success, negative error code otherwise
 */
int32_t debug_reg_read(void *dev, uint32_t reg, uint32_t *readval)
{
	/* Read the data from device */
	if (reg <= NUM_OF_REGISTERS) {
		if ((ad7606_spi_reg_read(dev, reg, (uint8_t *)readval) == 0)) {
			return 0;
		}
	}

	return -EINVAL;
}


/*!
 * @brief	Write into the debug register
 * @param	dev[in]- Pointer to IIO device instance
 * @param	reg[in]- Register address to write into
 * @param	writeval[in]- Register value to write
 * @return	0 in case of success, negative error code otherwise
 */
int32_t debug_reg_write(void *dev, uint32_t reg, uint32_t writeval)
{
	if (reg <= NUM_OF_REGISTERS) {
		if ((ad7606_spi_reg_write(dev, reg, (uint8_t)writeval) == 0)) {
			save_local_attributes();
			return 0;
		}
	}

	return -EINVAL;
}

/**
 * @brief	Read buffer data corresponding to AD7606 IIO device
 * @param	iio_dev_data[in] - Pointer to IIO device data structure
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t iio_ad7606_submit_buffer(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t nb_of_samples;
	uint32_t sample_index = 0;
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
		/* Read converted adc data */
		ret = ad7606_read_converted_sample(p_ad7606_dev_inst, &adc_raw,
						   active_chns[chn_indx]);
		if (ret) {
			return ret;
		}

		chn_indx++;
		if (chn_indx >= num_of_active_channels) {
			chn_indx = 0;
		}

		ret = no_os_cb_write(iio_dev_data->buffer->buf, &adc_raw, BYTES_PER_SAMPLE);
		if (ret) {
			return ret;
		}

		/* Trigger next conversion */
		ret = ad7606_convst(p_ad7606_dev_inst);
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
static int32_t iio_ad7606_prepare_transfer(void *dev_instance,
		uint32_t chn_mask)
{
	uint32_t mask = 0x1;
	uint8_t index = 0;
	int32_t ret;

	num_of_active_channels = 0;
	buf_size_updated = false;
	chn_indx = 0;

	/* The UART interrupt needs to be prioritized over the GPIO (end of conversion) interrupt.
		 * If not, the GPIO interrupt may occur during the period where there is a UART read happening
		 * for the READBUF command. If UART interrupts are not prioritized, then it would lead to missing of
		 * characters in the IIO command sent from the client. */
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	ret = no_os_irq_set_priority(trigger_irq_desc, TRIGGER_INT_ID,
				     RDY_GPIO_PRIORITY);
	if (ret) {
		return ret;
	}
#endif
#endif

	/* Get the active channels count based on the channel mask set in an IIO
	 * client application (channel mask starts from bit 0) */
	for (uint8_t chn = 0; chn < AD7606X_ADC_CHANNELS; chn++) {
		if (chn_mask & mask) {
			num_of_active_channels++;
			active_chns[index++] = chn;
		}

		mask <<= 1;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Trigger ADC conversion */
	ret = ad7606_convst(p_ad7606_dev_inst);
	if (ret) {
		return ret;
	}

	/* Enable trigger handler ISR */
	ret = iio_trig_enable(ad7606_hw_trig_desc);
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
static int32_t iio_ad7606_end_transfer(void *dev)
{
	int32_t ret;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Disable trigger handler ISR */
	ret = iio_trig_disable(ad7606_hw_trig_desc);
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
int32_t iio_ad7606_trigger_handler(struct iio_device_data *iio_dev_data)
{
	uint32_t adc_raw;
	int32_t ret;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
						   iio_dev_data->buffer->bytes_per_scan)) * iio_dev_data->buffer->bytes_per_scan;
		buf_size_updated = true;
	}

	/* Read converted adc data */
	ret = ad7606_read_converted_sample(p_ad7606_dev_inst, &adc_raw,
					   active_chns[chn_indx]);
	if (ret) {
		return ret;
	}

	chn_indx++;
	if (chn_indx >= num_of_active_channels) {
		chn_indx = 0;
	}

	ret = no_os_cb_write(iio_dev_data->buffer->buf, &adc_raw, BYTES_PER_SAMPLE);
	if (ret) {
		return ret;
	}

	/* Trigger next conversion */
	ret = ad7606_convst(p_ad7606_dev_inst);
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
	/* raw, scale and offset attributes are used in DMM mode of IIO client */
	{
		.name = "raw",
		.show = iio_ad7606_attr_get,
		.store = iio_ad7606_attr_set,
		.priv = RAW_ATTR_ID
	},
	{
		.name = "scale",
		.show = iio_ad7606_attr_get,
		.store = iio_ad7606_attr_set,
		.priv = SCALE_ATTR_ID
	},
	{
		.name = "offset",
		.show = iio_ad7606_attr_get,
		.store = iio_ad7606_attr_set,
		.priv = OFFSET_ATTR_ID
	},
#if defined(DEV_AD7606B) || defined(DEV_AD7606C_18) || defined(DEV_AD7606C_16)
	{
		.name = "chn_range",
		.show = get_chn_range,
		.store = set_chn_range,
	},
	{
		.name = "chn_offset",
		.show = get_chn_offset,
		.store = set_chn_offset,
	},
	{
		.name = "chn_phase offset",
		.show = get_chn_phase_offset,
		.store = set_chn_phase_offset,
	},
	{
		.name = "temperature",
		.show = get_chn_temperature,
		.store = set_chn_temperature,
	},
	{
		.name = "vref",
		.show = get_chn_vref,
		.store = set_chn_vref,
	},
	{
		.name = "vdrive",
		.show = get_chn_vdrive,
		.store = set_chn_vdrive,
	},
	{
		.name = "ALDO",
		.show = get_chn_aldo,
		.store = set_chn_aldo,
	},
	{
		.name = "DLDO",
		.show = get_chn_dldo,
		.store = set_chn_dldo,
	},
	{
		.name = "open_circuit_detect_manual",
		.show = get_chn_open_circuit_detect_manual,
		.store = set_chn_open_circuit_detect_manual,
	},
	{
		.name = "open_circuit_detect_auto",
		.show = get_chn_open_circuit_detect_auto,
		.store = set_chn_open_circuit_detect_auto,
	},
	{
		.name = "calibrate_adc_offset",
		.show = get_chn_calibrate_adc_offset,
		.store = set_chn_calibrate_adc_offset,
	},
	{
		.name = "calibrate_adc_gain",
		.show = get_chn_calibrate_adc_gain,
		.store = set_chn_calibrate_adc_gain,
	},
#if defined(DEV_AD7606C_18) || defined(DEV_AD7606C_16)
	{
		.name = "bandwidth",
		.show = get_bandwidth,
		.store = set_bandwidth,
	},
#endif
#endif

	END_ATTRIBUTES_ARRAY
};

/* IIOD device (global) attributes list */
static struct iio_attribute global_attributes[] = {
#if defined(DEV_AD7606B) || defined(DEV_AD7606C_18) || defined(DEV_AD7606C_16)
	{
		.name = "operating_mode",
		.show = get_operating_mode,
		.store = set_operating_mode,
	},
	{
		.name = "oversampling_ratio",
		.show = get_oversampling,
		.store = set_oversampling,
	},
#else
	{
		.name = "power_down_mode",
		.show = get_power_down_mode,
		.store = set_power_down_mode,
	},
	{
		.name = "dev_range",
		.show = get_range,
		.store = set_range,
	},
#endif
	{
		.name = "sampling_frequency",
		.show = iio_ad7606_attr_get,
		.store = iio_ad7606_attr_set,
		.priv = SAMPLING_FREQ_ATTR_ID
	},

	END_ATTRIBUTES_ARRAY
};

/* IIOD debug attributes list */
static struct iio_attribute debug_attributes[] = {
#if defined(DEV_AD7606B) || defined(DEV_AD7606C_18) || defined(DEV_AD7606C_16)
	{
		.name = "direct_reg_access",
		.show = NULL,
		.store = NULL,
	},
#endif

	END_ATTRIBUTES_ARRAY
};

static struct iio_channel iio_ad7606_channels[] = {
	{
		.name = "voltage0",
		.ch_type = IIO_VOLTAGE,
		.channel = 0,
		.scan_index = 0,
		.scan_type = &chn_scan[0],
		.attributes = channel_input_attributes,
		.ch_out = false,
		.indexed = true,
	},
	{
		.name = "voltage1",
		.ch_type = IIO_VOLTAGE,
		.channel = 1,
		.scan_index = 1,
		.scan_type = &chn_scan[1],
		.attributes = channel_input_attributes,
		.ch_out = false,
		.indexed = true
	},
	{
		.name = "voltage2",
		.ch_type = IIO_VOLTAGE,
		.channel = 2,
		.scan_index = 2,
		.scan_type = &chn_scan[2],
		.attributes = channel_input_attributes,
		.ch_out = false,
		.indexed = true
	},
	{
		.name = "voltage3",
		.ch_type = IIO_VOLTAGE,
		.channel = 3,
		.scan_index = 3,
		.scan_type = &chn_scan[3],
		.attributes = channel_input_attributes,
		.ch_out = false,
		.indexed = true
	},
#if (AD7606X_ADC_CHANNELS > 4)
	{
		.name = "voltage4",
		.ch_type = IIO_VOLTAGE,
		.channel = 4,
		.scan_index = 4,
		.scan_type = &chn_scan[4],
		.attributes = channel_input_attributes,
		.ch_out = false,
		.indexed = true
	},
	{
		.name = "voltage5",
		.ch_type = IIO_VOLTAGE,
		.channel = 5,
		.scan_index = 5,
		.scan_type = &chn_scan[5],
		.attributes = channel_input_attributes,
		.ch_out = false,
		.indexed = true
	},
#endif
#if (AD7606X_ADC_CHANNELS > 6)
	{
		.name = "voltage6",
		.ch_type = IIO_VOLTAGE,
		.channel = 6,
		.scan_index = 6,
		.scan_type = &chn_scan[6],
		.attributes = channel_input_attributes,
		.ch_out = false,
		.indexed = true
	},
	{
		.name = "voltage7",
		.ch_type = IIO_VOLTAGE,
		.channel = 7,
		.scan_index = 7,
		.scan_type = &chn_scan[7],
		.attributes = channel_input_attributes,
		.ch_out = false,
		.indexed = true
	}
#endif
};


/**
 * @brief	Init ad7606 IIO specific parameters
 * @param 	desc[in,out] - IIO device descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t iio_ad7606_init(struct iio_device **desc)
{
	struct iio_device *iio_ad7606_inst;
	uint8_t chn;
	polarity_e polarity;
	uint8_t chn_range;
	uint8_t read_val;

	iio_ad7606_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad7606_inst) {
		return -ENOMEM;
	}

	/* Update IIO device init parameters */
	for (chn = 0; chn < AD7606X_ADC_CHANNELS; chn++) {
		/* Get input channel range */
		if (ad7606_spi_reg_read(p_ad7606_dev_inst, AD7606_REG_RANGE_CH_ADDR(chn),
					&read_val) != 0) {
			free(iio_ad7606_inst);
			return -EINVAL;
		}

		if (((chn) % 2) != 0) {
			read_val >>= CHANNEL_RANGE_MSK_OFFSET;
			chn_range = read_val;
		} else {
			chn_range = (read_val & AD7606_RANGE_CH_MSK(chn));
		}

		/* Get polarity based on input channel range */
		polarity = ad7606_get_input_polarity(chn_range);
		attr_polarity_val[chn] = polarity;
		attr_chn_range[chn] = chn_range_val[chn_range];

		/* Update scale factor for each channel */
		update_vltg_conv_scale_factor(attr_chn_range[chn], attr_polarity_val[chn], chn);

		if (polarity == BIPOLAR) {
			/* Using offset-binary coding for bipolar mode */
			chn_scan[chn].sign = 's';
		} else {
			/* Using streight-binary coding for bipolar mode */
			chn_scan[chn].sign = 'u';
		}

		chn_scan[chn].realbits = AD7606X_ADC_RESOLUTION;
		chn_scan[chn].storagebits = CHN_STORAGE_BITS;
		chn_scan[chn].shift = 0;
		chn_scan[chn].is_big_endian = false;
	}

	iio_ad7606_inst->num_ch = sizeof(iio_ad7606_channels) / sizeof(
					  iio_ad7606_channels[0]);
	iio_ad7606_inst->channels = iio_ad7606_channels;
	iio_ad7606_inst->attributes = global_attributes;
	iio_ad7606_inst->debug_attributes = debug_attributes;

	iio_ad7606_inst->submit = iio_ad7606_submit_buffer;
	iio_ad7606_inst->pre_enable = iio_ad7606_prepare_transfer;
	iio_ad7606_inst->post_disable = iio_ad7606_end_transfer;
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_ad7606_inst->trigger_handler = iio_ad7606_trigger_handler;
#endif

	iio_ad7606_inst->debug_reg_read = debug_reg_read;
	iio_ad7606_inst->debug_reg_write = debug_reg_write;

	*desc = iio_ad7606_inst;

	return 0;
}

/*!
 * @brief	Remove the offset from data to change output data format to
 *			normal or stright  binary representation
 * @param	adc_raw_data[in] - Pointer to adc data read variable
 * @param	chn[in] - Input channel
 * @return	ADC reformated data
 */
static int32_t reformat_adc_raw_data(uint32_t adc_raw_data, uint8_t chn)
{
	int32_t adc_data;
	polarity_e polarity = attr_polarity_val[chn];

	/* Bipolar ADC Range:  (-FS) <-> 0 <-> (+FS) : 2^(ADC_RES-1) <-> 0 <-> 2^(ADC_RES-1)-1
	   Unipolar ADC Range: 0 <-> (+FS) : 0 <-> 2^ADC_RES
	 **/
	if (polarity == BIPOLAR) {
		/* Data output format is 2's complement for bipolar mode */
		if (adc_raw_data > ADC_MAX_COUNT_BIPOLAR) {
			/* Remove the offset from result to convert into negative reading */
			adc_data = ADC_MAX_COUNT_UNIPOLAR - adc_raw_data;
			adc_data = -adc_data;
		} else {
			adc_data = adc_raw_data;
		}
	} else {
		/* Data output format is straight binary for unipolar mode */
		adc_data = adc_raw_data;
	}

	return adc_data;
}

/*!
 * @brief	Function to convert adc raw data into equivalent voltage
 * @param	adc_data[in] - ADC data
 * @param	scale[in] - ADC raw to voltage conversion scale
 * @return	equivalent voltage
 */
static float convert_adc_data_to_voltage(int32_t adc_data, float scale)
{
	float voltage;

	/* Convert adc data into equivalent voltage.
	 * scale = (chn_range / MAX_ADC_CNT * 1000)
	 * */
	voltage = (adc_data  * (scale / 1000));

	return voltage;
}

/*!
 * @brief	Update scale factor for adc data to voltage conversion
 *			for IIO client
 * @param	chn_range[in] - Current channel voltage range
 * @param	polarity[in] - Channel polarity
 * @param	chn[in] - Input channel
 * @return	none
 */
static void update_vltg_conv_scale_factor(float chn_range, polarity_e polarity,
		uint8_t chn)
{
	/* Get the scale factor for voltage conversion from range */
	if (polarity == UNIPOLAR) {
		attr_scale_val[chn] = (chn_range / ADC_MAX_COUNT_UNIPOLAR) * 1000;
	} else {
		attr_scale_val[chn] = (chn_range / ADC_MAX_COUNT_BIPOLAR) * 1000;
	}
}

/**
 * @brief 	Save local variables
 * @return	none
 * @details	This function saves the local parameters with updated device values
 */
static void save_local_attributes(void)
{
	char buf[50];
	struct iio_ch_info channel;

	for (uint8_t chn = 0; chn < AD7606X_ADC_CHANNELS; chn++) {
		channel.ch_num = chn;

		/* Get channel range */
		(void)get_chn_range(p_ad7606_dev_inst, buf, 0, &channel, 0);

		/* Update scale */
		update_vltg_conv_scale_factor(attr_chn_range[channel.ch_num],
					      attr_polarity_val[channel.ch_num], channel.ch_num);
	}
}

/**
 * @brief Initialization of AD7606 IIO hardware trigger specific parameters
 * @param desc[in,out] - IIO hardware trigger descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad7606_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	struct iio_hw_trig_init_param ad7606_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;
	int32_t ret;

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad7606_hw_trig_init_params.irq_id = IRQ_INT_ID;
	ad7606_hw_trig_init_params.name = IIO_TRIGGER_NAME;
	ad7606_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad7606_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad7606_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad7606_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad7606_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad7606_hw_trig_init_params.iio_desc = p_ad7606_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad7606_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/**
 * @brief Release resources allocated for IIO device
 * @param desc[in] - IIO device descriptor
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad7606_iio_remove(struct iio_desc *desc)
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
 * @brief	Initialize the IIO interface for AD7606 IIO device
 * @return	none
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad7606_iio_initialize(void)
{
	int32_t init_status;

	/* IIO device descriptor */
	struct iio_device *p_iio_ad7606_dev;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	static struct iio_trigger ad7606_iio_trig_desc = {
		.is_synchronous = true,
	};

	/* IIO trigger init parameters */
	static struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad7606_iio_trig_desc,
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

	/* Initialize AD7606 device and peripheral interface */
	init_status = ad7606_init(&p_ad7606_dev_inst, &ad7606_init_str);
	if (init_status) {
		return init_status;
	}

	/* Initialize the IIO device parameters */
	init_status = iio_ad7606_init(&p_iio_ad7606_dev);
	if (init_status) {
		return init_status;
	}

	iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
	iio_device_init_params[0].raw_buf = adc_data_buffer;
	iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;

	iio_device_init_params[0].dev = p_ad7606_dev_inst;
	iio_device_init_params[0].dev_descriptor = p_iio_ad7606_dev;

	iio_init_params.nb_devs++;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_init_params.nb_trigs++;
#endif

	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_desc;
	iio_init_params.devs = iio_device_init_params;
	init_status = iio_init(&p_ad7606_iio_desc, &iio_init_params);
	if (init_status) {
		ad7606_iio_remove(p_ad7606_iio_desc);
		return init_status;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Initialize the IIO trigger specific parameters */
	init_status = ad7606_iio_trigger_param_init(&ad7606_hw_trig_desc);
	if (init_status) {
		return init_status;
	}

	init_status = init_pwm_trigger();
	if (init_status) {
		return init_status;
	}
#endif

	return 0;
}

/**
 * @brief 	Run the AD7606 IIO event handler
 * @return	none
 * @details	This function monitors the new IIO client event
 */
void ad7606_iio_event_handler(void)
{
	(void)iio_step(p_ad7606_iio_desc);
}
