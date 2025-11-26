/***************************************************************************//**
 *   @file    ad5710r_iio.c
 *   @brief   Implementation of AD5710R IIO Application Interface
 *   @details This module acts as an interface for AD5710R IIO device
********************************************************************************
 * Copyright (c) 2024-25 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <string.h>

#include "app_config.h"
#include "ad5710r.h"
#include "ad5710r_iio.h"
#include "ad5710r_user_config.h"
#include "ad5710r_regs.h"
#include "ad5710r_support.h"
#include "no_os_error.h"
#include "no_os_util.h"
#include "no_os_gpio.h"
#include "no_os_pwm.h"
#include "no_os_alloc.h"
#include "no_os_delay.h"
#include "common.h"
#include "iio_trigger.h"
#include "version.h"

/******** Forward declaration of getter/setter functions ********/
static int ad5710r_iio_attr_get(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv);

static int ad5710r_iio_attr_set(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv);

static int ad5710r_iio_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int ad5710r_iio_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/
#define AD5710R_CHN_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = ad5710r_iio_attr_get,\
		.store = ad5710r_iio_attr_set\
}

#define AD5710R_CHN_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = ad5710r_iio_attr_available_get,\
	.store = ad5710r_iio_attr_available_set\
}

#define AD5710R_CH(_name, _idx, _type) {\
	.name = _name, \
	.ch_type = _type,\
	.ch_out = true,\
	.indexed = true,\
	.channel = _idx,\
	.scan_index = _idx,\
	.scan_type = &ad5710r_iio_scan_type,\
	.attributes = ad5710r_iio_ch_attributes\
}

/* Bytes per sample (for DAC resolution of 16-bits) */
#define	BYTES_PER_SAMPLE	sizeof(uint16_t)

/* Bytes per sample with address (for DAC resolution of 16-bits) */
#define BYTES_PER_SAMPLE_WITH_ADDRESS	2 * BYTES_PER_SAMPLE

/* Number of data storage bits (needed for IIO client to send buffer of data) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

/* DAC data buffer size */
#if defined(USE_SDRAM)
#define dac_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(65536)		// 64kbytes
static int8_t dac_data_buffer[DATA_BUFFER_SIZE];
#endif

#define	BYTE_SIZE	(uint32_t)8
#define	BYTE_MASK	(uint32_t)0xff

/* Number of IIO devices */
#define NUM_OF_IIO_DEVICES	1

/* IIO trigger name */
#define AD5710R_IIO_TRIGGER_NAME		"ad5710r_iio_trigger"

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
/* Pointer to the struct representing the AD5710R IIO device */
struct ad5710r_desc *ad5710r_dev_desc = NULL;

/* IIO interface descriptor */
static struct iio_desc *ad5710r_iio_desc;

/* AD5710R IIO device descriptor */
struct iio_device *ad5710r_iio_dev;

#if (INTERFACE_MODE == SPI_INTERRUPT)
/* Descriptor to hold iio trigger details */
static struct iio_trigger ad5710r_iio_trig_desc = {
	.is_synchronous = true,
	.enable = NULL,
	.disable = NULL
};

/* AD5710R IIO hw trigger descriptor */
static struct iio_hw_trig *ad5710r_hw_trig_desc;
#endif

/* Active channel sequence */
static volatile uint8_t ad5710r_active_chns[DAC_CHANNELS];

/* Number of active channels */
static uint8_t num_of_active_channels = 0;

/* AD5710R attribute unique IDs */
enum ad5710r_attribute_ids {
	DAC_INPUT,
	DAC_RAW,
	DAC_OFFSET,
	DAC_SCALE,
	DAC_CHN_OP_SELECT,
	DAC_CHN_SW_LDAC_EN,
	DAC_CHN_HW_LDAC_EN,
	DAC_CHN_MODE_SELECT,

	DAC_VREF_SELECT,
	DAC_RANGE,
	DAC_MUX_OUT,
	DAC_SW_LDAC,
	DAC_HW_LDAC,
	DAC_ALL_CH_OP_MODE,
	DAC_MULTI_INPUT_CH,
	DAC_MULTI_DAC_CH,
	DAC_SAMPLING_FREQUENCY,
	DAC_STREAMING_TECHNIQUE
};

/* IIOD channels configurations */
struct scan_type ad5710r_iio_scan_type = {
	.sign = 'u',
	.realbits = DAC_RESOLUTION,
	.storagebits = DAC_RESOLUTION,
	.shift = 0,
#if (INTERFACE_MODE == SPI_DMA)
	.is_big_endian = false
#else
	.is_big_endian = false
#endif
};

/* Channel operating mode value string representation */
static char *ad5710r_operating_mode_str[] = {
	"normal_operation",
	"1kOhm_to_gnd",
	"7k7Ohm_to_gnd",
	"32kOhm_to_gnd"
};

/* Vref value string representation */
static char *ad5710r_vref_str[] = {
	"external_ref",
	"internal_ref"
};

/* Range select value string representation */
static char *ad5710r_range_select_str[] = {
	"0_to_VREF",
	"0_to_2VREF"
};

/* LDAC bit enable disable options */
static char *ad5710r_ldac_bit_en_str[] = {
	"disable",
	"enable"
};

/* LDAC trigger string representation */
static char *ad5710r_ldac_trig_str[] = {
	"ldac_trigger"
};

/* Register data streaming options */
static char* ad5710r_streaming_select_str[] = {
	"single_instruction_mode",
	"streaming_mode"
};

/* MUX out select value string representation */
static char *ad5710r_mux_out_sel[] = {
	"powered_down",
	"VOUT0_SENSE",
	"IOUT0_SENSE",
	"PVDD0_DAC0",
	"VOUT1_SENSE",
	"IOUT1_SENSE",
	"PVDD0_DAC1",
	"VOUT2_SENSE",
	"IOUT2_SENSE",
	"PVDD0_DAC2",
	"VOUT3_SENSE",
	"IOUT3_SENSE",
	"PVDD0_DAC3",
	"VOUT4_SENSE",
	"IOUT4_SENSE",
	"PVDD1_DAC4",
	"VOUT5_SENSE",
	"IOUT5_SENSE",
	"PVDD1_DAC5",
	"VOUT6_SENSE",
	"IOUT6_SENSE",
	"PVDD1_DAC6",
	"VOUT7_SENSE",
	"IOUT7_SENSE",
	"PVDD1_DAC7",
	"Internal_Die_Temperature",
	"tie_to_AGND_internally"
};

static char *ad5710r_chan_mode_sel[] = {
	"IMODE",
	"VMODE"
};

/* AD5710R channel specific attributes list */
static struct iio_attribute ad5710r_iio_ch_attributes[] = {
	AD5710R_CHN_ATTR("input_register", DAC_INPUT),
	AD5710R_CHN_ATTR("raw", DAC_RAW),
	AD5710R_CHN_ATTR("scale", DAC_SCALE),
	AD5710R_CHN_ATTR("offset", DAC_OFFSET),
	AD5710R_CHN_ATTR("operating_mode", DAC_CHN_OP_SELECT),
	AD5710R_CHN_AVAIL_ATTR("operating_mode_available", DAC_CHN_OP_SELECT),
	AD5710R_CHN_ATTR("sw_ldac_enable", DAC_CHN_SW_LDAC_EN),
	AD5710R_CHN_AVAIL_ATTR("sw_ldac_enable_available", DAC_CHN_SW_LDAC_EN),
	AD5710R_CHN_ATTR("hw_ldac_enable", DAC_CHN_HW_LDAC_EN),
	AD5710R_CHN_AVAIL_ATTR("hw_ldac_enable_available", DAC_CHN_HW_LDAC_EN),
	AD5710R_CHN_ATTR("ch_mode", DAC_CHN_MODE_SELECT),
	AD5710R_CHN_AVAIL_ATTR("ch_mode_available", DAC_CHN_MODE_SELECT),
	END_ATTRIBUTES_ARRAY,
};

/* AD5710R device (global) specific attributes list */
static struct iio_attribute ad5710r_iio_global_attributes[] = {
	AD5710R_CHN_ATTR("reference_select", DAC_VREF_SELECT),
	AD5710R_CHN_AVAIL_ATTR("reference_select_available", DAC_VREF_SELECT),
	AD5710R_CHN_ATTR("range", DAC_RANGE),
	AD5710R_CHN_AVAIL_ATTR("range_available", DAC_RANGE),
	AD5710R_CHN_ATTR("mux_out_select", DAC_MUX_OUT),
	AD5710R_CHN_AVAIL_ATTR("mux_out_select_available", DAC_MUX_OUT),
	AD5710R_CHN_ATTR("all_ch_operating_mode", DAC_ALL_CH_OP_MODE),
	AD5710R_CHN_AVAIL_ATTR("all_ch_operating_mode_available", DAC_ALL_CH_OP_MODE),
	AD5710R_CHN_ATTR("all_ch_input_registers", DAC_MULTI_INPUT_CH),
	AD5710R_CHN_ATTR("all_ch_raw", DAC_MULTI_DAC_CH),
	AD5710R_CHN_ATTR("sampling_frequency", DAC_SAMPLING_FREQUENCY),
	AD5710R_CHN_ATTR("data_streaming_mode", DAC_STREAMING_TECHNIQUE),
	AD5710R_CHN_AVAIL_ATTR("data_streaming_mode_available", DAC_STREAMING_TECHNIQUE),
	AD5710R_CHN_ATTR("sw_ldac_trigger", DAC_SW_LDAC),
	AD5710R_CHN_AVAIL_ATTR("sw_ldac_trigger_available", DAC_SW_LDAC),
	AD5710R_CHN_ATTR("hw_ldac_trigger", DAC_HW_LDAC),
	AD5710R_CHN_AVAIL_ATTR("hw_ldac_trigger_available", DAC_HW_LDAC),
	END_ATTRIBUTES_ARRAY,
};

/* IIO channels info */
static struct iio_channel ad5710r_iio_channels[] = {
	AD5710R_CH("Ch0", 0, IIO_VOLTAGE),
	AD5710R_CH("Ch1", 1, IIO_VOLTAGE),
	AD5710R_CH("Ch2", 2, IIO_VOLTAGE),
	AD5710R_CH("Ch3", 3, IIO_VOLTAGE),
	AD5710R_CH("Ch4", 4, IIO_VOLTAGE),
	AD5710R_CH("Ch5", 5, IIO_VOLTAGE),
	AD5710R_CH("Ch6", 6, IIO_VOLTAGE),
	AD5710R_CH("Ch7", 7, IIO_VOLTAGE),
};

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

/* Variable to store all channel operating modes */
static enum ad5710r_operating_mode all_chn_op_mode =
	AD5710R_CH_OPERATING_MODE_3;

/* Sampling rate/frequency value */
static uint32_t sampling_rate = MAX_SAMPLING_RATE;

/* Scale attribute value */
static float attr_scale_val;

/* Offset attribute value */
static int16_t attr_offset_val;

/* Variable to store streaming option */
enum reg_access_mode streaming_option = SINGLE_INSTRUCTION_MODE;

/* Global variable to store number of samples */
uint32_t num_of_samples;

/* Variable to store number of channels */
static uint16_t num_of_mux_sels = AD5710R_NUM_MUX_OUT_SELECTS;
static uint8_t num_of_chns = AD5710R_NUM_CH;
static uint8_t num_of_op_modes = AD5710R_MAX_CHANNEL_OP_MODE_0;

/* Array with channel addresses (2 bytes per channel) */
static uint16_t ch_addr_array[DAC_CHANNELS];

/* Pointer to device register map array */
static const uint32_t *ad5710r_reg = NULL;

#if (INTERFACE_MODE == SPI_DMA)
/* Dummy receive buffer for SPI DMA transfers */
static uint8_t local_buff;

/* Flag to indicate if SPI DMA enabled */
static bool spi_dma_enabled = false;

/* STM32 SPI Init params */
struct stm32_spi_init_param* spi_init_param;

/* Global variable for iio buffer */
uint8_t* global_iio_buff;
#endif

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/
/**
 * @brief	Get the IIO scale
 * @param	scale[in,out] - IIO scale value
 * @return	0 in case of success, negative error code otherwise
 */
static int ad5710r_get_scale(float *scale)
{
	if (!scale) {
		return -EINVAL;
	}

	switch (ad5710r_dev_desc->range) {
	case AD5710R_CH_OUTPUT_RANGE_0_VREF:
		*scale = (DAC_REF_VOLTAGE / DAC_MAX_COUNT) * 1000;
		break;

	case AD5710R_CH_OUTPUT_RANGE_0_2VREF:
		*scale = ((DAC_REF_VOLTAGE * 2) / DAC_MAX_COUNT) * 1000;
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief	Get the sampling rate supported by MCU platform
 * @param 	sampling_rate[in,out] - sampling rate value
 * @return	0 in case of success, negative error code otherwise
 */
static int ad5710r_get_sampling_rate(uint32_t *sampling_rate)
{
	uint32_t pwm_period_ns;
	int ret;

	if (!sampling_rate) {
		return -EINVAL;
	}

	ret = no_os_pwm_get_period(pwm_desc, &pwm_period_ns);
	if (ret) {
		return ret;
	}

	/* Convert period (nsec) to frequency (in hertz) */
	*sampling_rate = CONV_TRIGGER_PERIOD_NSEC(pwm_period_ns);

	return 0;
}

/**
 * @brief	Set the sampling rate supported by MCU platform
 * @param 	sampling_rate[in] - sampling rate value
 * @return	0 in case of success, negative error code otherwise
 */
static int ad5710r_set_sampling_rate(uint32_t sampling_rate)
{
	int ret;

	if (!sampling_rate) {
		return -EINVAL;
	}

	if (sampling_rate > MAX_SAMPLING_RATE) {
		sampling_rate = MAX_SAMPLING_RATE;
	}

#if (INTERFACE_MODE == SPI_DMA)
	pwm_init_params.period_ns = CONV_TRIGGER_PERIOD_NSEC(sampling_rate);

	ret = init_pwm();
	if (ret) {
		return ret;
	}
#else //SPI INTERRUPT
	ret = no_os_pwm_set_period(pwm_desc, CONV_TRIGGER_PERIOD_NSEC(sampling_rate));
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_set_duty_cycle(pwm_desc,
				       CONV_TRIGGER_DUTY_CYCLE_NSEC(sampling_rate, LDAC_PWM_DUTY_CYCLE_PERCENT));
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/*!
 * @brief	Getter function for AD5710R attributes
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of expected bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise
 */
static int ad5710r_iio_attr_get(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv)
{
	uint16_t val;
	int ret;

	switch (priv) {
	case DAC_RAW:
		ret = ad5710r_reg_read(ad5710r_dev_desc,
				       AD5710R_REG_ADDR_DAC_CHN(channel->ch_num),
				       &val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%d", val);

	case DAC_INPUT:
		ret = ad5710r_reg_read(ad5710r_dev_desc,
				       AD5710R_REG_ADDR_INPUT_CHN(channel->ch_num),
				       &val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%d", val);

	case DAC_SCALE:
		return sprintf(buf, "%0.10f", attr_scale_val);

	case DAC_OFFSET:
		return sprintf(buf, "%d", attr_offset_val);

	case DAC_CHN_OP_SELECT:
		ret = ad5710r_spi_read_mask(ad5710r_dev_desc,
					    AD5710R_REG_ADDR_OPERATING_MODE_CHN(channel->ch_num),
					    AD5710R_MASK_OPERATING_MODE(channel->ch_num),
					    &val);
		if (ret) {
			return ret;
		}
		ad5710r_dev_desc->chn_op_mode[channel->ch_num] = val;

		return sprintf(buf, "%s", ad5710r_operating_mode_str[val]);

	case DAC_CHN_HW_LDAC_EN:
		ret = ad5710r_spi_read_mask(ad5710r_dev_desc,
					    AD5710R_REG_ADDR_HW_LDAC_EN_0,
					    AD5710R_MASK_HW_LDAC_EN_0(channel->ch_num),
					    &val);
		if (ret) {
			return ret;
		}
		return sprintf(buf, "%s", ad5710r_ldac_bit_en_str[val]);

	case DAC_CHN_SW_LDAC_EN:
		ret = ad5710r_spi_read_mask(ad5710r_dev_desc,
					    AD5710R_REG_ADDR_SW_LDAC_EN_0,
					    AD5710R_MASK_SW_LDAC_EN_0(channel->ch_num),
					    &val);
		if (ret) {
			return ret;
		}
		return sprintf(buf, "%s", ad5710r_ldac_bit_en_str[val]);

	case DAC_CHN_MODE_SELECT:
		ret = ad5710r_spi_read_mask(ad5710r_dev_desc,
					    AD5710R_V_I_CH_OUTPUT_SELECT,
					    AD5710R_MASK_CH(channel->ch_num),
					    &val);
		if (ret) {
			return ret;
		}
		return sprintf(buf, "%s", ad5710r_chan_mode_sel[val]);

	case DAC_VREF_SELECT:
		ret = ad5710r_spi_read_mask(ad5710r_dev_desc,
					    AD5710R_REG_ADDR_REF_CONTROL_0,
					    AD5710R_MASK_REERENCE_SELECT,
					    &val);
		if (ret) {
			return ret;
		}
		ad5710r_dev_desc->vref_enable = val;

		return sprintf(buf, "%s", ad5710r_vref_str[val]);

	case DAC_RANGE:
		ret = ad5710r_spi_read_mask(ad5710r_dev_desc,
					    AD5710R_REG_ADDR_OUTPUT_CONTROL_0,
					    AD5710R_MASK_OUTPUT_RANGE,
					    &val);
		if (ret) {
			return ret;
		}
		ad5710r_dev_desc->range = val;

		return sprintf(buf, "%s", ad5710r_range_select_str[val]);

	case DAC_SW_LDAC:
	case DAC_HW_LDAC:
		return sprintf(buf, "%s", ad5710r_ldac_trig_str[0]);

	case DAC_MUX_OUT:
		ret = ad5710r_spi_read_mask(ad5710r_dev_desc,
					    AD5710R_REG_ADDR_MUX_OUT_SELECT,
					    AD5710R_MASK_MUX_SELECT,
					    &val);
		if (ret) {
			return ret;
		}
		ad5710r_dev_desc->mux_out_sel = val;

		return sprintf(buf, "%s", ad5710r_mux_out_sel[val]);

	case DAC_ALL_CH_OP_MODE:
		return sprintf(buf, "%s", ad5710r_operating_mode_str[all_chn_op_mode]);

	case DAC_MULTI_DAC_CH:
		ret = ad5710r_reg_read(ad5710r_dev_desc,
				       AD5710R_REG_ADDR_MULTI_DAC_CH,
				       &val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%d", val);

	case DAC_MULTI_INPUT_CH:
		ret = ad5710r_reg_read(ad5710r_dev_desc,
				       AD5710R_REG_ADDR_MULTI_INPUT_CH,
				       &val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%d", val);

	case DAC_SAMPLING_FREQUENCY:
#if (INTERFACE_MODE == SPI_DMA)
		if (streaming_option == SINGLE_INSTRUCTION_MODE) {
			ret = ad5710r_get_sampling_rate(&sampling_rate);
			if (ret) {
				return ret;
			}
		} else {
			/* Sampling rate is fixed for stream mode of data streaming */
			sampling_rate = MAX_SAMPLING_RATE_STREAMING_MODE;
		}
#else
		ret = ad5710r_get_sampling_rate(&sampling_rate);
		if (ret) {
			return ret;
		}
#endif

		return sprintf(buf, "%lu", sampling_rate);

	case DAC_STREAMING_TECHNIQUE:
		return sprintf(buf, "%s", ad5710r_streaming_select_str[streaming_option]);

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Setter function for AD5710R attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of expected bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int ad5710r_iio_attr_set(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv)
{
	int ret;
	uint8_t value;
	uint8_t chn;
	uint32_t write_val;

	switch (priv) {
	case DAC_SCALE:
	case DAC_OFFSET:
		/* Read-only attributes */
		break;

	case DAC_RAW:
		write_val = no_os_str_to_uint32(buf);

		ret = ad5710r_set_dac_value(ad5710r_dev_desc,
					    write_val,
					    channel->ch_num,
					    AD5710R_WRITE_DAC_REGS);
		if (ret) {
			return ret;
		}
		break;

	case DAC_INPUT:
		write_val = no_os_str_to_uint32(buf);

		ret = ad5710r_set_dac_value(ad5710r_dev_desc,
					    write_val,
					    channel->ch_num,
					    AD5710R_WRITE_INPUT_REGS);
		if (ret) {
			return ret;
		}
		break;

	case DAC_CHN_OP_SELECT:
		for (value = 0; value < NO_OS_ARRAY_SIZE(ad5710r_operating_mode_str); value++) {
			if (!strncmp(buf, ad5710r_operating_mode_str[value], strlen(buf)))
				break;
		}

		ret = ad5710r_set_operating_mode(ad5710r_dev_desc, channel->ch_num, value);
		if (ret) {
			return ret;
		}
		break;

	case DAC_CHN_HW_LDAC_EN:
		if (!strncmp(buf, ad5710r_ldac_bit_en_str[0], strlen(buf))) {
			value = 0;
		} else {
			value = 1;
		}

		value = (ad5710r_dev_desc->hw_ldac_mask & ~AD5710R_MASK_HW_LDAC_EN_0(
				 channel->ch_num)) | (value << channel->ch_num);

		ret = ad5710r_set_hw_ldac(ad5710r_dev_desc, value);
		if (ret) {
			return ret;
		}
		break;

	case DAC_CHN_SW_LDAC_EN:
		if (!strncmp(buf, ad5710r_ldac_bit_en_str[0], strlen(buf))) {
			value = 0;
		} else {
			value = 1;
		}

		value = (ad5710r_dev_desc->sw_ldac_mask & ~AD5710R_MASK_SW_LDAC_EN_0(
				 channel->ch_num)) | (value << channel->ch_num);

		ret = ad5710r_set_sw_ldac(ad5710r_dev_desc, value);
		if (ret) {
			return ret;
		}
		break;

	case DAC_CHN_MODE_SELECT:
		for (value = 0; value < NO_OS_ARRAY_SIZE(ad5710r_chan_mode_sel); value++) {
			if (!strncmp(buf, ad5710r_chan_mode_sel[value], strlen(buf)))
				break;
		}

		ret = ad5710r_channel_output_select(ad5710r_dev_desc, channel->ch_num, value);
		if (ret) {
			return ret;
		}

		break;

	case DAC_VREF_SELECT:
		if (!strncmp(buf, ad5710r_vref_str[0], strlen(buf))) {
			value = AD5710R_EXTERNAL_VREF_PIN_INPUT;
		} else {
			value = AD5710R_INTERNAL_VREF_PIN_2P5V;
		}

		ret = ad5710r_set_reference(ad5710r_dev_desc, value);
		if (ret) {
			return ret;
		}
		break;

	case DAC_RANGE:
		if (!strncmp(buf, ad5710r_range_select_str[0], strlen(buf))) {
			value = AD5710R_CH_OUTPUT_RANGE_0_VREF;
		} else {
			value = AD5710R_CH_OUTPUT_RANGE_0_2VREF;
		}

		ret = ad5710r_set_output_range(ad5710r_dev_desc, value);
		if (ret) {
			return ret;
		}

		/* Get the updated scale values */
		ret = ad5710r_get_scale(&attr_scale_val);
		if (ret) {
			return ret;
		}
		break;

	case DAC_SW_LDAC:
		if (!strncmp(buf, ad5710r_ldac_trig_str[0], strlen(buf))) {
			ret = ad5710r_sw_ldac_trigger(ad5710r_dev_desc);
			if (ret) {
				return ret;
			}
		}
		break;

	case DAC_HW_LDAC:
		if (!strncmp(buf, ad5710r_ldac_trig_str[0], strlen(buf))) {
			/* Reconfigure the LDAC pin as GPIO output (non-PWM) */
			ret = ad5710r_reconfig_ldac(ad5710r_dev_desc, AD5710R_LDAC_PWM);
			if (ret) {
				return ret;
			}

			ret = ad5710r_hw_ldac_trigger(ad5710r_dev_desc);
			if (ret) {
				return ret;
			}

			/* Reconfigure the LDAC pin as PWM */
			ret = ad5710r_reconfig_ldac(ad5710r_dev_desc, AD5710R_LDAC_GPIO_OUTPUT);
			if (ret) {
				return ret;
			}
		}
		break;

	case DAC_MUX_OUT:
		for (value = 0; value < NO_OS_ARRAY_SIZE(ad5710r_mux_out_sel); value++) {
			if (!strncmp(buf, ad5710r_mux_out_sel[value], strlen(buf)))
				break;
		}

		ret = ad5710r_set_mux_out_select(ad5710r_dev_desc, value);
		if (ret) {
			return ret;
		}
		break;


	case DAC_ALL_CH_OP_MODE:
		for (value = 0; value < NO_OS_ARRAY_SIZE(ad5710r_operating_mode_str); value++) {
			if (!strncmp(buf, ad5710r_operating_mode_str[value], strlen(buf)))
				break;
		}

		for (chn = 0; chn < AD5710R_NUM_CH; chn++) {
			ret = ad5710r_set_operating_mode(ad5710r_dev_desc, chn, value);
			if (ret) {
				return ret;
			}
		}
		all_chn_op_mode = value;
		break;

	case DAC_MULTI_DAC_CH:
		write_val = no_os_str_to_uint32(buf);

		ret = ad5710r_set_multidac_value(ad5710r_dev_desc,
						 write_val,
						 BYTE_MASK,
						 AD5710R_WRITE_DAC_REGS);
		if (ret) {
			return ret;
		}
		break;

	case DAC_MULTI_INPUT_CH:
		write_val = no_os_str_to_uint32(buf);

		ret = ad5710r_set_multidac_value(ad5710r_dev_desc,
						 write_val,
						 BYTE_MASK,
						 AD5710R_WRITE_INPUT_REGS);
		if (ret) {
			return ret;
		}
		break;

	case DAC_SAMPLING_FREQUENCY:
		sampling_rate = no_os_str_to_uint32(buf);

		if (streaming_option == SINGLE_INSTRUCTION_MODE) {
			ret = ad5710r_set_sampling_rate(sampling_rate);
			if (ret) {
				return ret;
			}
		}
		break;

	case DAC_STREAMING_TECHNIQUE:
		for (value = 0; value < NO_OS_ARRAY_SIZE(ad5710r_streaming_select_str);
		     value++) {
			if (!strncmp(buf, ad5710r_streaming_select_str[value], strlen(buf))) {
				break;
			}
		}

		/* Reconfigure system parameters if streaming option is changed */
		if (streaming_option != value) {
			streaming_option = value;
			reconfig_stm32_params();
		}

		break;

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Attribute available getter function for AD5710R attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of input bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int ad5710r_iio_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	uint8_t val;
	buf[0] = '\0';

	switch (priv) {
	case DAC_CHN_OP_SELECT:
		for (val = 0; val < num_of_op_modes; val++) {
			if (ad5710r_dev_desc->chip_id == AD5710R_ID && (val == num_of_op_modes - 1))
				val += 2;
			strcat(buf, ad5710r_operating_mode_str[val]);
			strcat(buf, " ");
		}
		break;

	case DAC_CHN_HW_LDAC_EN:
	case DAC_CHN_SW_LDAC_EN:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad5710r_ldac_bit_en_str); val++) {
			strcat(buf, ad5710r_ldac_bit_en_str[val]);
			strcat(buf, " ");
		}
		break;

	case DAC_CHN_MODE_SELECT:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad5710r_chan_mode_sel); val++) {
			strcat(buf, ad5710r_chan_mode_sel[val]);
			strcat(buf, " ");
		}
		break;

	case DAC_VREF_SELECT:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad5710r_vref_str); val++) {
			strcat(buf, ad5710r_vref_str[val]);
			strcat(buf, " ");
		}
		break;

	case DAC_RANGE:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad5710r_range_select_str); val++) {
			strcat(buf, ad5710r_range_select_str[val]);
			strcat(buf, " ");
		}
		break;

	case DAC_SW_LDAC:
	case DAC_HW_LDAC:
		return sprintf(buf, "%s", ad5710r_ldac_trig_str[0]);

	case DAC_MUX_OUT:
		for (val = 0; val < AD5710R_NUM_MUX_OUT_SELECTS; val++) {
			strcat(buf, ad5710r_mux_out_sel[val]);
			strcat(buf, " ");
		}
		break;

	case DAC_ALL_CH_OP_MODE:
		for (val = 0; val < num_of_op_modes; val++) {
			if (ad5710r_dev_desc->chip_id == AD5710R_ID && (val == num_of_op_modes - 1))
				val += 2;
			strcat(buf, ad5710r_operating_mode_str[val]);
			strcat(buf, " ");
		}
		break;

	case DAC_STREAMING_TECHNIQUE:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad5710r_streaming_select_str); val++) {
			strcat(buf, ad5710r_streaming_select_str[val]);
			strcat(buf, " ");
		}
		break;

	default:
		return -EINVAL;
	}

	/* Remove extra trailing space at the end of the buffer string */
	len = strlen(buf);
	buf[len - 1] = '\0';

	return len;
}

/*!
 * @brief	Attribute available setter function for AD5710R attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int ad5710r_iio_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	return len;
}

/**
 * @brief  Prepares the device for data transfer.
 * @param  dev[in, out]- Application descriptor.
 * @param  mask[in]- Number of bytes to transfer.
 * @return 0 in case of success, error code otherwise.
 */
static int32_t ad5710r_iio_prepare_transfer(void* dev, uint32_t mask)
{
	int32_t ret;
	uint8_t ch_mask = 0x1;
	uint8_t index = 0;
	uint8_t chn;

	/* Store active channels based on channel mask set in the
	 * IIO client */
	for (chn = 0; chn < DAC_CHANNELS; chn++) {
		if (ch_mask & mask) {
			ad5710r_active_chns[index++] = chn;
		}
		ch_mask <<= 1;
	}
	num_of_active_channels = index;

	/* Store active channels based on channel mask set in the
	 * IIO client */
	ret = ad5710r_set_hw_ldac(ad5710r_dev_desc, mask);
	if (ret) {
		return ret;
	}

	/* Disable crc before doing data streaming */
	ad5710r_dev_desc->crc_en = false;

#if (INTERFACE_MODE == SPI_INTERRUPT)
	ret = iio_trig_enable(ad5710r_hw_trig_desc);
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_enable(pwm_desc);
	if (ret) {
		return ret;
	}
#else // SPI_DMA
	/* Update interface configuration registers based on the streaming option set */
	static struct ad5710r_transfer_config multi_cfg = {
		.addr_asc = 1,
		.short_instr = 0,
	};

	if (streaming_option == SINGLE_INSTRUCTION_MODE) {
		multi_cfg.single_instr = 1;
		multi_cfg.stream_length_keep_value = 0;
		multi_cfg.stream_mode_length = 0;
	} else {
		multi_cfg.single_instr = 0;
		multi_cfg.stream_length_keep_value = 1;
		multi_cfg.stream_mode_length = num_of_active_channels * BYTES_PER_SAMPLE;
	}

	ret = ad5710r_update_interface_cfg(ad5710r_dev_desc, &multi_cfg);
	if (NO_OS_IS_ERR_VALUE(ret)) {
		return ret;
	}

	spi_init_param = ad5710r_init_params.spi_param->extra;
	spi_init_param->dma_init = &ad5710r_dma_init_param;

	spi_init_param->irq_num = Rx_DMA_IRQ_ID;
	spi_init_param->rxdma_ch = &rxdma_channel;
	if (streaming_option == SINGLE_INSTRUCTION_MODE) {
		spi_init_param->txdma_ch = &txdma_channel_single_instr_mode;
	} else {
		spi_init_param->txdma_ch = &txdma_channel_stream_mode;
	}

	/* Init SPI interface in DMA Mode */
	ret = no_os_spi_init(&ad5710r_dev_desc->spi, ad5710r_init_params.spi_param);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief  Close active channels.
 * @param  dev[in, out]- Application descriptor.
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad5710r_iio_close_channels(void *dev)
{
	int ret;

#if (INTERFACE_MODE == SPI_INTERRUPT)
	ret = iio_trig_disable(ad5710r_hw_trig_desc);
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_disable(pwm_desc);
	if (ret) {
		return ret;
	}
#else //SPI_DMA
	/* Stop timers */
	ret = stm32_timer_stop();
	if (ret) {
		return ret;
	}

	/* Abort DMA Transfers */
	ret = stm32_abort_dma_transfer();
	if (ret) {
		return ret;
	}

	/* De assert CS pin */
	ret = no_os_gpio_set_value(csb_gpio_desc, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	spi_init_param = ad5710r_init_params.spi_param->extra;
	spi_init_param->dma_init = NULL;

	/* Init SPI Interface in normal mode (Non DMA) */
	ret = no_os_spi_init(&ad5710r_dev_desc->spi, ad5710r_init_params.spi_param);
	if (ret) {
		return ret;
	}

	spi_dma_enabled = false;

	/* Reset the interface config registers with the defualt ones */
	ret = ad5710r_update_interface_cfg(ad5710r_dev_desc,
					   &ad5710r_init_params.spi_cfg);
	if (NO_OS_IS_ERR_VALUE(ret)) {
		return ret;
	}

#endif

	/* Reset the crc_en to the value prior to data streaming */
	ad5710r_dev_desc->crc_en = ad5710r_init_params.crc_en;

	return 0;
}

#if (INTERFACE_MODE == SPI_DMA)
/**
 * @brief Update the iio buffer by inserting channel register addresses.
 * @param iio_dev_data[in, out] - IIO device data instance.
 * @return 0 in case of success or negative value otherwise.
 * @detail The input iio buffer samples will be inserted with channel addresses
 *		   before the samples for single instruction based data streaming
 */
static int update_iio_buffer_with_ch_ids(struct iio_device_data* iio_dev_data)
{
	if (!iio_dev_data) {
		return -EINVAL;
	}

	uint16_t nb_of_samples_per_chn = iio_dev_data->buffer->size /
					 (BYTES_PER_SAMPLE * num_of_active_channels); // number of samples per channel
	uint32_t buff_len = iio_dev_data->buffer->size;
	uint32_t iio_buff_idx = buff_len - 1;		 // input iio buffer index
	uint32_t modified_buff_idx = 2 * buff_len - 1; // modified iio buffer index
	int8_t* iio_buff = iio_dev_data->buffer->buf->buff;
	int16_t sample_id;
	int8_t ch_id;

	/* Iterate over the samples per active channels in the iio buffer from backwards */
	for (sample_id = nb_of_samples_per_chn - 1; sample_id >= 0; sample_id--) {
		/* Iterate over the active channles per each sample in the iio buffer */
		for (ch_id = num_of_active_channels - 1; ch_id >= 0; ch_id--) {
			/* Copy data */
			iio_buff[modified_buff_idx--] = iio_buff[iio_buff_idx--];
			iio_buff[modified_buff_idx--] = iio_buff[iio_buff_idx--];

			/* Copy active channel address for the respective channel */
			iio_buff[modified_buff_idx--] = ch_addr_array[ad5710r_active_chns[ch_id]];
			iio_buff[modified_buff_idx--] = ch_addr_array[ad5710r_active_chns[ch_id]] >> 8;
		}
	}

	return 0;
}
#endif

/**
 * @brief Writes all the samples from the DAC buffer into the
		  DAC buffer.
 * @param iio_dev_data[in] - IIO device data instance.
 * @return Number of samples read.
 */
static int32_t ad5710r_iio_submit_samples(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	int8_t* iio_buff;
	uint8_t addr;

	if (!iio_dev_data) {
		return -EINVAL;
	}

	num_of_samples = iio_dev_data->buffer->size / BYTES_PER_SAMPLE;
	iio_buff = iio_dev_data->buffer->buf->buff;

#if (INTERFACE_MODE == SPI_DMA)
	if (!spi_dma_enabled) {
		struct no_os_spi_msg ad5710r_spi_msg = {
			.rx_buff = &local_buff,
		};

		if (streaming_option == SINGLE_INSTRUCTION_MODE) {
			/* Insert channel addresses in iio buffer before streaming the data */
			ret = update_iio_buffer_with_ch_ids(iio_dev_data);
			if (ret) {
				return ret;
			}

			/* SPI Message */
			ad5710r_spi_msg.tx_buff = (uint8_t*)iio_buff;
			ad5710r_spi_msg.bytes_number = num_of_samples * BYTES_PER_SAMPLE_WITH_ADDRESS;
		} else { // STREAMING_MODE
			/* Get 1st channel address from the channel mask */
			addr = AD5710R_REG_ADDR_DAC_CHN(no_os_find_first_set_bit(
								iio_dev_data->buffer->active_mask));

			/* Insert address and shift received data from iio buffer */
			memmove(iio_buff + 2, iio_buff,
				iio_dev_data->buffer->size);
			iio_buff[0] = 0x00;
			iio_buff[1] = addr;

			global_iio_buff = (uint8_t*)iio_buff;

			/* SPI Message */
			ad5710r_spi_msg.tx_buff = (uint8_t*)iio_buff;
			ad5710r_spi_msg.bytes_number = (num_of_samples * BYTES_PER_SAMPLE) + 2;
		}

		/* Set CS low */
		ret = no_os_gpio_set_value(csb_gpio_desc, NO_OS_GPIO_LOW);
		if (ret) {
			return ret;
		}

		ret = no_os_spi_transfer_dma_async(ad5710r_dev_desc->spi, &ad5710r_spi_msg, 1,
						   NULL, NULL);
		if (ret) {
			return ret;
		}

		/* Enable timers */
		if (streaming_option == SINGLE_INSTRUCTION_MODE) {
			ret = stm32_timer_enable();
			if (ret) {
				return ret;
			}
		}

		spi_dma_enabled = true;
	}
#endif

	return 0;
}

#if (INTERFACE_MODE == SPI_INTERRUPT)
/**
 * @brief	Pops one data-set from IIO buffer and writes into DAC when
			IRQ is triggered.
 * @param	iio_dev_data[in] - IIO device data instance.
 * @return	0 in case of success or negative value otherwise.
 */
static int32_t ad5710r_trigger_handler(struct iio_device_data *iio_dev_data)
{

	int32_t ret;
	uint8_t active_ch;
	static uint16_t dac_raw[DAC_CHANNELS];
	static uint8_t chan_idx;  // Current channel index

	if (!chan_idx || chan_idx == num_of_active_channels) {
		ret = iio_buffer_pop_scan(iio_dev_data->buffer, dac_raw);
		if (ret) {
			return ret;
		}
		chan_idx = 0;
	}
	active_ch = ad5710r_active_chns[chan_idx];

	ret = ad5710r_set_dac_value(ad5710r_dev_desc,
				    dac_raw[chan_idx],
				    active_ch,
				    AD5710R_WRITE_INPUT_REGS);
	if (ret) {
		return ret;
	}

	chan_idx += 1;
	return 0;
}

/**
 * @brief	Initialization of AD5710R IIO hardware trigger specific parameters
 * @param 	desc[in,out] - IIO hardware trigger descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad5710r_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	int32_t ret;
	struct iio_hw_trig_init_param ad5710r_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;

	if (!desc) {
		return -EINVAL;
	}

	hw_trig_desc = no_os_calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad5710r_hw_trig_init_params.irq_id = TRIGGER_INT_ID;
	ad5710r_hw_trig_init_params.name = AD5710R_IIO_TRIGGER_NAME;
	ad5710r_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad5710r_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad5710r_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad5710r_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad5710r_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad5710r_hw_trig_init_params.iio_desc = ad5710r_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad5710r_hw_trig_init_params);
	if (ret) {
		no_os_free(hw_trig_desc);
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}
#endif

/**
 * @brief	Search the debug register address in look-up table Or registers array
 * @param	addr[in]- Register address to search for
 * @param	reg_addr_offset[in, out] - Offset of register address from its base address for
 *			multi-byte register entity
 * @return	Index to register address from look-up detect in case of SUCCESS, negative error
 *			code otherwise
 */
static int32_t debug_reg_search(uint32_t addr, uint32_t *reg_addr_offset)
{
	uint32_t curr_indx;		// Indexing to registers array (look-up table)
	uint32_t reg_base_add; 	// Base register address
	bool found = false;		// Address found status flag

	/* Search for valid input register address in registers array */
	for (curr_indx = 0; curr_indx <= AD5710R_NUM_REGS; curr_indx++) {
		if (addr == AD5710R_ADDR(ad5710r_reg[curr_indx])) {
			*reg_addr_offset = 0;
			found = true;
			break;
		} else if ((addr < AD5710R_ADDR(ad5710r_reg[curr_indx])) && (curr_indx != 0)) {
			/* Get the input address offset from its base address for
			 * multi-byte register entity and break the loop indicating input
			 * address is located somewhere in the previous indexed register */
			if (AD5710R_LEN(ad5710r_reg[curr_indx - 1]) > 1) {
				*reg_addr_offset = addr - AD5710R_ADDR(ad5710r_reg[curr_indx - 1]);
				found = true;
			}
			break;
		}
	}

	/* Get the base address of register entity (single or multi byte) */
	if (found) {
		if (*reg_addr_offset > 0) {
			reg_base_add = ad5710r_reg[curr_indx - 1];
		} else {
			reg_base_add = ad5710r_reg[curr_indx];
		}
	} else {
		return -EINVAL;
	}

	return reg_base_add;
}

/*!
 * @brief	Read the debug register value
 * @param	dev[in, out]- Pointer to IIO device instance
 * @param	reg[in]- Register address to read from
 * @param	readval[out]- Pointer to variable to read data into
 * @return	0 in case of success, negative value otherwise
 */
static int32_t ad5710r_iio_debug_reg_read(void *dev,
		uint32_t reg,
		uint32_t *readval)
{
	int32_t ret;
	int32_t reg_base_add; 		// Base register address
	uint32_t reg_addr_offset;	// Offset of input register address from its base

	if (!dev || !readval || (reg > AD5710R_REG_ADDR_MAX)) {
		return -EINVAL;
	}

	reg_base_add = debug_reg_search(reg, &reg_addr_offset);
	if (reg_base_add < 0) {
		return -EINVAL;
	}

	ret = ad5710r_reg_read(ad5710r_dev_desc, reg_base_add, (uint16_t *)readval);
	if (NO_OS_IS_ERR_VALUE(ret)) {
		return ret;
	}

	/* Extract the specific byte location for register entity */
	*readval = (*readval >> (reg_addr_offset * BYTE_SIZE)) & BYTE_MASK;

	return 0;
}

/*!
 * @brief	Write the debug register value
 * @param	dev[in, out]- Pointer to IIO device instance
 * @param	reg[in]- Register address to write to
 * @param	writeval[out]- Pointer to variable to write data into
 * @return	0 in case of success, negative value otherwise
 */
static int32_t ad5710r_iio_debug_reg_write(void *dev,
		uint32_t reg,
		uint32_t writeval)
{
	int32_t ret;
	int32_t reg_base_add; 		// Base register address
	uint32_t reg_addr_offset; 	// Offset of input register address from its base
	uint16_t data;				// Register data

	if (!dev  || (reg > AD5710R_REG_ADDR_MAX)) {
		return -EINVAL;
	}

	reg_base_add = debug_reg_search(reg, &reg_addr_offset);
	if (reg_base_add < 0) {
		return -EINVAL;
	}

	/* Read the register contents */
	ret = ad5710r_reg_read(ad5710r_dev_desc, reg_base_add, &data);
	if (NO_OS_IS_ERR_VALUE(ret)) {
		return ret;
	}

	/* Modify the register contents to write user data at specific
	 * register entity location */
	data &= ~(BYTE_MASK << (reg_addr_offset * BYTE_SIZE));
	data |= (uint32_t)((writeval & BYTE_MASK) << (reg_addr_offset * BYTE_SIZE));

	/* Write data into device register */
	ret = ad5710r_reg_write(ad5710r_dev_desc, reg_base_add, data);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
* @brief	Init for reading/writing and parameterization of a
* 			AD5710R IIO device
* @param 	desc[in,out] - IIO device descriptor
* @return	0 in case of success, negative error code otherwise
*/
static int32_t ad5710r_iio_init(struct iio_device **desc)
{
	struct iio_device *iio_ad5710r_inst;
	int ret;

	if (!desc) {
		return -EINVAL;
	}

	iio_ad5710r_inst = no_os_calloc(1, sizeof(struct iio_device));
	if (!iio_ad5710r_inst) {
		return -EINVAL;
	}

	iio_ad5710r_inst->num_ch = NO_OS_ARRAY_SIZE(ad5710r_iio_channels);
	iio_ad5710r_inst->channels = ad5710r_iio_channels;
	iio_ad5710r_inst->attributes = ad5710r_iio_global_attributes;
	iio_ad5710r_inst->debug_attributes = NULL;

	iio_ad5710r_inst->submit = ad5710r_iio_submit_samples;
	iio_ad5710r_inst->pre_enable = ad5710r_iio_prepare_transfer;
	iio_ad5710r_inst->post_disable = ad5710r_iio_close_channels;
	iio_ad5710r_inst->read_dev = NULL;
	iio_ad5710r_inst->write_dev = NULL;
	iio_ad5710r_inst->debug_reg_read = ad5710r_iio_debug_reg_read;
	iio_ad5710r_inst->debug_reg_write = ad5710r_iio_debug_reg_write;
#if (INTERFACE_MODE == SPI_INTERRUPT)
	iio_ad5710r_inst->trigger_handler = ad5710r_trigger_handler;
#endif

	ret = ad5710r_get_scale(&attr_scale_val);
	if (ret) {
		free(iio_ad5710r_inst);
		return ret;
	}

	*desc = iio_ad5710r_inst;

	return 0;
}

/**
 * @brief	Remove the IIO interface to free all allocated resources for cleanup
 * @return	none
 * @details	This function handles cleanup of all dynamically allocated resources
 *			to prevent memory leaks during error conditions
 */
static void iio_app_remove(void)
{
	/* Free the PWM descriptors */
	deinit_pwm();

	/* Remove hardware trigger if initialized */
#if (INTERFACE_MODE == SPI_INTERRUPT)
	if (ad5710r_hw_trig_desc) {
		iio_hw_trig_remove(ad5710r_hw_trig_desc);
		ad5710r_hw_trig_desc = NULL;
	}
#endif

	/* Remove IIO desc if initialized */
	if (ad5710r_iio_desc) {
		iio_remove(ad5710r_iio_desc);
		ad5710r_iio_desc = NULL;
	}

	/* Free IIO device descriptor if allocated */
	if (ad5710r_iio_dev) {
		no_os_free(ad5710r_iio_dev);
		ad5710r_iio_dev = NULL;
	}

	/* Remove AD5710R device descriptor if initialized */
	if (ad5710r_dev_desc) {
		ad5710r_remove(ad5710r_dev_desc);
		ad5710r_dev_desc = NULL;
	}
}

/**
 * @brief	Initialize the IIO interface for AD5710R IIO device
 * @return	0 in case of success,negative error code otherwise
 */
int32_t ad5710r_iio_initialize(void)
{
	int32_t ret;
	enum ad5710r_id dev_name;
	uint8_t id;
	struct no_os_eeprom_desc *eeprom_desc;

#if (INTERFACE_MODE == SPI_INTERRUPT)
	/* IIO trigger init parameters */
	struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad5710r_iio_trig_desc,
		.name = AD5710R_IIO_TRIGGER_NAME,
	};
#endif

	struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES] = {{
			.name = (char *)ACTIVE_DEVICE_NAME,
			.raw_buf = dac_data_buffer,
			.raw_buf_len = DATA_BUFFER_SIZE / 2, // Allocate only half the buffer size to accommodate the other half for addresses
		}
	};

	/* IIO interface init parameters */
	struct iio_init_param iio_init_params = {
		.phy_type = USE_UART,
#if (INTERFACE_MODE == SPI_INTERRUPT)
		.trigs = &iio_trigger_init_params,
#endif
	};

	/* Add a fixed delay of 1 sec before system init for the PoR sequence to get completed */
	no_os_mdelay(1000);

	ret = eeprom_init(&eeprom_desc, &eeprom_init_params);
	if (ret) {
		return ret;
	}

	/* Read context attributes */
	static const char *mezzanine_names[] = {"EVAL-AD5710R-ARDZ"};

	/* Add delay between the i2c init and the eeprom read */
	no_os_mdelay(1000);

	/* Iterate through the mezzanine ids to detect the correct attached board */
	for (id = 0; id < NO_OS_ARRAY_SIZE(mezzanine_names); id++) {
		ret = get_iio_context_attributes_ex(&iio_init_params.ctx_attrs,
						    &iio_init_params.nb_ctx_attr,
						    eeprom_desc,
						    mezzanine_names[id],
						    STR(HW_CARRIER_NAME),
						    &hw_mezzanine_is_valid, FIRMWARE_VERSION);
		if (ret) {
			goto err;
		}
		if (hw_mezzanine_is_valid) {
			dev_name = id;
			break;
		}
	}

	if (hw_mezzanine_is_valid) {
		switch (dev_name) {
		case AD5710R_ID:
			ret = ad5710r_init(&ad5710r_dev_desc, &ad5710r_init_params);
			if (ret) {
				break;
			}
			iio_device_init_params[0].name = (char*)"ad5710r";
			ad5710r_reg = ad5710r_regs;
			num_of_mux_sels = AD5710R_NUM_MUX_OUT_SELECTS;
			num_of_op_modes = AD5710R_MAX_CHANNEL_OP_MODE_0;

			for (int i = 0; i < num_of_chns; i++) {
				ch_addr_array[i] = AD5710R_REG_ADDR_INPUT_CHN(i);
			}

			ret = ad5710r_iio_init(&ad5710r_iio_dev);
			if (ret) {
				goto err;
			}

			iio_init_params.nb_devs++;

			/* AD5710R IIO device init parameters */
			iio_device_init_params[0].dev_descriptor = ad5710r_iio_dev;
			iio_device_init_params[0].dev = &ad5710r_dev_desc;
#if (INTERFACE_MODE == SPI_INTERRUPT)
			iio_device_init_params[0].trigger_id = "trigger0";
			iio_init_params.nb_trigs++;
#endif

			break;

		default:
			return -EINVAL;

		}
	}

	/* Close the EEPROM once mezzanine verification is completed */
	ret = eeprom_close(eeprom_desc);
	if (ret) {
		return ret;
	}

	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_iio_com_desc;
	iio_init_params.devs = iio_device_init_params;
	ret = iio_init(&ad5710r_iio_desc, &iio_init_params);
	if (ret) {
		goto err;
	}

#if (INTERFACE_MODE == SPI_INTERRUPT)
	ret = ad5710r_iio_trigger_param_init(&ad5710r_hw_trig_desc);
	if (ret) {
		goto err;
	}
#endif

	ret = init_pwm();
	if (ret) {
		goto err;
	}

	return 0;

err:
	iio_app_remove();

	/* Free context attributes if allocated */
	remove_iio_context_attributes(iio_init_params.ctx_attrs);

	return ret;
}

/**
 * @brief 	Run the AD5710R IIO event handler
 * @return	none
 * @details	This function monitors the new IIO client event
 */
void ad5710r_iio_event_handler(void)
{
	iio_step(ad5710r_iio_desc);
}
