/***************************************************************************//**
 *   @file    ad405x_iio.c
 *   @brief   Implementation of AD405X IIO Application Interface
 *   @details This module acts as an interface for AD405X IIO device
********************************************************************************
 * Copyright (c) 2022-2025 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <string.h>
#include <math.h>

#include "app_config.h"
#include "app_support.h"
#include "ad405x.h"
#include "ad405x_iio.h"
#include "ad405x_user_config.h"
#include "common.h"
#include "no_os_delay.h"
#include "no_os_error.h"
#include "no_os_gpio.h"
#include "no_os_pwm.h"
#include "no_os_alloc.h"
#include "no_os_util.h"
#include "iio_trigger.h"
#include "version.h"

/******** Forward declaration of getter/setter functions ********/
static int iio_ad405x_attr_get(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv);

static int iio_ad405x_attr_set(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv);

static int iio_ad405x_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int iio_ad405x_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/
#define AD405X_CHN_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = iio_ad405x_attr_get,\
		.store = iio_ad405x_attr_set\
}

#define AD405X_CHN_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = iio_ad405x_attr_available_get,\
	.store = iio_ad405x_attr_available_set\
}

/* ADC data buffer size */
#if defined(SDRAM_SUPPORT_AVAILABLE)
#define adc_data_buffer				(int8_t *)SDRAM_START_ADDRESS
#else
/* I3C generics require an extra sample to be read to start data conversion.
 * This dummy data is included in the adc_data_buffer for DMA to easily
 * accomodate the requirement.
 */
static int8_t adc_data_buffer[DATA_BUFFER_SIZE + (DUMMY_DATA_COUNT * 4)];
#endif

/*	Number of IIO devices */
#define NUM_OF_IIO_DEVICES	         2

/* IIO trigger name */
#define AD405X_IIO_TRIGGER_NAME		ACTIVE_DEVICE_NAME"_iio_trigger"

/* Device names */
#define DEV_AD4050	"ad4050"
#define DEV_AD4052	"ad4052"
#define DEV_AD4060	"ad4060"
#define DEV_AD4062	"ad4062"

/* Factor multiplied to calculated conversion time to ensure proper data capture */
#define COMPENSATION_FACTOR	1.1

/* Internal conversion time in nsec */
#define CONVERSION_TIME_NS           250

#define MAX_SAMPLING_PERIOD_NSEC		2500000

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
/* Pointer to the struct representing the ad405x IIO device */
struct ad405x_dev *p_ad405x_dev = NULL;

/* IIO interface descriptor */
static struct iio_desc *p_ad405x_iio_desc;

/* ad405x IIO device descriptor */
struct iio_device *p_iio_ad405x_dev[NUM_OF_IIO_DEVICES];

/* ad405x IIO hw trigger descriptor */
struct iio_hw_trig *ad405x_hw_trig_desc;

/* IIO interface init parameters */
static struct iio_init_param iio_init_params = {
	.phy_type = USE_UART,
};

/* Variable to store the sampling rate */
static uint32_t ad405x_sample_rate;

/* Selected operating mode. Default is sample mode */
enum ad405x_operation_mode ad405x_operating_mode = AD405X_ADC_MODE_OP;

/* Selected interface mode. Default is DMA mode */
enum ad405x_interface_modes ad405x_interface_mode = SPI_DMA;

/* Variable to store data ready status of ADC */
volatile bool data_ready = false;

/* Variable to store start of buffer address */
volatile uint8_t *buff_start_addr;

/* ad405x attribute unique IDs */
enum ad405x_attribute_ids {
	ADC_RAW,
	ADC_SCALE,
	ADC_OFFSET,
	ADC_OPERATING_MODE,
	ADC_SAMPLE_RATE,
	RESTART_IIO,
	ADC_BURST_SAMPLE_RATE,
	ADC_FILTER_LENGTH
};

/* IIOD channels configurations */
static struct scan_type ad405x_iio_scan_type = {
#if (ADC_DATA_FORMAT == STRAIGHT_BINARY)
	.sign = 'u',
#else
	.sign = 's',
#endif
	.shift = 0,
	.is_big_endian = false
};

/* Operating mode range values string representation */
static char *ad405x_op_mode_str[] = {
	"sample_mode",
	"burst_averaging_mode",
	"averaging_mode",
	"config_mode",
};

/* Averaging filter length values string representation */
static char *ad405x_avg_filter_str[] = {
	"2", "4", "8", "16", "32", "64", "128",
	"256", "512", "1024", "2048", "4096"
};

/* String representation of burst mode sample rates */
static char *ad405x_burst_sample_rates_str[] = {
	"2msps",
	"1msps",
	"300ksps",
	"100ksps",
	"33p3ksps",
	"10ksps",
	"3ksps",
	"1ksps",
	"500sps",
	"333sps",
	"250sps",
	"200sps",
	"166sps",
	"140sps",
	"125sps",
	"111sps"
};

/* Burst mode sample rates (in kHz) */
static float ad405x_burst_sample_rates[] = {
	2000,
	1000,
	300,
	100,
	33.3,
	10,
	3,
	1,
	0.5,
	0.333,
	0.25,
	0.2,
	0.166,
	0.14,
	0.125,
	0.111
};

/* ad405x channel specific attributes list */
static struct iio_attribute iio_ad405x_ch_attributes[] = {
	AD405X_CHN_ATTR("raw", ADC_RAW),
	AD405X_CHN_ATTR("scale", ADC_SCALE),
	AD405X_CHN_ATTR("offset", ADC_OFFSET),
	END_ATTRIBUTES_ARRAY,
};

/* ad405x device (global) specific system config attributes list */
static struct iio_attribute iio_ad405x_global_attributes_system_config[] = {
	AD405X_CHN_ATTR("operating_mode", ADC_OPERATING_MODE),
	AD405X_CHN_AVAIL_ATTR("operating_mode_available", ADC_OPERATING_MODE),
	AD405X_CHN_ATTR("reconfigure_system", RESTART_IIO),
	AD405X_CHN_ATTR("reconfigure_system_available", RESTART_IIO),
	END_ATTRIBUTES_ARRAY
};

/* ad405x device (global) specific sample mode attributes list */
static struct iio_attribute iio_ad405x_global_attributes_sample_mode[] = {
	AD405X_CHN_ATTR("sampling_frequency", ADC_SAMPLE_RATE),
	END_ATTRIBUTES_ARRAY
};

/* ad405x device (global) specific burst averaging mode attributes list */
static struct iio_attribute iio_ad405x_global_attributes_burst_averaging_mode[]
	= {
	AD405X_CHN_ATTR("burst_sample_rate", ADC_BURST_SAMPLE_RATE),
	AD405X_CHN_AVAIL_ATTR("burst_sample_rate_available", ADC_BURST_SAMPLE_RATE),
	AD405X_CHN_ATTR("avg_filter_length", ADC_FILTER_LENGTH),
	AD405X_CHN_AVAIL_ATTR("avg_filter_length_available", ADC_FILTER_LENGTH),
	AD405X_CHN_ATTR("sampling_frequency", ADC_SAMPLE_RATE),
	END_ATTRIBUTES_ARRAY
};

/* ad405x device (global) specific averaging mode attributes list */
static struct iio_attribute iio_ad405x_global_attributes_averaging_mode[] = {
	AD405X_CHN_ATTR("avg_filter_length", ADC_FILTER_LENGTH),
	AD405X_CHN_AVAIL_ATTR("avg_filter_length_available", ADC_FILTER_LENGTH),
	AD405X_CHN_ATTR("sampling_frequency", ADC_SAMPLE_RATE),
	END_ATTRIBUTES_ARRAY
};

/* IIO channels info */
static struct iio_channel iio_ad405x_channels[] = {
	{
		.name = "voltage0",
		.ch_type = IIO_VOLTAGE,
		.ch_out = false,
		.indexed = true,
		.channel = 0,
		.scan_index = 0,
		.scan_type = &ad405x_iio_scan_type,
		.attributes = iio_ad405x_ch_attributes
	}
};

/* ad405x IIOD debug attributes list */
static struct iio_attribute ad405x_debug_attributes[] = {
	END_ATTRIBUTES_ARRAY
};

/* Global Pointer for IIO Device Data */
volatile struct iio_device_data* iio_dev_data_g;

/* Variable to store ADC resolution based on device and mode */
static uint8_t resolution;

/* Bitmask to isolate data bits based on resolution */
static uint32_t adc_data_mask;

/* Variable to store storage bits based on device and mode */
static uint8_t storage_bits;

/* Variable to store bytes per sample based on device and mode */
uint8_t bytes_per_sample;

/* Variable to store maximum count of the ADC based on device and mode*/
static uint32_t adc_max_count;

/* SPI Message */
struct no_os_spi_msg ad405x_spi_msg;

/* SPI init params */
struct stm32_spi_init_param* spi_init_param;

/* Restart IIO flag */
static bool restart_iio_flag = false;

/* Pointer to the support descriptor */
static struct ad405x_support_desc *iio_ad405x_support_desc;

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/
/*!
 * @brief	Function to configure pwm period
 * @param	requested_pwm_period[in] - Time period of sampling PWM.
 * @return	0 in case of success, negative error code otherwise
 */
static int configure_pwm_period(uint32_t requested_pwm_period)
{
	int ret;

#ifdef SPI_SUPPORT_AVAILABLE
	if (ad405x_interface_mode == SPI_DMA) {
		/* Updated the init params to keep sync when system_config is called */
		spi_dma_pwm_init_params.period_ns = requested_pwm_period;
		cs_init_params.period_ns = requested_pwm_period;

		ret = no_os_pwm_set_period(pwm_desc,
					   requested_pwm_period);
		if (ret) {
			return ret;
		}

		ret = no_os_pwm_set_period(cs_pwm_desc,
					   requested_pwm_period);
		if (ret) {
			return ret;
		}
	} else {

		/* Updated the init params to keep sync when system_config is called */
		spi_intr_pwm_init_params.period_ns = requested_pwm_period;

		ret = no_os_pwm_set_period(pwm_desc,
					   requested_pwm_period);
		if (ret) {
			return ret;
		}

		ret = no_os_pwm_set_duty_cycle(pwm_desc,
					       CONV_TRIGGER_DUTY_CYCLE_NSEC(requested_pwm_period));
		if (ret) {
			return ret;
		}

	}
#endif

#ifdef I3C_SUPPORT_AVAILABLE
	if ((ad405x_interface_mode == I3C_INTR) || (ad405x_interface_mode == I3C_DMA)) {

		if (ad405x_interface_mode == I3C_INTR) {
			i3c_intr_pwm_init_params.period_ns = requested_pwm_period;
		} else {
			i3c_dma_pwm_init_params.period_ns = requested_pwm_period;
		}
		ret = no_os_pwm_set_period(pwm_desc, requested_pwm_period);
		if (ret) {
			return ret;
		}

		/*
		 * The duty cycle is calculated from the end to provide a delay factor initially
		 * for the dummy conversion to complete. This becomes necessary in case of
		 * burst averaging since the conversion time is significantly larger when compared
		 * to sample mode.
		 *
		 * Pulse shall look like:- _____| |
		 */
		ret = no_os_pwm_set_duty_cycle(pwm_desc,
					       requested_pwm_period - CONV_TRIGGER_DUTY_CYCLE_NSEC(requested_pwm_period));
		if (ret) {
			return ret;
		}
	}
#endif
	/* Update the ADC parameter when success */
	ad405x_sample_rate = PWM_PERIOD_TO_FREQUENCY(requested_pwm_period);

	return 0;
}

/*!
 * @brief	Function to calculate max pwm period for a given custom attribute value
 * @param	attr_id[in]- Attribute private ID.
 * @param	attr_val[in]- Custom value of attr.
 * @param	configure_pwm[in]- Boolean check to set calculated period.
 * @return	0 in case of success, negative error code otherwise.
 */
static int calc_max_pwm_period(enum ad405x_attribute_ids attr_id,
			       uint8_t attr_val,
			       bool configure_pwm)
{
	/* Here calculate the min cnv_time */
	/* The conversion time is calculated using this formula:
		 * (M-1)*tOSC + tCONV + 24*tSCLK, where M stands for
		 * filter length, tOSC = internal sampling time, tCONV =
		 * ADC conversion time, tSCLK = SPI Clock */

	uint8_t avg_length;
	uint8_t fosc;
	uint64_t cnv_time;
	uint64_t temp_pwm_period;

	switch (attr_id) {
	case ADC_FILTER_LENGTH:
		avg_length = attr_val;
		fosc = p_ad405x_dev->rate;
		break;

	case ADC_BURST_SAMPLE_RATE:
		avg_length = p_ad405x_dev->filter_length;
		fosc = attr_val;
		break;

	default:
		avg_length = p_ad405x_dev->filter_length;
		fosc = p_ad405x_dev->rate;
		break;
	}

	cnv_time = (uint64_t)(((pow(2,
				    avg_length + 1) - 1) * (1000000 / ad405x_burst_sample_rates[fosc])
			       + CONVERSION_TIME_NS) * COMPENSATION_FACTOR);

	switch (ad405x_interface_mode) {
	case SPI_INTR:
		temp_pwm_period = no_os_max(cnv_time + MIN_DATA_CAPTURE_TIME_NS +
					    MIN_INTERRUPT_OVER_HEAD, PWM_FREQUENCY_TO_PERIOD(SAMPLING_RATE_SPI_INTR));
		break;
	case I3C_DMA:
		temp_pwm_period = no_os_max(cnv_time + MIN_DATA_CAPTURE_TIME_NS +
					    MIN_INTERRUPT_OVER_HEAD, PWM_FREQUENCY_TO_PERIOD(SAMPLING_RATE_I3C_DMA));
		break;
	case I3C_INTR:
		temp_pwm_period = no_os_max(cnv_time + MIN_DATA_CAPTURE_TIME_NS +
					    MIN_INTERRUPT_OVER_HEAD, PWM_FREQUENCY_TO_PERIOD(SAMPLING_RATE_I3C_INTR));
		break;
	case SPI_DMA:
	default:
		return 0;
	}

	if (configure_pwm) {
		return configure_pwm_period(temp_pwm_period);
	}

	return temp_pwm_period;
}

/*!
 * @brief	Function to determine closest supported attribute value
 * @param	attr_id[in]- Attribute private ID.
 * @param	attr_val[in]- Custom value of attr.
 * @return	0 in case of success, negative error code otherwise.
 */
static int calc_closest_burst_attr_val(enum ad405x_attribute_ids attr_id,
				       uint8_t *attr_val)
{
	uint64_t temp_pwm_period;
	int16_t closest_val = -1;
	uint8_t val;
	uint8_t lower_bound, upper_bound;

	switch (attr_id) {
	case ADC_FILTER_LENGTH:
		lower_bound = AD405X_LENGTH_2;
		switch (p_ad405x_dev->dev_type) {
		case ID_AD4050:
		case ID_AD4060:
			upper_bound = AD405X_LENGTH_256;
			break;
		case ID_AD4052:
		case ID_AD4062:
		default:
			upper_bound = AD405X_LENGTH_4096;
			break;
		}
		break;

	case ADC_BURST_SAMPLE_RATE:
		lower_bound = AD405X_2_MSPS;
		upper_bound = AD405X_111_SPS;
		break;

	default:
		return -EINVAL;
	}

	/* Loop through the options - find a value for the attribute closest
	 * to the user-supplied value, for which the sampling rate is also
	 * supported */

	for (val = lower_bound; val <= upper_bound; val++) {
		temp_pwm_period = calc_max_pwm_period(attr_id, val, false);
		if (temp_pwm_period < MAX_SAMPLING_PERIOD_NSEC) {
			if (closest_val == -1
			    || (abs(*attr_val - closest_val) > abs(*attr_val - val)))
				closest_val = val;
		}
	}

	*attr_val = closest_val;

	return calc_max_pwm_period(attr_id, *attr_val, true);
}

/*!
 * @brief	Getter function for ad405x attributes
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of expected bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise
 */
static int iio_ad405x_attr_get(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	int ret;
	uint32_t adc_raw_data;
	static int32_t offset = 0;
	float scale;
	static volatile uint32_t pwm_period;
	uint8_t reg_data;
	uint32_t value;
	struct no_os_gpio_desc **pp_gpio_cnv = &p_ad405x_dev->extra.spi_extra.gpio_cnv;

	switch (priv) {
	case ADC_RAW:
		if (p_ad405x_dev->comm_type == AD405X_SPI_COMM) {
			ret = no_os_gpio_remove(*pp_gpio_cnv);
			if (ret) {
				return ret;
			}

			ret = no_os_gpio_get(pp_gpio_cnv, ad405x_init_params.gpio_cnv);
			if (ret) {
				return ret;
			}

			ret = no_os_gpio_direction_output(*pp_gpio_cnv, NO_OS_GPIO_LOW);
			if (ret) {
				return ret;
			}
		}

		ret = ad405x_set_operation_mode(p_ad405x_dev, ad405x_operating_mode);
		if (ret) {
			return ret;
		}
		/*
		 * A read from the data register triggers a new conversion.
		 * So the 1st data is read to start a fresh conversion and get
		 * the updated data.
		 */
		if ((ad405x_interface_mode == I3C_DMA) || (ad405x_interface_mode == I3C_INTR)) {
			ret = ad405x_get_adc(p_ad405x_dev, (int32_t *)&adc_raw_data);
			if (ret) {
				return ret;
			}
			do {
				ret = no_os_gpio_get_value(p_ad405x_dev->gpio_gpio1, (uint8_t *)&value);
				if (ret) {
					return ret;
				}
			} while ((*(uint8_t *)&value) == NO_OS_GPIO_HIGH);
		}
		ret = ad405x_get_adc(p_ad405x_dev, (int32_t *)&adc_raw_data);
		if (ret) {
			return ret;
		}

		ret = ad405x_exit_command(p_ad405x_dev);
		if (ret) {
			return ret;
		}

		if (p_ad405x_dev->comm_type == AD405X_SPI_COMM) {
			ret = no_os_gpio_remove(*pp_gpio_cnv);
			if (ret) {
				return ret;
			}

			ret = no_os_gpio_get(pp_gpio_cnv, &pwm_gpio_params);
			if (ret) {
				return ret;
			}
		}

		/* Mask the ADC raw data to retain only the resolution bits */
		adc_raw_data &= adc_data_mask;

#if (ADC_DATA_FORMAT == TWOS_COMPLEMENT)
		if (adc_raw_data >= adc_max_count) {
			offset = -(NO_OS_BIT(resolution) - 1);
		} else {
			offset = 0;
		}
#endif
		return sprintf(buf, "%lu", adc_raw_data);

	case ADC_SCALE:
		scale = (((ADC_REF_VOLTAGE) / adc_max_count) * 1000);
		return sprintf(buf, "%g", scale);

	case ADC_OFFSET:
		return sprintf(buf, "%ld", offset);

	case ADC_OPERATING_MODE :
		return sprintf(buf, "%s", ad405x_op_mode_str[ad405x_operating_mode]);

	case ADC_BURST_SAMPLE_RATE:
		return sprintf(buf, "%s", ad405x_burst_sample_rates_str[p_ad405x_dev->rate]);

	case ADC_FILTER_LENGTH:
		ret = ad405x_read(p_ad405x_dev, AD405X_REG_AVG_CONFIG, &reg_data, 1);
		if (ret) {
			return ret;
		}

		reg_data &= AD405X_AVG_WIN_LEN_MSK;
		if (reg_data != p_ad405x_dev->filter_length) {
			ret = ad405x_set_avg_filter_length(p_ad405x_dev, reg_data);
			if (ret) {
				return ret;
			}
		}
		return sprintf(buf, "%s", ad405x_avg_filter_str[p_ad405x_dev->filter_length]);

	case ADC_SAMPLE_RATE:
		ret = no_os_pwm_get_period(pwm_desc, (uint32_t *)&pwm_period);
		if (ret) {
			return ret;
		}

		value = PWM_PERIOD_TO_FREQUENCY(pwm_period);

		if (ad405x_operating_mode == AD405X_AVERAGING_MODE_OP) {
			/* In Averaging Mode, the sampling rate is PWM frequency
			 * divided by the averaging length. */
			value /= (1 << (p_ad405x_dev->filter_length + 1));
		}

		return sprintf(buf, "%ld", value);

	case RESTART_IIO:
		return sprintf(buf, "%s", "enable");

	default:
		break;
	}

	return len;
}

/*!
 * @brief	Setter function for ad405x attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of expected bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int iio_ad405x_attr_set(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	enum ad405x_operation_mode op_mode;
	enum ad405x_avg_filter_l filter_len;
	enum ad405x_sample_rate burst_rate;
	uint32_t requested_sampling_period;
	uint32_t requested_sampling_rate;
	uint32_t max_burst_avg_sampling_period;
	int32_t ret;
	uint8_t value = 0;

	switch (priv) {
	case ADC_RAW:
	case ADC_SCALE:
	case ADC_OFFSET :
		/* These attributes are constant for the firmware
		* configuration and cannot be set during run time. */
		return len;

	case ADC_OPERATING_MODE :
		for (op_mode = AD405X_ADC_MODE_OP;
		     op_mode <= AD405X_CONFIG_MODE_OP;
		     op_mode++) {
			if (!strncmp(buf, ad405x_op_mode_str[op_mode], strlen(buf))) {
				value = op_mode;
				break;
			}
		}

		ret = ad405x_set_operation_mode(p_ad405x_dev, value);
		if (ret) {
			return ret;
		}

		ad405x_operating_mode = value;

#ifdef SPI_SUPPORT_AVAILABLE
		/* Choose SPI DMA interface mode when in sample mode */
		if (ad405x_operating_mode == AD405X_ADC_MODE_OP) {
			ad405x_interface_mode = SPI_DMA;
		} else {
			ad405x_interface_mode = SPI_INTR;
		}
#else
		ad405x_interface_mode = I3C_DMA;
#endif
		return len;

	case ADC_BURST_SAMPLE_RATE:
		for (burst_rate = AD405X_2_MSPS; burst_rate <= AD405X_111_SPS; burst_rate++) {
			if (!strncmp(buf, ad405x_burst_sample_rates_str[burst_rate], strlen(buf))) {
				value = burst_rate;
				break;
			}
		}

		/* Find closest supported val */
		ret = calc_closest_burst_attr_val(ADC_BURST_SAMPLE_RATE,
						  &value);
		if (ret) {
			return -EINVAL;
		}

		ret = ad405x_set_sample_rate(p_ad405x_dev, value);
		if (ret) {
			return ret;
		}

		return len;

	case ADC_FILTER_LENGTH:
		for (filter_len = AD405X_LENGTH_2; filter_len <= AD405X_LENGTH_4096;
		     filter_len++) {
			if (!strncmp(buf, ad405x_avg_filter_str[filter_len], strlen(buf))) {
				value = filter_len;
				break;
			}
		}

		if (ad405x_operating_mode == AD405X_BURST_AVERAGING_MODE_OP) {
			/* Find closest supported val */
			ret = calc_closest_burst_attr_val(ADC_FILTER_LENGTH,
							  &value);
			if (ret) {
				return -EINVAL;
			}
		}

		ret = ad405x_set_avg_filter_length(p_ad405x_dev, value);
		if (ret) {
			return ret;
		}

		return len;

	case ADC_SAMPLE_RATE:
		requested_sampling_rate = no_os_str_to_uint32(buf);
		if (requested_sampling_rate == 0) {
			return -EINVAL;
		}

		if (ad405x_operating_mode == AD405X_ADC_MODE_OP) {
			switch (ad405x_interface_mode) {
			case SPI_DMA:
				if (requested_sampling_rate > SAMPLING_RATE_SPI_DMA) {
					requested_sampling_rate = SAMPLING_RATE_SPI_DMA;
				}
				break;
			case SPI_INTR:
				if (requested_sampling_rate > SAMPLING_RATE_SPI_INTR) {
					requested_sampling_rate = SAMPLING_RATE_SPI_INTR;
				}
				break;
			case I3C_DMA:
				if (requested_sampling_rate > SAMPLING_RATE_I3C_DMA) {
					requested_sampling_rate = SAMPLING_RATE_I3C_DMA;
				}
				break;
			case I3C_INTR:
				if (requested_sampling_rate > SAMPLING_RATE_I3C_INTR) {
					requested_sampling_rate = SAMPLING_RATE_I3C_INTR;
				}
				break;
			default:
				return len;
			}

			requested_sampling_period = PWM_FREQUENCY_TO_PERIOD(requested_sampling_rate);
		} else if (ad405x_operating_mode == AD405X_BURST_AVERAGING_MODE_OP) {
			max_burst_avg_sampling_period = calc_max_pwm_period(ADC_SAMPLE_RATE,
							0,
							false);
			if (!max_burst_avg_sampling_period) {
				return len;
			}
			if (requested_sampling_rate > PWM_PERIOD_TO_FREQUENCY(
				    max_burst_avg_sampling_period)) {
				requested_sampling_period = max_burst_avg_sampling_period;
			} else {
				requested_sampling_period = PWM_FREQUENCY_TO_PERIOD(requested_sampling_rate);
			}
		} else {
			switch (ad405x_interface_mode) {
			case SPI_INTR:
				if (ad405x_sample_rate > (SAMPLING_RATE_SPI_INTR / (1 <<
							  (p_ad405x_dev->filter_length + 1)))) {
					requested_sampling_rate = SAMPLING_RATE_SPI_INTR;
				}
				break;
			case I3C_DMA:
				if (ad405x_sample_rate > (SAMPLING_RATE_I3C_DMA / (1 <<
							  (p_ad405x_dev->filter_length + 1)))) {
					requested_sampling_rate = SAMPLING_RATE_I3C_DMA;
				}
				break;
			case I3C_INTR:
				if (ad405x_sample_rate > (SAMPLING_RATE_I3C_INTR / (1 <<
							  (p_ad405x_dev->filter_length + 1)))) {
					requested_sampling_rate = SAMPLING_RATE_I3C_INTR;
				}
				break;
			case SPI_DMA:
			default:
				return len;
			}

			requested_sampling_period = PWM_FREQUENCY_TO_PERIOD(requested_sampling_rate);
			requested_sampling_period /= (1 << (p_ad405x_dev->filter_length + 1));
		}

		ret = configure_pwm_period(requested_sampling_period);
		if (ret) {
			return ret;
		}

		return len;

	case RESTART_IIO:
		/* Set flag to true */
		restart_iio_flag = true;

	default :
		break;
	}

	return len;
}

/*!
 * @brief	Attribute available getter function for ad405x attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of input bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int iio_ad405x_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	switch (priv) {
	case ADC_OPERATING_MODE :
		return sprintf(buf,
			       "%s %s",
			       ad405x_op_mode_str[0],
			       ad405x_op_mode_str[1]);

	case ADC_BURST_SAMPLE_RATE:
		return sprintf(buf,
			       "%s %s %s %s %s %s %s %s %s %s %s %s %s %s %s %s",
			       ad405x_burst_sample_rates_str[0],
			       ad405x_burst_sample_rates_str[1],
			       ad405x_burst_sample_rates_str[2],
			       ad405x_burst_sample_rates_str[3],
			       ad405x_burst_sample_rates_str[4],
			       ad405x_burst_sample_rates_str[5],
			       ad405x_burst_sample_rates_str[6],
			       ad405x_burst_sample_rates_str[7],
			       ad405x_burst_sample_rates_str[8],
			       ad405x_burst_sample_rates_str[9],
			       ad405x_burst_sample_rates_str[10],
			       ad405x_burst_sample_rates_str[11],
			       ad405x_burst_sample_rates_str[12],
			       ad405x_burst_sample_rates_str[13],
			       ad405x_burst_sample_rates_str[14],
			       ad405x_burst_sample_rates_str[15]);

	case ADC_FILTER_LENGTH:
		if ((p_ad405x_dev->dev_type == ID_AD4050)
		    || (p_ad405x_dev->dev_type == ID_AD4060)) {
			return sprintf(buf,
				       "%s %s %s %s %s %s %s %s",
				       ad405x_avg_filter_str[0],
				       ad405x_avg_filter_str[1],
				       ad405x_avg_filter_str[2],
				       ad405x_avg_filter_str[3],
				       ad405x_avg_filter_str[4],
				       ad405x_avg_filter_str[5],
				       ad405x_avg_filter_str[6],
				       ad405x_avg_filter_str[7]);
		} else {
			return sprintf(buf,
				       "%s %s %s %s %s %s %s %s %s %s %s %s",
				       ad405x_avg_filter_str[0],
				       ad405x_avg_filter_str[1],
				       ad405x_avg_filter_str[2],
				       ad405x_avg_filter_str[3],
				       ad405x_avg_filter_str[4],
				       ad405x_avg_filter_str[5],
				       ad405x_avg_filter_str[6],
				       ad405x_avg_filter_str[7],
				       ad405x_avg_filter_str[8],
				       ad405x_avg_filter_str[9],
				       ad405x_avg_filter_str[10],
				       ad405x_avg_filter_str[11]);
		}

	case RESTART_IIO:
		return sprintf(buf, "%s", "enable");

	default:
		break;
	}

	return len;
}

/*!
 * @brief	Attribute available setter function for ad405x attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int iio_ad405x_attr_available_set(void *device,
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
static int32_t iio_ad405x_prepare_transfer(void *dev, uint32_t mask)
{
	return iio_ad405x_support_desc->pre_enable(dev, mask);
}

/**
 * @brief  Terminate current data transfer.
 * @param  dev[in, out]- Application descriptor.
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t iio_ad405x_end_transfer(void *dev)
{
	return iio_ad405x_support_desc->post_disable(dev);
}

/**
 * @brief Writes all the samples from the ADC buffer into the
		  IIO buffer.
 * @param iio_dev_data[in] - IIO device data instance.
 * @return Number of samples read.
 */
static int32_t iio_ad405x_submit_samples(struct iio_device_data *iio_dev_data)
{
	return iio_ad405x_support_desc->submit(iio_dev_data);
}

/**
 * @brief	Reads data from the ADC and pushes it into IIO buffer when the
			IRQ is triggered.
 * @param	iio_dev_data[in] - IIO device data instance.
 * @return	0 in case of success or negative value otherwise.
 */
static int32_t ad405x_trigger_handler(struct iio_device_data *iio_dev_data)
{
	return iio_ad405x_support_desc->trigger_handler(iio_dev_data);
}

/*!
 * @brief Interrupt Service Routine to monitor data ready event.
 * @param context[in] - Callback context (unused)
 * @return none
 */
void data_capture_callback(void *context)
{
	data_ready = true;
}

/*!
 * @brief	Read the debug register value
 * @param	dev[in, out]- Pointer to IIO device instance
 * @param	reg[in]- Register address to read from
 * @param	readval[out]- Pointer to variable to read data into
 * @return	0 in case of success, negative value otherwise
 */
static int32_t iio_ad405x_debug_reg_read(void *dev,
		uint32_t reg,
		uint32_t *readval)
{
	int32_t ret;
	uint8_t reg_val;

	if (!dev || !readval) {
		return -EINVAL;
	}

	/* Creating a new uint8_t helps support both endianness */
	ret = ad405x_read(p_ad405x_dev, (uint8_t)reg, &reg_val, 1);
	if (NO_OS_IS_ERR_VALUE(ret)) {
		return ret;
	}

	*readval = reg_val;

	return 0;
}

/*
 * @brief	Write the debug register value
 * @param	dev[in, out]- Pointer to IIO device instance
 * @param	reg[in]- Register address to write data to
 * @param	writeval[out]- Pointer to variable to write data from
 * @return	0 in case of success, negative value otherwise
 */
static int32_t iio_ad405x_debug_reg_write(void *dev,
		uint32_t reg,
		uint32_t writeval)
{
	int32_t ret;
	uint8_t val;

	if (!dev) {
		return -EINVAL;
	}

	/* Creating a new uint8_t helps support both endianness */
	val = (uint8_t) writeval;
	ret = ad405x_write(p_ad405x_dev, (uint8_t) reg, &val, 1);
	if (NO_OS_IS_ERR_VALUE(ret)) {
		return ret;
	}

	return 0;
}

/**
 * @brief Verify if the platform supports the device.
 * @param dev_type[in] - The device type
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad405x_validate_platform(enum ad405x_type dev_type)
{
	switch (dev_type) {
#ifdef SPI_SUPPORT_AVAILABLE
	case ID_AD4050 ... ID_AD4052:
		return 0;
#endif
#ifdef I3C_SUPPORT_AVAILABLE
	case ID_AD4060 ... ID_AD4062:
		return 0;
#endif
	default:
		return -EINVAL;
	}

	return -EINVAL;
}

/**
 * @brief Assign device name and resolution
 * @param dev_type[in] - The device type
 * @param dev_name[out] - The device name
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad405x_assign_device(uint8_t dev_type,
				    char **dev_name)
{
	int ret;

	ret = ad405x_validate_platform((enum ad405x_type) dev_type);
	if (ret) {
		return -ENOTSUP;
	}

	switch (dev_type) {
	case ID_AD4050:
		*dev_name = DEV_AD4050;
		if (ad405x_operating_mode == AD405X_ADC_MODE_OP) {
			resolution = AD4050_SAMPLE_RES;
			storage_bits = STORAGE_BITS_SAMPLE;
		} else {
			resolution = AD4050_AVG_RES;
			storage_bits = STORAGE_BITS_AVG;
		}
		break;

	case ID_AD4052:
		*dev_name = DEV_AD4052;
		if (ad405x_operating_mode == AD405X_ADC_MODE_OP) {
			resolution = AD4052_SAMPLE_RES;
			storage_bits = STORAGE_BITS_SAMPLE;
		} else {
			resolution = AD4052_AVG_RES;
			storage_bits = STORAGE_BITS_AVG;
		}
		break;

	case ID_AD4060:
		*dev_name = DEV_AD4060;
		ad405x_init_params.comm_init.i3c_init->pid = AD405X_I3C_GEN_PID(0xA,
				AD405X_INSTANCE_ID);
		if (ad405x_operating_mode == AD405X_ADC_MODE_OP) {
			resolution = AD4060_SAMPLE_RES;
			storage_bits = STORAGE_BITS_SAMPLE;
		} else {
			resolution = AD4060_AVG_RES;
			storage_bits = STORAGE_BITS_AVG;
		}
		break;

	case ID_AD4062:
		*dev_name = DEV_AD4062;
		ad405x_init_params.comm_init.i3c_init->pid = AD405X_I3C_GEN_PID(0xC,
				AD405X_INSTANCE_ID);
		if (ad405x_operating_mode == AD405X_ADC_MODE_OP) {
			resolution = AD4062_SAMPLE_RES;
			storage_bits = STORAGE_BITS_SAMPLE;
		} else {
			resolution = AD4062_AVG_RES;
			storage_bits = STORAGE_BITS_AVG;
		}
		break;
	}

	bytes_per_sample = BYTES_PER_SAMPLE(storage_bits);
	adc_data_mask = NO_OS_GENMASK(resolution - 1, 0);

#if (ADC_DATA_FORMAT == STRAIGHT_BINARY)
	adc_max_count = (uint32_t)(1 << (resolution));
#else
	adc_max_count = (uint32_t)(1 << (resolution - 1));
#endif

	switch (dev_type) {
	case ID_AD4050 ... ID_AD4052:
		/* Choose SPI DMA interface mode when in sample mode */
		if (ad405x_operating_mode == AD405X_ADC_MODE_OP) {
			ad405x_interface_mode = SPI_DMA;
		} else {
			ad405x_interface_mode = SPI_INTR;
		}
		break;
	case ID_AD4060 ... ID_AD4062:
		ad405x_interface_mode = I3C_DMA;
		break;
	}

	switch (ad405x_interface_mode) {
	case SPI_DMA:
		ad405x_sample_rate = SAMPLING_RATE_SPI_DMA;
		break;
	case SPI_INTR:
		ad405x_sample_rate = SAMPLING_RATE_SPI_INTR;
		break;
	case I3C_DMA:
		ad405x_sample_rate = SAMPLING_RATE_I3C_DMA;
		break;
	case I3C_INTR:
		ad405x_sample_rate = SAMPLING_RATE_I3C_INTR;
		break;
	}

	ad405x_init_params.dev_type = (enum ad405x_type) dev_type;

	iio_ad405x_support_desc = (struct ad405x_support_desc *) support_desc[dev_type];
	if (!iio_ad405x_support_desc)
		return -EINVAL;

	return 0;
}

/**
* @brief	Init for reading/writing and parameterization of a
* 			ad405x IIO device
* @param 	desc[in,out] - IIO device descriptor
* @return	0 in case of success, negative error code otherwise
*/
static int32_t iio_ad405x_init(struct iio_device **desc)
{
	struct iio_device *iio_ad405x_inst;

	iio_ad405x_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad405x_inst) {
		return -EINVAL;
	}

	iio_ad405x_channels[0].scan_type->realbits = resolution;

	iio_ad405x_channels[0].scan_type->storagebits = storage_bits;

	if (ad405x_interface_mode == I3C_DMA) {
		iio_ad405x_channels[0].scan_type->is_big_endian = true;
	} else {
		iio_ad405x_channels[0].scan_type->is_big_endian = false;
	}

	iio_ad405x_inst->num_ch = NO_OS_ARRAY_SIZE(iio_ad405x_channels);
	iio_ad405x_inst->channels = iio_ad405x_channels;
	iio_ad405x_inst->debug_attributes = ad405x_debug_attributes;
	if (ad405x_operating_mode == AD405X_ADC_MODE_OP) {
		iio_ad405x_inst->attributes = iio_ad405x_global_attributes_sample_mode;
	} else if (ad405x_operating_mode == AD405X_BURST_AVERAGING_MODE_OP) {
		iio_ad405x_inst->attributes = iio_ad405x_global_attributes_burst_averaging_mode;
	} else {
		iio_ad405x_inst->attributes = iio_ad405x_global_attributes_averaging_mode;
	}

	iio_ad405x_inst->submit = iio_ad405x_submit_samples;
	iio_ad405x_inst->pre_enable = iio_ad405x_prepare_transfer;
	iio_ad405x_inst->post_disable = iio_ad405x_end_transfer;
	iio_ad405x_inst->read_dev = NULL;
	iio_ad405x_inst->write_dev = NULL;
	iio_ad405x_inst->debug_reg_read = iio_ad405x_debug_reg_read;
	iio_ad405x_inst->debug_reg_write = iio_ad405x_debug_reg_write;
	iio_ad405x_inst->trigger_handler = ad405x_trigger_handler;

	*desc = iio_ad405x_inst;

	return 0;
}

/**
 * @brief	Initialization of AD405X IIO hardware trigger specific parameters
 * @param 	desc[in,out] - IIO hardware trigger descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad405x_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	int32_t ret;
	struct iio_hw_trig_init_param ad405x_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad405x_hw_trig_init_params.name = AD405X_IIO_TRIGGER_NAME;
	ad405x_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad405x_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad405x_hw_trig_init_params.iio_desc = p_ad405x_iio_desc;
	if (ad405x_interface_mode == SPI_INTR) {
		ad405x_hw_trig_init_params.irq_id = TRIGGER_INT_ID_SPI_INTR;
		ad405x_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
		ad405x_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
		ad405x_hw_trig_init_params.cb_info.handle = IIO_TRIGGER_HANDLE_SPI;
	} else if (ad405x_interface_mode == I3C_INTR) {
		ad405x_hw_trig_init_params.irq_id = TRIGGER_INT_ID_I3C_INTR;
		ad405x_hw_trig_init_params.irq_ctrl = pwm_irq_desc;
		ad405x_hw_trig_init_params.cb_info.event = NO_OS_EVT_LPTIM_PWM_PULSE_FINISHED;
		ad405x_hw_trig_init_params.cb_info.peripheral = NO_OS_LPTIM_IRQ;
		ad405x_hw_trig_init_params.cb_info.handle = IIO_TRIGGER_HANDLE_I3C;
	}

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad405x_hw_trig_init_params);
	if (ret) {
		no_os_free(hw_trig_desc);
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/**
 * @brief   Initialize the ad405x Board Init Params
 * @param   desc[in,out] - IIO Device Descriptor
 * @return  0 in case of success, negative value otherwise
 */
static int board_iio_params_init(struct iio_device** desc)
{
	struct iio_device* iio_dev;

	if (!desc) {
		return -EINVAL;
	}

	iio_dev = calloc(1, sizeof(*iio_dev));
	if (!iio_dev) {
		return -ENOMEM;
	}

	iio_dev->num_ch = 0;
	iio_dev->attributes = iio_ad405x_global_attributes_system_config;

	*desc = iio_dev;

	return 0;
}

/**
 * @brief	DeInitialize the IIO params.
 * @return	0 in case of success, negative error code otherwise
 */
int iio_params_deinit(void)
{
	uint8_t indx = 0;

	for (indx = 0 ; indx < iio_init_params.nb_devs; indx++) {
		if (p_iio_ad405x_dev[indx] != NULL) {
			no_os_free(p_iio_ad405x_dev[indx]);
			p_iio_ad405x_dev[indx] = NULL;
		}
	}

	iio_init_params.nb_devs = 0;

	return 0;
}

/**
 * @brief	Initialize the IIO interface for AD405X IIO device
 * @return	0 in case of success,negative error code otherwise
 */
int32_t iio_app_initialize(void)
{
	int32_t init_status;
	uint8_t dev_type;
	uint8_t indx;
	/* EVB HW validation status */
	bool hw_mezzanine_is_valid;

	struct no_os_eeprom_desc *eeprom_desc;

	/* Read context attributes */
	static const char *mezzanine_names[] = {
		"EVAL-AD4050-ARDZ",
		"EVAL-AD4052-ARDZ",
		"EVAL-AD4056-ARDZ",
		"EVAL-AD4058-ARDZ",
		"EVAL-AD4060-ARDZ",
		"EVAL-AD4062-ARDZ"
	};

#if	(APP_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	static struct iio_trigger ad405x_iio_trig_desc = {
		.is_synchronous = true,
		.enable = NULL,
		.disable = NULL
	};

	static struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad405x_iio_trig_desc,
		.name = AD405X_IIO_TRIGGER_NAME,
	};
#endif

	/* IIOD init parameters */
	static struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES];

	/* Add a fixed delay of 1 sec before system init for the PoR sequence to get completed */
	no_os_mdelay(1000);

	init_status = eeprom_init(&eeprom_desc, &eeprom_init_params);
	if (init_status) {
		return init_status;
	}

	/* Add delay between the i2c init and the eeprom read */
	no_os_mdelay(1000);

	/* Iterate twice to detect the correct attached board */
	for (indx = 0; indx < NO_OS_ARRAY_SIZE(mezzanine_names); indx++) {
		init_status = get_iio_context_attributes_ex(&iio_init_params.ctx_attrs,
				&iio_init_params.nb_ctx_attr,
				eeprom_desc,
				mezzanine_names[indx],
				STR(HW_CARRIER_NAME),
				&hw_mezzanine_is_valid,
				FIRMWARE_VERSION);
		if (init_status) {
			return init_status;
		}

		if (hw_mezzanine_is_valid) {
			dev_type = indx;
			break;
		}

		if (indx != NO_OS_ARRAY_SIZE(mezzanine_names) - 1) {
			init_status = remove_iio_context_attributes(iio_init_params.ctx_attrs);
			if (init_status) {
				return init_status;
			}
		}
	}

	/* Close the EEPROM once mezzanine verification is completed */
	init_status = eeprom_close(eeprom_desc);
	if (init_status) {
		return init_status;
	}

	/* Initialize board IIO paramaters */
	init_status = board_iio_params_init(&p_iio_ad405x_dev[iio_init_params.nb_devs]);
	if (init_status) {
		return init_status;
	}

	iio_device_init_params[iio_init_params.nb_devs].name = "system_config";
	iio_device_init_params[iio_init_params.nb_devs].dev_descriptor =
		p_iio_ad405x_dev[iio_init_params.nb_devs];
	iio_init_params.nb_devs++;

	if ((init_status == 0) && (hw_mezzanine_is_valid)) {
		do {
			/* Initialize AD405X device and peripheral interface */
			init_status = ad405x_assign_device(dev_type,
							   &iio_device_init_params[iio_init_params.nb_devs].name);
			if (init_status) {
				break;
			}

			init_status = init_system_post_verification();
			if (init_status) {
				break;
			}

			init_status = ad405x_init(&p_ad405x_dev, ad405x_init_params);
			if (init_status) {
				break;
			}

			init_status = ad405x_set_gp_mode(p_ad405x_dev,
							 AD405X_GP_1,
							 AD405X_GP_MODE_DRDY);
			if (init_status) {
				ad405x_remove(p_ad405x_dev);
				break;
			}
#if (ADC_DATA_FORMAT == STRAIGHT_BINARY)
			init_status = ad405x_set_data_format(p_ad405x_dev, AD405X_STRAIGHT_BINARY);
#else
			init_status = ad405x_set_data_format(p_ad405x_dev, AD405X_TWOS_COMPLEMENT);
#endif
			if (init_status) {
				ad405x_remove(p_ad405x_dev);
				break;
			}

			init_status = init_pwm();
			if (init_status) {
				ad405x_remove(p_ad405x_dev);
				break;
			}

			if ((ad405x_interface_mode == I3C_INTR) ||
			    (ad405x_interface_mode == I3C_DMA)) {
				/*
				 * I3C generics do not have a config mode to fall to. So they must be in their
				 * configured operating mode at all time.
				 */
				init_status = ad405x_set_operation_mode(p_ad405x_dev, ad405x_operating_mode);
				if (init_status) {
					deinit_pwm();
					ad405x_remove(p_ad405x_dev);
					break;
				}
			}
			if (ad405x_operating_mode == AD405X_BURST_AVERAGING_MODE_OP) {
				calc_max_pwm_period(ADC_SAMPLE_RATE, 0, true);
			}


			init_status = iio_ad405x_init(&p_iio_ad405x_dev[iio_init_params.nb_devs]);
			if (init_status) {
				deinit_pwm();
				ad405x_remove(p_ad405x_dev);
				break;
			}

			iio_device_init_params[iio_init_params.nb_devs].dev = p_ad405x_dev;
			iio_device_init_params[iio_init_params.nb_devs].dev_descriptor =
				p_iio_ad405x_dev[iio_init_params.nb_devs];
			iio_device_init_params[iio_init_params.nb_devs].raw_buf = adc_data_buffer;

			if (APP_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE) {
				iio_device_init_params[iio_init_params.nb_devs].raw_buf_len =
					DATA_BUFFER_SIZE_CONT;
			} else if (ad405x_interface_mode == I3C_DMA) {
				/*
				 * AD406x devices (I3C devices) start a transaction when the CONV_READ
				 * register is read. So everytime we read ADC data it is the result of
				 * previous conversion. In windowed mode of data capture this would
				 * create a gap between the 1st and the 2nd data. To remove this break in
				 * continuity of the data, 1 extra sample is reserved in the beginning of
				 * adc_data_buffer and is used by I3C to read an extra sample.
				 * So effectively for a request of N samples from the application the
				 * firmware reads 1+N samples and drops the 1st sample.
				 */
				iio_device_init_params[iio_init_params.nb_devs].raw_buf = (int8_t *)(
							adc_data_buffer + bytes_per_sample);
				iio_device_init_params[iio_init_params.nb_devs].raw_buf_len = DATA_BUFFER_SIZE;
			} else {
				iio_device_init_params[iio_init_params.nb_devs].raw_buf_len = DATA_BUFFER_SIZE;
			}

#if (APP_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
			if ((ad405x_interface_mode == SPI_INTR)
			    || (ad405x_interface_mode == I3C_INTR)) {
				iio_device_init_params[iio_init_params.nb_devs].trigger_id = "trigger0";
				iio_init_params.nb_trigs++;
				iio_init_params.trigs = &iio_trigger_init_params;
			}
#endif

			iio_init_params.nb_devs++;
		} while (false);
	}

	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_iio_com_desc;
	iio_init_params.devs = iio_device_init_params;
	init_status = iio_init(&p_ad405x_iio_desc, &iio_init_params);
	if (init_status) {
		goto iio_fail;
	}

	if ((APP_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE) &&
	    ((ad405x_interface_mode == SPI_INTR) || (ad405x_interface_mode == I3C_INTR))) {
		init_status = ad405x_iio_trigger_param_init(&ad405x_hw_trig_desc);
		if (init_status) {
			return init_status;
		}
	}

	return 0;
iio_fail:
	/* Free the PWM descriptors */
	deinit_pwm();

	/* Free AD405x device descriptors */
	ad405x_remove(p_ad405x_dev);

	/* De-Initialize the IIO Parameters */
	iio_params_deinit();

	/* Remove the IIO context attributes */
	remove_iio_context_attributes(iio_init_params.ctx_attrs);

	/* Remove IIO */
	iio_remove(p_ad405x_iio_desc);

	return init_status;
}

/**
 * @brief 	Run the AD405X IIO event handler
 * @return	none
 * @details	This function monitors the new IIO client event
 */
void iio_app_event_handler(void)
{
	if (restart_iio_flag) {
		/* Remove and free the pointers allocated during IIO init */
#if (APP_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		iio_hw_trig_remove(ad405x_hw_trig_desc);
#endif
		deinit_pwm();

		ad405x_remove(p_ad405x_dev);

		iio_params_deinit();

		remove_iio_context_attributes(iio_init_params.ctx_attrs);

		iio_remove(p_ad405x_iio_desc);

		/* Reset the restart_iio flag */
		restart_iio_flag = false;

		iio_app_initialize();
	}

#ifdef USE_VIRTUAL_COM_PORT
	ux_device_stack_tasks_run();
#endif
	iio_step(p_ad405x_iio_desc);
}
