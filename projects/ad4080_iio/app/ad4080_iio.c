/***************************************************************************//**
 *   @file    ad4080_iio.c
 *   @brief   Implementation of AD4080 IIO Appication Interface
 *   @details This module acts as an interface for AD4080 IIO device
********************************************************************************
 * Copyright (c) 2023-25 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "common.h"
#include "app_config.h"
#include "ad4080_iio.h"
#include "ad4080.h"
#include "ad4080_regs.h"
#include "ad4080_user_config.h"
#include "no_os_error.h"
#include "no_os_delay.h"
#include "no_os_alloc.h"
#include "iio.h"
#include "iio_trigger.h"
#include "version.h"

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

static int iio_ad4080_attr_get(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv);

static int iio_ad4080_attr_set(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv);

static int iio_ad4080_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int iio_ad4080_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

int32_t ad4080_deassert_oscillators(void);

int32_t ad4080_iio_start_fifo_mode_capture(uint32_t samples,
		bool update_fifo_watermark);

int32_t ad4080_read_fifo_data(struct ad4080_dev *dev, uint8_t *adc_data,
			      int32_t samples);

int32_t ad4080_iio_end_fifo_mode_capture(uint32_t *formatted_fifo_data,
		uint8_t *raw_fifo_data, uint32_t samples);

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/* Bytes per sample (*Note: 4 bytes needed per sample for data range
 * of 0 to 32-bit) */
#define	BYTES_PER_SAMPLE	sizeof(uint32_t)

/* Number of data storage bits (needed for IIO client) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

/* AD4080 IIO trigger name */
#define AD4080_IIO_TRIGGER_NAME	"ad4080_iio_trigger"

/* Scan type definition */
#define AD4080_SCAN {\
	.sign = 's',\
	.realbits = AD4080_ADC_RESOLUTION_BITS, \
	.storagebits = CHN_STORAGE_BITS,\
	.shift = 0,\
	.is_big_endian = false\
}

/* Channel attribute definition */
#define AD4080_CHN_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = iio_ad4080_attr_get,\
		.store = iio_ad4080_attr_set\
}

/* Channel attribute available definition */
#define AD4080_CHN_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = iio_ad4080_attr_available_get,\
	.store = iio_ad4080_attr_available_set\
}

/* AD4080 Channel Definition */
#define IIO_AD4080_CHANNEL(_idx) {\
	.name = "voltage" # _idx,\
	.ch_type = IIO_VOLTAGE,\
	.channel = _idx,\
	.scan_index = _idx,\
	.indexed = true,\
	.scan_type = &ad4080_iio_scan_type[_idx],\
	.ch_out = false,\
	.attributes = ad4080_iio_ch_attributes,\
}

/* ADC data buffer size */
#if defined(USE_SDRAM)
#define adc_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			65536
static int8_t adc_data_buffer[DATA_BUFFER_SIZE];
#endif

/* Default scale value for AD4080 */
#define AD4080_DEFAULT_SCALE	((((float)ADC_REF_VOLTAGE) / ADC_MAX_COUNT) * 1E3)

/* Last register address for AD4080 */
#define AD4080_LAST_REG_ADDR	AD4080_REG_FILTER_CONFIG

/* Fifo depth limit (watermark count) for data capture */
#define FIFO_SIZE				16384

/* Maximum threshold code */
#define MAX_THRESHOLD_CODE		0x7FF

/* Maximum hysteresis code */
#define MAX_HYSTERESIS_CODE		0x7FF

/* Number of bits for Offset correction coefficient value */
#define OFFSET_CORRECTION_COEFF_VAL_BITS			12

/* Timeout count to avoid stuck into potential infinite loop while checking
 * for new data into an acquisition buffer. The actual timeout factor is determined
 * through 'sampling_frequency' attribute of IIO app, but this period here makes sure
 * we are not stuck into a forever loop in case data capture is interrupted
 * or has failed in between.
 * Note: This timeout factor is dependent upon the MCU clock frequency. Below timeout
 * is tested for SDP-K1 platform @180Mhz default core clock */
#define BUF_READ_TIMEOUT	0xffffffff

/******************************************************************************/
/*************************** Variables and User Defined Data Types ************/
/******************************************************************************/

/* Pointer to the structure representing the AD4080 IIO device */
static struct ad4080_dev *ad4080_dev_inst = NULL;

/* IIO interface descriptor */
static struct iio_desc *ad4080_iio_desc = NULL;

/* ad4080 IIO device descriptor */
static struct iio_device *ad4080_iio_dev = NULL;

/* Scale */
static float scale[NUMBER_OF_CHANNELS] = {
	AD4080_DEFAULT_SCALE
};

/*
 * @enum  ad4080_attribute_id
 * @brief AD4080 Attribute IDs
 */
enum ad4080_attribute_id {
	RAW_ATTR_ID,
	SCALE_ATTR_ID,
	OFFSET_ATTR_ID,
	FIFO_FULL_ATTR_ID,
	FIFO_READ_DONE_ATTR_ID,
	FIFO_MODE_ATTR_ID,
	FIFO_WATERMARK_ATTR_ID,
	THRESHOLD_EVENT_DETECTED_ATTR_ID,
	DATA_LANES_ATTR_ID,
	GPIO1_OUTPUT_ENABLE_ATTR_ID,
	GPIO2_OUTPUT_ENABLE_ATTR_ID,
	GPIO3_OUTPUT_ENABLE_ATTR_ID,
	GPIO1_OUTPUT_FUNC_ATTR_ID,
	GPIO2_OUTPUT_FUNC_ATTR_ID,
	GPIO3_OUTPUT_FUNC_ATTR_ID,
	HI_THRESHOLD_ATTR_ID,
	LO_THRESHOLD_ATTR_ID,
	HYSTERESIS_ATTR_ID,
	FILTER_SEL_ATTR_ID,
	SINC_DEC_RATE_ATTR_ID,
	EVENT_TRIGGER_ATTR_ID,
	OPERATING_MODE_ATTR_ID,
	ODR_ATTR_ID,
	AFE_CTRL_ATTR_ID,
	SELECT_SAMPLING_FREQ_ATTR_ID,
};

/* IIOD channels attributes list */
static struct iio_attribute ad4080_iio_ch_attributes[] = {
	AD4080_CHN_ATTR("raw", RAW_ATTR_ID),
	AD4080_CHN_ATTR("scale", SCALE_ATTR_ID),
	AD4080_CHN_ATTR("offset", OFFSET_ATTR_ID),
	END_ATTRIBUTES_ARRAY
};

/* IIOD device (global) attributes list */
static struct iio_attribute ad4080_iio_global_attributes[] = {
	AD4080_CHN_ATTR("select_conversion_rate", SELECT_SAMPLING_FREQ_ATTR_ID),
	AD4080_CHN_AVAIL_ATTR("select_conversion_rate_available", SELECT_SAMPLING_FREQ_ATTR_ID),
	AD4080_CHN_ATTR("fifo_mode", FIFO_MODE_ATTR_ID),
	AD4080_CHN_AVAIL_ATTR("fifo_mode_available", FIFO_MODE_ATTR_ID),
	AD4080_CHN_ATTR("fifo_watermark", FIFO_WATERMARK_ATTR_ID),
	AD4080_CHN_ATTR("fifo_full", FIFO_FULL_ATTR_ID),
	AD4080_CHN_AVAIL_ATTR("fifo_full_available", FIFO_FULL_ATTR_ID),
	AD4080_CHN_ATTR("fifo_read_done", FIFO_READ_DONE_ATTR_ID),
	AD4080_CHN_AVAIL_ATTR("fifo_read_done_available", FIFO_READ_DONE_ATTR_ID),
	AD4080_CHN_ATTR("threshold_event_detected", THRESHOLD_EVENT_DETECTED_ATTR_ID),
	AD4080_CHN_AVAIL_ATTR("threshold_event_detected_available", THRESHOLD_EVENT_DETECTED_ATTR_ID),

#ifdef USE_QUAD_SPI
	AD4080_CHN_ATTR("data_lanes", DATA_LANES_ATTR_ID),
	AD4080_CHN_AVAIL_ATTR("data_lanes_available", DATA_LANES_ATTR_ID),
#endif

	AD4080_CHN_ATTR("gpio1_output_enable", GPIO1_OUTPUT_ENABLE_ATTR_ID),
	AD4080_CHN_AVAIL_ATTR("gpio1_output_enable_available", GPIO1_OUTPUT_ENABLE_ATTR_ID),
	AD4080_CHN_ATTR("gpio2_output_enable", GPIO2_OUTPUT_ENABLE_ATTR_ID),
	AD4080_CHN_AVAIL_ATTR("gpio2_output_enable_available", GPIO2_OUTPUT_ENABLE_ATTR_ID),
	AD4080_CHN_ATTR("gpio3_output_enable", GPIO3_OUTPUT_ENABLE_ATTR_ID),
	AD4080_CHN_AVAIL_ATTR("gpio3_output_enable_available", GPIO3_OUTPUT_ENABLE_ATTR_ID),

	AD4080_CHN_ATTR("gpio1_output_func_sel", GPIO1_OUTPUT_FUNC_ATTR_ID),
	AD4080_CHN_AVAIL_ATTR("gpio1_output_func_sel_available", GPIO1_OUTPUT_FUNC_ATTR_ID),
	AD4080_CHN_ATTR("gpio2_output_func_sel", GPIO2_OUTPUT_FUNC_ATTR_ID),
	AD4080_CHN_AVAIL_ATTR("gpio2_output_func_sel_available", GPIO2_OUTPUT_FUNC_ATTR_ID),
	AD4080_CHN_ATTR("gpio3_output_func_sel", GPIO3_OUTPUT_FUNC_ATTR_ID),
	AD4080_CHN_AVAIL_ATTR("gpio3_output_func_sel_available", GPIO3_OUTPUT_FUNC_ATTR_ID),

	AD4080_CHN_ATTR("hi_threshold_mv", HI_THRESHOLD_ATTR_ID),
	AD4080_CHN_ATTR("lo_threshold_mv", LO_THRESHOLD_ATTR_ID),
	AD4080_CHN_ATTR("hysteresis_mv", HYSTERESIS_ATTR_ID),

	AD4080_CHN_ATTR("filter_sel", FILTER_SEL_ATTR_ID),
	AD4080_CHN_AVAIL_ATTR("filter_sel_available", FILTER_SEL_ATTR_ID),

	AD4080_CHN_ATTR("sinc_dec_rate", SINC_DEC_RATE_ATTR_ID),
	AD4080_CHN_AVAIL_ATTR("sinc_dec_rate_available", SINC_DEC_RATE_ATTR_ID),

	AD4080_CHN_ATTR("event_trigger", EVENT_TRIGGER_ATTR_ID),
	AD4080_CHN_AVAIL_ATTR("event_trigger_available", EVENT_TRIGGER_ATTR_ID),

	AD4080_CHN_ATTR("operating_mode", OPERATING_MODE_ATTR_ID),
	AD4080_CHN_AVAIL_ATTR("operating_mode_available", OPERATING_MODE_ATTR_ID),

	AD4080_CHN_ATTR("sampling_frequency", ODR_ATTR_ID),

	AD4080_CHN_ATTR("afe_enable", AFE_CTRL_ATTR_ID),
	AD4080_CHN_AVAIL_ATTR("afe_enable_available", AFE_CTRL_ATTR_ID),

	END_ATTRIBUTES_ARRAY
};

/* AD4080 channel scan type */
static struct scan_type ad4080_iio_scan_type[NUMBER_OF_CHANNELS] = {
	AD4080_SCAN
};

/* IIO Channels */
static struct iio_channel iio_ad4080_channels[NUMBER_OF_CHANNELS] = {
	IIO_AD4080_CHANNEL(0)
};

/* FIFO modes */
static const char *ad4080_fifo_modes[] = {
	"disabled",
	"immediate_trigger_mode",
	"read_latest_watermark_mode",
	"read_all_fifo_mode"
};

/* FIFO FULL / FIFO READ DONE status values */
static const char *ad4080_fifo_status_val_str[] = {
	"false",
	"true"
};

/* Threshold Event Detect status */
static const char *ad4080_threshold_event_detected_status_str[] = {
	"None",
	"lo",
	"hi",
	"lo_and_hi"
};

/* AD4080 CMOS Data Lanes */
static const char *ad4080_data_lanes_str[] = {
	"single",
	"quad"
};

/* AD4080 GPIO Output Enable */
static const char *ad4080_gpio_output_enable_str[] = {
	"disable",
	"enable"
};

/* AD4080 GPIO Output Function Select */
static const char *ad4080_gpio_output_func_str[] = {
	"adi_nspi_sdo_data",
	"gpio_fifo_full",
	"gpio_fifo_read_done",
	"gpio_filter_result_ready",
	"gpio_ht_detect",
	"gpio_lt_detect",
	"gpio_status_alert",
	"gpio_gpo_data",
	"gpio_filter_sync_input",
	"gpio_ext_event_trigger_fifo",
	"gpio_cnv_inhibit_input"
};

/* AD4080 Filter Select */
static const char *ad4080_filter_sel_str[] = {
	"disabled",
	"sinc1",
	"sinc5",
	"sinc5_plus_compensation"
};

/* AD4080 Sinc Decimation Rate */
static const char *ad4080_sinc_dec_rate_str[] = {
	"2",
	"4",
	"8",
	"16",
	"32",
	"64",
	"128",
	"256",
	"512",
	"1024"
};

/* AD4080 Event Trigger Source */
static const char *ad4080_event_trigger_str[] = {
	"none",
	"int_event_lo",
	"int_event_hi",
	"int_event_lo_or_hi"
};

/* AD4080 operating mode */
static const char *ad4080_operating_mode_str[] = {
	"normal",
	"standby",
	"sleep",
};

/* AD4080 AFE Control */
static const char *ad4080_afe_ctrl[] = {
	"disable",
	"enable",
};

/* AD4080 sampling frequency options */
static const uint32_t ad4080_sel_sampling_freq_options[] = {
	40000000,
	20000000,
	10000000,
};

/* Default sampling freq */
uint32_t ad4080_sampling_freq = AD4080_CNV_CLK_FREQ_HZ;

/* LSB (in millivolts) for the HI/LO Threshold register */
static const float threshold_lsb = 1.46484;

/* LSB (in millivolts) for the Hysteresis register */
static const float hysteresis_lsb = 1.46484;

/* Array to store raw data from ADC FIFO */
static uint8_t fifo_data[1 + AD4080_SIGN_EXTENDED_RESOLUTION_BYTES * FIFO_SIZE];

/* Offset Correction Coefficient.
 * Twos complement data format where LSB = 0.00572 mV.
 * 0x800 represents −2048 × LSB, and 0x7FF represents +2047 × LSB. */
static const float offset_correction_coefficient = 0.00572;

/* Formatted data of ADC FIFO raw data */
static uint32_t actual_fifo_data[FIFO_SIZE];

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/* FIFO watermark */
static uint16_t watermark = FIFO_SIZE;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/*!
 * @brief	Getter function for AD4080 attributes
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of expected bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise
 */
static int iio_ad4080_attr_get(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	int ret;
	uint8_t val;
	uint8_t offset_reg_val[2];
	uint8_t reg_data;
	uint8_t filter_type;
	int16_t offset;
	uint16_t fifo_watermark;
	uint16_t threshold_code;
	uint16_t threshold_register;
	uint16_t hysteresis_code;
	uint32_t odr;
	uint32_t total_decimation;
	enum ad4080_cnv_spi_lvds_lanes data_lanes;
	float offset_mv;
	float threshold_mv;
	float hysteresis_mv;

	switch (priv) {
	case RAW_ATTR_ID:
		/* Disable FIFO */
		ret = ad4080_set_fifo_mode(ad4080_dev_inst, AD4080_FIFO_DISABLE);
		if (ret) {
			return ret;
		}

		/* Start FIFO mode capture */
		ret = ad4080_iio_start_fifo_mode_capture(1, true);
		if (ret) {
			return ret;
		}

		/* Read FIFO data */
		ret = ad4080_read_fifo_data(ad4080_dev_inst, fifo_data, 1);

		/* End FIFO mode capture irrespective of status of read FIFO data operation
		 * and retain previous ret value */
		ret |= ad4080_iio_end_fifo_mode_capture(actual_fifo_data, fifo_data, 1);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%ld", actual_fifo_data[0]);

	case SCALE_ATTR_ID:
		return sprintf(buf, "%10f", scale[0]);

	case OFFSET_ATTR_ID:
		/* Read the offset register */
		ret = ad4080_read(ad4080_dev_inst, AD4080_REG_OFFSET, &offset_reg_val[0]);
		if (ret) {
			return ret;
		}

		ret = ad4080_read(ad4080_dev_inst, AD4080_REG_OFFSET + 1, &offset_reg_val[1]);
		if (ret) {
			return ret;
		}

		/* Sign extend the 11th bit since offset value is 12-bit */
		offset = no_os_sign_extend16(offset_reg_val[1] << 8 | offset_reg_val[0],
					     OFFSET_CORRECTION_COEFF_VAL_BITS - 1);

		offset_mv = offset * offset_correction_coefficient;

		return sprintf(buf, "%f", offset_mv);

	case FIFO_MODE_ATTR_ID:
		return sprintf(buf, "%s", ad4080_fifo_modes[ad4080_dev_inst->fifo_mode]);

	case FIFO_WATERMARK_ATTR_ID:
		ret = ad4080_get_fifo_watermark(ad4080_dev_inst, &fifo_watermark);

		if (ret) {
			return ret;
		}

		return sprintf(buf, "%d", fifo_watermark);

	case FIFO_FULL_ATTR_ID:
		ret = ad4080_read(ad4080_dev_inst, AD4080_REG_DEVICE_STATUS, &val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%s",
			       ad4080_fifo_status_val_str[no_os_field_get(NO_OS_BIT(7),
					       val)]);

	case FIFO_READ_DONE_ATTR_ID:
		ret = ad4080_read(ad4080_dev_inst, AD4080_REG_DEVICE_STATUS, &val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%s",
			       ad4080_fifo_status_val_str[no_os_field_get(NO_OS_BIT(6),
					       val)]);

	case THRESHOLD_EVENT_DETECTED_ATTR_ID:
		ret = ad4080_read(ad4080_dev_inst, AD4080_REG_DEVICE_STATUS, &val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%s",
			       ad4080_threshold_event_detected_status_str[
			no_os_field_get(NO_OS_GENMASK(5, 4), val)]);

	case DATA_LANES_ATTR_ID:
		ret = ad4080_get_cnv_spi_lvds_lanes(ad4080_dev_inst, &data_lanes);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%s", ad4080_data_lanes_str[data_lanes]);

	case GPIO1_OUTPUT_ENABLE_ATTR_ID:
	case GPIO2_OUTPUT_ENABLE_ATTR_ID:
	case GPIO3_OUTPUT_ENABLE_ATTR_ID:
		return sprintf(buf, "%s",
			       ad4080_gpio_output_enable_str[
			ad4080_dev_inst->gpio_op_enable[AD4080_GPIO_1 +
						       (priv - GPIO1_OUTPUT_ENABLE_ATTR_ID)]]);

	case GPIO1_OUTPUT_FUNC_ATTR_ID:
	case GPIO2_OUTPUT_FUNC_ATTR_ID:
	case GPIO3_OUTPUT_FUNC_ATTR_ID:
		return sprintf(buf,
			       "%s",
			       ad4080_gpio_output_func_str[
			ad4080_dev_inst->gpio_op_func_sel[AD4080_GPIO_1 +
						       (priv - GPIO1_OUTPUT_FUNC_ATTR_ID)]]);

	case HI_THRESHOLD_ATTR_ID:
	case LO_THRESHOLD_ATTR_ID:
		/* Read the 12-bit code from the relevant Threshold register,
		 * convert to float and report back value in millivolts */
		threshold_register = AD4080_REG_EVENT_DETECTION_HI +
				     2 * (priv - HI_THRESHOLD_ATTR_ID);

		/* Read the threshold register */
		ret = ad4080_read(ad4080_dev_inst, threshold_register, &val);
		if (ret) {
			return ret;
		}

		threshold_code = val;
		ret = ad4080_read(ad4080_dev_inst, threshold_register + 1, &val);
		if (ret) {
			return ret;
		}

		threshold_code |= (val << 8);

		if (threshold_code <= MAX_THRESHOLD_CODE) {
			return sprintf(buf, "%10f", (float)threshold_code * threshold_lsb);
		} else {
			threshold_mv = (float)threshold_code * threshold_lsb;
			threshold_mv -= (float)0x1000 * threshold_lsb;

			return sprintf(buf, "%10f", threshold_mv);
		}

	case HYSTERESIS_ATTR_ID:
		/* Read the 12-bit code from the relevant Hysteresis register,
		 * convert to float and report back value in millivolts */
		ret = ad4080_read(ad4080_dev_inst, AD4080_REG_EVENT_HYSTERESIS, &val);
		if (ret) {
			return ret;
		}

		hysteresis_code = val;
		ret = ad4080_read(ad4080_dev_inst, AD4080_REG_EVENT_HYSTERESIS + 1, &val);
		if (ret) {
			return ret;
		}

		hysteresis_code |= (val << 8);

		if (hysteresis_code <= MAX_HYSTERESIS_CODE) {
			return sprintf(buf, "%10f", (float)hysteresis_code * hysteresis_lsb);
		} else {
			hysteresis_mv = (float)hysteresis_code * hysteresis_lsb;
			hysteresis_mv -= (float)0x1000 * hysteresis_lsb;

			return sprintf(buf, "%10f", hysteresis_mv);
		}

	case FILTER_SEL_ATTR_ID:
		ret = ad4080_read(ad4080_dev_inst, AD4080_REG_FILTER_CONFIG, &val);
		if (ret) {
			return ret;
		}

		val = no_os_field_get(NO_OS_GENMASK(1, 0), val);
		return sprintf(buf, "%s", ad4080_filter_sel_str[val]);

	case SINC_DEC_RATE_ATTR_ID:
		ret = ad4080_read(ad4080_dev_inst, AD4080_REG_FILTER_CONFIG, &val);
		if (ret) {
			return ret;
		}

		val = no_os_field_get(NO_OS_GENMASK(6, 3), val);
		return sprintf(buf, "%s", ad4080_sinc_dec_rate_str[val]);

	case EVENT_TRIGGER_ATTR_ID:
		/* Check if the int_event bitfield is enabled.
		 * If enabled, use HI_ROUTE, LO_ROUTE bitfield values to determine
		 * the selected internal event. */
		ret = ad4080_read(ad4080_dev_inst, AD4080_REG_GENERAL_CONFIG, &val);
		if (ret) {
			return ret;
		}

		if (no_os_field_get(NO_OS_BIT(7), val) == 0) {
			return sprintf(buf, "%s", ad4080_event_trigger_str[0]);
		} else {
			if (no_os_field_get(NO_OS_GENMASK(6, 5), val) == 0) {
				return -EINVAL;
			} else {
				return sprintf(buf, "%s",
					       ad4080_event_trigger_str[
						       no_os_field_get(NO_OS_GENMASK(6, 5), val)]);
			}
		}

	case OPERATING_MODE_ATTR_ID:
		ret = ad4080_read(ad4080_dev_inst, AD4080_REG_DEVICE_CONFIG, &val);
		if (ret) {
			return ret;
		}

		val = no_os_field_get(AD4080_OP_MODE_MSK, val);
		switch (val) {
		case 0:
			return sprintf(buf, "%s", ad4080_operating_mode_str[0]);
		case 2:
			return sprintf(buf, "%s", ad4080_operating_mode_str[1]);
		case 3:
			return sprintf(buf, "%s", ad4080_operating_mode_str[2]);
		default:
			return -EINVAL;
		}

	case ODR_ATTR_ID:
		ret = ad4080_read(ad4080_dev_inst, AD4080_REG_FILTER_CONFIG, &reg_data);
		if (ret) {
			return ret;
		}

		total_decimation = 1;
		val = no_os_field_get(NO_OS_GENMASK(6, 3), reg_data);
		filter_type = no_os_field_get(NO_OS_GENMASK(1, 0), reg_data);
		odr = ad4080_sampling_freq;

		switch (filter_type) {
		case 0:
			break;
		case 1:
			total_decimation = (1 << (val + 1));
			break;
		case 2:
			total_decimation = no_os_min((uint16_t)(1 << (1 + val)), 256);
			break;
		case 3:
			total_decimation = no_os_min((uint16_t)(1 << (1 + val)), 256);
			total_decimation *= 2;
			break;
		default:
			break;
		}

		odr /= total_decimation;
		return sprintf(buf, "%ld", odr);

	case AFE_CTRL_ATTR_ID:
		ret = no_os_gpio_get_value(gpio_afe_ctrl_desc, &val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%s", ad4080_afe_ctrl[val]);

	case SELECT_SAMPLING_FREQ_ATTR_ID:
		return sprintf(buf, "%ld", ad4080_sampling_freq);

	default:
		break;
	}

	return len;
}

/*!
 * @brief	Setter function for AD4080 attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of expected bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int iio_ad4080_attr_set(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	int ret = 0;
	uint8_t val;
	char *end;
	int16_t offset;
	uint16_t threshold_register;
	uint16_t threshold_code;
	uint16_t hysteresis_code;
	float offset_mv;
	float threshold_mv;
	float hysteresis_mv;
	enum ad4080_gpio gpio;

	switch (priv) {
	case RAW_ATTR_ID:
	case SCALE_ATTR_ID:
		/* ADC Raw, Scale are constant for the firmware configuration */
		break;

	case OFFSET_ATTR_ID:
		/* Convert the characters into float value */
		offset_mv = strtof(buf, NULL);

		/* Format and get the value to write into the register */
		offset = (int16_t)(offset_mv / offset_correction_coefficient);

		/* Write the offset register */
		ret = ad4080_write(ad4080_dev_inst, AD4080_REG_OFFSET, offset);
		if (ret) {
			return ret;
		}

		ret = ad4080_write(ad4080_dev_inst, AD4080_REG_OFFSET + 1, offset >> 8);
		if (ret) {
			return ret;
		}

		break;

	case FIFO_MODE_ATTR_ID:
		for (val = 0; val <= AD4080_EVENT_TRIGGER; val++) {
			if (!strcmp(buf, ad4080_fifo_modes[val])) {
				break;
			}
		}

		if (val > AD4080_IMMEDIATE_TRIGGER) {
			/* If event trigger mode is applied, set watermark first. */
			ret = ad4080_set_fifo_watermark(ad4080_dev_inst, watermark);
			if (ret) {
				return ret;
			}
		}

		ret = ad4080_set_fifo_mode(ad4080_dev_inst, val);
		if (ret) {
			return ret;
		}

		break;

	case FIFO_WATERMARK_ATTR_ID:
		/* Set the FIFO watermark if requested watermark is less than ADC FIFO size */
		watermark = no_os_str_to_uint32(buf);
		if (watermark > AD4080_FIFO_SIZE) {
			return -EINVAL;
		}

		ret = ad4080_set_fifo_watermark(ad4080_dev_inst, watermark);
		if (ret) {
			return ret;
		}

		break;

	case FIFO_FULL_ATTR_ID:
	case FIFO_READ_DONE_ATTR_ID:
	case THRESHOLD_EVENT_DETECTED_ATTR_ID:
		break;

	case DATA_LANES_ATTR_ID: {
		for (val = 0; val <= AD4080_MULTIPLE_LANES; val++) {
			if (!strcmp(buf, ad4080_data_lanes_str[val])) {
				ret = ad4080_set_cnv_spi_lvds_lanes(ad4080_dev_inst, val);
				if (ret) {
					return ret;
				}

				break;
			}
		}
		break;
	}

	case GPIO1_OUTPUT_ENABLE_ATTR_ID:
	case GPIO2_OUTPUT_ENABLE_ATTR_ID:
	case GPIO3_OUTPUT_ENABLE_ATTR_ID:
		for (val = 0; val <= AD4080_GPIO_OUTPUT; val++) {
			if (!strcmp(buf, ad4080_gpio_output_enable_str[val])) {
				gpio = AD4080_GPIO_1 + (priv - GPIO1_OUTPUT_ENABLE_ATTR_ID);
				ret = ad4080_set_gpio_output_enable(ad4080_dev_inst, gpio, val);

				if (ret) {
					return ret;
				}
				break;
			}
		}
		break;

	case GPIO1_OUTPUT_FUNC_ATTR_ID:
	case GPIO2_OUTPUT_FUNC_ATTR_ID:
	case GPIO3_OUTPUT_FUNC_ATTR_ID:
		for (val = 0; val <= AD4080_GPIO_CNV_INHIBIT_INPUT; val++) {
			if (!strcmp(buf, ad4080_gpio_output_func_str[val])) {
				gpio = AD4080_GPIO_1 + (priv - GPIO1_OUTPUT_FUNC_ATTR_ID);
				ret = ad4080_set_gpio_output_func(ad4080_dev_inst, gpio, val);
				if (ret) {
					return ret;
				}
				break;
			}
		}
		break;

	case HI_THRESHOLD_ATTR_ID:
	case LO_THRESHOLD_ATTR_ID:
		threshold_register = AD4080_REG_EVENT_DETECTION_HI +
				     2 * (priv - HI_THRESHOLD_ATTR_ID);
		threshold_mv = strtof(buf, &end);

		if (threshold_mv >= 0) {
			threshold_code = (uint16_t)(threshold_mv / threshold_lsb);
			threshold_code = no_os_min(threshold_code, 0x7ff);
		} else {
			threshold_code = (uint16_t)((int16_t)(threshold_mv / threshold_lsb) +
						    (int16_t) 0x1000);
		}

		ret = ad4080_write(ad4080_dev_inst,
				   threshold_register,
				   threshold_code & 0xFF);
		if (ret) {
			return ret;
		}

		ret = ad4080_write(ad4080_dev_inst,
				   threshold_register + 1,
				   threshold_code >> 8);
		if (ret) {
			return ret;
		}

		break;

	case HYSTERESIS_ATTR_ID:
		hysteresis_mv = strtof(buf, &end);
		if (hysteresis_mv >= 0) {
			hysteresis_code = (uint16_t)(hysteresis_mv / threshold_lsb);
			hysteresis_code = no_os_min(hysteresis_code, 0x7ff);
		} else {
			hysteresis_code = (uint16_t)((int16_t)(hysteresis_mv / hysteresis_lsb) +
						     (int16_t) 0x1000);
		}

		ret = ad4080_write(ad4080_dev_inst,
				   AD4080_REG_EVENT_HYSTERESIS,
				   hysteresis_code & 0xFF);
		if (ret) {
			return ret;
		}

		ret = ad4080_write(ad4080_dev_inst,
				   AD4080_REG_EVENT_HYSTERESIS + 1,
				   hysteresis_code >> 8);
		if (ret) {
			return ret;
		}

		break;

	case FILTER_SEL_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad4080_filter_sel_str); val++) {
			if (!strcmp(buf, ad4080_filter_sel_str[val])) {
				ret = ad4080_update_bits(ad4080_dev_inst, AD4080_REG_FILTER_CONFIG,
							 NO_OS_GENMASK(1, 0), val);
				if (ret) {
					return ret;
				}
				break;
			}
		}
		break;

	case SINC_DEC_RATE_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad4080_sinc_dec_rate_str); val++) {
			if (!strcmp(buf, ad4080_sinc_dec_rate_str[val])) {
				ret = ad4080_update_bits(ad4080_dev_inst,
							 AD4080_REG_FILTER_CONFIG,
							 NO_OS_GENMASK(6, 3),
							 (val << 3));
				if (ret) {
					return ret;
				}
				break;
			}
		}
		break;

	case EVENT_TRIGGER_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad4080_event_trigger_str); val++) {
			if (!strcmp(buf, ad4080_event_trigger_str[val])) {
				break;
			}
		}

		switch (val) {
		case 0:
			ret = ad4080_update_bits(ad4080_dev_inst,
						 AD4080_REG_GENERAL_CONFIG,
						 NO_OS_BIT(7),
						 0);
			if (ret) {
				return ret;
			}
			break;

		default:
			ret = ad4080_update_bits(ad4080_dev_inst,
						 AD4080_REG_GENERAL_CONFIG,
						 NO_OS_BIT(7),
						 NO_OS_BIT(7));
			if (ret) {
				return ret;
			}

			ret = ad4080_update_bits(ad4080_dev_inst,
						 AD4080_REG_GENERAL_CONFIG,
						 NO_OS_GENMASK(6, 5),
						 (val << 5));
			if (ret) {
				return ret;
			}
		}

		break;

	case OPERATING_MODE_ATTR_ID:
		/* Search for requested operating mode in the corresponding array */
		for (val = 0; val <= AD4080_OP_LOW_POWER; val++) {
			if (!strcmp(buf, ad4080_operating_mode_str[val])) {
				break;
			}
		}

		switch (val) {
		case AD4080_OP_NORMAL:
			ret = ad4080_update_bits(ad4080_dev_inst,
						 AD4080_REG_DEVICE_CONFIG,
						 AD4080_OP_MODE_MSK,
						 0);
			break;

		case AD4080_OP_STANDBY:
			ret = ad4080_update_bits(ad4080_dev_inst,
						 AD4080_REG_DEVICE_CONFIG,
						 AD4080_OP_MODE_MSK,
						 2);
			break;

		case AD4080_OP_LOW_POWER:
			ret = ad4080_update_bits(ad4080_dev_inst,
						 AD4080_REG_DEVICE_CONFIG,
						 AD4080_OP_MODE_MSK,
						 3);
			break;

		default:
			ret = -EINVAL;
		}

		if (ret) {
			return ret;
		}

		break;

	case AFE_CTRL_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad4080_afe_ctrl); val++) {
			if (!strcmp(buf, ad4080_afe_ctrl[val])) {
				ret = no_os_gpio_set_value(gpio_afe_ctrl_desc, val);
				break;
			}
		}
		break;

	case SELECT_SAMPLING_FREQ_ATTR_ID:
		/* Search for requested sampling frequency in the array of supported sampling frequencies */
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad4080_sel_sampling_freq_options); val++) {
			if (no_os_str_to_uint32(buf) == ad4080_sel_sampling_freq_options[val]) {
				break;
			}
		}

		/* Deassert the oscillators */
		ret = ad4080_deassert_oscillators();
		if (ret) {
			return ret;
		}

		switch (val) {
		case 0:
			/* Assert the 40 MHz oscillator */
			ret = no_os_gpio_set_value(gpio_osc_en_40m_desc, NO_OS_GPIO_HIGH);
			break;

		case 1:
			/* Assert the 20 MHz oscillator */
			ret = no_os_gpio_set_value(gpio_osc_en_20m_desc, NO_OS_GPIO_HIGH);
			break;

		case 2:
			/* Assert the 10 MHz oscillator */
			ret = no_os_gpio_set_value(gpio_osc_en_10m_desc, NO_OS_GPIO_HIGH);
			break;

		default:
			ret = -EINVAL;
		}

		if (ret) {
			return ret;
		}

		/* Store the updated sampling frequency */
		ad4080_sampling_freq = ad4080_sel_sampling_freq_options[val];

		break;

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Attribute available getter function for AD4080 attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of input bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int iio_ad4080_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	switch (priv) {
	case FIFO_MODE_ATTR_ID:
		return sprintf(buf,
			       "%s %s %s %s",
			       ad4080_fifo_modes[0],
			       ad4080_fifo_modes[1],
			       ad4080_fifo_modes[2],
			       ad4080_fifo_modes[3]);

	case FIFO_FULL_ATTR_ID:
	case FIFO_READ_DONE_ATTR_ID:
		return sprintf(buf, "%s %s", ad4080_fifo_status_val_str[0],
			       ad4080_fifo_status_val_str[1]);

	case THRESHOLD_EVENT_DETECTED_ATTR_ID:
		return sprintf(buf,
			       "%s %s %s %s",
			       ad4080_threshold_event_detected_status_str[0],
			       ad4080_threshold_event_detected_status_str[1],
			       ad4080_threshold_event_detected_status_str[2],
			       ad4080_threshold_event_detected_status_str[3]);

	case DATA_LANES_ATTR_ID:
#ifdef USE_QUAD_SPI
		return sprintf(buf,
			       "%s %s",
			       ad4080_data_lanes_str[0],
			       ad4080_data_lanes_str[1]);
#else
		return sprintf(buf, "%s", ad4080_data_lanes_str[0]);
#endif

	case GPIO1_OUTPUT_ENABLE_ATTR_ID:
	case GPIO2_OUTPUT_ENABLE_ATTR_ID:
	case GPIO3_OUTPUT_ENABLE_ATTR_ID:
		return sprintf(buf,
			       "%s %s",
			       ad4080_gpio_output_enable_str[0],
			       ad4080_gpio_output_enable_str[1]);

	case GPIO1_OUTPUT_FUNC_ATTR_ID:
	case GPIO2_OUTPUT_FUNC_ATTR_ID:
	case GPIO3_OUTPUT_FUNC_ATTR_ID:
		return sprintf(buf,
			       "%s %s %s %s %s %s %s %s %s %s %s",
			       ad4080_gpio_output_func_str[0],
			       ad4080_gpio_output_func_str[1],
			       ad4080_gpio_output_func_str[2],
			       ad4080_gpio_output_func_str[3],
			       ad4080_gpio_output_func_str[4],
			       ad4080_gpio_output_func_str[5],
			       ad4080_gpio_output_func_str[6],
			       ad4080_gpio_output_func_str[7],
			       ad4080_gpio_output_func_str[8],
			       ad4080_gpio_output_func_str[9],
			       ad4080_gpio_output_func_str[10]);

	case FILTER_SEL_ATTR_ID:
		return sprintf(buf,
			       "%s %s %s %s",
			       ad4080_filter_sel_str[0],
			       ad4080_filter_sel_str[1],
			       ad4080_filter_sel_str[2],
			       ad4080_filter_sel_str[3]);

	case SINC_DEC_RATE_ATTR_ID:
		return sprintf(buf,
			       "%s %s %s %s %s %s %s %s %s %s",
			       ad4080_sinc_dec_rate_str[0],
			       ad4080_sinc_dec_rate_str[1],
			       ad4080_sinc_dec_rate_str[2],
			       ad4080_sinc_dec_rate_str[3],
			       ad4080_sinc_dec_rate_str[4],
			       ad4080_sinc_dec_rate_str[5],
			       ad4080_sinc_dec_rate_str[6],
			       ad4080_sinc_dec_rate_str[7],
			       ad4080_sinc_dec_rate_str[8],
			       ad4080_sinc_dec_rate_str[9]);

	case EVENT_TRIGGER_ATTR_ID:
		return sprintf(buf,
			       "%s %s %s %s",
			       ad4080_event_trigger_str[0],
			       ad4080_event_trigger_str[1],
			       ad4080_event_trigger_str[2],
			       ad4080_event_trigger_str[3]);

	case OPERATING_MODE_ATTR_ID:
		return sprintf(buf,
			       "%s %s %s",
			       ad4080_operating_mode_str[0],
			       ad4080_operating_mode_str[1],
			       ad4080_operating_mode_str[2]);

	case AFE_CTRL_ATTR_ID:
		return sprintf(buf, "%s %s", ad4080_afe_ctrl[0], ad4080_afe_ctrl[1]);

	case SELECT_SAMPLING_FREQ_ATTR_ID:
		return sprintf(buf,
			       "%ld %ld %ld",
			       ad4080_sel_sampling_freq_options[0],
			       ad4080_sel_sampling_freq_options[1],
			       ad4080_sel_sampling_freq_options[2]);

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Attribute available setter function for AD4080 attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of input bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int iio_ad4080_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	return len;
}

/**
 * @brief  Reads data from the ADC FIFO.
 * @param  dev[in, out]- Pointer to IIO device instance
 * @param  adc_data[in, out]- Pointer to array to hold FIFO data
 * @param  samples[in]- Number of samples to read from FIFO
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad4080_read_fifo_data(struct ad4080_dev *dev, uint8_t *adc_data,
			      int32_t samples)
{
	int32_t ret = 0;
	uint32_t bytes_to_transfer = 0;

	if (!dev || !adc_data) {
		return -EINVAL;
	}

	bytes_to_transfer = samples * AD4080_SIGN_EXTENDED_RESOLUTION_BYTES;
	bytes_to_transfer +=
		(ad4080_dev_inst->cnv_spi_lvds_lanes == AD4080_ONE_LANE) ? 1 : 4;

#ifdef USE_QUAD_SPI
	/* Transfer using QSPI */
	struct no_os_spi_msg spi_msg = {
		.bytes_number = bytes_to_transfer,
		.tx_buff = (uint8_t *)NULL,
		.rx_buff = (uint8_t *)adc_data,
		.cs_change = 1
	};

	qspi_init_params.lanes =
		(ad4080_dev_inst->cnv_spi_lvds_lanes == AD4080_ONE_LANE) ?
		NO_OS_SPI_SINGLE_LANE :
		NO_OS_SPI_QUAD_LANE;

	/* Remove the QSPI descriptor */
	ret = no_os_spi_remove(quad_spi_desc);
	if (ret) {
		return ret;
	}

	/* Initialize the Quad SPI with new set of init parameters */
	ret = no_os_spi_init(&quad_spi_desc, &qspi_init_params);
	if (ret) {
		return ret;
	}

	/* Start the transfer over Quad SPI using DMA */
	ret = no_os_spi_transfer_dma(quad_spi_desc,
				     &spi_msg,
				     1);
	if (ret) {
		return ret;
	}
#else
	/* Transfer using Data SPI */
	struct no_os_spi_msg spi_msg = {
		.bytes_number = bytes_to_transfer,
		.tx_buff = (uint8_t *)adc_data,
		.rx_buff = (uint8_t *)adc_data,
		.cs_change = 1
	};

	/* Remove the SPI descriptor */
	ret = no_os_spi_remove(dev->spi_desc);
	if (ret) {
		return ret;
	}

	/* Re-initialize the SPI to configure it as Data SPI */
	ret = no_os_spi_init(&dev->spi_desc, &data_spi_init_params);
	if (ret) {
		return ret;
	}

	/* Transfer using Data SPI */
	ret = no_os_spi_transfer(dev->spi_desc, &spi_msg, 1);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief  Prepares the device for data transfer.
 * @param  dev[in, out]- Device descriptor.
 * @param  mask[in]- Channels select mask.
 * @return 0 in case of success, error code otherwise.
 */
static int32_t iio_ad4080_prepare_transfer(void *dev, uint32_t mask)
{
	return 0;
}

/**
 * @brief  Ends the data transfer.
 * @param  dev[in, out]- Device descriptor.
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t iio_ad4080_end_transfer(void *dev)
{
	return 0;
}

/**
 * @brief  Deassert all oscillators.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad4080_deassert_oscillators(void)
{
	int32_t ret;

	/* Try to deassert all oscillator GPIOs */
	ret = no_os_gpio_set_value(gpio_osc_en_40m_desc, NO_OS_GPIO_LOW);
	ret |= no_os_gpio_set_value(gpio_osc_en_20m_desc, NO_OS_GPIO_LOW);
	ret |= no_os_gpio_set_value(gpio_osc_en_10m_desc, NO_OS_GPIO_LOW);

	return ret;
}

/**
 * @brief  Initiates data capture into FIFO.
 * @param samples[in] - Number of requested samples.
 * @param update_fifo_watermark[in] - Indication to update ADC FIFO watermark.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad4080_iio_start_fifo_mode_capture(uint32_t samples,
		bool update_fifo_watermark)
{
	int32_t ret;
	uint8_t val;
	uint32_t timeout = BUF_READ_TIMEOUT;

	/* Check for number of samples requested */
	if (samples > FIFO_SIZE) {
		return -EINVAL;
	}

	/* Enable GPIO3 and set it to track FIFO_FULL */
	ret = ad4080_update_bits(ad4080_dev_inst,
				 AD4080_REG_GPIO_CONFIG_A,
				 AD4080_GPIO_EN_MSK(3),
				 no_os_field_prep(AD4080_GPIO_EN_MSK(3), 1));
	if (ret) {
		return ret;
	}

	ret = ad4080_update_bits(ad4080_dev_inst,
				 AD4080_REG_GPIO_CONFIG_C,
				 AD4080_GPIO_SEL_MSK(3),
				 no_os_field_prep(AD4080_GPIO_SEL_MSK(3), AD4080_GPIO_FIFO_FULL));
	if (ret) {
		return ret;
	}

	/* If FIFO watermark has not been updated previously, update here */
	if (update_fifo_watermark) {
		/* Update FIFO Watermark to max FIFO depth, if requested number of samples
		* is greater than 16384 */
		ret = ad4080_update_bits(ad4080_dev_inst, AD4080_REG_FIFO_WATERMARK, 0xFF,
					 samples & 0xFF);
		if (ret) {
			return ret;
		}

		ret = ad4080_update_bits(ad4080_dev_inst, AD4080_REG_FIFO_WATERMARK + 1, 0xFF,
					 samples >> 8);
		if (ret) {
			return ret;
		}
	}

	/* Set FIFO in immediate trigger Mode, if not previously configured to any mode */
	if (ad4080_dev_inst->fifo_mode == AD4080_FIFO_DISABLE) {
		ret = ad4080_set_fifo_mode(ad4080_dev_inst, AD4080_IMMEDIATE_TRIGGER);
		if (ret) {
			return ret;
		}
	}

	/* Wait for FIFO full flag */
	while (timeout--) {
		no_os_gpio_get_value(gpio_gp3_desc, &val);
		if (val  == NO_OS_GPIO_HIGH) {
			break;
		}
	}

	/* Deassert all oscillators once data has been captured into FIFO */
	ret = ad4080_deassert_oscillators();
	if (ret) {
		return ret;
	}

	if (timeout == 0) {
		return -ETIMEDOUT;
	}

	return 0;
}

/**
 * @brief  Initiates data capture into FIFO.
 * @param formatted_fifo_data[out] - Pointer to formatted FIFO data buffer.
 * @param raw_fifo_data[in] - Pointer to raw FIFO data buffer.
 * @param samples[in] - Number of requested samples.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t ad4080_iio_end_fifo_mode_capture(uint32_t *formatted_fifo_data,
		uint8_t *raw_fifo_data, uint32_t samples)
{
	int32_t ret;
	uint8_t data_offset =
		(ad4080_dev_inst->cnv_spi_lvds_lanes == AD4080_ONE_LANE) ? 1 : 4;

	if (!raw_fifo_data) {
		return -EINVAL;
	}

	/* Assert back the chosen oscillator */
	switch (ad4080_sampling_freq) {
	case 40000000:
		/* 40 MHz */
		no_os_gpio_set_value(gpio_osc_en_40m_desc,
				     NO_OS_GPIO_HIGH);
		break;

	case 20000000:
		/* 20 MHz */
		no_os_gpio_set_value(gpio_osc_en_20m_desc,
				     NO_OS_GPIO_HIGH);
		break;

	case 10000000:
		/* 10 MHz */
		no_os_gpio_set_value(gpio_osc_en_10m_desc,
				     NO_OS_GPIO_HIGH);
		break;
	}

	/* Switch to Config SPI */
	no_os_spi_remove(ad4080_dev_inst->spi_desc);
	ret = no_os_spi_init(&ad4080_dev_inst->spi_desc,
			     &config_spi_init_params);
	if (ret) {
		return ret;
	}

	/* Disable FIFO */
	ret = ad4080_set_fifo_mode(ad4080_dev_inst, AD4080_FIFO_DISABLE);
	if (ret) {
		return ret;
	}

	/* Format the FIFO data based on data offset */
	for (uint32_t idx = 0; idx < samples; idx++) {
		formatted_fifo_data[idx] =
			no_os_get_unaligned_be24(&raw_fifo_data[3 * idx + data_offset]);
	}

	return 0;
}

/**
 * @brief Writes all the samples from the ADC buffer into the
		  IIO buffer.
 * @param iio_dev_data[in] - IIO device data instance.
 * @return 0 in case of success, negative value otherwise
 */
static int32_t iio_ad4080_submit_samples(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t remaining_samples = 0;
	bool update_fifo_watermark = false;

	/* Update remaining/requested sample count */
	remaining_samples = iio_dev_data->buffer->size /
			    iio_dev_data->buffer->bytes_per_scan;

	if (remaining_samples > FIFO_SIZE) {
		return -EINVAL;
	}

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	/* If FIFO watermark not set previously, set the watermark as the
	 * requested number of samples */
	if (ad4080_dev_inst->fifo_mode == AD4080_FIFO_DISABLE) {
		watermark = remaining_samples;
		update_fifo_watermark = true;
	}

	/* Start FIFO mode capture */
	ret = ad4080_iio_start_fifo_mode_capture(remaining_samples,
			update_fifo_watermark);
	if (ret) {
		goto end_capture;
	}

	/* Read FIFO data */
	ret = ad4080_read_fifo_data(ad4080_dev_inst, fifo_data, remaining_samples);
	if (ret) {
		goto end_capture;
	}

end_capture:
	/* End FIFO mode capture and retain the previous ret value if any of
	 * previous stages is failed */
	ret |= ad4080_iio_end_fifo_mode_capture(actual_fifo_data, fifo_data,
						remaining_samples);
	if (ret) {
		return ret;
	}

	/* Write data into circular buffer if all stages are success */
	ret = no_os_cb_write(iio_dev_data->buffer->buf,
			     actual_fifo_data,
			     (remaining_samples * BYTES_PER_SAMPLE));
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Read the debug register value
 * @param	dev[in, out]- Pointer to IIO device instance
 * @param	reg[in]- Register address to read from
 * @param	readval[in, out]- Pointer to variable to read data into
 * @return	0 in case of success, negative value otherwise
 */
static int32_t iio_ad4080_debug_reg_read(void *dev,
		uint32_t reg,
		uint32_t *read_val)
{
	if (!dev || !read_val || (reg > AD4080_LAST_REG_ADDR)) {
		return -EINVAL;
	}

	return ad4080_read(ad4080_dev_inst, reg, (uint8_t *)read_val);
}

/*
 * @brief	Write the debug register value
 * @param	dev[in, out]- Pointer to IIO device instance
 * @param	reg[in]- Register address to write data to
 * @param	writeval[in]- Pointer to variable to write data from
 * @return	0 in case of success, negative value otherwise
 */
static int32_t iio_ad4080_debug_reg_write(void *dev,
		uint32_t reg,
		uint32_t write_val)
{
	if (!dev || (reg > AD4080_LAST_REG_ADDR)) {
		return -EINVAL;
	}

	return ad4080_write(ad4080_dev_inst, reg, write_val);
}

/**
* @brief	Init for reading/writing and parameterization of a
* 			ad4080 IIO device
* @param 	desc[in,out] - IIO device descriptor
* @return	0 in case of success, negative error code otherwise
*/
static int32_t iio_ad4080_init(struct iio_device **desc)
{
	struct iio_device *iio_ad4080_inst;

	iio_ad4080_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad4080_inst) {
		return -EINVAL;
	}

	iio_ad4080_inst->num_ch = NO_OS_ARRAY_SIZE(iio_ad4080_channels);
	iio_ad4080_inst->channels = iio_ad4080_channels;
	iio_ad4080_inst->attributes = ad4080_iio_global_attributes;

	iio_ad4080_inst->submit = iio_ad4080_submit_samples;
	iio_ad4080_inst->pre_enable = iio_ad4080_prepare_transfer;
	iio_ad4080_inst->post_disable = iio_ad4080_end_transfer;
	iio_ad4080_inst->read_dev = NULL;
	iio_ad4080_inst->write_dev = NULL;
	iio_ad4080_inst->debug_reg_read = iio_ad4080_debug_reg_read;
	iio_ad4080_inst->debug_reg_write = iio_ad4080_debug_reg_write;

	*desc = iio_ad4080_inst;

	return 0;
}

/**
 * @brief	Initialize the IIO interface for AD4080 IIO device
 * @return	0 in case of success,negative error code otherwise
 */
int32_t ad4080_iio_initialize(void)
{
	/* IIO init status variable */
	int32_t init_status = 0;

	/* IIO Device init params */
	struct iio_device_init iio_device_init_params = {
		.name = (char *)ACTIVE_DEVICE_NAME,
		.raw_buf = adc_data_buffer,
		.raw_buf_len = DATA_BUFFER_SIZE
	};

	/* IIO interface init parameters */
	struct iio_init_param iio_init_params = {
		.phy_type = USE_UART,
		.nb_devs = 0,
		.devs = &iio_device_init_params,
	};

	/* EVB HW validation status */
	bool hw_mezzanine_is_valid = false;

	/* Provide necessary delay */
	no_os_mdelay(2000);

	do {
		/* Read context attributes */
		init_status = get_iio_context_attributes_ex(&iio_init_params.ctx_attrs,
				&iio_init_params.nb_ctx_attr,
				eeprom_desc,
				HW_MEZZANINE_NAME,
				STR(HW_CARRIER_NAME),
				&hw_mezzanine_is_valid,
				FIRMWARE_VERSION);
		if (init_status) {
			break;
		}

		/* If hardware is detected, then initialize it */
		if (hw_mezzanine_is_valid) {
			/* Initialize AD4080 device and peripheral interface */
			ad4080_init_params.spi_init = &config_spi_init_params;
			init_status = ad4080_init(&ad4080_dev_inst, ad4080_init_params);
			if (init_status) {
				break;
			}

			/* Initialize the AD4080 IIO device*/
			init_status = iio_ad4080_init(&ad4080_iio_dev);
			if (init_status) {
				/* Remove ADC descriptor */
				ad4080_remove(ad4080_dev_inst);
				break;
			}

			/* Increment number of devs */
			iio_init_params.nb_devs++;
		}
	} while (0);

	/* Initialize the IIO interface */
	iio_device_init_params.dev = &ad4080_dev_inst;
	iio_device_init_params.dev_descriptor = ad4080_iio_dev;

	iio_init_params.uart_desc = uart_iio_comm_desc;

	init_status = iio_init(&ad4080_iio_desc, &iio_init_params);
	if (init_status) {
		goto iio_fail;
	}

	return 0;

iio_fail:
	/* Remove IIO */
	iio_remove(ad4080_iio_desc);

	/* Free AD4080 IIO device */
	no_os_free(ad4080_iio_dev);

	/* Remove ADC descriptor */
	ad4080_remove(ad4080_dev_inst);

	/* Remove the IIO Context attributes */
	no_os_free(iio_init_params.ctx_attrs);

	return init_status;
}

/**
 * @brief 	Run the AD4080 IIO event handler
 * @return	none
 * @details	This function monitors the new IIO client event
 */
void ad4080_iio_event_handler(void)
{
	(void)iio_step(ad4080_iio_desc);
}
