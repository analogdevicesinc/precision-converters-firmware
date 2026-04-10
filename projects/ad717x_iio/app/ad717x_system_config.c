/***************************************************************************//**
 *   @file    ad717x_system_config.c
 *   @brief   Implementation of AD717x System Configuration
 ********************************************************************************
 * Copyright (c) 2026 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
 ******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "ad717x_system_config.h"
#include "ad717x_user_config.h"
#include "no_os_alloc.h"
#include "no_os_error.h"
#include "no_os_util.h"

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/
static int iio_ad717x_setup_attr_get(void *device,
				     char *buf,
				     uint32_t len,
				     const struct iio_ch_info *channel,
				     intptr_t priv);

static int iio_ad717x_setup_attr_set(void *device,
				     char *buf,
				     uint32_t len,
				     const struct iio_ch_info *channel,
				     intptr_t priv);

static int iio_ad717x_setup_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int iio_ad717x_setup_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int iio_ad717x_ch_attr_get(void *device,
				  char *buf,
				  uint32_t len,
				  const struct iio_ch_info *channel,
				  intptr_t priv);

static int iio_ad717x_ch_attr_set(void *device,
				  char *buf,
				  uint32_t len,
				  const struct iio_ch_info *channel,
				  intptr_t priv);

static int iio_ad717x_ch_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int iio_ad717x_ch_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/* AD717x setup attribute definition */
#define AD717x_SETUP_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = iio_ad717x_setup_attr_get,\
	.store = iio_ad717x_setup_attr_set\
}

/* AD717x setup attribute available definition */
#define AD717x_SETUP_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = iio_ad717x_setup_attr_available_get,\
	.store = iio_ad717x_setup_attr_available_set\
}

/* AD717x setup channel definition */
#define AD717x_SETUP_CH(_idx) {\
	.name = "setup" # _idx,\
	.ch_type = IIO_VOLTAGE,\
	.ch_out = false,\
	.indexed = true,\
	.channel = _idx,\
	.attributes = ad717x_setup_channel_attributes,\
}

/* AD717x channel attribute definition */
#define AD717x_CH_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = iio_ad717x_ch_attr_get,\
	.store = iio_ad717x_ch_attr_set\
}

/* AD717x channel attribute available definition */
#define AD717x_CH_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = iio_ad717x_ch_attr_available_get,\
	.store = iio_ad717x_ch_attr_available_set\
}

/* AD717x ADC channel definition */
#define AD717x_ADC_CH(_idx) {\
	.name = "ch" # _idx,\
	.ch_type = IIO_VOLTAGE,\
	.ch_out = false,\
	.indexed = true,\
	.channel = _idx,\
	.attributes = ad717x_adc_channel_attributes,\
}

/* Index into boolean option arrays */
#define AD717x_OPT_IDX_FALSE  0
#define AD717x_OPT_IDX_TRUE   1

/* Index into filter order option array */
#define AD717x_FILTER_IDX_SINC5_SINC1  0
#define AD717x_FILTER_IDX_SINC3        1

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/**
 * @enum ad717x_setup_attribute_ids
 * @brief AD717x setup attribute unique IDs
 */
enum ad717x_setup_attribute_ids {
	POLARITY_ATTR_ID,
	REFERENCE_SOURCE_ATTR_ID,
	REFERENCE_BUFFER_ATTR_ID,
	INPUT_BUFFER_ATTR_ID,
	DATA_RATE_ATTR_ID,
	FILTER_ATTR_ID,
	POST_FILTER_ENABLE_ATTR_ID,
	POST_FILTER_ATTR_ID,
	RECONFIGURE_SYSTEM_ATTR_ID,
};

/**
 * @enum ad717x_ch_attribute_ids
 * @brief AD717x ADC channel attribute unique IDs
 */
enum ad717x_ch_attribute_ids {
	SETUP_ATTR_ID,
};

/* Setup channel attributes */
static struct iio_attribute ad717x_setup_channel_attributes[] = {
	AD717x_SETUP_ATTR("polarity", POLARITY_ATTR_ID),
	AD717x_SETUP_AVAIL_ATTR("polarity_available", POLARITY_ATTR_ID),
	AD717x_SETUP_ATTR("reference_source", REFERENCE_SOURCE_ATTR_ID),
	AD717x_SETUP_AVAIL_ATTR("reference_source_available", REFERENCE_SOURCE_ATTR_ID),
	AD717x_SETUP_ATTR("reference_buffer", REFERENCE_BUFFER_ATTR_ID),
	AD717x_SETUP_AVAIL_ATTR("reference_buffer_available", REFERENCE_BUFFER_ATTR_ID),
	AD717x_SETUP_ATTR("input_buffer", INPUT_BUFFER_ATTR_ID),
	AD717x_SETUP_AVAIL_ATTR("input_buffer_available", INPUT_BUFFER_ATTR_ID),
	AD717x_SETUP_ATTR("data_rate", DATA_RATE_ATTR_ID),
	AD717x_SETUP_AVAIL_ATTR("data_rate_available", DATA_RATE_ATTR_ID),
	AD717x_SETUP_ATTR("filter", FILTER_ATTR_ID),
	AD717x_SETUP_AVAIL_ATTR("filter_available", FILTER_ATTR_ID),
	AD717x_SETUP_ATTR("post_filter_enable", POST_FILTER_ENABLE_ATTR_ID),
	AD717x_SETUP_AVAIL_ATTR("post_filter_enable_available", POST_FILTER_ENABLE_ATTR_ID),
	AD717x_SETUP_ATTR("post_filter", POST_FILTER_ATTR_ID),
	AD717x_SETUP_AVAIL_ATTR("post_filter_available", POST_FILTER_ATTR_ID),
	END_ATTRIBUTES_ARRAY
};

/* ADC channel attributes (only setup assignment) */
static struct iio_attribute ad717x_adc_channel_attributes[] = {
	AD717x_CH_ATTR("setup", SETUP_ATTR_ID),
	AD717x_CH_AVAIL_ATTR("setup_available", SETUP_ATTR_ID),
	END_ATTRIBUTES_ARRAY
};

/* Global device attributes for reconfiguration */
static struct iio_attribute ad717x_system_config_global_attributes[] = {
	AD717x_SETUP_ATTR("reconfigure_system", RECONFIGURE_SYSTEM_ATTR_ID),
	AD717x_SETUP_AVAIL_ATTR("reconfigure_system_available", RECONFIGURE_SYSTEM_ATTR_ID),
	END_ATTRIBUTES_ARRAY
};

/* Polarity options */
static const char *ad717x_polarity_options[] = {
	"unipolar",
	"bipolar",
};

/* Reference source options */
static const char *ad717x_reference_source_options[] = {
	"external",
	"internal",
	"avdd_avss",
};

/* Buffer enable/disable options */
static const char *ad717x_buffer_options[] = {
	"disable",
	"enable",
};

/* Filter order options */
static const char *ad717x_filter_options[] = {
	"sinc5_sinc1",
	"sinc3",
};

/* Post filter enable/disable options */
static const char *ad717x_post_filter_enable_options[] = {
	"disable",
	"enable",
};

/* Post filter (enhanced filter) options */
static const char *ad717x_post_filter_options[] = {
	"sps27_db47_ms36p7",
	"sps25_db62_ms40",
	"sps20_db86_ms50",
	"sps16p6_db82_ms60",
};

/* Enum values corresponding to ad717x_reference_source_options */
static const enum ad717x_reference_source ad717x_reference_source_values[] = {
	EXTERNAL_REF,
	INTERNAL_REF,
	AVDD_AVSS,
};

/* Enum values corresponding to ad717x_post_filter_options */
static const enum ad717x_enhfilt ad717x_post_filter_values[] = {
	sps27_db47_ms36p7,
	sps25_db62_ms40,
	sps20_db86_ms50,
	sps16p6_db82_ms60,
};

/* Enhanced filter (post filter) output data rates in SPS */
static const float ad717x_enhfilt_odr[] = {
	[sps27_db47_ms36p7]  = 27.27f,
	[sps25_db62_ms40]    = 25.0f,
	[sps20_db86_ms50]    = 20.0f,
	[sps16p6_db82_ms60]  = 16.67f,
};

/* Maximum number of ODR register values (0x00 to 0x16) */
#define AD717x_NUM_ODR_OPTIONS	23

/* Data rate (ODR) sinc5+sinc1 option strings, initialized with
 * AD4111/AD4112/AD4113/AD4114 values from sinc5_sinc1_odr_map.
 * Overwritten at runtime based on detected device. */
static const char *ad717x_data_rate_sinc5_sinc1_options[AD717x_NUM_ODR_OPTIONS]
= {
	"31250.00",	/* 0x00 */
	"31250.00",	/* 0x01 */
	"31250.00",	/* 0x02 */
	"31250.00",	/* 0x03 */
	"31250.00",	/* 0x04 */
	"31250.00",	/* 0x05 */
	"15625.00",	/* 0x06 */
	"10417.00",	/* 0x07 */
	"5208.00",	/* 0x08 */
	"2597.00",	/* 0x09 */
	"1007.00",	/* 0x0A */
	"503.80",	/* 0x0B */
	"381.00",	/* 0x0C */
	"200.30",	/* 0x0D */
	"100.20",	/* 0x0E */
	"59.52",	/* 0x0F */
	"49.68",	/* 0x10 */
	"20.01",	/* 0x11 */
	"16.63",	/* 0x12 */
	"10.00",	/* 0x13 */
	"5.00",		/* 0x14 */
	"2.50",		/* 0x15 */
	"1.25",		/* 0x16 */
};

/* Data rate (ODR) sinc3 option strings, initialized with
 * AD4111/AD4112/AD4113/AD4114 values from sinc3_odr_map.
 * Overwritten at runtime based on detected device. */
static const char *ad717x_data_rate_sinc3_options[AD717x_NUM_ODR_OPTIONS] = {
	"31250.00",	/* 0x00 */
	"31250.00",	/* 0x01 */
	"31250.00",	/* 0x02 */
	"31250.00",	/* 0x03 */
	"31250.00",	/* 0x04 */
	"31250.00",	/* 0x05 */
	"15625.00",	/* 0x06 */
	"10417.00",	/* 0x07 */
	"5208.00",	/* 0x08 */
	"3906.00",	/* 0x09 */
	"1157.00",	/* 0x0A */
	"539.00",	/* 0x0B */
	"401.00",	/* 0x0C */
	"206.00",	/* 0x0D */
	"102.00",	/* 0x0E */
	"59.98",	/* 0x0F */
	"50.00",	/* 0x10 */
	"20.01",	/* 0x11 */
	"16.67",	/* 0x12 */
	"10.00",	/* 0x13 */
	"5.00",		/* 0x14 */
	"2.50",		/* 0x15 */
	"1.25",		/* 0x16 */
};

/* Number of valid ODR options for the current device */
static uint8_t num_data_rate_options = AD717x_NUM_ODR_OPTIONS;

/* Start index of valid ODR options for the current device */
static uint8_t data_rate_start_idx = 0;

/* Setup assignment options for ADC channels */
static const char *ad717x_setup_options[] = {
	"setup0",
	"setup1",
	"setup2",
	"setup3",
	"setup4",
	"setup5",
	"setup6",
	"setup7",
};

/* Reconfigure system options */
static const char *ad717x_reconfigure_system_options[] = {
	"enable",
};

/* Device pointer for attribute handlers to access */
static ad717x_dev *p_ad717x_dev = NULL;

/* Number of active setups for the current device */
static uint8_t num_active_setups = 0;

/* Number of active channels for the current device */
static uint8_t num_active_channels = 0;

/* Setup channels (setup0-setup7) */
static struct iio_channel ad717x_setup_channels[] = {
	AD717x_SETUP_CH(0),
	AD717x_SETUP_CH(1),
	AD717x_SETUP_CH(2),
	AD717x_SETUP_CH(3),
	AD717x_SETUP_CH(4),
	AD717x_SETUP_CH(5),
	AD717x_SETUP_CH(6),
	AD717x_SETUP_CH(7)
};

/* ADC channels (ch0-ch15) */
static struct iio_channel ad717x_adc_channels[] = {
	AD717x_ADC_CH(0),
	AD717x_ADC_CH(1),
	AD717x_ADC_CH(2),
	AD717x_ADC_CH(3),
	AD717x_ADC_CH(4),
	AD717x_ADC_CH(5),
	AD717x_ADC_CH(6),
	AD717x_ADC_CH(7),
	AD717x_ADC_CH(8),
	AD717x_ADC_CH(9),
	AD717x_ADC_CH(10),
	AD717x_ADC_CH(11),
	AD717x_ADC_CH(12),
	AD717x_ADC_CH(13),
	AD717x_ADC_CH(14),
	AD717x_ADC_CH(15)
};

/* Combined channel list: setup channels + ADC channels */
static struct iio_channel ad717x_sys_config_all_channels[AD717x_MAX_SETUPS +
					  AD717x_MAX_CHANNELS];

/* Flag to trigger a system reconfiguration */
static bool restart_iio_flag = false;

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/
/**
 * @brief Populate ODR string options for the given device and filter type
 * @param dev_id - Active device type
 * @param odr_options - Array of string pointers to populate
 * @param num_options[out] - Number of valid ODR options
 * @param start_idx[out] - Start index of valid ODR options
 * @param is_sinc3 - true for sinc3 filter, false for sinc5+sinc1
 */
static void ad717x_populate_odr_options(enum ad717x_device_type dev_id,
					const char *odr_options[],
					uint8_t *num_options,
					uint8_t *start_idx,
					bool is_sinc3)
{
	switch (dev_id) {
	case ID_AD4115:
		odr_options[0]  = "125000.00";
		odr_options[1]  = "125000.00";
		odr_options[2]  = "62500.00";
		odr_options[3]  = "62500.00";
		odr_options[4]  = "31250.00";
		odr_options[5]  = "25000.00";
		odr_options[6]  = "15625.00";
		odr_options[7]  = "10417.00";
		odr_options[8]  = "5000.00";
		odr_options[9]  = is_sinc3 ? "3906.00" : "2500.00";
		odr_options[10] = is_sinc3 ? "1157.00" : "1000.00";
		odr_options[11] = is_sinc3 ? "539.00"  : "500.00";
		odr_options[12] = is_sinc3 ? "401.00"  : "397.50";
		odr_options[13] = is_sinc3 ? "206.00"  : "200.00";
		odr_options[14] = is_sinc3 ? "102.00"  : "100.00";
		odr_options[15] = "59.98";
		odr_options[16] = is_sinc3 ? "50.00" : "49.96";
		odr_options[17] = "20.00";
		odr_options[18] = "16.67";
		odr_options[19] = "10.00";
		odr_options[20] = "5.00";
		odr_options[21] = "2.50";
		*num_options = 22;
		*start_idx = 0;
		break;

	case ID_AD4111:
	case ID_AD4112:
	case ID_AD4113:
	case ID_AD4114:
		odr_options[0]  = "31250.00";
		odr_options[1]  = "31250.00";
		odr_options[2]  = "31250.00";
		odr_options[3]  = "31250.00";
		odr_options[4]  = "31250.00";
		odr_options[5]  = "31250.00";
		odr_options[6]  = "15625.00";
		odr_options[7]  = "10417.00";
		odr_options[8]  = "5208.00";
		odr_options[9]  = is_sinc3 ? "3906.00" : "2597.00";
		odr_options[10] = is_sinc3 ? "1157.00" : "1007.00";
		odr_options[11] = is_sinc3 ? "539.00"  : "503.80";
		odr_options[12] = is_sinc3 ? "401.00"  : "381.00";
		odr_options[13] = is_sinc3 ? "206.00"  : "200.30";
		odr_options[14] = is_sinc3 ? "102.00"  : "100.20";
		odr_options[15] = is_sinc3 ? "59.98"   : "59.52";
		odr_options[16] = is_sinc3 ? "50.00"   : "49.68";
		odr_options[17] = "20.01";
		odr_options[18] = is_sinc3 ? "16.67" : "16.63";
		odr_options[19] = "10.00";
		odr_options[20] = "5.00";
		odr_options[21] = "2.50";
		odr_options[22] = "1.25";
		*num_options = AD717x_NUM_ODR_OPTIONS;
		*start_idx = 0;
		break;

	case ID_AD7172_2:
	case ID_AD7172_4:
	case ID_AD7173_8:
		odr_options[0]  = "31250.00";
		odr_options[1]  = "31250.00";
		odr_options[2]  = "31250.00";
		odr_options[3]  = "31250.00";
		odr_options[4]  = "31250.00";
		odr_options[5]  = "31250.00";
		odr_options[6]  = "15625.00";
		odr_options[7]  = "10417.00";
		odr_options[8]  = "5208.00";
		odr_options[9]  = is_sinc3 ? "2604.00" : "2597.00";
		odr_options[10] = is_sinc3 ? "1008.00" : "1007.00";
		odr_options[11] = is_sinc3 ? "504.00"  : "503.80";
		odr_options[12] = is_sinc3 ? "400.60"  : "381.00";
		odr_options[13] = "200.30";
		odr_options[14] = "100.20";
		odr_options[15] = is_sinc3 ? "59.98"   : "59.52";
		odr_options[16] = is_sinc3 ? "50.00"   : "49.68";
		odr_options[17] = "20.01";
		odr_options[18] = is_sinc3 ? "16.67" : "16.63";
		odr_options[19] = "10.00";
		odr_options[20] = "5.00";
		odr_options[21] = "2.50";
		odr_options[22] = "1.25";
		*num_options = AD717x_NUM_ODR_OPTIONS;
		*start_idx = 0;
		break;

	case ID_AD7176_2:
	case ID_AD7175_2:
	case ID_AD7175_8:
		odr_options[0]  = "250000.00";
		odr_options[1]  = "125000.00";
		odr_options[2]  = "62500.00";
		odr_options[3]  = "50000.00";
		odr_options[4]  = "31250.00";
		odr_options[5]  = "25000.00";
		odr_options[6]  = "15625.00";
		odr_options[7]  = "10000.00";
		odr_options[8]  = "5000.00";
		odr_options[9]  = "2500.00";
		odr_options[10] = "1000.00";
		odr_options[11] = "500.00";
		odr_options[12] = is_sinc3 ? "400.00" : "397.50";
		odr_options[13] = "200.00";
		odr_options[14] = "100.00";
		odr_options[15] = is_sinc3 ? "60.00" : "59.94";
		odr_options[16] = is_sinc3 ? "50.00" : "49.96";
		odr_options[17] = "20.00";
		odr_options[18] = "16.67";
		odr_options[19] = "10.00";
		odr_options[20] = "5.00";
		*num_options = 21;
		*start_idx = 0;
		break;

	case ID_AD7177_2:
		odr_options[7]  = "10000.00";
		odr_options[8]  = "5000.00";
		odr_options[9]  = "2500.00";
		odr_options[10] = "1000.00";
		odr_options[11] = "500.00";
		odr_options[12] = is_sinc3 ? "400.00" : "397.50";
		odr_options[13] = "200.00";
		odr_options[14] = "100.00";
		odr_options[15] = is_sinc3 ? "60.00" : "59.94";
		odr_options[16] = is_sinc3 ? "50.00" : "49.96";
		odr_options[17] = "20.00";
		odr_options[18] = "16.67";
		odr_options[19] = "10.00";
		odr_options[20] = "5.00";
		*num_options = 21;
		*start_idx = 7;
		break;

	case ID_AD4116:
		odr_options[0]  = "62500.00";
		odr_options[1]  = "62500.00";
		odr_options[2]  = "62500.00";
		odr_options[3]  = "62500.00";
		odr_options[4]  = "31250.00";
		odr_options[5]  = "31250.00";
		odr_options[6]  = "15625.00";
		odr_options[7]  = "10417.00";
		odr_options[8]  = is_sinc3 ? "5208.00" : "5194.00";
		odr_options[9]  = is_sinc3 ? "2500.00" : "2597.00";
		odr_options[10] = is_sinc3 ? "1008.00" : "1007.00";
		odr_options[11] = is_sinc3 ? "500.00"  : "499.90";
		odr_options[12] = is_sinc3 ? "400.00"  : "390.60";
		odr_options[13] = is_sinc3 ? "200.00"  : "200.30";
		odr_options[14] = "100.00";
		odr_options[15] = is_sinc3 ? "60.00"  : "59.75";
		odr_options[16] = is_sinc3 ? "50.00"  : "49.84";
		odr_options[17] = "20.00";
		odr_options[18] = is_sinc3 ? "16.67" : "16.65";
		odr_options[19] = "10.00";
		odr_options[20] = "5.00";
		odr_options[21] = "2.50";
		odr_options[22] = "1.25";
		*num_options = AD717x_NUM_ODR_OPTIONS;
		*start_idx = 0;
		break;

	default:
		break;
	}
}

/**
 * @brief Get the effective sampling frequency with all channels enabled
 * @param device - AD717x device instance
 * @param sampling_freq[out] - Pointer to store the sampling frequency in Hz
 * @return 0 in case of success, negative error code otherwise
 * @details The ADC cycles through each channel sequentially. The
 *          overall effective sampling frequency is the minimum ODR across
 *          all channels divided by the total number of channels.
 */
int32_t ad717x_get_sampling_frequency(ad717x_dev *device,
				      float *sampling_freq)
{
	uint8_t ch;
	uint8_t setup_sel;
	uint8_t indx;
	const char **odr_options;
	float odr_val;
	float min_odr = 0;
	uint8_t num_channels;

	if (!device || !sampling_freq) {
		return -EINVAL;
	}

	num_channels = device->num_channels;
	if (num_channels == 0) {
		return -EINVAL;
	}

	for (ch = 0; ch < num_channels; ch++) {
		setup_sel = device->chan_map[ch].setup_sel;

		if (setup_sel >= device_map_table[device->active_device].num_setups) {
			return -EINVAL;
		}

		if (device->filter_configuration[setup_sel].enhfilten) {
			/* Enhanced filter overrides ODR setting with a fixed rate */
			odr_val = ad717x_enhfilt_odr[device->filter_configuration[setup_sel].enhfilt];
		} else {
			indx = (uint8_t)device->filter_configuration[setup_sel].odr;
			if (indx >= AD717x_NUM_ODR_OPTIONS) {
				return -EINVAL;
			}

			odr_options = (device->filter_configuration[setup_sel].oder == sinc3) ?
				      ad717x_data_rate_sinc3_options :
				      ad717x_data_rate_sinc5_sinc1_options;

			odr_val = strtof(odr_options[indx], NULL);
		}

		if (ch == 0 || odr_val < min_odr) {
			min_odr = odr_val;
		}
	}

	*sampling_freq = min_odr / num_channels;

	return 0;
}

/**
 * @brief Getter function for AD717x setup attributes
 * @param device[in, out] - Pointer to IIO device instance
 * @param buf[in] - IIO input data buffer
 * @param len[in] - Number of expected bytes
 * @param channel[in] - Input channel
 * @param priv[in] - Attribute private ID
 * @return len in case of success, negative error code otherwise
 */
static int iio_ad717x_setup_attr_get(void *device,
				     char *buf,
				     uint32_t len,
				     const struct iio_ch_info *channel,
				     intptr_t priv)
{
	uint8_t setup_id;
	uint8_t indx;
	const char **odr_options;

	if (!p_ad717x_dev) {
		return -EINVAL;
	}

	setup_id = channel->ch_num;

	if (setup_id >= num_active_setups) {
		return -EINVAL;
	}

	switch (priv) {
	case POLARITY_ATTR_ID:
		return sprintf(buf, "%s",
			       ad717x_polarity_options[default_setups[setup_id].bi_unipolar ?
											    AD717x_OPT_IDX_TRUE : AD717x_OPT_IDX_FALSE]);

	case REFERENCE_SOURCE_ATTR_ID:
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_reference_source_values);
		     indx++) {
			if (ad717x_reference_source_values[indx] ==
			    default_setups[setup_id].ref_source) {
				return sprintf(buf, "%s", ad717x_reference_source_options[indx]);
			}
		}
		return -EINVAL;

	case REFERENCE_BUFFER_ATTR_ID:
		return sprintf(buf, "%s",
			       ad717x_buffer_options[default_setups[setup_id].ref_buff ? AD717x_OPT_IDX_TRUE :
										       AD717x_OPT_IDX_FALSE]);

	case INPUT_BUFFER_ATTR_ID:
		return sprintf(buf, "%s",
			       ad717x_buffer_options[default_setups[setup_id].input_buff ?
											 AD717x_OPT_IDX_TRUE : AD717x_OPT_IDX_FALSE]);

	case DATA_RATE_ATTR_ID:
		indx = (uint8_t)default_ad717x_filtcons[setup_id].odr;
		odr_options = (default_ad717x_filtcons[setup_id].oder == sinc3) ?
			      ad717x_data_rate_sinc3_options : ad717x_data_rate_sinc5_sinc1_options;
		if (indx >= AD717x_NUM_ODR_OPTIONS) {
			return -EINVAL;
		}
		return sprintf(buf, "%s", odr_options[indx]);

	case FILTER_ATTR_ID:
		return sprintf(buf, "%s",
			       ad717x_filter_options[default_ad717x_filtcons[setup_id].oder == sinc5_sinc1 ?
											    AD717x_FILTER_IDX_SINC5_SINC1 : AD717x_FILTER_IDX_SINC3]);

	case POST_FILTER_ENABLE_ATTR_ID:
		return sprintf(buf, "%s",
			       ad717x_post_filter_enable_options[default_ad717x_filtcons[setup_id].enhfilten ?
											   AD717x_OPT_IDX_TRUE : AD717x_OPT_IDX_FALSE]);

	case POST_FILTER_ATTR_ID:
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_post_filter_values); indx++) {
			if (ad717x_post_filter_values[indx] ==
			    default_ad717x_filtcons[setup_id].enhfilt) {
				return sprintf(buf, "%s", ad717x_post_filter_options[indx]);
			}
		}
		return -EINVAL;

	case RECONFIGURE_SYSTEM_ATTR_ID:
		return sprintf(buf, "%s", "enable");

	default:
		return -EINVAL;
	}

	return len;
}

/**
 * @brief Setter function for AD717x setup attributes
 * @param device[in, out] - Pointer to IIO device instance
 * @param buf[in] - IIO input data buffer
 * @param len[in] - Number of expected bytes
 * @param channel[in] - Input channel
 * @param priv[in] - Attribute private ID
 * @return len in case of success, negative error code otherwise
 */
static int iio_ad717x_setup_attr_set(void *device,
				     char *buf,
				     uint32_t len,
				     const struct iio_ch_info *channel,
				     intptr_t priv)
{
	uint8_t setup_id;
	uint8_t indx;
	const char **odr_options;

	if (!p_ad717x_dev || !buf) {
		return -EINVAL;
	}

	setup_id = channel->ch_num;

	if (setup_id >= num_active_setups) {
		return -EINVAL;
	}

	switch (priv) {
	case POLARITY_ATTR_ID:
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_polarity_options); indx++) {
			if (!strcmp(buf, ad717x_polarity_options[indx])) {
				default_setups[setup_id].bi_unipolar = (indx == AD717x_OPT_IDX_TRUE);
				return len;
			}
		}
		return -EINVAL;

	case REFERENCE_SOURCE_ATTR_ID:
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_reference_source_options);
		     indx++) {
			if (!strcmp(buf, ad717x_reference_source_options[indx])) {
				default_setups[setup_id].ref_source = ad717x_reference_source_values[indx];
				return len;
			}
		}
		return -EINVAL;

	case REFERENCE_BUFFER_ATTR_ID:
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_buffer_options); indx++) {
			if (!strcmp(buf, ad717x_buffer_options[indx])) {
				default_setups[setup_id].ref_buff = (indx == AD717x_OPT_IDX_TRUE);
				return len;
			}
		}
		return -EINVAL;

	case INPUT_BUFFER_ATTR_ID:
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_buffer_options); indx++) {
			if (!strcmp(buf, ad717x_buffer_options[indx])) {
				default_setups[setup_id].input_buff = (indx == AD717x_OPT_IDX_TRUE);
				return len;
			}
		}
		return -EINVAL;

	case DATA_RATE_ATTR_ID:
		if (default_ad717x_filtcons[setup_id].oder == sinc3) {
			odr_options = ad717x_data_rate_sinc3_options;
		} else {
			odr_options = ad717x_data_rate_sinc5_sinc1_options;
		}

		for (indx = data_rate_start_idx; indx < num_data_rate_options; indx++) {
			if (!strcmp(buf, odr_options[indx])) {
				default_ad717x_filtcons[setup_id].odr = indx;
				return len;
			}
		}
		return -EINVAL;

	case FILTER_ATTR_ID:
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_filter_options); indx++) {
			if (!strcmp(buf, ad717x_filter_options[indx])) {
				default_ad717x_filtcons[setup_id].oder =
					(indx == AD717x_FILTER_IDX_SINC5_SINC1) ? sinc5_sinc1 : sinc3;
				return len;
			}
		}
		return -EINVAL;

	case POST_FILTER_ENABLE_ATTR_ID:
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_post_filter_enable_options);
		     indx++) {
			if (!strcmp(buf, ad717x_post_filter_enable_options[indx])) {
				default_ad717x_filtcons[setup_id].enhfilten = (indx == AD717x_OPT_IDX_TRUE);
				return len;
			}
		}
		return -EINVAL;

	case POST_FILTER_ATTR_ID:
		for (indx = 0; indx < NO_OS_ARRAY_SIZE(ad717x_post_filter_options); indx++) {
			if (!strcmp(buf, ad717x_post_filter_options[indx])) {
				default_ad717x_filtcons[setup_id].enhfilt = ad717x_post_filter_values[indx];
				return len;
			}
		}
		return -EINVAL;

	case RECONFIGURE_SYSTEM_ATTR_ID:
		if (!strcmp(buf, ad717x_reconfigure_system_options[0])) {
			restart_iio_flag = true;
			return len;
		}
		return -EINVAL;

	default:
		return -EINVAL;
	}

	return -EINVAL;
}

/**
 * @brief Attribute available getter function for AD717x setup attributes
 * @param device[in, out] - Pointer to IIO device instance
 * @param buf[in] - IIO input data buffer
 * @param len[in] - Number of input bytes
 * @param channel[in] - Input channel
 * @param priv[in] - Attribute private ID
 * @return len in case of success, negative error code otherwise
 */
static int iio_ad717x_setup_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	uint8_t indx;
	uint32_t offset = 0;
	uint8_t setup_id = channel->ch_num;
	const char **odr_options;

	switch (priv) {
	case POLARITY_ATTR_ID:
		return sprintf(buf,
			       "%s %s",
			       ad717x_polarity_options[0],
			       ad717x_polarity_options[1]);

	case REFERENCE_SOURCE_ATTR_ID:
		return sprintf(buf,
			       "%s %s %s",
			       ad717x_reference_source_options[0],
			       ad717x_reference_source_options[1],
			       ad717x_reference_source_options[2]);

	case REFERENCE_BUFFER_ATTR_ID:
	case INPUT_BUFFER_ATTR_ID:
		return sprintf(buf,
			       "%s %s",
			       ad717x_buffer_options[0],
			       ad717x_buffer_options[1]);

	case DATA_RATE_ATTR_ID:
		/* Select the correct array based on the current filter for this setup */
		if (default_ad717x_filtcons[setup_id].oder == sinc3) {
			odr_options = ad717x_data_rate_sinc3_options;
		} else {
			odr_options = ad717x_data_rate_sinc5_sinc1_options;
		}

		/* Print available data rate options, skipping duplicates */
		for (indx = data_rate_start_idx; indx < num_data_rate_options; indx++) {
			if (indx > data_rate_start_idx
			    && !strcmp(odr_options[indx], odr_options[indx - 1])) {
				continue;
			}
			if (offset > 0) {
				offset += sprintf(buf + offset, " ");
			}
			offset += sprintf(buf + offset, "%s", odr_options[indx]);
		}
		return offset;

	case FILTER_ATTR_ID:
		return sprintf(buf,
			       "%s %s",
			       ad717x_filter_options[0],
			       ad717x_filter_options[1]);

	case POST_FILTER_ENABLE_ATTR_ID:
		return sprintf(buf,
			       "%s %s",
			       ad717x_post_filter_enable_options[0],
			       ad717x_post_filter_enable_options[1]);

	case POST_FILTER_ATTR_ID:
		return sprintf(buf,
			       "%s %s %s %s",
			       ad717x_post_filter_options[0],
			       ad717x_post_filter_options[1],
			       ad717x_post_filter_options[2],
			       ad717x_post_filter_options[3]);

	case RECONFIGURE_SYSTEM_ATTR_ID:
		return sprintf(buf, "%s", ad717x_reconfigure_system_options[0]);

	default:
		break;
	}

	return len;
}

/**
 * @brief Attribute available setter function for AD717x setup attributes
 * @param device[in, out] - Pointer to IIO device instance
 * @param buf[in] - IIO input data buffer
 * @param len[in] - Number of input bytes
 * @param channel[in] - Input channel
 * @param priv[in] - Attribute private ID
 * @return len in case of success, negative error code otherwise
 */
static int iio_ad717x_setup_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	/* Available attributes are read-only */
	return len;
}

/**
 * @brief Getter function for AD717x ADC channel attributes
 * @param device[in, out] - Pointer to IIO device instance
 * @param buf[in] - IIO input data buffer
 * @param len[in] - Number of expected bytes
 * @param channel[in] - Input channel
 * @param priv[in] - Attribute private ID
 * @return len in case of success, negative error code otherwise
 */
static int iio_ad717x_ch_attr_get(void *device,
				  char *buf,
				  uint32_t len,
				  const struct iio_ch_info *channel,
				  intptr_t priv)
{
	uint8_t ch_num;
	uint8_t setup_sel;
	bool use_input_pairs;

	if (!p_ad717x_dev) {
		return -EINVAL;
	}

	use_input_pairs = device_map_table[p_ad717x_dev->active_device].use_input_pairs;
	ch_num = channel->ch_num -
		 device_map_table[p_ad717x_dev->active_device].num_setups;

	if (ch_num >= num_active_channels) {
		return -EINVAL;
	}

	switch (priv) {
	case SETUP_ATTR_ID:
		if (use_input_pairs) {
			setup_sel = default_ad411x_chan_maps[ch_num].setup_sel;
		} else {
			setup_sel = default_ad717x_chan_maps[ch_num].setup_sel;
		}

		if (setup_sel >= num_active_setups) {
			return -EINVAL;
		}

		return sprintf(buf, "%s", ad717x_setup_options[setup_sel]);

	default:
		return -EINVAL;
	}

	return len;
}

/**
 * @brief Setter function for AD717x ADC channel attributes
 * @param device[in, out] - Pointer to IIO device instance
 * @param buf[in] - IIO input data buffer
 * @param len[in] - Number of expected bytes
 * @param channel[in] - Input channel
 * @param priv[in] - Attribute private ID
 * @return len in case of success, negative error code otherwise
 */
static int iio_ad717x_ch_attr_set(void *device,
				  char *buf,
				  uint32_t len,
				  const struct iio_ch_info *channel,
				  intptr_t priv)
{
	uint8_t ch_num;
	uint32_t indx;
	bool use_input_pairs;

	if (!p_ad717x_dev || !buf) {
		return -EINVAL;
	}

	use_input_pairs = device_map_table[p_ad717x_dev->active_device].use_input_pairs;
	ch_num = channel->ch_num -
		 device_map_table[p_ad717x_dev->active_device].num_setups;

	if (ch_num >= num_active_channels) {
		return -EINVAL;
	}

	switch (priv) {
	case SETUP_ATTR_ID:
		for (indx = 0; indx < num_active_setups; indx++) {
			if (!strcmp(buf, ad717x_setup_options[indx])) {
				if (use_input_pairs) {
					default_ad411x_chan_maps[ch_num].setup_sel = indx;
				} else {
					default_ad717x_chan_maps[ch_num].setup_sel = indx;
				}
				return len;
			}
		}
		return -EINVAL;

	default:
		return -EINVAL;
	}

	return -EINVAL;
}

/**
 * @brief Attribute available getter function for AD717x ADC channel attributes
 * @param device[in, out] - Pointer to IIO device instance
 * @param buf[in] - IIO input data buffer
 * @param len[in] - Number of input bytes
 * @param channel[in] - Input channel
 * @param priv[in] - Attribute private ID
 * @return len in case of success, negative error code otherwise
 */
static int iio_ad717x_ch_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	uint8_t indx;
	int offset = 0;

	switch (priv) {
	case SETUP_ATTR_ID:
		for (indx = 0; indx < num_active_setups; indx++) {
			if (offset > 0) {
				offset += sprintf(buf + offset, " ");
			}
			offset += sprintf(buf + offset, "%s", ad717x_setup_options[indx]);
		}
		return offset;

	default:
		break;
	}

	return len;
}

/**
 * @brief Attribute available setter function for AD717x ADC channel attributes
 * @param device[in, out] - Pointer to IIO device instance
 * @param buf[in] - IIO input data buffer
 * @param len[in] - Number of input bytes
 * @param channel[in] - Input channel
 * @param priv[in] - Attribute private ID
 * @return len in case of success, negative error code otherwise
 */
static int iio_ad717x_ch_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	/* Available attributes are read-only */
	return len;
}

/**
 * @brief Initialize the IIO device descriptor for AD717x system configuration
 * @param desc[in,out] - IIO device descriptor
 * @param device[in] - AD717x device instance
 * @return 0 in case of success, negative error code otherwise
 */
int32_t iio_ad717x_system_config_init(struct iio_device **desc,
				      ad717x_dev *device)
{
	struct iio_device *iio_ad717x_sys_cfg_inst;
	uint8_t indx;
	uint8_t total_channels;

	if (!desc || !device) {
		return -EINVAL;
	}

	/* Store device pointer for attribute handlers */
	p_ad717x_dev = device;

	/* Populate ODR options for the active device */
	ad717x_populate_odr_options(device->active_device,
				    ad717x_data_rate_sinc5_sinc1_options,
				    &num_data_rate_options,
				    &data_rate_start_idx,
				    false);

	ad717x_populate_odr_options(device->active_device,
				    ad717x_data_rate_sinc3_options,
				    &num_data_rate_options,
				    &data_rate_start_idx,
				    true);

	/* Get device-dependent setup and channel counts */
	num_active_setups = device_map_table[device->active_device].num_setups;
	num_active_channels = device_map_table[device->active_device].num_channels;
	total_channels = num_active_setups + num_active_channels;

	/* Allocate IIO device descriptor */
	iio_ad717x_sys_cfg_inst = no_os_calloc(1, sizeof(struct iio_device));
	if (!iio_ad717x_sys_cfg_inst) {
		return -ENOMEM;
	}

	/* Build combined channel list: setup channels first, then ADC channels */
	for (indx = 0; indx < num_active_setups; indx++) {
		ad717x_sys_config_all_channels[indx] = ad717x_setup_channels[indx];
	}
	for (indx = 0; indx < num_active_channels; indx++) {
		ad717x_adc_channels[indx].channel = indx +
						    num_active_setups; // Adjust IIO channel number for ADC setups
		ad717x_sys_config_all_channels[num_active_setups + indx] =
			ad717x_adc_channels[indx];
	}

	/* Assign channels and global attributes to device */
	iio_ad717x_sys_cfg_inst->channels = ad717x_sys_config_all_channels;
	iio_ad717x_sys_cfg_inst->num_ch = total_channels;
	iio_ad717x_sys_cfg_inst->attributes = ad717x_system_config_global_attributes;

	*desc = iio_ad717x_sys_cfg_inst;

	return 0;
}

/**
 * @brief Check if the AD717x system reconfigure is requested
 * @return true if the system reconfigure is requested, false otherwise
 */
bool iio_ad717x_is_system_reconfigured(void)
{
	return restart_iio_flag;
}

/**
 * @brief Remove the AD717x system configuration device
 * @param desc - AD717x System Configuration device instance
 * @return 0 in case of success, negative error code otherwise
 */
int32_t iio_ad717x_system_config_remove(struct iio_device *desc)
{
	if (desc) {
		no_os_free(desc);
		desc = NULL;
	}

	/* Reset state */
	num_active_setups = 0;
	num_active_channels = 0;
	restart_iio_flag = false;

	return 0;
}
