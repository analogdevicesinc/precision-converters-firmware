/***************************************************************************//**
 *   @file    ad5706r_iio.c
 *   @brief   Implementation of AD5706R IIO Appication Interface
 *   @details This module acts as an interface for AD5706R IIO device
********************************************************************************
 * Copyright (c) 2024-2026 Analog Devices, Inc.
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
#include "app_config.h"
#include "ad5706r_iio.h"
#include "ad5706r_regs.h"
#include "common.h"
#include "ad5706r_user_config.h"
#include "no_os_error.h"
#include "iio_trigger.h"
#include "iio.h"
#include "no_os_util.h"
#include "no_os_alloc.h"
#include "version.h"
#include "no_os_delay.h"

/******** Forward declaration of getter/setter functions ********/
static int iio_ad5706r_attr_get(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv);

static int iio_ad5706r_attr_set(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv);

static int iio_ad5706r_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int iio_ad5706r_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/* AD5706R Channel attributes */
#define AD5706_CHN_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = iio_ad5706r_attr_get,\
		.store = iio_ad5706r_attr_set\
}

/* AD5706R Channel Available attributes */
#define AD5706_CHN_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = iio_ad5706r_attr_available_get,\
	.store = iio_ad5706r_attr_available_set\
}

/* AD5706R Channels */
#define AD5706_DAC_CH(_name, _dev, _idx) {\
	.name = _name # _idx, \
	.ch_type = IIO_CURRENT,\
	.ch_out = true,\
	.indexed = true,\
	.channel = _idx,\
	.scan_index = _idx,\
	.scan_type = &iio_ad5706r_scan_type[_dev][_idx],\
	.attributes = iio_ad5706r_dac_ch_attributes[_dev]\
}

/* DAC data buffer size */
#if defined(USE_SDRAM)
#define dac_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(32768)		// 32kbytes
static int8_t dac_data_buffer[DATA_BUFFER_SIZE];
#endif

/* Bytes per sample */
#define	BYTES_PER_SAMPLE	sizeof(uint16_t)	// For DAC resolution of 16-bits

/* Number of data storage bits (needed for IIO client to send buffer of data) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

/* Byte size and mask */
#define	BYTE_SIZE	(uint32_t)8
#define	BYTE_MASK	(uint32_t)0xff

/* DAC Max count */
#define DAC_MAX_COUNT (NO_OS_BIT(AD5706_DAC_RESOLUTION ) - 1)

/* Multi DAC CH Sel Reg Value */
#define AD5706R_REG_VAL	0x01

/* Multi DAC CH Sel Reg default value */
#define AD5706R_REG_DEFAULT_VAL	0x0

/* Default reference volts */
#define AD5706_DEFAULT_REF_VOLTS    (float)2.5

/* Number of bytes for SW LDAC Data update */
#define AD5706_N_BYTES_SW_LDAC		4

/* Number of bits for SW LDAC update
 * Note: This is the number of bytes needed per transaction
 * for DAC input register update using SW LDAC based trigger */
#define AD5706_SW_LDAC_N_BYTES_WITH_DATA		8
/******************************************************************************/
/*************************** Variables and User Defined Data Types ************/
/******************************************************************************/

/* Pointer to the struct representing the AD5706R IIO device */
struct ad5706r_dev *ad5706r_dev_inst[NUM_IIO_DEVICES] = { NULL, NULL };

/* IIO interface descriptor */
static struct iio_desc *ad5706r_iio_desc;

/* AD5706R IIO device descriptor */
struct iio_device *ad5706r_iio_dev[NUM_IIO_DEVICES];

/* AD5706R attribute unique IDs */
enum ad5706r_attribute_ids {
	/* IIO Device Attributes */
	DAC_DEV_ADDR_ATTR_ID,
	DAC_SAMPLE_RATE_ATTR_ID,
	DAC_HW_LDAC_TG_STATE_ATTR_ID,
	DAC_HW_LDAC_TG_PWM_ATTR_ID,
	DAC_HW_SHUTDOWN_STATE_ATTR_ID,
	DAC_ADDR_ASCENSION_ATTR_ID,
	DAC_SINGLE_INSTR_ATTR_ID,
	DAC_MUX_OUT_SEL_ATTR_ID,
	MULTI_DAC_INPUT_A_ATTR_ID,
	MULTI_DAC_SW_LDAC_ATTR_ID,
	DAC_REF_SELECT_ATTR_ID,
	DAC_REF_VOLTS_ATTR_ID,
	RESTART_IIO_ATTR_ID,
	NUM_OF_DEV_ATTR,

	/* IIO Channel Attributes */
	DAC_CH_RAW_ATTR_ID,
	DAC_CH_SCALE_ATTR_ID,
	DAC_CH_OFFSET_ATTR_ID,
	DAC_CH_INPUT_A_ATTR_ID,
	DAC_CH_INPUT_B_ATTR_ID,
	DAC_CH_HW_ACTIVE_EDGE_ATTR_ID,
	DAC_CH_RANGE_SEL_ATTR_ID,
	DAC_CH_LDAC_TRIGGER_ATTR_ID,
	DAC_CH_TOGGLE_TRIGGER_ATTR_ID,
	DAC_CH_DITHER_TRIGGER_ATTR_ID,
	DAC_CH_OUTPUT_STATE_ATTR_ID,
	MULTI_DAC_CH_SEL_ATTR_ID,
	NUM_OF_CHN_ATTR = DAC_CH_OUTPUT_STATE_ATTR_ID - NUM_OF_DEV_ATTR
};

/* AD5706R Channel Scan structure */
#define AD5706_DEFAULT_CHN_SCAN {\
		.sign = 'u',\
		.realbits = AD5706_DAC_RESOLUTION,\
		.storagebits = CHN_STORAGE_BITS,\
		.shift = 0,\
		.is_big_endian = false\
}

/* IIOD channel scan configurations */
struct scan_type iio_ad5706r_scan_type[NUM_IIO_DEVICES][AD5706R_NUM_CH] = {
	{
		AD5706_DEFAULT_CHN_SCAN,
		AD5706_DEFAULT_CHN_SCAN,
		AD5706_DEFAULT_CHN_SCAN,
		AD5706_DEFAULT_CHN_SCAN,
	},
	{
		AD5706_DEFAULT_CHN_SCAN,
		AD5706_DEFAULT_CHN_SCAN,
		AD5706_DEFAULT_CHN_SCAN,
		AD5706_DEFAULT_CHN_SCAN,
	},
};

/* DAC channel attributes structure */
static struct iio_attribute
	iio_ad5706r_dac_ch_attributes[NUM_IIO_DEVICES][NUM_OF_CHN_ATTR + 11]
	= {
	{
		AD5706_CHN_ATTR("raw", DAC_CH_RAW_ATTR_ID),
		AD5706_CHN_ATTR("scale", DAC_CH_SCALE_ATTR_ID),
		AD5706_CHN_ATTR("offset", DAC_CH_OFFSET_ATTR_ID),
		AD5706_CHN_ATTR("input_register_a", DAC_CH_INPUT_A_ATTR_ID),
		AD5706_CHN_ATTR("input_register_b", DAC_CH_INPUT_B_ATTR_ID),
		AD5706_CHN_ATTR("hw_active_edge", DAC_CH_HW_ACTIVE_EDGE_ATTR_ID),
		AD5706_CHN_AVAIL_ATTR("hw_active_edge_available", DAC_CH_HW_ACTIVE_EDGE_ATTR_ID),
		AD5706_CHN_ATTR("range_sel", DAC_CH_RANGE_SEL_ATTR_ID),
		AD5706_CHN_AVAIL_ATTR("range_sel_available", DAC_CH_RANGE_SEL_ATTR_ID),
		AD5706_CHN_ATTR("ldac_trigger_chn", DAC_CH_LDAC_TRIGGER_ATTR_ID),
		AD5706_CHN_AVAIL_ATTR("ldac_trigger_chn_available", DAC_CH_LDAC_TRIGGER_ATTR_ID),
		AD5706_CHN_ATTR("toggle_trigger_chn", DAC_CH_TOGGLE_TRIGGER_ATTR_ID),
		AD5706_CHN_AVAIL_ATTR("toggle_trigger_chn_available", DAC_CH_TOGGLE_TRIGGER_ATTR_ID),
		AD5706_CHN_ATTR("dither_trigger_chn", DAC_CH_DITHER_TRIGGER_ATTR_ID),
		AD5706_CHN_AVAIL_ATTR("dither_trigger_chn_available", DAC_CH_DITHER_TRIGGER_ATTR_ID),
		AD5706_CHN_ATTR("output_state", DAC_CH_OUTPUT_STATE_ATTR_ID),
		AD5706_CHN_AVAIL_ATTR("output_state_available", DAC_CH_OUTPUT_STATE_ATTR_ID),
		AD5706_CHN_ATTR("multi_dac_sel_ch", MULTI_DAC_CH_SEL_ATTR_ID),
		AD5706_CHN_AVAIL_ATTR("multi_dac_sel_ch_available", MULTI_DAC_CH_SEL_ATTR_ID),
		END_ATTRIBUTES_ARRAY,
	},
	{},
};

/* AD5706R device (global) specific attributes list */
static struct iio_attribute
	iio_ad5706r_global_attributes[NUM_IIO_DEVICES][NUM_OF_DEV_ATTR + 8]
	= {
	{
		AD5706_CHN_ATTR("sampling_frequency", DAC_SAMPLE_RATE_ATTR_ID),
		AD5706_CHN_ATTR("hw_ldac_tg_state", DAC_HW_LDAC_TG_STATE_ATTR_ID),
		AD5706_CHN_AVAIL_ATTR("hw_ldac_tg_state_available", DAC_HW_LDAC_TG_STATE_ATTR_ID),
		AD5706_CHN_ATTR("hw_ldac_tg_pwm", DAC_HW_LDAC_TG_PWM_ATTR_ID),
		AD5706_CHN_AVAIL_ATTR("hw_ldac_tg_pwm_available", DAC_HW_LDAC_TG_PWM_ATTR_ID),
		AD5706_CHN_ATTR("hw_shutdown_state", DAC_HW_SHUTDOWN_STATE_ATTR_ID),
		AD5706_CHN_AVAIL_ATTR("hw_shutdown_state_available", DAC_HW_SHUTDOWN_STATE_ATTR_ID),
		AD5706_CHN_ATTR("addr_ascension", DAC_ADDR_ASCENSION_ATTR_ID),
		AD5706_CHN_AVAIL_ATTR("addr_ascension_available", DAC_ADDR_ASCENSION_ATTR_ID),
		AD5706_CHN_ATTR("single_instr", DAC_SINGLE_INSTR_ATTR_ID),
		AD5706_CHN_AVAIL_ATTR("single_instr_available", DAC_SINGLE_INSTR_ATTR_ID),
		AD5706_CHN_ATTR("mux_out_sel", DAC_MUX_OUT_SEL_ATTR_ID),
		AD5706_CHN_AVAIL_ATTR("mux_out_sel_available", DAC_MUX_OUT_SEL_ATTR_ID),
		AD5706_CHN_ATTR("multi_dac_input_a", MULTI_DAC_INPUT_A_ATTR_ID),
		AD5706_CHN_ATTR("multi_dac_sw_ldac_trigger", MULTI_DAC_SW_LDAC_ATTR_ID),
		AD5706_CHN_ATTR("multi_dac_sw_ldac_trigger_available", MULTI_DAC_SW_LDAC_ATTR_ID),
		AD5706_CHN_ATTR("ref_select", DAC_REF_SELECT_ATTR_ID),
		AD5706_CHN_AVAIL_ATTR("ref_select_available", DAC_REF_SELECT_ATTR_ID),
		AD5706_CHN_ATTR("reference_volts", DAC_REF_VOLTS_ATTR_ID),

		END_ATTRIBUTES_ARRAY,
	},
	{
		AD5706_CHN_ATTR("dev_addr", DAC_DEV_ADDR_ATTR_ID),
		AD5706_CHN_ATTR("reconfigure_system", RESTART_IIO_ATTR_ID),
		AD5706_CHN_AVAIL_ATTR("reconfigure_system_available", RESTART_IIO_ATTR_ID),

		END_ATTRIBUTES_ARRAY,
	}
};

/* IIOD channels configurations */
static struct iio_channel
	ad5706r_iio_channels[NUM_IIO_DEVICES][AD5706R_NUM_CH] = {
	{
		AD5706_DAC_CH("Chn", 0, 0),
		AD5706_DAC_CH("Chn", 0, 1),
		AD5706_DAC_CH("Chn", 0, 2),
		AD5706_DAC_CH("Chn", 0, 3)
	},
	{
		AD5706_DAC_CH("Chn", 1, 0),
		AD5706_DAC_CH("Chn", 1, 1),
		AD5706_DAC_CH("Chn", 1, 2),
		AD5706_DAC_CH("Chn", 1, 3)
	},
};

/* Device names */
static const char *ad5706r_device_names[NUM_IIO_DEVICES] = {
	"ad5706r", "system_config"
};

/* DAC update rate */
uint32_t ad5706r_update_rate = AD5706_MAX_UPDATE_RATE;

/* LDAC State option */
static const char *ad5706r_hw_ldac_tg_state_option[] = {
	"low",
	"high"
};

/* Shutdown state option */
static const char *ad5706r_hw_shutdown_state_option[] = {
	"low",
	"high"
};

/* PWM values */
static const char *ad5706r_hw_ldac_tg_pwm_options[] = {
	"disable",
	"enable"
};

/* Edge trigger values */
static const char *ad5706r_hw_edge_trigger_options[] = {
	"rising_edge",
	"falling_edge",
	"any_edge"
};

/* Channel range select values */
static const char *ad5706r_ch_range_sel_options[] = {
	"50mA",
	"150mA",
	"200mA",
	"300mA"
};

/* LDAC channel trigger values */
static const char *ad5706r_ldac_trigger_chn_options[] = {
	"None",
	"sw_ldac",
	"hw_ldac"
};

/* Toggle trigger values */
static const char *ad5706r_toggle_trigger_chn_options[] = {
	"None",
	"sw_toggle",
	"hw_toggle"
};

/* Dither trigger values */
static const char *ad5706r_dither_trigger_chn_options[] = {
	"None",
	"sw_dither",
	"hw_dither"
};

/* Output state values */
static const char *ad5706r_output_state_options[] = {
	"shutdown_to_tristate_sw",
	"shutdown_to_gnd_sw",
	"normal_sw",
	"shutdown_to_tristate_hw",
	"shutdown_to_gnd_hw",
	"normal_hw",
};

/* Address ascension values */
static const char *ad5706r_addr_ascension_options[] = {
	"decrement",
	"increment"
};

/* Single instruction values */
static const char *ad5706r_single_instr_options[] = {
	"single_instruction",
	"streaming"
};

/* Mux out options */
static const char *ad5706r_mux_out_options[] = {
	"agnd_1",
	"avdd",
	"vref",
	"agnd_2",
	"iout0_vmon",
	"iout1_vmon",
	"iout2_vmon",
	"iout3_vmon",
	"iout0_imon",
	"iout1_imon",
	"iout2_imon",
	"iout3_imon",
	"pvdd0",
	"pvdd1",
	"pvdd2",
	"pvdd3",
	"tdiode_ch0",
	"tdiode_ch1",
	"tdiode_ch2",
	"tdiode_ch3",
	"mux_in0",
	"mux_in1",
	"mux_in2",
	"mux_in3"
};

/* Multi DAC SW trigger options */
static const char *ad5706r_multi_sw_ldac_trigger_option[] = {
	"trigger"
};

/* Reference select options */
static const char *ad5706r_ref_options[] = {
	"external",
	"internal"
};

/* Multi DAC Channel Select options */
static const char *ad5706r_multi_dac_ch_sel_options[] = {
	"exclude",
	"include"
};

/* Flag to check if PWM has been enabled */
static bool ad5706r_pwm_attr_enabled = false;

/* Offset value */
static uint16_t offset = 0;

/* Output current ranges */
static uint16_t ad5706r_current_ranges_mA[] = {
	50, 150, 200, 300
};

/* Use scale factor of 1000 so that (raw + offset) * scale yields
 * microamperes. */
static float scale_factor = 1000;

/* Default values of LDAC Trigger mode. */
static char *ldac_trigger_mode[AD5706R_NUM_CH] = {
	"None",
	"None",
	"None",
	"None"
};

/* Default values of Toggle Trigger mode. */
static char *toggle_trigger_mode[AD5706R_NUM_CH] = {
	"None",
	"None",
	"None",
	"None"
};

/* Default values of Dither Trigger mode. */
static char *dither_trigger_mode[AD5706R_NUM_CH] = {
	"None",
	"None",
	"None",
	"None"
};

/* Restart IIO option */
static const char *restart_iio_options[] = {
	"Enable",
};

/* Restart IIO flag */
static bool restart_iio_flag = false;

/* Variable to hold the state of LDAC */
static bool ldac_state = false;

/* Variable to hold the state of Shutdown */
static bool shutdown_state = false;

/* Reference volts */
static float ref_volts = AD5706_DEFAULT_REF_VOLTS;

/* Active channel sequence */
static uint8_t ad5706r_active_chns[AD5706R_NUM_CH];

/* Number of active channels */
static uint8_t num_of_active_channels = 0;

#if (INTERFACE_MODE == SPI_DMA)
/* Flag to indicate if SPI DMA enabled */
static bool spi_dma_enabled = false;

/* RX Buffer for SPI Messages in DMA Mode */
static const uint8_t rxbuff = 0x0;

/* SPI Message for SPI DMA Transactions */
static struct no_os_spi_msg ad5706r_spi_msg = {
	.rx_buff = (uint8_t *)&rxbuff
};

/* Number of bytes per SPI transaction for SPI DMA */
uint8_t n_bytes = 0;
#else
/* Descriptor to hold iio trigger details */
static struct iio_trigger ad5706r_iio_trig_desc = {
	.is_synchronous = true,
	.enable = NULL,
	.disable = NULL
};

/* IIO trigger name */
#define AD5706_IIO_TRIGGER_NAME		"ad5706r_iio_trigger"

/* AD5706R HW trigger descriptor */
static struct iio_hw_trig *ad5706r_hw_trig_desc;
#endif

#if (INTERFACE_MODE == SPI_INTERRUPT)
/* IIO trigger init parameters */
struct iio_trigger_init iio_trigger_init_params = {
	.descriptor = &ad5706r_iio_trig_desc,
	.name = AD5706_IIO_TRIGGER_NAME,
};
#endif

/* IIO interface init parameters */
struct iio_init_param iio_init_params = {
	.phy_type = USE_UART,
#if (INTERFACE_MODE == SPI_INTERRUPT)
	.trigs = &iio_trigger_init_params,
#endif
};

/* Flag to check if HW mode has been enabled in the ad5706r_active_chns[] */
bool hw_mode_enabled = false;

/* Flag to check if SW mode has been enabled in the ad5706r_active_chns[] */
bool sw_mode_enabled = false;

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief Set the sampling rate and get the updated value supported by MCU platform
 * @param update_rate[in,out] - Update rate value
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad5706r_set_sampling_rate(uint32_t* sampling_rate)
{
	int32_t ret;
	uint32_t pwm_period_ns;

	if (!sampling_rate) {
		return -EINVAL;
	}

	if (*sampling_rate > AD5706_MAX_UPDATE_RATE) {
		*sampling_rate = AD5706_MAX_UPDATE_RATE;
	}

	/* Configure the LDAC PWM period and duty ratio */
	ret = no_os_pwm_set_period(ldac_pwm_desc,
				   FREQ_TO_NSEC(*sampling_rate));
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_set_duty_cycle(ldac_pwm_desc,
				       LDAC_DUTY_CYCLE_NSEC(FREQ_TO_NSEC(*sampling_rate)));
	if (ret) {
		return ret;
	}

	/* Configure the DAC Update PWM period and duty ratio */
	ret = no_os_pwm_set_period(dac_update_pwm_desc,
				   FREQ_TO_NSEC(*sampling_rate));
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_set_duty_cycle(dac_update_pwm_desc,
				       DAC_UPDATE_DUTY_CYCLE_NSEC(FREQ_TO_NSEC(*sampling_rate)));
	if (ret) {
		return ret;
	}

	/* Get the updated value set by hardware */
	ret = no_os_pwm_get_period(ldac_pwm_desc, &pwm_period_ns);
	if (ret) {
		return ret;
	}

	/* Convert period (nsec) to frequency (hertz) */
	*sampling_rate = (1.0 / pwm_period_ns) * 1000000000;

	return 0;
}

/**
 * @brief	Reconfigure LDAC pin as GPIO output
 * @param 	device[in] - AD5706R device instance
 * @return	0 in case of success, negative error code otherwise
 */
int ad5706r_reconfig_ldac(struct ad5706r_dev *device)
{
	int ret;

	if (!device) {
		return -EINVAL;
	}

	ret = no_os_gpio_remove(device->gpio_ldac_tgp);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_get(&device->gpio_ldac_tgp, ad5706r_init_params.gpio_ldac_tgp);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(device->gpio_ldac_tgp, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Set the MDSPI address
 * @param	md_addr[in]- Address value to apply on the lines
 * @return	0 in case of success, negative value otherwise
 */
static int32_t ad5706r_set_md_addr(uint8_t md_addr)
{
	int32_t ret;
	uint8_t a0_val, a1_val;

	/* Select the address bits */
	switch (md_addr) {
	case 0:
		a0_val = NO_OS_GPIO_LOW;
		a1_val = NO_OS_GPIO_LOW;
		break;

	case 1:
		a0_val = NO_OS_GPIO_LOW;
		a1_val = NO_OS_GPIO_HIGH;
		break;

	case 2:
		a0_val = NO_OS_GPIO_HIGH;
		a1_val = NO_OS_GPIO_LOW;
		break;
	case 3:
		a0_val = NO_OS_GPIO_HIGH;
		a1_val = NO_OS_GPIO_HIGH;
		break;

	default:
		return -EINVAL;
	}

	ret = no_os_gpio_set_value(gpio_ad0_desc, a0_val);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_set_value(gpio_ad1_desc, a1_val);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Getter function for AD5706R attributes
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of expected bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise
 */
static int iio_ad5706r_attr_get(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv)
{
	int ret;
	uint16_t raw;
	float scale;
	uint16_t val;
	struct ad5706r_dev *dev = device;
	float ref_scale;

	switch (priv) {
	case DAC_CH_RAW_ATTR_ID:
		ret = ad5706r_spi_reg_read(dev,
					   AD5706R_REG_DAC_DATA_READBACK_CH(channel->ch_num),
					   &raw);
		if (ret) {
			return ret;
		}
		return sprintf(buf, "%d", raw);

	case DAC_CH_OFFSET_ATTR_ID:
		return sprintf(buf, "%d", offset);

	case DAC_CH_SCALE_ATTR_ID:
		val = dev->range[channel->ch_num];
		ref_scale = ref_volts / AD5706_DEFAULT_REF_VOLTS;
		scale = ((float)ad5706r_current_ranges_mA[val]) / (NO_OS_BIT(
					AD5706_DAC_RESOLUTION) - 1);

		return sprintf(buf, "%0.10f", (scale * ref_scale) / scale_factor);
	case DAC_DEV_ADDR_ATTR_ID:
		return sprintf(buf, "%d", ad5706r_init_params.dev_addr);

	case DAC_SAMPLE_RATE_ATTR_ID:
		return sprintf(buf, "%lu", ad5706r_update_rate);

	case DAC_HW_LDAC_TG_STATE_ATTR_ID:
		if (ldac_state) {
			return sprintf(buf, "%s", ad5706r_hw_ldac_tg_state_option[1]);
		} else {
			return sprintf(buf, "%s", ad5706r_hw_ldac_tg_state_option[0]);
		}

	case DAC_HW_SHUTDOWN_STATE_ATTR_ID:
		if (shutdown_state) {
			return sprintf(buf, "%s", ad5706r_hw_shutdown_state_option[1]);
		} else {
			return sprintf(buf, "%s", ad5706r_hw_shutdown_state_option[0]);
		}

	case DAC_HW_LDAC_TG_PWM_ATTR_ID:
		return sprintf(buf, "%s",
			       ad5706r_hw_ldac_tg_pwm_options[ad5706r_pwm_attr_enabled]);

	case DAC_CH_INPUT_A_ATTR_ID:
		ret = ad5706r_spi_reg_read(dev, AD5706R_REG_DAC_INPUT_A_CH(channel->ch_num),
					   &val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%d", val);

	case DAC_CH_INPUT_B_ATTR_ID:
		ret = ad5706r_spi_reg_read(dev,
					   AD5706R_REG_FUNC_DAC_INPUT_B_CH(channel->ch_num),
					   &val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%d", val);

	case MULTI_DAC_INPUT_A_ATTR_ID:
		ret = ad5706r_spi_reg_read(dev, AD5706R_REG_MULTI_DAC_INPUT_A, &val);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%d", val);

	case DAC_CH_HW_ACTIVE_EDGE_ATTR_ID:
		return sprintf(buf, "%s",
			       ad5706r_hw_edge_trigger_options[(dev->ldac_cfg.edge_trig[channel->ch_num])]);

	case DAC_CH_RANGE_SEL_ATTR_ID:
		val = dev->range[channel->ch_num];

		return sprintf(buf, "%s", ad5706r_ch_range_sel_options[val]);

	case DAC_CH_LDAC_TRIGGER_ATTR_ID:
		/* Return the last used mode */
		return sprintf(buf, "%s", ldac_trigger_mode[channel->ch_num]);

	case DAC_CH_TOGGLE_TRIGGER_ATTR_ID:
		/* Return the last used mode */
		return sprintf(buf, "%s", toggle_trigger_mode[channel->ch_num]);

	case DAC_CH_DITHER_TRIGGER_ATTR_ID:
		/* Return the last used mode */
		return sprintf(buf, "%s", dither_trigger_mode[channel->ch_num]);

	case DAC_CH_OUTPUT_STATE_ATTR_ID:
		return sprintf(buf, "%s",
			       ad5706r_output_state_options[dev->op_mode[channel->ch_num]]);

	case DAC_ADDR_ASCENSION_ATTR_ID:
		val = ((struct ad5706r_dev *)device)->spi_cfg.addr_asc;

		return sprintf(buf, "%s", ad5706r_addr_ascension_options[val]);

	case DAC_SINGLE_INSTR_ATTR_ID:
		val = dev->spi_cfg.single_instr;

		return sprintf(buf, "%s", ad5706r_single_instr_options[val]);

	case DAC_MUX_OUT_SEL_ATTR_ID:
		return sprintf(buf, "%s",
			       ad5706r_mux_out_options[dev->mux_out_sel]);

	case MULTI_DAC_SW_LDAC_ATTR_ID:
		return sprintf(buf, "%s", ad5706r_multi_sw_ldac_trigger_option[0]);

	case DAC_REF_SELECT_ATTR_ID:
		return sprintf(buf, "%s",
			       ad5706r_ref_options[dev->vref_enable]);

	case MULTI_DAC_CH_SEL_ATTR_ID:
		val = no_os_field_get(AD5706R_CHANNEL_SEL(channel->ch_num),
				      dev->ldac_cfg.multi_dac_ch_mask);

		return sprintf(buf,
			       "%s", ad5706r_multi_dac_ch_sel_options[val]);

	case DAC_REF_VOLTS_ATTR_ID:
		return sprintf(buf, "%0.3f", ref_volts);

	case RESTART_IIO_ATTR_ID:
		return sprintf(buf, "%s", restart_iio_options[0]);

	default:
		break;
	}

	return len;
}

/*!
 * @brief	Setter function for AD5706R attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of expected bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int iio_ad5706r_attr_set(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv)
{
	int ret;
	uint16_t val;
	uint16_t write_val;
	uint8_t gpio_state;
	struct ad5706r_dev *dev = device;

	switch (priv) {
	case DAC_CH_RAW_ATTR_ID:
		write_val = no_os_str_to_uint32(buf);

		/* Write to DAC_INPUT_A_Chn */
		ret = ad5706r_set_dac_a_value(dev, channel->ch_num, write_val);
		if (ret) {
			return ret;
		}

		break;

	case DAC_CH_INPUT_A_ATTR_ID:
		write_val = no_os_str_to_uint32(buf);

		/* TODO check if mode has to be changed to synchronous, before writing */
		ret = ad5706r_set_dac_a_value(dev, channel->ch_num, write_val);
		if (ret) {
			return ret;
		}

		break;

	case DAC_CH_INPUT_B_ATTR_ID:
		write_val = no_os_str_to_uint32(buf);

		/* TODO check if mode has to be changed to synchronous, before writing */
		ret = ad5706r_set_dac_b_value(dev, channel->ch_num, write_val);
		if (ret) {
			return ret;
		}

		break;

	case MULTI_DAC_INPUT_A_ATTR_ID:
		write_val = no_os_str_to_uint32(buf);

		ret = ad5706r_set_multi_dac_a_value(dev, write_val);
		if (ret) {
			return ret;
		}

		break;

	case DAC_CH_OFFSET_ATTR_ID:
	case DAC_CH_SCALE_ATTR_ID:
		/* These attributes are constant for the firmware
		* configuration and cannot be set during run time. */
		return len;
	case DAC_DEV_ADDR_ATTR_ID:
		write_val = no_os_str_to_uint32(buf);

		ad5706r_init_params.dev_addr = write_val;

		ad5706r_dev_inst[0]->dev_addr = write_val;

		break;

	case DAC_SAMPLE_RATE_ATTR_ID:
		ad5706r_update_rate = no_os_str_to_uint32(buf);

		/* Configure the DAC update rate supported by the selected platform */
		ret = ad5706r_set_sampling_rate(&ad5706r_update_rate);
		if (ret) {
			return ret;
		}

		break;

	case DAC_HW_SHUTDOWN_STATE_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad5706r_hw_shutdown_state_option); val++) {
			if (!strcmp(buf, ad5706r_hw_shutdown_state_option[val])) {
				break;
			}
		}

		if (val) {
			ret = no_os_gpio_set_value(gpio_shutdown_desc, NO_OS_GPIO_HIGH);
			if (ret) {
				return ret;
			}
			shutdown_state = true;

		} else {
			ret = no_os_gpio_set_value(gpio_shutdown_desc, NO_OS_GPIO_LOW);
			if (ret) {
				return ret;
			}
			shutdown_state = false;
		}

		break;

	case DAC_HW_LDAC_TG_STATE_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad5706r_hw_ldac_tg_state_option); val++) {
			if (!strcmp(buf, ad5706r_hw_ldac_tg_state_option[val])) {
				break;
			}
		}

		/* Configure the default output state of the GPIO based on the option chosen
		and set it to the respective value */
		if (val) {
			gpio_state = NO_OS_GPIO_HIGH;
			ldac_state = true;
		} else {
			gpio_state = NO_OS_GPIO_LOW;
			ldac_state = false;
		}
		ret = no_os_gpio_remove((dev->gpio_ldac_tgp));
		if (ret) {
			return ret;
		}

		ret = no_os_gpio_get(&(dev->gpio_ldac_tgp),
				     ad5706r_init_params.gpio_ldac_tgp);
		if (ret) {
			return ret;
		}

		ret = no_os_gpio_direction_output((dev->gpio_ldac_tgp), gpio_state);
		if (ret) {
			return ret;
		}

		ret = no_os_gpio_set_value((dev->gpio_ldac_tgp), gpio_state);
		if (ret) {
			return ret;
		}

		break;

	case DAC_HW_LDAC_TG_PWM_ATTR_ID:
		/* Enable/Disable LDAC PWM */
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad5706r_hw_ldac_tg_pwm_options); val++) {
			if (!strcmp(buf, ad5706r_hw_ldac_tg_pwm_options[val])) {
				break;
			}
		}

		if (val) {
			/* Configure LDAC as a PWM GPIO */
			ret = no_os_gpio_get(&(dev->gpio_ldac_tgp),
					     &ldac_pwm_gpio_params);
			if (ret) {
				return ret;
			}

			/* Enable PWM, if not already enabled */
			if (!ad5706r_pwm_attr_enabled) {
				ret = no_os_pwm_enable(ldac_pwm_desc);
				if (ret) {
					return ret;
				}
			}
			ad5706r_pwm_attr_enabled = true;
		} else {
			/* Disable PWM, if not already disabled */
			if (ad5706r_pwm_attr_enabled) {
				ret = no_os_pwm_disable(ldac_pwm_desc);
				if (ret) {
					return ret;
				}
			}
			ad5706r_pwm_attr_enabled = false;
		}

		break;

	case DAC_CH_HW_ACTIVE_EDGE_ATTR_ID:
		for (val = AD5706R_RISING_EDGE_TRIG; val <= AD5706R_ANY_EDGE_TRIG; val++) {
			if (!strcmp(buf, ad5706r_hw_edge_trigger_options[val])) {
				break;
			}
		}

		ret = ad5706r_set_edge_trigger(dev, channel->ch_num, val);
		if (ret) {
			return ret;
		}

		break;

	case DAC_CH_RANGE_SEL_ATTR_ID:
		for (val = AD5706R_50mA; val <= AD5706R_300mA; val++) {
			if (!strcmp(buf, ad5706r_ch_range_sel_options[val])) {
				break;
			}
		}

		ret = ad5706r_set_ch_output_range(dev, channel->ch_num, val);
		if (ret) {
			return ret;
		}

		break;

	case DAC_CH_LDAC_TRIGGER_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad5706r_ldac_trigger_chn_options); val++) {
			if (!strcmp(buf, ad5706r_ldac_trigger_chn_options[val])) {
				break;
			}
		}

		/* Check if a valid option was found */
		if (val >= NO_OS_ARRAY_SIZE(ad5706r_ldac_trigger_chn_options)) {
			return -EINVAL;
		}

		if (!val) {
			dev->ldac_cfg.ldac_sync_async_mask |=
				(AD5706R_CHANNEL_SEL(channel->ch_num));

			/* Change DAC mode to Direct Reg Access */
			ret = ad5706r_set_ldac_config(dev,
						      channel->ch_num,
						      AD5706R_DIRECT_WRITE_REG,
						      &dev->ldac_cfg);
			if (ret) {
				return ret;
			}
		} else if (val == 1) {
			dev->ldac_cfg.ldac_hw_sw_mask |= (AD5706R_CHANNEL_SEL(
					channel->ch_num));

			/* Change LDAC mode to SW LDAC */
			ret = ad5706r_set_ldac_config(dev,
						      channel->ch_num,
						      AD5706R_SW_LDAC,
						      &((struct ad5706r_dev *)device)->ldac_cfg);
			if (ret) {
				return ret;
			}

		} else {
			dev->ldac_cfg.ldac_hw_sw_mask &= ~(AD5706R_CHANNEL_SEL(
					channel->ch_num));

			/* Change LDAC mode to HW LDAC */
			ret = ad5706r_set_ldac_config(dev,
						      channel->ch_num,
						      AD5706R_HW_LDAC,
						      &dev->ldac_cfg);
			if (ret) {
				return ret;
			}
		}

		/* Update last used trigger mode */
		ldac_trigger_mode[channel->ch_num] = (char *)
						     ad5706r_ldac_trigger_chn_options[val];

		/* Update the Dither and Toggle Modes to None */
		toggle_trigger_mode[channel->ch_num] = (char *)
						       ad5706r_toggle_trigger_chn_options[0];
		dither_trigger_mode[channel->ch_num] = (char *)
						       ad5706r_dither_trigger_chn_options[0];

		break;

	case DAC_CH_TOGGLE_TRIGGER_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad5706r_toggle_trigger_chn_options);
		     val++) {
			if (!strcmp(buf, ad5706r_toggle_trigger_chn_options[val])) {
				break;
			}
		}

		/* Check if a valid option was found */
		if (val >= NO_OS_ARRAY_SIZE(ad5706r_toggle_trigger_chn_options)) {
			return -EINVAL;
		}

		if (!val) {
			dev->ldac_cfg.ldac_sync_async_mask |=
				(AD5706R_CHANNEL_SEL(channel->ch_num));

			/* Change DAC Mode to Direct Reg Access */
			ret = ad5706r_set_ldac_config(dev,
						      channel->ch_num,
						      AD5706R_DIRECT_WRITE_REG,
						      &dev->ldac_cfg);
			if (ret) {
				return ret;
			}
		} else if (val == 1) {
			dev->ldac_cfg.ldac_hw_sw_mask |= (AD5706R_CHANNEL_SEL(
					channel->ch_num));

			/* Change LDAC mode to SW LDAC */
			ret = ad5706r_set_ldac_config(dev,
						      channel->ch_num,
						      AD5706R_SW_TOGGLE,
						      &dev->ldac_cfg);
			if (ret) {
				return ret;
			}
		} else {
			dev->ldac_cfg.ldac_hw_sw_mask &= ~(AD5706R_CHANNEL_SEL(
					channel->ch_num));

			/* Change LDAC mode to HW LDAC */
			ret = ad5706r_set_ldac_config(dev,
						      channel->ch_num,
						      AD5706R_HW_TOGGLE,
						      &dev->ldac_cfg);
			if (ret) {
				return ret;
			}

		}

		/* Update last used trigger mode */
		toggle_trigger_mode[channel->ch_num] = (char *)
						       ad5706r_toggle_trigger_chn_options[val];

		/* Update the Dither and LDAC Modes to None */
		ldac_trigger_mode[channel->ch_num] = (char *)
						     ad5706r_ldac_trigger_chn_options[0];
		dither_trigger_mode[channel->ch_num] = (char *)
						       ad5706r_dither_trigger_chn_options[0];

		break;

	case DAC_CH_DITHER_TRIGGER_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad5706r_dither_trigger_chn_options);
		     val++) {
			if (!strcmp(buf, ad5706r_dither_trigger_chn_options[val])) {
				break;
			}
		}

		/* Check if a valid option was found */
		if (val >= NO_OS_ARRAY_SIZE(ad5706r_dither_trigger_chn_options)) {
			return -EINVAL;
		}

		if (!val) {
			dev->ldac_cfg.ldac_sync_async_mask |=
				(AD5706R_CHANNEL_SEL(channel->ch_num));

			/* Change DAC Mode to Direct Reg Access */
			ret = ad5706r_set_ldac_config(dev,
						      channel->ch_num,
						      AD5706R_DIRECT_WRITE_REG,
						      &dev->ldac_cfg);
			if (ret) {
				return ret;
			}
		} else if (val == 1) {
			dev->ldac_cfg.ldac_hw_sw_mask |= (AD5706R_CHANNEL_SEL(
					channel->ch_num));

			/* Change LDAC mode to SW Dither */
			ret = ad5706r_set_ldac_config(dev,
						      channel->ch_num,
						      AD5706R_SW_DITHER,
						      &dev->ldac_cfg);
			if (ret) {
				return ret;
			}
		} else {
			dev->ldac_cfg.ldac_hw_sw_mask &= ~(AD5706R_CHANNEL_SEL(
					channel->ch_num));

			/* Change LDAC mode to HW Dither */
			ret = ad5706r_set_ldac_config(dev,
						      channel->ch_num,
						      AD5706R_HW_DITHER,
						      &dev->ldac_cfg);
			if (ret) {
				return ret;
			}

		}

		/* Update last used trigger mode */
		dither_trigger_mode[channel->ch_num] = (char *)
						       ad5706r_dither_trigger_chn_options[val];

		/* Update the Toggle and LDAC Modes to None */
		ldac_trigger_mode[channel->ch_num] = (char *)
						     ad5706r_ldac_trigger_chn_options[0];
		toggle_trigger_mode[channel->ch_num] = (char *)
						       ad5706r_toggle_trigger_chn_options[0];

		break;

	case DAC_CH_OUTPUT_STATE_ATTR_ID:
		for (val = AD5706R_SHUTDOWN_SW; val <= AD5706R_NORMAL_HW; val++) {
			if (!strcmp(buf, ad5706r_output_state_options[val])) {
				break;
			}
		}

		ret = ad5706r_set_operating_mode(dev, channel->ch_num, val);
		if (ret) {
			return ret;
		}

		break;

	case DAC_ADDR_ASCENSION_ATTR_ID:
		for (val = 0; val <= 1; val++) {
			if (!strcmp(buf, ad5706r_addr_ascension_options[val])) {
				break;
			}
		}

		dev->spi_cfg.addr_asc = (bool)val;

		ret = ad5706r_spi_write_mask(dev,
					     AD5706R_REG_INTERFACE_CONFIG_A,
					     AD5706R_INT_CONFIG_A_ADDR_ASC_MASK,
					     dev->spi_cfg.addr_asc);
		if (ret) {
			return ret;
		}

		break;

	case DAC_SINGLE_INSTR_ATTR_ID:
		for (val = 0; val <= 1; val++) {
			if (!strcmp(buf, ad5706r_single_instr_options[val])) {
				break;
			}
		}

		dev->spi_cfg.single_instr = (bool)val;

		ret = ad5706r_spi_write_mask(dev,
					     AD5706R_REG_INTERFACE_CONFIG_B,
					     AD5706R_INT_CONFIG_B_SINGLE_INSTR_MASK,
					     dev->spi_cfg.single_instr);
		if (ret) {
			return ret;
		}

		break;

	case DAC_MUX_OUT_SEL_ATTR_ID:
		for (val = AD5706R_AGND; val <= AD5706R_MUXIN3; val++) {
			if (!strcmp(buf, ad5706r_mux_out_options[val])) {
				break;
			}
		}

		/* Configure the selected mux output value */
		ret = ad5706r_set_mux_out_select(dev, val);
		if (ret) {
			return ret;
		}

		break;

	case MULTI_DAC_SW_LDAC_ATTR_ID:
		ret = ad5706r_multi_dac_sw_ldac_trigger(dev);
		if (ret) {
			return ret;
		}

		break;

	case DAC_REF_SELECT_ATTR_ID:
		for (val = AD5706R_EXTERNAL_VREF_PIN_INPUT;
		     val <= AD5706R_INTERNAL_VREF_PIN_2P5V;
		     val++) {
			if (!strcmp(buf, ad5706r_ref_options[val])) {
				break;
			}
		}

		/* Configure the selected reference */
		ret = ad5706r_set_reference(dev, val);
		if (ret) {
			return ret;
		}

		break;

	case MULTI_DAC_CH_SEL_ATTR_ID:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad5706r_multi_dac_ch_sel_options); val++) {
			if (!strcmp(buf, ad5706r_multi_dac_ch_sel_options[val])) {
				break;
			}
		}

		if (val) {
			/* Enable channel */
			dev->ldac_cfg.multi_dac_ch_mask |= AD5706R_CHANNEL_SEL(
					channel->ch_num);
		} else {
			/* Disable channel */
			dev->ldac_cfg.multi_dac_ch_mask &= ~
							   (AD5706R_CHANNEL_SEL(channel->ch_num));
		}

		/* Configure the channel setting */
		ret = ad5706r_spi_reg_write(dev,
					    AD5706R_REG_MULTI_DAC_CH_SEL,
					    dev->ldac_cfg.multi_dac_ch_mask);
		if (ret) {
			return ret;
		}

		break;

	case DAC_REF_VOLTS_ATTR_ID:
		ref_volts = strtof(buf, NULL);

		break;

	case RESTART_IIO_ATTR_ID:
		/* Set flag to true */
		restart_iio_flag = true;

	default :
		break;
	}

	return len;
}

/*!
 * @brief	Attribute available getter function for AD5706R attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of input bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int iio_ad5706r_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	uint8_t val;

	switch (priv) {
	case DAC_HW_LDAC_TG_STATE_ATTR_ID :
		return sprintf(buf,
			       "%s %s",
			       ad5706r_hw_ldac_tg_state_option[0],
			       ad5706r_hw_ldac_tg_state_option[1]);

	case DAC_HW_SHUTDOWN_STATE_ATTR_ID :
		return sprintf(buf,
			       "%s %s",
			       ad5706r_hw_shutdown_state_option[0],
			       ad5706r_hw_shutdown_state_option[1]);

	case DAC_HW_LDAC_TG_PWM_ATTR_ID:
		return sprintf(buf,
			       "%s %s",
			       ad5706r_hw_ldac_tg_pwm_options[0],
			       ad5706r_hw_ldac_tg_pwm_options[1]);

	case DAC_CH_HW_ACTIVE_EDGE_ATTR_ID:
		return sprintf(buf,
			       "%s %s %s",
			       ad5706r_hw_edge_trigger_options[0],
			       ad5706r_hw_edge_trigger_options[1],
			       ad5706r_hw_edge_trigger_options[2]);

	case DAC_CH_RANGE_SEL_ATTR_ID:
		return sprintf(buf, "%s %s %s %s", ad5706r_ch_range_sel_options[0],
			       ad5706r_ch_range_sel_options[1], ad5706r_ch_range_sel_options[2],
			       ad5706r_ch_range_sel_options[3]);

	case DAC_CH_LDAC_TRIGGER_ATTR_ID:
		return sprintf(buf, "%s %s %s", ad5706r_ldac_trigger_chn_options[0],
			       ad5706r_ldac_trigger_chn_options[1],
			       ad5706r_ldac_trigger_chn_options[2]);

	case DAC_CH_TOGGLE_TRIGGER_ATTR_ID:
		return sprintf(buf,
			       "%s %s %s",
			       ad5706r_toggle_trigger_chn_options[0],
			       ad5706r_toggle_trigger_chn_options[1],
			       ad5706r_toggle_trigger_chn_options[2]);

	case DAC_CH_DITHER_TRIGGER_ATTR_ID:
		return sprintf(buf,
			       "%s %s %s",
			       ad5706r_dither_trigger_chn_options[0],
			       ad5706r_dither_trigger_chn_options[1],
			       ad5706r_dither_trigger_chn_options[2]);

	case DAC_CH_OUTPUT_STATE_ATTR_ID:
		return sprintf(buf,
			       "%s %s %s %s %s %s",
			       ad5706r_output_state_options[0],
			       ad5706r_output_state_options[1],
			       ad5706r_output_state_options[2],
			       ad5706r_output_state_options[3],
			       ad5706r_output_state_options[4],
			       ad5706r_output_state_options[5]);

	case DAC_ADDR_ASCENSION_ATTR_ID:
		return sprintf(buf,
			       "%s %s",
			       ad5706r_addr_ascension_options[0],
			       ad5706r_addr_ascension_options[1]);

	case DAC_SINGLE_INSTR_ATTR_ID:
		return sprintf(buf,
			       "%s %s",
			       ad5706r_single_instr_options[0],
			       ad5706r_single_instr_options[1]);

	case DAC_MUX_OUT_SEL_ATTR_ID:
		buf[0] = '\0';

		for (val = 0; val < NO_OS_ARRAY_SIZE(ad5706r_mux_out_options); val++) {
			strcat(buf, ad5706r_mux_out_options[val]);
			strcat(buf, " ");
		}

		/* Remove extra trailing space at the end of the buffer string */
		len = strlen(buf);
		buf[len - 1] = '\0';

		break;

	case MULTI_DAC_SW_LDAC_ATTR_ID:
		return sprintf(buf, "%s", ad5706r_multi_sw_ldac_trigger_option[0]);

	case DAC_REF_SELECT_ATTR_ID:
		return sprintf(buf,
			       "%s %s",
			       ad5706r_ref_options[0],
			       ad5706r_ref_options[1]);

	case MULTI_DAC_CH_SEL_ATTR_ID:
		return sprintf(buf,
			       "%s %s",
			       ad5706r_multi_dac_ch_sel_options[0],
			       ad5706r_multi_dac_ch_sel_options[1]);

	case RESTART_IIO_ATTR_ID:
		return sprintf(buf, "%s", restart_iio_options[0]);

	default:
		break;
	}

	return len;
}

/*!
 * @brief	Attribute available setter function for AD5706R attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int iio_ad5706r_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	return len;
}

/*!
 * @brief	Search for the base address for multi byte register
 * @param	addr[in] - Address as requested by the IIO client
 * @param	reg_addr_offset[in] - Offset between the base and requested addresses
 * @return	Base address
 */
static int32_t debug_reg_search(uint32_t addr, uint32_t *reg_addr_offset)
{
	uint8_t curr_indx;		// Indexing to registers array (look-up table)
	uint32_t reg_base_add;		// Base register address
	bool found = false;		// Address found status flag

	/* Search for valid input register address in registers array */
	for (curr_indx = 0; curr_indx < AD5706R_NUM_REGS; curr_indx++) {
		if (addr == AD5706R_ADDR(ad5706r_regs[curr_indx])) {
			*reg_addr_offset = 0;
			found = true;
			break;
		} else if (addr < AD5706R_ADDR(ad5706r_regs[curr_indx])) {
			/* Get the input address offset from its base address for
			 * multi-byte register entity and break the loop indicating input
			 * address is located somewhere in the previous indexed register */
			if (AD5706R_LEN(ad5706r_regs[curr_indx - 1]) > 1) {
				*reg_addr_offset = addr - AD5706R_ADDR(ad5706r_regs[curr_indx - 1]);
				found = true;
			}
			break;
		}
	}

	/* Get the base address of register entity (single or multi byte) */
	if (found) {
		if (*reg_addr_offset > 0) {
			reg_base_add = ad5706r_regs[curr_indx - 1];
		} else {
			reg_base_add = ad5706r_regs[curr_indx];
		}
	} else {
		return AD5706R_R2B | addr;
	}

	return reg_base_add;
}

/**
 * @brief  Prepares the device for data transfer.
 * @param  dev[in, out]- Application descriptor.
 * @param  mask[in]- Number of bytes to transfer.
 * @return 0 in case of success, error code otherwise.
 */
static int32_t ad5706r_iio_prepare_transfer(void* dev, uint32_t mask)
{
	int ret;
	uint8_t ch_mask = 0x1;
	uint8_t index = 0;
	uint8_t chn;
	struct ad5706r_dev *device = dev;
	uint8_t id = 0;
	uint8_t channel_mask = 0x1;

	hw_mode_enabled = false;
	sw_mode_enabled = false;

	/* Store active channels based on channel mask set in the
	* IIO client */
	for (chn = 0; chn < AD5706R_NUM_CH; chn++) {
		if (ch_mask & mask) {
			ad5706r_active_chns[index++] = chn;
		}
		ch_mask <<= 1;
	}

	num_of_active_channels = index;

	/* Determine if HW Mode is enabled in any of the actively enabled channels */
	for (chn = 0; chn < num_of_active_channels; chn++) {
		channel_mask = 1 << ad5706r_active_chns[chn];
		id = no_os_field_get(channel_mask, device->ldac_cfg.ldac_hw_sw_mask);

		if (id == 0) {
			hw_mode_enabled = true;
			break;
		}
	}
	channel_mask = 0x1;

	/* Determine if SW Mode is enabled in any of the actively enabled channels */
	for (chn = 0; chn < num_of_active_channels; chn++) {
		channel_mask = 1 << ad5706r_active_chns[chn];
		id = no_os_field_get(channel_mask, device->ldac_cfg.ldac_hw_sw_mask);

		if (id == 1) {
			sw_mode_enabled = true;
			break;
		}
	}

#if (INTERFACE_MODE == SPI_INTERRUPT)
	/* Enable LDAC PWM */
	if (hw_mode_enabled) {
		ret = no_os_pwm_enable(ldac_pwm_desc);
		if (ret) {
			return ret;
		}
	}

	/* Enable triggers and DAC Update PWM only if any channel is configured in SW Mode*/
	if (device->ldac_cfg.ldac_hw_sw_mask) {
		ret = no_os_pwm_enable(dac_update_pwm_desc);
		if (ret) {
			return ret;
		}

		ret = iio_trig_enable(ad5706r_hw_trig_desc);
		if (ret) {
			return ret;
		}
	}
#else // SPI_DMA
	/* Configure DMA parameters */
	spi_init_param = ad5706r_init_params.spi_init_prm->extra;
	spi_init_param->pwm_init = &cs_init_params;
	spi_init_param->dma_init = &ad5706r_dma_init_param;
	spi_init_param->irq_num = Rx_DMA_IRQ_ID;
	spi_init_param->rxdma_ch  = &rxdma_channel;
	spi_init_param->txdma_ch  = &txdma_channel;

	/* Init SPI interface in DMA Mode */
	ret = no_os_spi_init(&device->spi_desc, ad5706r_init_params.spi_init_prm);
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
static int32_t ad5706r_iio_close_channels(void *dev)
{
	int ret;
	struct ad5706r_dev *device = dev;

	/* Reset the list of enabled Channels */
	memset(ad5706r_active_chns, 0x0, AD5706R_NUM_CH);
	num_of_active_channels = 0;

#if (INTERFACE_MODE == SPI_INTERRUPT)
	/* Disable triggers and DAC Update PWM only if any channel is configured in SW Mode*/
	if (device->ldac_cfg.ldac_hw_sw_mask) {
		ret = iio_trig_disable(ad5706r_hw_trig_desc);
		if (ret) {
			return ret;
		}

		ret = no_os_pwm_disable(dac_update_pwm_desc);
		if (ret) {
			return ret;
		}
	}
#endif

	/* Disable LDAC PWM only if there are any channels configured in HW Mode*/
	if (hw_mode_enabled) {
		ret = no_os_pwm_disable(ldac_pwm_desc);
		if (ret) {
			return ret;
		}
	}

#if (INTERFACE_MODE == SPI_DMA)
	spi_init_param = ad5706r_init_params.spi_init_prm->extra;

	/* Disable DAC Update PWM only if there is any channel configured in Sw Mode*/
	if (sw_mode_enabled) {
		/* Disable DAC Update PWM */
		ret = no_os_pwm_disable(dac_update_pwm_desc);
		if (ret) {
			return ret;
		}

		/* Disable Tx trigger PWM */
		ret = no_os_pwm_disable(tx_trigger_desc);
		if (ret) {
			return ret;
		}
	}

	ret = ad5706r_abort_dma_transfers(device);
	if (ret) {
		return ret;
	}

	spi_init_param->dma_init = NULL;
	spi_init_param->pwm_init = NULL;

	/* Init SPI Interface in normal mode (Non DMA) */
	ret = no_os_spi_init(&device->spi_desc, ad5706r_init_params.spi_init_prm);
	if (ret) {
		return ret;
	}

	spi_dma_enabled = false;
#endif

	return 0;
}

/*!
 * @brief	Read the debug register value
 * @param	dev[in, out]- Pointer to IIO device instance
 * @param	reg[in]- Register address to read from
 * @param	readval[in, out]- Pointer to variable to read data into
 * @return	0 in case of success, negative value otherwise
 */
static int32_t iio_ad5706r_debug_reg_read(void *dev,
		uint32_t reg,
		uint32_t *read_val)
{
	int32_t ret;
	int32_t reg_base_add; 		// Base register address
	uint32_t reg_addr_offset = 0; 	// Offset of input register address from its base

	if (!dev || !read_val || (reg > AD5706R_REG_USER_SPARE_3)) {
		return -EINVAL;
	}

	reg_base_add = debug_reg_search(reg, &reg_addr_offset);
	if (reg_base_add < 0) {
		return -EINVAL;
	}

	/* Read the register contents */
	ret = ad5706r_spi_reg_read(dev, reg_base_add, (uint16_t *)read_val);
	if (NO_OS_IS_ERR_VALUE(ret)) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Write value to the debug register
 * @param	dev[in, out]- Pointer to IIO device instance
 * @param	reg[in]- Register address to write to
 * @param	writeval[in]- Variable to write data from
 * @return	0 in case of success, negative value otherwise
 */
static int32_t iio_ad5706r_debug_reg_write(void *dev,
		uint32_t reg,
		uint32_t writeval)
{
	int32_t ret;
	int32_t reg_base_add; 		// Base register address
	uint32_t reg_addr_offset = 0; 	// Offset of input register address from its base

	if (!dev  || (reg > AD5706R_REG_USER_SPARE_3)) {
		return -EINVAL;
	}

	reg_base_add = debug_reg_search(reg, &reg_addr_offset);
	if (reg_base_add < 0) {
		return -EINVAL;
	}

	/* Write data into device register */
	ret = ad5706r_spi_reg_write(dev, reg_base_add, writeval);
	if (ret) {
		return ret;
	}

	return 0;
}

#if (INTERFACE_MODE == SPI_INTERRUPT)
/**
 * @brief Pops one data-set from IIO buffer and writes into DAC
 * @param iio_dev_data[in] - IIO device data instance.
 * @return 0 in case of success or negative value otherwise.
 */
static int32_t ad5706r_trigger_handler(struct iio_device_data *iio_dev_data)
{
	struct ad5706r_dev *device = iio_dev_data->dev;
	int ret;

	/* Trigger SW LDAC if any channel configured in SW Mode */
	if (device->ldac_cfg.ldac_hw_sw_mask) {
		ret = ad5706r_sw_ldac_trigger(device);
		if (ret) {
			return ret;
		}
	}

	return 0;
}
#endif

/**
 * @brief Build the command word for Func Mode Update
 * @param device[in] - AD5706R Device descriptor
 * @return 0 in case of success or negative value otherwise.
 */
void ad5706r_populate_func_mode_data(struct ad5706r_dev *device)
{
#if (INTERFACE_MODE == SPI_DMA)
	uint16_t hw_sw_mask = device->ldac_cfg.ldac_hw_sw_mask;
	uint8_t sw_ldac_reg_len =  AD5706R_LEN(AD5706R_REG_DAC_SW_LDAC);
	uint8_t sw_ldac_reg_addr = AD5706R_ADDR(AD5706R_REG_DAC_SW_LDAC);
	uint8_t local_buff[AD5706_N_BYTES_SW_LDAC] = { 0x0 };
	uint32_t iio_buf_wr_id = 0;
	uint32_t byte_addr = 0;
	uint32_t index;

	/* Trigger SW LDAC if any channel configured in SW Mode */
	if (hw_sw_mask) {
		if (!device->spi_cfg.addr_asc) {
			no_os_memswap64(&hw_sw_mask, sw_ldac_reg_len, sw_ldac_reg_len);
			sw_ldac_reg_addr = AD5706R_ADDR(AD5706R_REG_DAC_SW_LDAC) + sw_ldac_reg_len - 1;
		}

		/* Build the Tx Buffer for a write operation */
		local_buff[byte_addr++] = AD5706R_MD_ADDR(device->dev_addr);
		local_buff[byte_addr++] = sw_ldac_reg_addr;

		/* Copy the hw_sw_mask to the local buffer */
		memcpy(&local_buff[byte_addr], &hw_sw_mask, sw_ldac_reg_len);
	}

	/* Fill up the IIO buffer with the SW LDAC Bits */
	for (index = 0; index < DATA_BUFFER_SIZE / AD5706_N_BYTES_SW_LDAC; index++) {
		memcpy(&dac_data_buffer[iio_buf_wr_id], local_buff, AD5706_N_BYTES_SW_LDAC);
		iio_buf_wr_id += AD5706_N_BYTES_SW_LDAC;
	}

	/* Update the number of bytes per SPI transaction */
	if (hw_sw_mask) {
		n_bytes = AD5706_N_BYTES_SW_LDAC;
	} else {
		n_bytes = 0;
	}

	/* Update the SPI Message */
	ad5706r_spi_msg.tx_buff = (uint8_t *)dac_data_buffer;
	ad5706r_spi_msg.bytes_number = DATA_BUFFER_SIZE;
#endif
}

/**
 * @brief Build the comman word for a SW LDAC Update with the data to be
 * written to the respective input register
 * @param device[in] - AD5706R Device descriptor
 * @param iio_dev_data[in] - IIO Device Data
 * @return 0 in case of success or negative value otherwise.
 */
void ad5706r_populate_ldac_mode_data(struct ad5706r_dev *device,
				     struct iio_device_data *iio_dev_data)
{
#if (INTERFACE_MODE == SPI_DMA)
	uint16_t hw_sw_mask = device->ldac_cfg.ldac_hw_sw_mask;
	int8_t* iio_buff = iio_dev_data->buffer->buf->buff;
	static uint8_t sw_ldac_buff[DATA_BUFFER_SIZE] = {0x0};
	uint32_t modified_buff_idx = 0;
	uint8_t reg_offset_dac_input_a;
	uint16_t func_en_check_mask;
	uint8_t reg_offset_sw_ldac;
	uint32_t iio_buff_idx = 0;
	uint32_t sample_id;
	uint8_t active_chn;

	/* Modify the address and data bits depending on the status of address ascension */
	if (device->spi_cfg.addr_asc) {
		reg_offset_sw_ldac = 0;
		reg_offset_dac_input_a = 0;
		no_os_memswap64(&hw_sw_mask, AD5706R_LEN(AD5706R_REG_DAC_SW_LDAC),
				AD5706R_LEN(AD5706R_REG_DAC_SW_LDAC));
	} else {
		reg_offset_sw_ldac = AD5706R_LEN(AD5706R_REG_DAC_SW_LDAC) - 1;
		reg_offset_dac_input_a = AD5706R_LEN(AD5706R_REG_DAC_INPUT_A_CH(0)) - 1;
	}

	for (sample_id = 0; sample_id < iio_dev_data->buffer->samples; sample_id++) {
		for (active_chn = 0; active_chn < num_of_active_channels; active_chn++) {
			func_en_check_mask = no_os_field_get(NO_OS_BIT(ad5706r_active_chns[active_chn]),
							     device->ldac_cfg.func_en_mask);

			/* Check if channel has been configured in LDAC Mode */
			if (!func_en_check_mask) {
				/* Populate the SPI Command for SW LDAC Write */
				sw_ldac_buff[modified_buff_idx++] = AD5706R_MD_ADDR(device->dev_addr);
				sw_ldac_buff[modified_buff_idx++] = AD5706R_ADDR(AD5706R_REG_DAC_INPUT_A_CH(
						ad5706r_active_chns[active_chn])) + reg_offset_dac_input_a;

				/* Populate the data */
				sw_ldac_buff[modified_buff_idx++] = iio_buff[iio_buff_idx + 1];
				sw_ldac_buff[modified_buff_idx++] = iio_buff[iio_buff_idx];
			}

			iio_buff_idx += BYTES_PER_SAMPLE;
		}

		/* Prepare for a write operation to SW LDAC Register */
		if (hw_sw_mask) {
			sw_ldac_buff[modified_buff_idx++] = AD5706R_MD_ADDR(device->dev_addr);
			sw_ldac_buff[modified_buff_idx++] = AD5706R_ADDR(AD5706R_REG_DAC_SW_LDAC) +
							    reg_offset_sw_ldac;

			sw_ldac_buff[modified_buff_idx++] = AD5706R_WRITE_BIT_LONG_INSTR;
			sw_ldac_buff[modified_buff_idx++] = hw_sw_mask;
		}
	}

	/* Update the number of bytes per SPI transaction */
	if (hw_sw_mask) {
		n_bytes = AD5706_SW_LDAC_N_BYTES_WITH_DATA;
	} else {
		n_bytes = AD5706_SW_LDAC_N_BYTES_WITH_DATA - AD5706_N_BYTES_SW_LDAC;
	}

	/* Update the SPI Message */
	ad5706r_spi_msg.tx_buff = (uint8_t *)sw_ldac_buff;
	ad5706r_spi_msg.bytes_number = modified_buff_idx;
#endif
}

/**
 * @brief Writes all the samples from the IIO buffer into the DAC buffer.
 * @param iio_dev_data[in] - IIO device data instance.
 * @return Number of samples read.
 */
static int32_t ad5706r_iio_submit_samples(struct iio_device_data *iio_dev_data)
{
	bool ldac_update = false;
	uint8_t func_en_check_mask = 0x1;
	uint8_t ch_id;
	int ret;

	if (!iio_dev_data || !iio_dev_data->dev) {
		return -EINVAL;
	}

	struct ad5706r_dev *device = iio_dev_data->dev;

#if (INTERFACE_MODE == SPI_DMA)
	if (!spi_dma_enabled) {
		/* Check if there are channels configured in LDAC Mode */
		if (device->ldac_cfg.func_en_mask != 0xFF) {
			/* Check if any enabled channel has been configured in LDAC Mode */
			for (ch_id = 0; ch_id < num_of_active_channels; ch_id++) {
				func_en_check_mask = no_os_field_get(NO_OS_BIT(ad5706r_active_chns[ch_id]),
								     device->ldac_cfg.func_en_mask);
				/* If FUNC_EN is disabled, channel operates in LDAC Mode */
				if (!func_en_check_mask) {
					ldac_update = true;
				}
			}
		}

		if (ldac_update) {
			/* LDAC Update Mode */
			ad5706r_populate_ldac_mode_data(device, iio_dev_data);
		} else {
			/* Toggle and Dither Modes */
			ad5706r_populate_func_mode_data(device);
		}

		/* Init Tx Trigger */
		ret = ad5706r_init_tx_trigger();
		if (ret) {
			return ret;
		}

		/* Initiate SPI DMA transfer */
		ret = no_os_spi_transfer_dma_async(device->spi_desc, &ad5706r_spi_msg, 1, NULL,
						   NULL);
		if (ret) {
			return ret;
		}

		/* Enable timers */
		ad5706r_timers_enable(device);

		spi_dma_enabled = true;
	}
#endif

	return 0;
}

/**
* @brief	Init for reading/writing and parameterization of a
* 			AD5706R IIO device
* @param 	desc[in,out] - IIO device descriptor
* @param dev_indx[in] - IIO device number
* @return	0 in case of success, negative error code otherwise
*/
static int32_t iio_ad5706r_init(struct iio_device **desc, uint8_t dev_indx)
{
	struct iio_device *iio_ad5706r_inst;

	iio_ad5706r_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad5706r_inst) {
		return -EINVAL;
	}

	/* Pointing the channels to the 0th device instance since the attributes are similar
	 * for all devices */
	iio_ad5706r_inst->num_ch = NO_OS_ARRAY_SIZE(ad5706r_iio_channels[0]);
	iio_ad5706r_inst->channels = ad5706r_iio_channels[0];
	iio_ad5706r_inst->attributes = iio_ad5706r_global_attributes[0];
	iio_ad5706r_inst->submit = ad5706r_iio_submit_samples;
	iio_ad5706r_inst->pre_enable = ad5706r_iio_prepare_transfer;
	iio_ad5706r_inst->post_disable = ad5706r_iio_close_channels;
	iio_ad5706r_inst->read_dev = NULL;
	iio_ad5706r_inst->write_dev = NULL;
	iio_ad5706r_inst->debug_reg_read = iio_ad5706r_debug_reg_read;
	iio_ad5706r_inst->debug_reg_write = iio_ad5706r_debug_reg_write;
#if (INTERFACE_MODE == SPI_INTERRUPT)
	iio_ad5706r_inst->trigger_handler = ad5706r_trigger_handler;
#endif

	*desc = iio_ad5706r_inst;

	return 0;
}

#if (INTERFACE_MODE == SPI_INTERRUPT)
/**
 * @brief	Initialization of AD5706R IIO hardware trigger specific parameters
 * @param 	desc[in,out] - IIO hardware trigger descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad5706r_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	int32_t ret;
	struct iio_hw_trig_init_param ad5706r_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;

	if (!desc) {
		return -EINVAL;
	}

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad5706r_hw_trig_init_params.irq_id = TRIGGER_INT_ID;
	ad5706r_hw_trig_init_params.name = AD5706_IIO_TRIGGER_NAME;
	ad5706r_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad5706r_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad5706r_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad5706r_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad5706r_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad5706r_hw_trig_init_params.iio_desc = ad5706r_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad5706r_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;

}
#endif

/**
 * @brief Init for reading/writing and parameterization of a AD5706r Board IIO device
 * @param desc[in,out] - IIO device descriptor
 * @param dev_indx[in] - IIO Device index
 * @return 0 in case of success, negative error code otherwise
 */
static int board_iio_params_init(struct iio_device** desc,
				 uint8_t dev_indx)
{
	struct iio_device* iio_dev;

	if (!desc) {
		return -EINVAL;
	}

	iio_dev = calloc(1, sizeof(*iio_dev));
	if (!iio_dev) {
		return -ENOMEM;
	}

	iio_dev->num_ch = NO_OS_ARRAY_SIZE(ad5706r_iio_channels[dev_indx]);
	iio_dev->channels = ad5706r_iio_channels[dev_indx];
	iio_dev->attributes = iio_ad5706r_global_attributes[dev_indx];

	*desc = iio_dev;

	return 0;
}

/**
 * @brief	Initialize the IIO interface for AD5706R IIO device
 * @return	0 in case of success,negative error code otherwise
 */
int32_t ad5706r_iio_initialize(void)
{
	/* IIO init status variable */
	int32_t init_status;

	/* IIOD init params */
	struct iio_device_init iio_device_init_params[NUM_IIO_DEVICES];

	/* Variable to readback the reg data */
	uint16_t data_readback;

	/* Iterator for EEPROM reads */
	uint8_t read_id;
	static bool entered = false;

	/* Flag to check if no-OS driver for device has been initialized */
	static bool no_os_init_done = false;

	/* Flag to check if MDSPI Address check has passed once during the runtime */
	static bool mdspi_passed = false;

	/* Permissible HW Mezzanine names */
	static const char *mezzanine_names[] = {
		"EVAL-AD5706R-ARDZ"
	};

	/* EVB HW validation status */
	static bool hw_mezzanine_is_valid;

	/* Init the system peripherals */
	if (!entered) {
		/* Add a fixed delay of 1 sec before system init for the PoR sequence to get completed */
		no_os_udelay(1000000);

		init_status = init_system();
		if (init_status) {
			return init_status;
		}
		entered = true;
	}

	/* Add delay between the eeprom i2c init and the eeprom read */
	no_os_mdelay(1000);

	/* Read context attributes */
	for (read_id = 0; read_id < NO_OS_ARRAY_SIZE(mezzanine_names); read_id++) {
		init_status = get_iio_context_attributes_ex(&iio_init_params.ctx_attrs,
				&iio_init_params.nb_ctx_attr,
				eeprom_desc,
				mezzanine_names[read_id],
				STR(HW_CARRIER_NAME),
				&hw_mezzanine_is_valid,
				FIRMWARE_VERSION);
		if (init_status) {
			return init_status;
		}

		if (hw_mezzanine_is_valid) {
			break;
		}
	}

	if (hw_mezzanine_is_valid) {
		do {
			/* Apply md address on the address lines */
			init_status = ad5706r_set_md_addr(ad5706r_init_params.dev_addr);
			if (init_status) {
				return init_status;
			}

			/* Initialize no-OS drivers only in 2 cases:
			 * 1) During the first time of initialization
			 * 2) If MDSPI Address check fails during the first time of initializaton
			 * and when the MCU calls the ad5706r_iio_initialize() again */
			if ((!no_os_init_done) || (!mdspi_passed)) {
				/* Initialize AD5706R device and peripheral interface */
				init_status = ad5706r_init(&ad5706r_dev_inst[iio_init_params.nb_devs],
							   &ad5706r_init_params);
				if (init_status) {
					break;
				}
				no_os_init_done = true;
			}

			/* Validate the Device address by writing to one of the registers in the DAC regmap
			 * and verifying the readback */
			init_status = ad5706r_spi_reg_write(ad5706r_dev_inst[iio_init_params.nb_devs],
							    AD5706R_REG_BANDGAP_CONTROL,
							    AD5706R_REG_VAL);
			if (init_status) {
				return init_status;
			}

			init_status = ad5706r_spi_reg_read(ad5706r_dev_inst[iio_init_params.nb_devs],
							   AD5706R_REG_BANDGAP_CONTROL,
							   &data_readback);
			if (init_status) {
				return init_status;
			}

			if (data_readback != AD5706R_REG_VAL) {
				break;
			}

			/* Reset the Multi DAC Ch sel reg to default value */
			init_status = ad5706r_spi_reg_write(ad5706r_dev_inst[iio_init_params.nb_devs],
							    AD5706R_REG_BANDGAP_CONTROL,
							    AD5706R_REG_DEFAULT_VAL);
			if (init_status) {
				return init_status;
			}

			init_status = iio_ad5706r_init(&ad5706r_iio_dev[iio_init_params.nb_devs],
						       iio_init_params.nb_devs);
			if (init_status) {
				return init_status;
			}

			/* Initialize the IIO interface */
			iio_device_init_params[iio_init_params.nb_devs].name =
				(char *)ad5706r_device_names[iio_init_params.nb_devs];
			iio_device_init_params[iio_init_params.nb_devs].raw_buf = dac_data_buffer;
			iio_device_init_params[iio_init_params.nb_devs].raw_buf_len = DATA_BUFFER_SIZE;

			iio_device_init_params[iio_init_params.nb_devs].dev =
				ad5706r_dev_inst[iio_init_params.nb_devs];
			iio_device_init_params[iio_init_params.nb_devs].dev_descriptor =
				ad5706r_iio_dev[iio_init_params.nb_devs];

#if (INTERFACE_MODE == SPI_INTERRUPT)
			iio_device_init_params[iio_init_params.nb_devs].trigger_id = "trigger0";
			iio_init_params.nb_trigs++;
#endif

			iio_init_params.nb_devs++;

			/* Flag is set only if the sequence completes device initialization once
			 * with the correct MDSPI Address */
			mdspi_passed = true;
		} while (0);
	}

	/* Initialize board IIO paramaters */
	init_status = board_iio_params_init(&ad5706r_iio_dev[iio_init_params.nb_devs],
					    1);
	if (init_status) {
		return init_status;
	}

	iio_device_init_params[iio_init_params.nb_devs].name = (char *)
			ad5706r_device_names[1];
	iio_device_init_params[iio_init_params.nb_devs].dev_descriptor =
		ad5706r_iio_dev[iio_init_params.nb_devs];
	iio_init_params.nb_devs++;

	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_iio_comm_desc;
	iio_init_params.devs = iio_device_init_params;
	init_status = iio_init(&ad5706r_iio_desc, &iio_init_params);
	if (init_status) {
		return init_status;
	}

#if (INTERFACE_MODE == SPI_INTERRUPT)
	init_status = ad5706r_iio_trigger_param_init(&ad5706r_hw_trig_desc);
	if (init_status) {
		return init_status;
	}
#endif

	init_status = init_pwm();
	if (init_status) {
		return init_status;
	}

	return 0;
}

/**
 * @brief	DeInitialize the IIO parameters.
 */
void iio_params_deinit(void)
{
	uint8_t indx = 0;

	for (indx = 0 ; indx < iio_init_params.nb_devs; indx++) {
		if (ad5706r_iio_dev[indx] != NULL) {
			no_os_free(ad5706r_iio_dev[indx]);
			ad5706r_iio_dev[indx] = NULL;
		}
	}

	iio_init_params.nb_devs = 0;
	iio_init_params.nb_trigs = 0;
}

/**
 * @brief 	Run the AD5706R IIO event handler
 * @return	none
 * @details	This function monitors the new IIO client event
 */
void ad5706r_iio_event_handler(void)
{

	if (restart_iio_flag) {
#if (INTERFACE_MODE == SPI_INTERRUPT)
		ret = iio_hw_trig_remove(ad5706r_hw_trig_desc);
		if (ret) {
			return ret;
		}
#endif

		no_os_pwm_remove(ldac_pwm_desc);
		no_os_pwm_remove(dac_update_pwm_desc);
		iio_params_deinit();

		iio_remove(ad5706r_iio_desc);
		remove_iio_context_attributes(iio_init_params.ctx_attrs);

		/* Reset the restart_iio flag */
		restart_iio_flag = false;

		ad5706r_iio_initialize();
	}

	(void)iio_step(ad5706r_iio_desc);
}
