/***************************************************************************//**
*   @file    ad777x_iio.c
*   @brief   Source file of AD777x IIO interfaces
********************************************************************************
* Copyright (c) 2022-2024 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <string.h>

#include "ad777x_user_config.h"
#include "ad777x_support.h"
#include "no_os_error.h"
#include "iio_trigger.h"
#include "no_os_util.h"
#include "no_os_gpio.h"
#include "no_os_irq.h"
#include "ad777x_iio.h"
#include "app_config.h"
#include "common.h"
#include "ad7779.h"
#include "iio.h"
#if (INTERFACE_MODE == TDM_MODE)
#include "stm32_tdm_support.h"
#endif

/* Forward declaration of getter/setter functions */
static int ad777x_get_attribute(void *device,char *buf, uint32_t len,
				const struct iio_ch_info *channel,intptr_t priv);

static int ad777x_set_attribute(void *device,char *buf,uint32_t len,
				const struct iio_ch_info *channel,intptr_t priv);

static int ad777x_get_avail_attribute(void *device, char *buf, uint32_t len,
				      const struct iio_ch_info *channel, intptr_t priv);

static int ad777x_set_avail_attribute(void *device, char *buf, uint32_t len,
				      const struct iio_ch_info *channel, intptr_t priv);

static void ad777x_update_scale_factor(void);

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/* Channel Attribute definition */
#define AD777x_CH_ATTR(_name, _idx) {\
	.name = _name,\
	.priv  = _idx,\
	.show = ad777x_get_attribute,\
	.store = ad777x_set_attribute\
}

/* Available Attribute definition */
#define AD777x_CH_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = ad777x_get_avail_attribute,\
	.store = ad777x_set_avail_attribute \
}

/* Channel definition */
#define AD777x_CHANNEL(_name, _idx) {\
	.name = _name # _idx, \
	.ch_type = IIO_VOLTAGE,\
	.ch_out = false,\
	.indexed = true,\
	.channel = _idx,\
	.scan_index = _idx,\
	.scan_type = &ad777x_scan_type,\
	.attributes = ad777x_channel_attributes\
}

/* Number of data storage bits (needed for IIO client) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

/* On-board Internal Reference - Fixed */
#define AD777x_INTERNAL_REF_VAL	2.5

/* Applied External Reference */
#define AD777x_EXTERNAL_REF_VAL	2.5

/* SAR ADC Resolution */
#define AD777X_SAR_RESOLUTION  12

/* SAR ADC Reference */
#define AD777X_SAR_REFERENCE	3.3

/* Default scale for SAR ADC */
#define AD777X_SAR_SCALE  AD777X_SAR_REFERENCE/(1 << (AD777X_SAR_RESOLUTION-1))

/* SRC Load High */
#define SRC_LOAD_UPDATE_HIGH	0x1

/* SRC Load Low */
#define SRC_LOAD_UPDATE_LOW		0x0

/* Attenuation factor for SRC Mux input */
#define AD777X_SRC_ATT_FACTOR	6

/* ADC data buffer size */
#if defined(USE_SDRAM)
#define adc_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(128000)
static int8_t adc_data_buffer[DATA_BUFFER_SIZE];
#endif

/* IIO trigger name */
#define AD777X_IIO_TRIGGER_NAME		"ad777x_iio_trigger"

/* Number of IIO Devices */
#define NUM_OF_IIO_DEVICES			1

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* Device Name */
static const char dev_name[] = ACTIVE_DEVICE;

/* Pointer to struct representing the AD777x IIO device */
ad7779_dev *p_ad777x_dev_inst = NULL;

/* AD777x IIO hw trigger descriptor */
static struct iio_hw_trig *ad777x_hw_trig_desc;

/* IIO interface descriptor */
static struct iio_desc *p_ad777x_iio_desc;

/* Channel Attribute ID */
enum ad777x_attribute_id {
	RAW_ATTR_ID,
	SCALE_ATTR_ID,
	OFFSET_ATTR_ID,
	PGA_ATTR_ATTR_ID,
	PHASE_COMP_ATTR_ID,
	GAIN_COMP_ATTR_ID,
	OFFSET_COMP_ATTR_ID,
	SAMPLING_FREQ_ATTR_ID,
	SRC_ATTR_ID_INT,
	SRC_ATTR_ID_DEC,
	CONV_MODE_ATTR_ID,
	ERROR_FLAG1_ATTR_ID,
	ERROR_FLAG2_ATTR_ID,
	ERROR_FLAG3_ATTR_ID,
	AUXAINP_AUXAINN_MUX_ATTR_ID,
	DVBE_AVSSX_MUX_ATTR_ID,
	REF1P_REF1N_MUX_ATTR_ID,
	REF2P_REF2N_MUX_ATTR_ID,
	REF_OUT_AVSSX_MUX_ATTR_ID,
	VCM_AVSSX_MUX_ATTR_ID,
	AREG1CAP_AVSSX_MUX_ATTR_ID,
	AREG2CAP_AVSSX_MUX_ATTR_ID,
	DREGCAP_DGND_MUX_ATTR_ID,
	AVDD1A_AVSSX_MUX_ATTR_ID,
	AVDD1B_AVSSX_MUX_ATTR_ID,
	AVDD2A_AVSSX_MUX_ATTR_ID,
	AVDD2B_AVSSX_MUX_ATTR_ID,
	IOVDD_DGND_MUX_ATTR_ID,
	AVDD4_AVSSX_MUX_ATTR_ID,
	DGND_AVSS1A_MUX_ATTR_ID,
	DGND_AVSS1B_MUX_ATTR_ID,
	DGND_AVSSX_MUX_ATTR_ID,
	AVDD4_AVSSX_ATT_MUX_ATTR_ID,
	REF1P_AVSSX_MUX_ATTR_ID,
	REF2P_AVSSX_MUX_ATTR_ID,
	AVSSX_AVDD4_MUX_ATTR_ID,
	SINC5_STATE_ATTR_ID,
};

/* Channel Scan Type */
struct scan_type ad777x_scan_type = {
	.storagebits = CHN_STORAGE_BITS,
#if (ADC_TYPE == SD_ADC)
	.realbits = ADC_RESOLUTION,
#else
	.realbits = CHN_STORAGE_BITS,
#endif
	.shift = 0,
	.is_big_endian = false,
#if defined (BIPOLAR)
	.sign = 's'
#else
	.sign = 'u'
#endif
};

/* Global attributes */
static struct iio_attribute ad777x_global_attributes[] = {
	AD777x_CH_ATTR("sampling_frequency", SAMPLING_FREQ_ATTR_ID),
	AD777x_CH_ATTR("sampling_rate_converter_int", SRC_ATTR_ID_INT),
	AD777x_CH_ATTR("sampling_rate_converter_dec", SRC_ATTR_ID_DEC),
	AD777x_CH_ATTR("conversion_mode", CONV_MODE_ATTR_ID),
	AD777x_CH_AVAIL_ATTR("conversion_mode_available", CONV_MODE_ATTR_ID),
	AD777x_CH_ATTR("error_status1", ERROR_FLAG1_ATTR_ID),
	AD777x_CH_ATTR("error_status2", ERROR_FLAG2_ATTR_ID),
	AD777x_CH_ATTR("error_status3", ERROR_FLAG3_ATTR_ID),
	AD777x_CH_ATTR("auxainp_auxainn", AUXAINP_AUXAINN_MUX_ATTR_ID),
	AD777x_CH_ATTR("dvbe_avssx", DVBE_AVSSX_MUX_ATTR_ID),
	AD777x_CH_ATTR("ref1p_ref1n", REF1P_REF1N_MUX_ATTR_ID),
	AD777x_CH_ATTR("ref2p_ref2n", REF2P_REF2N_MUX_ATTR_ID),
	AD777x_CH_ATTR("ref_out_avssx", REF_OUT_AVSSX_MUX_ATTR_ID),
	AD777x_CH_ATTR("vcm_avssx", VCM_AVSSX_MUX_ATTR_ID),
	AD777x_CH_ATTR("areg1cap_avssx", AREG1CAP_AVSSX_MUX_ATTR_ID),
	AD777x_CH_ATTR("areg2cap_avssx", AREG2CAP_AVSSX_MUX_ATTR_ID),
	AD777x_CH_ATTR("dregcap_dgnd", DREGCAP_DGND_MUX_ATTR_ID),
	AD777x_CH_ATTR("avdd1a_avssx", AVDD1A_AVSSX_MUX_ATTR_ID),
	AD777x_CH_ATTR("avdd1b_avssx", AVDD1B_AVSSX_MUX_ATTR_ID),
	AD777x_CH_ATTR("avdd2a_avssx", AVDD2A_AVSSX_MUX_ATTR_ID),
	AD777x_CH_ATTR("avdd2b_avssx", AVDD2B_AVSSX_MUX_ATTR_ID),
	AD777x_CH_ATTR("iovdd_dgnd", IOVDD_DGND_MUX_ATTR_ID),
	AD777x_CH_ATTR("avdd4_avssx", AVDD4_AVSSX_MUX_ATTR_ID),
	AD777x_CH_ATTR("dgnd_avss1a", DGND_AVSS1A_MUX_ATTR_ID),
	AD777x_CH_ATTR("dgnd_avss1b", DGND_AVSS1B_MUX_ATTR_ID),
	AD777x_CH_ATTR("ref1p_avssx", REF1P_AVSSX_MUX_ATTR_ID),
	AD777x_CH_ATTR("ref2p_avssx", REF2P_AVSSX_MUX_ATTR_ID),
	AD777x_CH_ATTR("avssx_avdd4", AVSSX_AVDD4_MUX_ATTR_ID),
#if defined(DEV_AD7771)
	AD777x_CH_AVAIL_ATTR("sinc_5_state", SINC5_STATE_ATTR_ID),
	AD777x_CH_ATTR("sinc_5_state_available", SINC5_STATE_ATTR_ID),
#endif

	END_ATTRIBUTES_ARRAY
};

/* Channel attributes */
static struct iio_attribute ad777x_channel_attributes[] = {
	AD777x_CH_ATTR("raw", RAW_ATTR_ID),
	AD777x_CH_ATTR("scale", SCALE_ATTR_ID),
	AD777x_CH_ATTR("offset", OFFSET_ATTR_ID),
	AD777x_CH_ATTR("gain_comp", GAIN_COMP_ATTR_ID),
	AD777x_CH_ATTR("phase_comp", PHASE_COMP_ATTR_ID),
	AD777x_CH_ATTR("offset_comp", OFFSET_COMP_ATTR_ID),
	AD777x_CH_ATTR("pga", PGA_ATTR_ATTR_ID),
	AD777x_CH_AVAIL_ATTR("pga_available", PGA_ATTR_ATTR_ID),

	END_ATTRIBUTES_ARRAY
};

/* IIOD Channel Configurations for AD777x */
static struct iio_channel ad777x_iio_channels[] = {
	AD777x_CHANNEL("Chn", 0),
#if (AD777x_NUM_CHANNELS > 1) // in case of SD ADC
	AD777x_CHANNEL("Chn", 1),
	AD777x_CHANNEL("Chn", 2),
	AD777x_CHANNEL("Chn", 3),
	AD777x_CHANNEL("Chn", 4),
	AD777x_CHANNEL("Chn", 5),
	AD777x_CHANNEL("Chn", 6),
	AD777x_CHANNEL("Chn", 7),
#endif
};

/* Scale attribute value per channel */
static float attr_scale_val[AD777x_NUM_CHANNELS];

/* Permissible values for the conversion mode */
static char *conv_mode_values[] = {
	"LOW_PWR",
	"HIGH_RES"
};

/* Permissible values for Sinc5 filter */
static char *sinc5_values[] = {
	"ENABLE",
	"DISABLE"
};

/* Permissible values for PGA */
static char *pga_values[] = {
	"GAIN_1",
	"GAIN_2",
	"GAIN_4",
	"GAIN_8"
};

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

/* Flag to denote that sample has been captured */
static bool data_capture_done = false;

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/* Global pointer to copy the private iio_device_data
 * structure from ad777x_trigger_handler() */
struct iio_device_data *ad777x_iio_dev_data;

/* Pointer to the ADC data buffer */
static uint8_t *ad777x_dma_buff;

/* Flag to indicate if data read request is for raw read Operation
 * or data capture operation */
bool data_capture_operation = false;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/


/*!
 * @brief Getter/Setter function for ADC attributes
 * @param device[in]- Pointer to IIO device instance
 * @param buf[in]- IIO input data buffer
 * @param len[in]- Number of input bytes
 * @param channel[in] - ADC input channel
 * @param priv[in] - Attribute private ID
 * @return len in case of success, negative error code otherwise
 */
static int ad777x_get_attribute(void *device, char *buf, uint32_t len,
				const struct iio_ch_info *channel, intptr_t priv)
{
	static uint32_t adc_raw = 0;	// Per channel ADC raw data
	uint8_t error_status = 0;
	int32_t offset = 0;
	int32_t ret;
	uint16_t sar_raw = 0;
	float sar_conv_value;
	uint16_t dec_rate_int;
	uint16_t dec_rate_float;

	switch (priv) {
	case RAW_ATTR_ID:
		ret = ad777x_raw_data_read(device, channel->ch_num, &adc_raw);
		if (ret) {
			return ret;
		}
		return sprintf(buf, "%lu", adc_raw);

	case SCALE_ATTR_ID:
		return sprintf(buf, "%10f", attr_scale_val[channel->ch_num]);

	case OFFSET_ATTR_ID:
#if defined (BIPOLAR)
		if (adc_raw >= ADC_MAX_COUNT_BIPOLAR) {
			offset = -ADC_MAX_COUNT_UNIPOLAR;
		} else {
			offset = 0;
		}
#else
		offset = 0;
#endif
		return sprintf(buf, "%ld", offset);

	case SRC_ATTR_ID_INT:
		ret = ad7779_get_dec_rate(p_ad777x_dev_inst, &dec_rate_int, &dec_rate_float);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%d", dec_rate_int);

	case SRC_ATTR_ID_DEC:
		ret = ad7779_get_dec_rate(p_ad777x_dev_inst, &dec_rate_int, &dec_rate_float);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%d", dec_rate_float);

	case PGA_ATTR_ATTR_ID:
		return snprintf(buf, len, "%s",
				pga_values[p_ad777x_dev_inst->gain[channel->ch_num]]);

	case AUXAINP_AUXAINN_MUX_ATTR_ID:
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_AUXAINP_AUXAINN,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = (sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE;

		return snprintf(buf, len, "%10f", sar_conv_value);

	case DVBE_AVSSX_MUX_ATTR_ID:
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_DVBE_AVSSX, &sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = (sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE;

		return sprintf(buf, "%10fV", sar_conv_value);

	case REF1P_REF1N_MUX_ATTR_ID:
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_REF1P_REF1N,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = (sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE;

		return sprintf(buf, "%10fV", sar_conv_value);

	case REF2P_REF2N_MUX_ATTR_ID:
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_REF2P_REF2N,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = sar_raw * AD777X_SAR_SCALE;

		return sprintf(buf, "%10fV", sar_conv_value);

	case REF_OUT_AVSSX_MUX_ATTR_ID:
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_REF_OUT_AVSSX,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = (sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE;

		return sprintf(buf, "%10fV", sar_conv_value);

	case VCM_AVSSX_MUX_ATTR_ID:
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_VCM_AVSSX, &sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = (sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE;

		return sprintf(buf, "%10fV", sar_conv_value);

	case AREG1CAP_AVSSX_MUX_ATTR_ID:
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_AREG1CAP_AVSSX_ATT,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = ((sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE) *
				 AD777X_SRC_ATT_FACTOR;

		return sprintf(buf, "%10fV", sar_conv_value);

	case AREG2CAP_AVSSX_MUX_ATTR_ID:
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_AREG2CAP_AVSSX_ATT,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = ((sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE) *
				 AD777X_SRC_ATT_FACTOR;

		return sprintf(buf, "%10fV", sar_conv_value);

	case DREGCAP_DGND_MUX_ATTR_ID:
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_DREGCAP_DGND_ATT,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = ((sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE) *
				 AD777X_SRC_ATT_FACTOR;

		return sprintf(buf, "%10fV", sar_conv_value);

	case AVDD1A_AVSSX_MUX_ATTR_ID:
		/* Perform SAR Conversion with selected MUX configuration */
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_AVDD1A_AVSSX_ATT,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = ((sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE) *
				 AD777X_SRC_ATT_FACTOR;

		return sprintf(buf, "%10fV", sar_conv_value);

	case AVDD1B_AVSSX_MUX_ATTR_ID:
		/* Perform SAR Conversion with selected MUX configuration */
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_AVDD1B_AVSSX_ATT,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = ((sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE) *
				 AD777X_SRC_ATT_FACTOR;

		return sprintf(buf, "%10fV", sar_conv_value);

	case AVDD2A_AVSSX_MUX_ATTR_ID:
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_AVDD2A_AVSSX_ATT,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = ((sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE) *
				 AD777X_SRC_ATT_FACTOR;

		return sprintf(buf, "%10fV", sar_conv_value);

	case AVDD2B_AVSSX_MUX_ATTR_ID:
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_AVDD2B_AVSSX_ATT,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = ((sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE) *
				 AD777X_SRC_ATT_FACTOR;

		return sprintf(buf, "%10fV", sar_conv_value);

	case IOVDD_DGND_MUX_ATTR_ID:
		/* Perform SAR Conversion with selected MUX configuration */
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_IOVDD_DGND_ATT,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = ((sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE) *
				 AD777X_SRC_ATT_FACTOR;

		return sprintf(buf, "%10fV", sar_conv_value);

	case AVDD4_AVSSX_MUX_ATTR_ID:
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_AVDD4_AVSSX,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = (sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE;

		return sprintf(buf, "%10fV", sar_conv_value);

	case DGND_AVSS1A_MUX_ATTR_ID:
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_DGND_AVSS1A_ATT,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = ((sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE) *
				 AD777X_SRC_ATT_FACTOR;

		return sprintf(buf, "%10fV", sar_conv_value);

	case DGND_AVSS1B_MUX_ATTR_ID:
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_DGND_AVSS1B_ATT,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = ((sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE) *
				 AD777X_SRC_ATT_FACTOR;

		return sprintf(buf, "%10fV", sar_conv_value);

	case DGND_AVSSX_MUX_ATTR_ID:
		/* Perform SAR Conversion with selected MUX configuration */
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_DGND_AVSSX_ATT,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = ((sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE) *
				 AD777X_SRC_ATT_FACTOR;

		return sprintf(buf, "%10fV", sar_conv_value);

	case AVDD4_AVSSX_ATT_MUX_ATTR_ID:
		/* Perform SAR Conversion with selected MUX configuration */
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_AVDD4_AVSSX_ATT,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = ((sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE) *
				 AD777X_SRC_ATT_FACTOR;

		return sprintf(buf, "%10fV", sar_conv_value);

	case REF1P_AVSSX_MUX_ATTR_ID:
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_REF1P_AVSSX,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = (sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE;

		return sprintf(buf, "%10fV", sar_conv_value);

	case REF2P_AVSSX_MUX_ATTR_ID:
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_REF2P_AVSSX,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = (sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE;

		return sprintf(buf, "%10fV", sar_conv_value);

	case AVSSX_AVDD4_MUX_ATTR_ID:
		ret = ad7779_do_single_sar_conv(p_ad777x_dev_inst, AD7779_AVSSX_AVDD4_ATT,
						&sar_raw);
		if (ret) {
			return ret;
		}
		sar_conv_value = ((sar_raw * AD777X_SAR_SCALE) - AD777X_SAR_REFERENCE) *
				 AD777X_SRC_ATT_FACTOR;

		return sprintf(buf, "%10fV", sar_conv_value);

	case PHASE_COMP_ATTR_ID:
		return snprintf(buf, len, "%d",
				p_ad777x_dev_inst->sync_offset[channel->ch_num]);

	case GAIN_COMP_ATTR_ID:
		return snprintf(buf, len, "%lu", p_ad777x_dev_inst->gain_corr[channel->ch_num]);

	case OFFSET_COMP_ATTR_ID:
		return snprintf(buf, len, "%lu",
				p_ad777x_dev_inst->offset_corr[channel->ch_num]);

	case SAMPLING_FREQ_ATTR_ID:
		return snprintf(buf, len, "%d",
				AD777x_SAMPLING_FREQUENCY / AD777x_NUM_CHANNELS);

	case CONV_MODE_ATTR_ID:
		return snprintf(buf, len, "%s", conv_mode_values[p_ad777x_dev_inst->pwr_mode]);

	case ERROR_FLAG1_ATTR_ID:
		/* Read the error status register 1 */
		ret = ad7779_spi_int_reg_read(p_ad777x_dev_inst, AD7779_REG_STATUS_REG_1,
					      &error_status);
		if (ret) {
			return ret;
		}

		/* Set the Error LED if any bit in the Error Status 1 register is set */
		if (error_status) {
			ret = no_os_gpio_set_value(gpio_error_desc, NO_OS_GPIO_HIGH);
			if (ret) {
				return ret;
			}
		} else {
			/* Reset the error LED if there is no error */
			ret = no_os_gpio_set_value(gpio_error_desc, NO_OS_GPIO_LOW);
			if (ret) {
				return ret;
			}
		}

		return snprintf(buf, len, "0x%x", error_status);

	case ERROR_FLAG2_ATTR_ID:
		/* Read the error status register 2 */
		ret = ad7779_spi_int_reg_read(p_ad777x_dev_inst, AD7779_REG_STATUS_REG_2,
					      &error_status);
		if (ret) {
			return ret;
		}

		/* Set the Error LED if any bit in the Error Status 1 register is set */
		if (error_status) {
			ret = no_os_gpio_set_value(gpio_error_desc, NO_OS_GPIO_HIGH);
			if (ret) {
				return ret;
			}
		} else {
			/* Reset the error LED if there is no error */
			ret = no_os_gpio_set_value(gpio_error_desc, NO_OS_GPIO_LOW);
			if (ret) {
				return ret;
			}
		}

		return snprintf(buf, len, "0x%x", error_status);

	case ERROR_FLAG3_ATTR_ID:
		/* Read the error status register 3 */
		ret = ad7779_spi_int_reg_read(p_ad777x_dev_inst, AD7779_REG_STATUS_REG_3,
					      &error_status);
		if (ret) {
			return ret;
		}

		/* Set the Error LED if any bit in the Error Status 1 register is set */
		if (error_status) {
			ret = no_os_gpio_set_value(gpio_error_desc, NO_OS_GPIO_HIGH);
			if (ret) {
				return ret;
			}
		} else {
			/* Reset the error LED if there is no error */
			ret = no_os_gpio_set_value(gpio_error_desc, NO_OS_GPIO_LOW);
			if (ret) {
				return ret;
			}
		}

		return snprintf(buf, len, "0x%x", error_status);

	case SINC5_STATE_ATTR_ID:
		return snprintf(buf, len, "%s", sinc5_values[p_ad777x_dev_inst->sinc5_state]);

	default:
		break;
	}

	return 0;

}

static int ad777x_set_attribute(void *device, char *buf, uint32_t len,
				const struct iio_ch_info *channel, intptr_t priv)
{
	ad7779_pwr_mode conv_mode_id;
	ad7779_sar_mux sar_mux_id;
	ad7779_state sinc5_id;
	ad7779_gain gain_id;
	int32_t ret;
	static uint16_t src_int_value;
	static uint16_t src_dec_value;

	switch (priv) {
	case PGA_ATTR_ATTR_ID:
		for (gain_id = AD7779_GAIN_1; gain_id <= AD7779_GAIN_8; gain_id++) {
			if (!strncmp(buf, pga_values[gain_id], strlen(buf))) {
				break;
			}
		}

		/* Set the selected gain value */
		ret = ad7779_set_gain(device, channel->ch_num, gain_id);
		if (ret) {
			return ret;
		}

		/* Update the scale factors for conversion */
		ad777x_update_scale_factor();
		break;


	case PHASE_COMP_ATTR_ID:
		ret = ad7779_set_sync_offset(p_ad777x_dev_inst, channel->ch_num,
					     no_os_str_to_uint32(buf));
		if (ret) {
			return ret;
		}
		break;

	case GAIN_COMP_ATTR_ID:
		ret = ad7779_set_gain_corr(p_ad777x_dev_inst, channel->ch_num,
					   no_os_str_to_uint32(buf));
		if (ret) {
			return ret;
		}
		break;

	case OFFSET_COMP_ATTR_ID:
		ret = ad7779_set_offset_corr(p_ad777x_dev_inst, channel->ch_num,
					     no_os_str_to_uint32(buf));
		if (ret) {
			return ret;
		}
		break;

	case CONV_MODE_ATTR_ID:
		for (conv_mode_id = AD7779_LOW_PWR; conv_mode_id <= AD7779_HIGH_RES;
		     conv_mode_id++) {
			if (!strncmp(buf, conv_mode_values[conv_mode_id], strlen(buf))) {
				break;
			}
		}

		/* Set the selected conversion mode */
		ret = ad7779_set_power_mode(p_ad777x_dev_inst, conv_mode_id);
		if (ret) {
			return ret;
		}
		break;

	case SINC5_STATE_ATTR_ID:
		for (sinc5_id = AD7779_ENABLE; sinc5_id <= AD7779_DISABLE; sinc5_id++) {
			if (!strncmp(buf, sinc5_values[sinc5_id], strlen(buf))) {
				break;
			}
		}

		/* Set the selected sinc5 state */
		ret = ad7771_set_sinc5_filter_state(p_ad777x_dev_inst, sinc5_id);
		if (ret) {
			return ret;
		}
		break;

	case SRC_ATTR_ID_INT:
		/* Get the SRC integer and Decimal value set */
		ret = ad7779_get_dec_rate(p_ad777x_dev_inst, &src_int_value, &src_dec_value);
		if (ret) {
			return ret;
		}

		/* Update SRC Integer Value, retain the decimal value */
		ret = ad7779_set_dec_rate(p_ad777x_dev_inst, no_os_str_to_uint32(buf),
					  src_dec_value);
		if (ret) {
			return ret;
		}

		/* Write to SRC Update register to reflect the values */
		ret = ad7779_spi_int_reg_write(p_ad777x_dev_inst, AD7779_REG_SRC_UPDATE,
					       SRC_LOAD_UPDATE_HIGH);
		if (ret) {
			return ret;
		}
		ret = ad7779_spi_int_reg_write(p_ad777x_dev_inst, AD7779_REG_SRC_UPDATE,
					       SRC_LOAD_UPDATE_LOW);
		if (ret) {
			return ret;
		}

		break;

	case SRC_ATTR_ID_DEC:
		/* Get the SRC integer and Decimal value set */
		ret = ad7779_get_dec_rate(p_ad777x_dev_inst, &src_int_value, &src_dec_value);
		if (ret) {
			return ret;
		}

		/* Update SRC decimal value, retain the integer value */
		ret = ad7779_set_dec_rate(p_ad777x_dev_inst, src_int_value,
					  no_os_str_to_uint32(buf));
		if (ret) {
			return ret;
		}

		/* Write to SRC Update register to reflect the values */
		ret = ad7779_spi_int_reg_write(p_ad777x_dev_inst, AD7779_REG_SRC_UPDATE,
					       SRC_LOAD_UPDATE_HIGH);
		if (ret) {
			return ret;
		}
		ret = ad7779_spi_int_reg_write(p_ad777x_dev_inst, AD7779_REG_SRC_UPDATE,
					       SRC_LOAD_UPDATE_LOW);
		if (ret) {
			return ret;
		}

		break;

	/* Read-only Attributes */
	case RAW_ATTR_ID:
	case SCALE_ATTR_ID:
	case OFFSET_ATTR_ID:
	case SAMPLING_FREQ_ATTR_ID:
	default:
		break;
	}

	return 0;
}

/*!
 * @brief	Attribute available getter function
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of SUCCESS, negative error code otherwise
 */
static int ad777x_get_avail_attribute(void *device, char *buf, uint32_t len,
				      const struct iio_ch_info *channel, intptr_t priv)
{
	switch (priv) {
	case PGA_ATTR_ATTR_ID:
		return snprintf(buf, len, "%s %s %s %s",
				pga_values[0],
				pga_values[1],
				pga_values[2],
				pga_values[3]);

	case CONV_MODE_ATTR_ID:
		return snprintf(buf, len,
				"%s %s",
				conv_mode_values[0],
				conv_mode_values[1]);

	case SINC5_STATE_ATTR_ID:
		return snprintf(buf, len,
				"%s %s",
				sinc5_values[0],
				sinc5_values[1]);

	default:
		break;
	}

	return len;
}

static int ad777x_set_avail_attribute(void *device, char *buf, uint32_t len,
				      const struct iio_ch_info *channel, intptr_t priv)
{
	switch (priv) {
	case PGA_ATTR_ATTR_ID:
	case CONV_MODE_ATTR_ID:
	case SINC5_STATE_ATTR_ID:
	default:
		break;
	}

	return len;
}

/*!
 * @brief Update scale factor for adc data to voltage conversion
 * @return None
 */
static void ad777x_update_scale_factor(void)
{
	uint8_t gain_values[] = { 1, 2, 4, 8 };
	uint8_t chan_id;
	uint8_t pga;
	float vref;

	/* Update Reference */
	if (p_ad777x_dev_inst->ref_type == AD7779_INT_REF) {
		vref = AD777x_INTERNAL_REF_VAL;
	} else {
		vref = AD777x_EXTERNAL_REF_VAL;
	}

	for (chan_id = 0; chan_id < AD777x_NUM_CHANNELS; chan_id++) {
		/* Update PGA */
		pga = gain_values[p_ad777x_dev_inst->gain[chan_id]];

		/* Update Scale factor */
#if defined (BIPOLAR)
		attr_scale_val[chan_id] = (vref / (ADC_MAX_COUNT_BIPOLAR * pga)) * 1000;

#else
		attr_scale_val[chan_id] = (vref / (ADC_MAX_COUNT_UNIPOLAR * pga)) * 1000;
#endif
	}
}

/*!
 * @brief Read the device register value
 * @param dev[in]- Pointer to IIO device instance
 * @param reg[in]- Register address to read from
 * @param readval[out]- Pointer to variable to read data into
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad777x_debug_reg_read(void *dev, uint32_t reg, uint32_t *readval)
{
	int32_t ret;

	if (!dev || !readval || (reg > AD7779_REG_SRC_UPDATE)) {
		return -EINVAL;
	}

	/* Set the SPI operating mode to register access mode */
	ret = ad7779_set_spi_op_mode(p_ad777x_dev_inst, AD7779_INT_REG);
	if (ret) {
		return ret;
	}

	ret = ad7779_spi_int_reg_read(p_ad777x_dev_inst, reg, readval);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief Write into the device register
 * @param dev[in] - Pointer to IIO device instance
 * @param reg[in] - Register address to write into
 * @param writeval[in] - Register value to write
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad777x_debug_reg_write(void *dev, uint32_t reg,
				      uint32_t writeval)
{
	int32_t ret;

	if (!dev || (reg > AD7779_REG_SRC_UPDATE)) {
		return -EINVAL;
	}

	/* Set the SPI operating mode to register access mode */
	ret = ad7779_set_spi_op_mode(p_ad777x_dev_inst, AD7779_INT_REG);
	if (ret) {
		return ret;
	}

	ret = ad7779_spi_int_reg_write(p_ad777x_dev_inst, reg, writeval);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Prepare for data transfer
 * @param dev[in] - IIO device instance
 * @param ch_mask[in] - Channels select mask
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad777x_prepare_transfer(void *dev, uint32_t ch_mask)
{
	uint32_t mask = 0x1;
	uint8_t index = 0;
	uint8_t chn;
	int32_t ret;

	buf_size_updated = false;
	data_capture_operation = true;

#if (ADC_TYPE == SD_ADC)
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE) && (INTERFACE_MODE == SPI_MODE)
	ret = ad7779_set_spi_op_mode(p_ad777x_dev_inst, AD7779_INT_REG);
	if (ret) {
		return ret;
	}

	ret = ad7779_set_spi_op_mode(p_ad777x_dev_inst, AD7779_SD_CONV);
	if (ret) {
		return ret;
	}
#endif

	/* The UART interrupt needs to be prioritized over the GPIO (end of conversion) interrupt.
	 * If not, the GPIO interrupt may occur during the period where there is a UART read happening
	 * for the READBUF command. If UART interrupts are not prioritized, then it would lead to missing of
	 * characters in the IIO command sent from the client. */
	ad777x_configure_intr_priority();

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = iio_trig_enable(ad777x_hw_trig_desc);
	if (ret) {
		return ret;
	}
#if (INTERFACE_MODE == TDM_MODE)
	ret = start_tdm_dma_to_cb_transfer(ad777x_tdm_desc, ad777x_iio_dev_data,
					   TDM_DMA_READ_SIZE, BYTES_PER_SAMPLE, TDM_DMA_READ_SIZE);
	if (ret) {
		return ret;
	}
#endif
#else // BURST_DATA_CAPTURE
#if (INTERFACE_MODE == SPI_MODE)
	/* Enable interrupts to detect EOC */
	ret = no_os_irq_enable(trigger_irq_desc, IRQ_INT_ID);
	if (ret) {
		return ret;
	}
#endif // SPI_MODE
#endif
#else // SAR_ADC
	/* Set the SPI line to read back registers */
	ret = ad7779_set_spi_op_mode(p_ad777x_dev_inst, AD7779_INT_REG);
	if (ret) {
		return ret;
	}
	ret = ad7779_set_sar_cfg(dev, AD7779_ENABLE, SAR_MUX_CONF);
	if (ret) {
		return ret;
	}

	/* Delay to settle MUX input channels */
	no_os_mdelay(10);

	ret = ad7779_set_spi_op_mode(dev, AD7779_SAR_CONV);
	if (ret) {
		return ret;
	}
#endif // ADC_TYPE

	return 0;
}

/**
 * @brief Terminate data transfer
 * @param dev[in] - IIO device instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad777x_end_transfer(void *dev)
{
	int32_t ret;

#if (INTERFACE_MODE == SPI_MODE)
#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	ret = no_os_irq_disable(trigger_irq_desc, IRQ_INT_ID);
	if (ret) {
		return ret;
	}
#else
	ret = iio_trig_disable(ad777x_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif
	/* Set the SPI operating mode back to register access mode */
	ret = ad7779_set_spi_op_mode(p_ad777x_dev_inst, AD7779_INT_REG);
	if (ret) {
		return ret;
	}
#else // TDM_MODE
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Stop TDM DMA data capture */
	ret = no_os_tdm_stop(ad777x_tdm_desc);
	if (ret) {
		return ret;
	}
#endif
#endif // TDM_MODE

	data_capture_operation = false;

	return 0;
}

/**
 * @brief Read SAR ADC data in burst mode via SPI
 * @param nb_of_samples[in] - Number of samples requested by IIO
 * @param iio_dev_data[in] - IIO Device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad777x_read_burst_data_sar_spi(uint32_t nb_of_samples,
		struct iio_device_data *iio_dev_data)
{
	uint16_t sar_adc_code;
	uint32_t sample_index = 0;
	int32_t ret;

#if (ADC_TYPE == SAR_ADC)
	while (sample_index < nb_of_samples) {
		ret = ad7779_sar_data_read(p_ad777x_dev_inst, SAR_MUX_CONF, &sar_adc_code);
		if (ret) {
			return ret;
		}

		ret = no_os_cb_write(iio_dev_data->buffer->buf, &sar_adc_code,
				     BYTES_PER_SAMPLE * AD777x_NUM_CHANNELS);
		if (ret) {
			return ret;
		}
		sample_index+= AD777x_NUM_CHANNELS;
	}
#endif

	return 0;
}

/**
 * @brief Read data in burst mode via SPI
 * @param nb_of_samples[in] - Number of samples requested by IIO
 * @param iio_dev_data[in] - IIO Device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad777x_read_burst_data_spi(uint32_t nb_of_samples,
		struct iio_device_data *iio_dev_data)
{
	uint32_t adc_raw_buff[AD777x_NUM_CHANNELS] = { 0x0 };
	uint32_t timeout;
	uint32_t sample_index = 0;
	int32_t ret;

	/* Start conversion by setting the ADC to SD conversion mode */
	ret = ad7779_set_spi_op_mode(p_ad777x_dev_inst, AD7779_SD_CONV);
	if (ret) {
		return ret;
	}

	while (sample_index < nb_of_samples) {
		timeout = AD777x_CONV_TIMEOUT;

		/* Check for data capture completion */
		while (!data_capture_done) {
			timeout--;
		}
		if (timeout == 0) {
			return -ETIMEDOUT;
		}
		data_capture_done = false;

		ret = ad777x_read_all_channels(p_ad777x_dev_inst, adc_raw_buff);
		if (ret) {
			return ret;
		}

		ret = no_os_cb_write(iio_dev_data->buffer->buf, adc_raw_buff,
				     BYTES_PER_SAMPLE * AD777x_NUM_CHANNELS);
		if (ret) {
			return ret;
		}
		sample_index+= AD777x_NUM_CHANNELS;
	}

	/* Stop conversion by setting ADC to INT reg mode */
	ret = ad7779_set_spi_op_mode(p_ad777x_dev_inst, AD7779_INT_REG);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Read data in burst mode via TDM-DMA
 * @param nb_of_bytes[in] - Number of bytes requested
 * @param iio_dev_data[in] - IIO Device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad777x_read_burst_data_tdm(uint32_t nb_of_bytes,
		struct iio_device_data *iio_dev_data)
{
	uint32_t ad777x_buff_available_size;
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
						   &ad777x_dma_buff, &ad777x_buff_available_size);
		if (ret) {
			return ret;
		}

		/* Trigger TDM-DMA read to capture data into buffer in the background */
		ret = no_os_tdm_read(ad777x_tdm_desc, ad777x_dma_buff,
				     nb_of_bytes/BYTES_PER_SAMPLE);
		if (ret) {
			return ret;
		}

		/* Wait until DMA buffer is full */
		timeout = AD777x_CONV_TIMEOUT;
		while (!dma_buffer_full) {
			timeout--;
		}

		/* Update the data buffer pointer to a new index post DMA write operation */
		ret = no_os_cb_end_async_write(iio_dev_data->buffer->buf);
		if (ret) {
			return ret;
		}

		ret = no_os_tdm_stop(ad777x_tdm_desc);
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
 * @brief Read buffer data corresponding to AD777x ADC IIO device
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad777x_submit_buffer(struct iio_device_data *iio_dev_data)
{
	uint32_t nb_of_samples;
	int32_t ret;
	uint32_t timeout = AD777x_CONV_TIMEOUT;
	nb_of_samples = iio_dev_data->buffer->size / BYTES_PER_SAMPLE;

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
#if (ADC_TYPE == SD_ADC)
#if (INTERFACE_MODE == SPI_MODE)
	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	ret = ad777x_read_burst_data_spi(nb_of_samples, iio_dev_data);
	if (ret) {
		return ret;
	}
#else
	ret = ad777x_read_burst_data_tdm(iio_dev_data->buffer->size, iio_dev_data);
	if (ret) {
		return ret;
	}
#endif // INTERFACE_MODE
#else // SAR_ADC
	ret = ad777x_read_burst_data_sar_spi(nb_of_samples, iio_dev_data);
	if (ret) {
		return ret;
	}
#endif // ADC_TYPE
#endif // DATA_CAPTURE_MODE

	return 0;
}

/**
 * @brief Push data into IIO buffer when trigger handler IRQ is invoked
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 * @note This function is utilized only in case of continuous capture
 * in SPI Mode. In TDM Mode, it is utilized only to copy the private data
 * member iio_dev_data
 */
int32_t ad777x_trigger_handler(struct iio_device_data *iio_dev_data)
{
	uint32_t adc_raw[AD777x_NUM_CHANNELS] = { 0x0 };
	int32_t ret;

#if (INTERFACE_MODE == TDM_MODE)
	/* Disable IIO trigger after first occurrence to the trigger handler.
	 * The handler is enabled only once to point the private iio_dev_data to a
	 * global ad777x_iio_dev_data structure variable for future IIO CB operations */
	ret = iio_trig_disable(ad777x_hw_trig_desc);
	if (ret) {
		return ret;
	}

	ad777x_iio_dev_data = iio_dev_data;
#else
	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
						   iio_dev_data->buffer->bytes_per_scan)) * iio_dev_data->buffer->bytes_per_scan;
		buf_size_updated = true;
	}

	ret = ad777x_read_all_channels(p_ad777x_dev_inst, adc_raw);
	if (ret) {
		return ret;
	}

	ret =  no_os_cb_write(iio_dev_data->buffer->buf, adc_raw,
			      BYTES_PER_SAMPLE*AD777x_NUM_CHANNELS);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/*!
 * @brief Interrupt Service Routine to monitor end of conversion event.
 * @param ctx[in] - Callback context (unused)
 * @return none
 * @note Callback registered for the the DRDY interrupt to indicate
 * end of conversion in case of burst data capturing with SPI operation.
 */
void data_capture_callback(void *ctx)
{
	data_capture_done = true;
}

/**
 * @brief	Init for reading/writing and parameterization
 * 			of a AD777x IIO device
 * @param 	desc[in,out] - IIO device descriptor
 * @return	0 in case of success or negative value otherwise
 */
int32_t iio_ad777x_init(struct iio_device **desc)
{
	struct iio_device *iio_ad777x_inst;

	iio_ad777x_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad777x_inst) {
		return -EINVAL;
	}

	iio_ad777x_inst->num_ch = NO_OS_ARRAY_SIZE(ad777x_iio_channels);
	iio_ad777x_inst->channels = ad777x_iio_channels;
	iio_ad777x_inst->attributes = ad777x_global_attributes;
	iio_ad777x_inst->debug_reg_read = ad777x_debug_reg_read;
	iio_ad777x_inst->debug_reg_write = ad777x_debug_reg_write;
	iio_ad777x_inst->pre_enable = ad777x_prepare_transfer;
	iio_ad777x_inst->post_disable = ad777x_end_transfer;
	iio_ad777x_inst->submit = ad777x_submit_buffer;
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	iio_ad777x_inst->trigger_handler = ad777x_trigger_handler;
#endif
	*desc = iio_ad777x_inst;

	/* Update the scale factor for all channels */
	ad777x_update_scale_factor();

	return 0;
}

/**
 * @brief Initialization of AD777x IIO hardware trigger specific parameters
 * @param desc[in,out] - IIO hardware trigger descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad777x_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	struct iio_hw_trig_init_param ad777x_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;
	int32_t ret;

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad777x_hw_trig_init_params.irq_id = IRQ_INT_ID;
	ad777x_hw_trig_init_params.name = AD777X_IIO_TRIGGER_NAME;
	ad777x_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad777x_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad777x_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad777x_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad777x_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad777x_hw_trig_init_params.iio_desc = p_ad777x_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad777x_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/**
 * @brief	Initialize the IIO interface for AD777x IIO device
 * @return	none
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad777x_iio_initialize(void)
{
	int32_t init_status;

	/* IIO device descriptor */
	struct iio_device *p_iio_ad777x_dev;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	static struct iio_trigger ad777x_iio_trig_desc = {
		.is_synchronous = true,
	};

	/* IIO trigger init parameters */
	struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad777x_iio_trig_desc,
		.name = AD777X_IIO_TRIGGER_NAME,
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

	/* Initialize the AD777x device */
	init_status = ad7779_init(&p_ad777x_dev_inst, ad777x_init_params);
	if (init_status) {
		return init_status;
	}

#if (INTERFACE_MODE == TDM_MODE)
	/* Delay to ensure that all registers are loaded with the
	 * configurations set via the ad7779_init() */
	no_os_mdelay(100);

	/* Enable one DOUT line (DOUT0) to capture all channels' data
	 * in case of TDM Mode as TDM uses a single line to read data */
	init_status = ad777x_enable_single_dout(p_ad777x_dev_inst);
	if (init_status) {
		return init_status;
	}
#endif
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
		/* Initialize the AD777x IIO application interface */
		init_status = iio_ad777x_init(&p_iio_ad777x_dev);
		if (init_status) {
			return init_status;
		}

		iio_device_init_params[0].name = ACTIVE_DEVICE;
		iio_device_init_params[0].raw_buf = adc_data_buffer;
		iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;

		iio_device_init_params[0].dev = p_ad777x_dev_inst;
		iio_device_init_params[0].dev_descriptor = p_iio_ad777x_dev;

		iio_init_params.nb_devs++;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		iio_init_params.nb_trigs++;
#endif
	}

	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_iio_com_desc;
	iio_init_params.devs = iio_device_init_params;
	init_status = iio_init(&p_ad777x_iio_desc, &iio_init_params);
	if (init_status) {
		return init_status;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Initialize the AD777x IIO trigger specific parameters */
	init_status = ad777x_iio_trigger_param_init(&ad777x_hw_trig_desc);
	if (init_status) {
		return init_status;
	}
#endif

	return 0;
}


/**
 * @brief Run the AD777x IIO event handler
 * @return None
 */
void ad777x_iio_event_handler(void)
{
	(void)iio_step(p_ad777x_iio_desc);
}
