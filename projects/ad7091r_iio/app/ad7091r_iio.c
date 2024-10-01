/***************************************************************************//**
 *   @file    ad7091r_iio.c
 *   @brief   Implementation of AD7091R IIO application interfaces
********************************************************************************
 * Copyright (c) 2024 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "app_config.h"
#include "ad7091r_iio.h"
#include "ad7091r_user_config.h"
#include "ad7091r_support.h"
#include "common.h"
#include "iio_trigger.h"
#include "no_os_util.h"
#include "no_os_pwm.h"
#include "no_os_irq.h"

/******** Forward declaration of getter/setter functions ********/
static int ad7091r_iio_attr_get(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv);

static int ad7091r_iio_attr_set(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv);

static int ad7091r_iio_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int ad7091r_iio_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

/******************************************************************************/
/************************ Macros and Constants ********************************/
/******************************************************************************/

#define AD7091R_CHN_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = ad7091r_iio_attr_get,\
		.store = ad7091r_iio_attr_set\
}

#define AD7091R_CHN_AVAIL_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = ad7091r_iio_attr_available_get,\
		.store = ad7091r_iio_attr_available_set\
}

#define AD7091R_CH(_name, _dev, _idx, _type) {\
	.name = _name, \
	.ch_type = _type,\
	.ch_out = false,\
	.indexed = true,\
	.channel = _idx,\
	.scan_index = _idx,\
	.scan_type = &ad7091r_iio_scan_type,\
	.attributes = ad7091r_iio_ch_attributes[_dev]\
}

#define AD7091R_DEFAULT_CHN_SCAN {\
	.sign = 'u',\
	.realbits = ADC_RESOLUTION,\
	.storagebits = CHN_STORAGE_BITS,\
	.shift = 0,\
	.is_big_endian = true\
}

/*	Number of IIO devices */
#define NUM_OF_IIO_DEVICES	1

/* IIO trigger name */
#define AD7091R_IIO_TRIGGER_NAME "ad7091r_iio_trigger"

#define	BYTES_PER_SAMPLE	sizeof(uint16_t)

/* Number of data storage bits (needed for IIO client to send buffer of data) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

/* AD70901r register maximum value */
#define REGISTER_MAX_VAL AD7091R8_REG_CH_HYSTERESIS(7)

/* ADC data buffer size */
#if defined(USE_SDRAM)
#define adc_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(32768)		// 32kbytes
static uint8_t adc_data_buffer[DATA_BUFFER_SIZE] = {0};
#endif

/* Timeout count to avoid stuck into potential infinite loop while checking
 * for new data into an acquisition buffer. The actual timeout factor is determined
 * through 'sampling_frequency' attribute of IIO app, but this period here makes sure
 * we are not stuck into a forever loop in case data capture is interrupted
 * or failed in between.
 * Note: This timeout factor is dependent upon the MCU clock frequency. Below timeout
 * is tested for SDP-K1 platform @180Mhz default core clock */
#define BUF_READ_TIMEOUT 0xffffffff

/* Local buffer size */
#define MAX_LOCAL_BUF_SIZE	8000

/* Maximum value the DMA NDTR register can take */
#define MAX_DMA_NDTR		(no_os_min(65535, MAX_LOCAL_BUF_SIZE))

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* AD7091R device descriptor */
struct ad7091r8_dev *ad7091r_dev_desc;

/* AD7091R IIO descriptor */
static struct iio_desc *ad7091r_iio_desc;

/* AD7091R IIO hw trigger descriptor */
static struct iio_hw_trig *ad7091r_hw_trig_desc;

#if (INTERFACE_MODE == SPI_INTERRUPT) && \
	(DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
static struct iio_trigger ad7091r_iio_trig_desc = {
	.is_synchronous = true,
	.enable = NULL,
	.disable = NULL
};

static struct iio_trigger_init iio_trigger_init_params = {
	.descriptor = &ad7091r_iio_trig_desc,
	.name = AD7091R_IIO_TRIGGER_NAME,
};
#endif

/* IIO interface init parameters */
static struct iio_init_param iio_init_params = {
	.phy_type = USE_UART,
#if (INTERFACE_MODE == SPI_INTERRUPT) && \
	(DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	.trigs = &iio_trigger_init_params,
#endif
};

/* IIOD init parameters */
static struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES];

/* Attribute IDs */
enum ad7091r_iio_attr_id {
	// Channels attributes
	ADC_RAW,
	ADC_SCALE,
	ADC_OFFSET,
	ADC_LOW_LIMIT_REG,
	ADC_HIGH_LIMIT_REG,
	ADC_HYSTERESIS_REG,
	ADC_THRESHOLD_ALERT,
	NUM_OF_CHN_ATTR,

	// Device Attributes
	REFERENCE_SELECT,
	REFERENCE_IN_VOLTS,
	DEVICE_MODE,
	SAMPLING_FREQUENCY,
	ALERT_BUSY_GPO0_EN,
	ALERT_POL_OR_GPO0_VAL,
	NUM_OF_DEV_ATTR = ALERT_POL_OR_GPO0_VAL - NUM_OF_CHN_ATTR
};

/* IIO channels scan structure */
struct scan_type ad7091r_iio_scan_type = AD7091R_DEFAULT_CHN_SCAN;

/* Device mode select value string representation */
static const char *ad7091r_device_mode_sel[] = {
	"mode_0",
	"mode_1",
	"mode_2",
	"mode_3"
};

/* Alert Busy GPO0 select value string representation */
static const char *ad7091r_alert_bsy_gpo0_sel[] = {
	"gpo0",
	"alert",
	"busy"
};

/* Alert polarity or GPO0 value select value string representation */
static const char *ad7091r_alert_pol_sel[] = {
	"active_low_polarity_or_gpo0_low",
	"active_high_polarity_or_gpo0_high"
};

/* Thresold falling or rising alert value string representation */
static const char *ad7091r_thresh_val[] = {
	"no_alert",
	"high_alert",
	"low_alert"
};

/* Reference select string representation */
static const char* ad7091r_reference_sel[] = {
	"external_ref",
	"internal_ref"
};

/* IIO channels attributes list */
static struct iio_attribute
	ad7091r_iio_ch_attributes[NUM_OF_IIO_DEVICES][NUM_OF_CHN_ATTR + 2] = {
	{
		AD7091R_CHN_ATTR("raw", ADC_RAW),
		AD7091R_CHN_ATTR("scale", ADC_SCALE),
		AD7091R_CHN_ATTR("offset", ADC_OFFSET),
		AD7091R_CHN_ATTR("thresh_falling_value", ADC_LOW_LIMIT_REG),
		AD7091R_CHN_ATTR("thresh_rising_value", ADC_HIGH_LIMIT_REG),
		AD7091R_CHN_ATTR("thresh_either_hysteresis", ADC_HYSTERESIS_REG),
		AD7091R_CHN_ATTR("thresh_alert", ADC_THRESHOLD_ALERT),
		AD7091R_CHN_AVAIL_ATTR("thresh_alert_available", ADC_THRESHOLD_ALERT),

		END_ATTRIBUTES_ARRAY
	}
};

/* IIO global attributes list */
static struct iio_attribute
	ad7091r_iio_global_attributes[NUM_OF_IIO_DEVICES][NUM_OF_DEV_ATTR + 5] = {
	{
		AD7091R_CHN_ATTR("reference_sel", REFERENCE_SELECT),
		AD7091R_CHN_AVAIL_ATTR("reference_sel_available", REFERENCE_SELECT),
		AD7091R_CHN_ATTR("reference_value_volts", REFERENCE_IN_VOLTS),
		AD7091R_CHN_ATTR("device_mode", DEVICE_MODE),
		AD7091R_CHN_AVAIL_ATTR("device_mode_available", DEVICE_MODE),
		AD7091R_CHN_ATTR("sampling_frequency", SAMPLING_FREQUENCY),
		AD7091R_CHN_ATTR("alert_bsy_gpo0_en", ALERT_BUSY_GPO0_EN),
		AD7091R_CHN_AVAIL_ATTR("alert_bsy_gpo0_en_available", ALERT_BUSY_GPO0_EN),
		AD7091R_CHN_ATTR("alert_pol_or_gp0_value", ALERT_POL_OR_GPO0_VAL),
		AD7091R_CHN_AVAIL_ATTR("alert_pol_or_gp0_value_available", ALERT_POL_OR_GPO0_VAL),

		END_ATTRIBUTES_ARRAY
	}
};

/* IIO channels info */
static struct iio_channel
ad7091r_iio_channels[NUM_OF_IIO_DEVICES][AD7091R_NUM_CHANNELS(
			ACTIVE_DEVICE_ID)] = {
	{
		AD7091R_CH("Chn0", 0, 0, IIO_VOLTAGE),
		AD7091R_CH("Chn1", 0, 1, IIO_VOLTAGE),
		AD7091R_CH("Chn2", 0, 2, IIO_VOLTAGE),
		AD7091R_CH("Chn3", 0, 3, IIO_VOLTAGE),
		AD7091R_CH("Chn4", 0, 4, IIO_VOLTAGE),
		AD7091R_CH("Chn5", 0, 5, IIO_VOLTAGE),
		AD7091R_CH("Chn6", 0, 6, IIO_VOLTAGE),
		AD7091R_CH("Chn7", 0, 7, IIO_VOLTAGE)
	}
};

/* Offset attribute value */
static uint16_t offset = 0;

/* Variable to store the sampling rate */
static uint32_t sampling_frequency = MAX_SAMPLING_RATE;

/* Variable to store the GPO0 mode */
static enum ad7091r8_gpo0_mode gpo0_mode =
	AD7091R8_GPO0_ENABLED;

/* Variable to store the GPO0 value */
static bool gpo0_val = false;

/* Variable to store the internal reference status */
static bool is_int_ref = false;

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/* Flag for checking the end of conversion in burst mode */
volatile bool ad7091r_conversion_flag = false;

/* Variable to store number of requested samples */
static uint32_t nb_of_samples;

/* Global Pointer for IIO Device Data */
volatile struct iio_device_data* global_iio_dev_data;

/* Global variable for number of samples */
uint32_t global_nb_of_samples;

/* Global variable for data read from CB functions */
int32_t data_read;

/* Variable to store start of buffer address */
volatile uint32_t* buff_start_addr;

/* Flag to indicate if DMA has been configured for windowed capture */
volatile bool dma_config_updated = false;

#if (INTERFACE_MODE == SPI_DMA)
/* STM32 SPI Init params */
struct stm32_spi_init_param* spi_init_param;

/* Rx DMA channel descriptor */
struct no_os_dma_ch* rxch;

/* Tx DMA channel descriptor */
struct no_os_dma_ch* txch;

/* Local buffer */
uint8_t local_buf[MAX_LOCAL_BUF_SIZE];
#endif

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief Set the sampling rate and get the updated value supported by MCU platform
 * @param samplig_rate[in,out] - Sampling rate value
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad7091r_set_sampling_rate(uint32_t* sampling_rate)
{
	int32_t ret;
	uint32_t pwm_period_ns;
	uint32_t dcycle_ns;

	if (!sampling_rate) {
		return -EINVAL;
	}

	if (*sampling_rate > MAX_SAMPLING_RATE) {
		*sampling_rate = MAX_SAMPLING_RATE;
	}

#if (INTERFACE_MODE == SPI_DMA)
	cs_init_params.period_ns = CONV_TRIGGER_PERIOD_NSEC(*sampling_rate);
	pwm_init_params.period_ns = CONV_TRIGGER_PERIOD_NSEC(*sampling_rate);
	pwm_init_params.duty_cycle_ns = CONV_TRIGGER_PERIOD_NSEC(
						*sampling_rate) -
					PWM_DUTY_CYCLE_NSEC;
	ret = init_pwm_trigger();
	if (ret) {
		return ret;
	}
#else //SPI INTERRUPT
	ret = no_os_pwm_set_period(pwm_desc,
				   CONV_TRIGGER_PERIOD_NSEC(*sampling_rate));
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_set_duty_cycle(pwm_desc,
				       CONV_TRIGGER_PERIOD_NSEC(*sampling_rate) - PWM_DUTY_CYCLE_NSEC);
	if (ret) {
		return ret;
	}
#endif

	/* Get the updated value set by hardware */
	ret = no_os_pwm_get_period(pwm_desc, &pwm_period_ns);
	if (ret) {
		return ret;
	}

	/* Convert period (nsec) to frequency (hertz) */
	*sampling_rate = (1.0 / pwm_period_ns) * 1000000000;

	return 0;
}

/*!
 * @brief	Getter function for AD7091R attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of expected bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int ad7091r_iio_attr_get(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv)
{
	int ret;
	uint16_t read_val = 0;
	enum ad7091r8_alert_type alert;

	switch (priv) {
	case ADC_RAW:
		/* Use the HAL apis to toggle the CNV pin in case of STM32 platform to
		 * meet the pulse cnv timing requirements as per the datasheet */
		ret = ad7091r8_read_one_stm(channel->ch_num, &read_val);
		if (ret) {
			return ret;
		}
		read_val = no_os_field_get(AD7091R8_REG_RESULT_DATA_MASK, read_val);

		return sprintf(buf, "%d", read_val);

	case ADC_SCALE:
		return sprintf(buf, "%0.10f",
			       ((float)ad7091r_dev_desc->vref_mv) / ADC_MAX_COUNT);

	case ADC_OFFSET:
		return sprintf(buf, "%d", offset);

	case ADC_THRESHOLD_ALERT:
		ret = ad7091r8_get_alert(ad7091r_dev_desc, channel->ch_num, &alert);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%s", ad7091r_thresh_val[alert]);

	case ADC_LOW_LIMIT_REG:
		ret = ad7091r8_get_limit(ad7091r_dev_desc, AD7091R8_LOW_LIMIT, channel->ch_num,
					 &read_val);
		if (ret) {
			return ret;
		}

		read_val = no_os_field_get(AD7091R8_REG_RESULT_DATA_MASK, read_val);

		return sprintf(buf, "%d", read_val);

	case ADC_HIGH_LIMIT_REG:
		ret = ad7091r8_get_limit(ad7091r_dev_desc, AD7091R8_HIGH_LIMIT, channel->ch_num,
					 &read_val);
		if (ret) {
			return ret;
		}

		read_val = no_os_field_get(AD7091R8_REG_RESULT_DATA_MASK, read_val);

		return sprintf(buf, "%d", read_val);

	case ADC_HYSTERESIS_REG:
		ret = ad7091r8_get_limit(ad7091r_dev_desc, AD7091R8_HYSTERESIS, channel->ch_num,
					 &read_val);
		if (ret) {
			return ret;
		}

		read_val = no_os_field_get(AD7091R8_REG_RESULT_DATA_MASK, read_val);

		return sprintf(buf, "%d", read_val);

	case DEVICE_MODE:
		return sprintf(buf, "%s",
			       ad7091r_device_mode_sel[ad7091r_dev_desc->sleep_mode]);

	case SAMPLING_FREQUENCY:
		return sprintf(buf, "%lu", sampling_frequency);

	case ALERT_BUSY_GPO0_EN:
		read_val = no_os_hweight8(gpo0_mode);

		return sprintf(buf, "%s", ad7091r_alert_bsy_gpo0_sel[read_val]);

	case ALERT_POL_OR_GPO0_VAL:
		return sprintf(buf, "%s", ad7091r_alert_pol_sel[gpo0_val]);

	case REFERENCE_SELECT:
		return sprintf(buf, "%s", ad7091r_reference_sel[is_int_ref]);

	case REFERENCE_IN_VOLTS:
		return sprintf(buf, "%3.2fV", (float)ad7091r_dev_desc->vref_mv / 1000.0);

	default:
		return -EINVAL;
	}
}

/*!
 * @brief	Setter function for AD7091R attributes.
 * @param	device[in, out]- Pointer to IIO device instance.
 * @param	buf[in]- IIO input data buffer.
 * @param	len[in]- Number of expected bytes.
 * @param	channel[in] - input channel.
 * @param	priv[in] - Attribute private ID.
 * @return	len in case of success, negative error code otherwise.
 */
static int ad7091r_iio_attr_set(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv)
{
	int ret;
	uint8_t val = 0;
	float ref_val = 0;
	uint32_t write_val;

	switch (priv) {
	case ADC_RAW:
	case ADC_SCALE:
	case ADC_OFFSET:
	case ADC_THRESHOLD_ALERT:
	case SAMPLING_FREQUENCY:
		sampling_frequency = no_os_str_to_uint32(buf);

		ret = ad7091r_set_sampling_rate(&sampling_frequency);
		if (ret) {
			return ret;
		}

		break;

	case ADC_LOW_LIMIT_REG:
		write_val = no_os_str_to_uint32(buf);

		ret = ad7091r8_set_limit(ad7091r_dev_desc, AD7091R8_LOW_LIMIT, channel->ch_num,
					 write_val);
		if (ret) {
			return ret;
		}

		break;

	case ADC_HIGH_LIMIT_REG:
		write_val = no_os_str_to_uint32(buf);

		ret = ad7091r8_set_limit(ad7091r_dev_desc, AD7091R8_HIGH_LIMIT, channel->ch_num,
					 write_val);
		if (ret) {
			return ret;
		}

		break;

	case ADC_HYSTERESIS_REG:
		write_val = no_os_str_to_uint32(buf);

		ret = ad7091r8_set_limit(ad7091r_dev_desc, AD7091R8_HYSTERESIS, channel->ch_num,
					 write_val);
		if (ret) {
			return ret;
		}

		break;

	case DEVICE_MODE:
		for (val = AD7091R8_SLEEP_MODE_0; val <= AD7091R8_SLEEP_MODE_3; val++) {
			if (!strcmp(buf, ad7091r_device_mode_sel[val])) {
				break;
			}
		}

		ret = ad7091r8_set_sleep_mode(ad7091r_dev_desc, val);
		if (ret) {
			return ret;
		}

		break;

	case ALERT_BUSY_GPO0_EN:
		if (!strcmp(buf, ad7091r_alert_bsy_gpo0_sel[1])) {
			val = AD7091R8_GPO0_ALERT;
		} else if (!strcmp(buf, ad7091r_alert_bsy_gpo0_sel[2])) {
			val = AD7091R8_GPO0_BUSY;
		}

		ret = ad7091r8_set_gpo0_mode(ad7091r_dev_desc, val, true);
		if (ret) {
			return ret;
		}

		gpo0_mode = val;

		break;

	case ALERT_POL_OR_GPO0_VAL:
		if (!strcmp(buf, ad7091r_alert_pol_sel[1])) {
			gpo0_val = true;
		} else {
			gpo0_val = false;
		}
		ret = ad7091r8_set_port(ad7091r_dev_desc, AD7091R8_GPO0, gpo0_val);
		if (ret) {
			return ret;
		}

		break;

	case REFERENCE_SELECT:
		if (!strcmp(buf, ad7091r_reference_sel[1])) {
			/* Reference is internal on-chip driven fixed at 2.5V */
			ad7091r_dev_desc->vref_mv = ADC_INTERNAL_VREF_MV;

			ret = ad7091r8_set_sleep_mode(ad7091r_dev_desc, AD7091R8_SLEEP_MODE_1);
			if (ret) {
				return ret;
			}
			is_int_ref = true;
		} else {
			/* External Reference */
			ret = ad7091r8_set_sleep_mode(ad7091r_dev_desc, AD7091R8_SLEEP_MODE_0);
			if (ret) {
				return ret;
			}
			is_int_ref = false;
		}

		break;

	case REFERENCE_IN_VOLTS:
		if (!is_int_ref) {
			/* Configure reference value when using extrernal reference setting */
			ref_val = strtof(buf, NULL);
			if (ref_val < ADC_MIN_VREF || ref_val > ADC_VDD_V) {
				return -EINVAL;
			}
			ad7091r_dev_desc->vref_mv = ref_val * 1000;
		}

		break;

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Attribute available getter function for AD7091R attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of SUCCESS, negative error code otherwise
 */
static int ad7091r_iio_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	uint8_t val;
	buf[0] = '\0';

	switch (priv) {
	case ADC_THRESHOLD_ALERT:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad7091r_thresh_val); val++) {
			strcat(buf, ad7091r_thresh_val[val]);
			strcat(buf, " ");
		}
		break;

	case DEVICE_MODE:
		for (val = 0; val <= AD7091R8_SLEEP_MODE_3; val++) {
			strcat(buf, ad7091r_device_mode_sel[val]);
			strcat(buf, " ");
		}
		break;

	case ALERT_BUSY_GPO0_EN:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad7091r_alert_bsy_gpo0_sel); val++) {
			strcat(buf, ad7091r_alert_bsy_gpo0_sel[val]);
			strcat(buf, " ");
		}
		break;

	case ALERT_POL_OR_GPO0_VAL:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad7091r_alert_pol_sel); val++) {
			strcat(buf, ad7091r_alert_pol_sel[val]);
			strcat(buf, " ");
		}
		break;

	case REFERENCE_SELECT:
		for (val = 0; val < NO_OS_ARRAY_SIZE(ad7091r_reference_sel); val++) {
			strcat(buf, ad7091r_reference_sel[val]);
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
 * @brief	Attribute available setter function for AD7091R attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of SUCCESS, negative error code otherwise
 */
static int ad7091r_iio_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	return len;
}


/*!
 * @brief	Start the data capture
 * @return	0 in case of success, negative error code otherwise
 */
static int ad7091r_adc_start_data_capture(void)
{
	int ret;

#if (INTERFACE_MODE == SPI_INTERRUPT)
	/* Clear any pending interrupts occured from a spurious falling edge of
	 * Bsy pin during the configuration of ADC's GP0 register and channel
	 * sequencer update before enabling the trigger */
	ret = no_os_irq_clear_pending(trigger_irq_desc, TRIGGER_INT_ID);
	if (ret) {
		return ret;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = iio_trig_enable(ad7091r_hw_trig_desc);
	if (ret) {
		return ret;
	}
#else
	ret = no_os_irq_enable(trigger_irq_desc, TRIGGER_INT_ID);
	if (ret) {
		return ret;
	}
#endif

	ret = no_os_pwm_enable(pwm_desc);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/*!
 * @brief	Stop the data capture
 * @return	0 in case of success, negative error code otherwise
 */
static int ad7091r_adc_stop_data_capture(void)
{
	int ret;

#if (INTERFACE_MODE == SPI_INTERRUPT)
	/* Stop Generating PWM signal */
	ret = no_os_pwm_disable(pwm_desc);
	if (ret) {
		return ret;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Disable the interrupt */
	ret = iio_trig_disable(ad7091r_hw_trig_desc);
	if (ret) {
		return ret;
	}
#else
	ret = no_os_irq_disable(trigger_irq_desc, TRIGGER_INT_ID);
	if (ret) {
		return ret;
	}
#endif
#else //SPI_DMA
	/* Stop timers */
	stm32_timer_stop();

	/* Abort DMA Transfers */
	stm32_abort_dma_transfer();

	/* Configure CS and CNV back to GPIO Mode */
	stm32_cs_output_gpio_config(true);

	spi_init_param = ad7091r_init_params.spi_init->extra;
	spi_init_param->dma_init = NULL;

	/* Init SPI Interface in normal mode (Non DMA) */
	ret = no_os_spi_init(&ad7091r_dev_desc->spi_desc, ad7091r_init_params.spi_init);
	if (ret) {
		return ret;
	}
#endif //INTERFACE_MODE

	return 0;
}

/**
 * @brief  Prepares the device for data transfer.
 * @param  dev[in, out]- Application descriptor.
 * @param  mask[in]- Number of bytes to transfer.
 * @return 0 in case of success, error code otherwise.
 */
static int32_t ad7091r_iio_prepare_transfer(void *dev, uint32_t mask)
{
	int32_t ret;
	buf_size_updated = false;
	uint16_t read_val;

	/* Configure GP0 as busy pin */
	ret = ad7091r8_set_gpo0_mode(ad7091r_dev_desc, AD7091R8_GPO0_BUSY, true);
	if (ret) {
		return ret;
	}

	/* Make Busy active high */
	read_val = no_os_field_prep(REG_CONF_GPO0_MASK, 1);
	ret = ad7091r8_spi_write_mask(ad7091r_dev_desc, AD7091R8_REG_CONF,
				      REG_CONF_GPO0_MASK, read_val);
	if (ret) {
		return ret;
	}

	/* Write channel mask to the sequencer register */
	ret = ad7091r8_pulse_convst(dev);
	if (ret)
		return ret;

	ret = ad7091r8_spi_reg_write(ad7091r_dev_desc, AD7091R8_REG_CHANNEL, mask);
	if (ret) {
		return ret;
	}

	/* Perform a single dummy read as mentioned in the data sheet
	 * before initiating the data transfer (Latency = 1 sample) */
	ret = ad7091r8_sequenced_read(ad7091r_dev_desc, &read_val);
	if (ret) {
		return ret;
	}

	/* Reconfigure the CNV pin as PWM */
	ret = ad7091r_reconfig_conv(ad7091r_dev_desc, CNV_GPIO_OUTPUT);
	if (ret) {
		return ret;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE) \
	&& (INTERFACE_MODE == SPI_INTERRUPT)
	ret = ad7091r_adc_start_data_capture();
	if (ret) {
		return ret;
	}
#endif

#if (INTERFACE_MODE == SPI_DMA)
	spi_init_param = ad7091r_init_params.spi_init->extra;
	spi_init_param->pwm_init = &cs_init_params;
	spi_init_param->dma_init = &ad7091r_dma_init_param;

	rxch = (struct no_os_dma_ch*)no_os_calloc(1, sizeof(*rxch));
	if (!rxch) {
		return -ENOMEM;
	}

	txch = (struct no_os_dma_ch*)no_os_calloc(1, sizeof(*txch));
	if (!txch) {
		return -ENOMEM;
	}

	rxch->irq_num = Rx_DMA_IRQ_ID;
	rxch->extra = &rxdma_channel;
	txch->extra = &txdma_channel;

	spi_init_param->rxdma_ch = rxch;
	spi_init_param->txdma_ch = txch;

	/* Init SPI interface in DMA Mode */
	ret = no_os_spi_init(&ad7091r_dev_desc->spi_desc, ad7091r_init_params.spi_init);
	if (ret) {
		return ret;
	}

	/* Configure CS in Alternate function Mode */
	stm32_cs_output_gpio_config(false);

	/* Init PWM */
	ret = init_pwm_trigger();
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief  Terminate current data transfer
 * @param  dev[in, out]- Application descriptor.
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad7091r_iio_end_transfer(void *dev)
{
	int32_t ret;
	buf_size_updated = false;
	ad7091r_conversion_flag = false;
	dma_config_updated = false;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE) \
	|| (INTERFACE_MODE == SPI_DMA)
	ret = ad7091r_adc_stop_data_capture();
	if (ret) {
		return ret;
	}
#endif

	/* Reconfigure the conversion pin as GPIO for normal ADC conversion (non-PWM) */
	ret = ad7091r_reconfig_conv(ad7091r_dev_desc, CNV_PWM);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief Interrupt Service Routine to monitor end of conversion event.
 * @param context[in] - Callback context (unused)
 * @return none
 */
void burst_capture_callback(void *context)
{
	ad7091r_conversion_flag = true;
}

/**
 * @brief Push data into IIO buffer when trigger handler IRQ is invoked
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad7091r_trigger_handler(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint8_t adc_sample[2] = {0xf8, 0x00};

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
			 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
						   iio_dev_data->buffer->bytes_per_scan)) *
						  iio_dev_data->buffer->bytes_per_scan;
		buf_size_updated = true;
	}

	/* Read data over spi interface (in continuous read mode) */
	ret = no_os_spi_write_and_read(ad7091r_dev_desc->spi_desc,
				       adc_sample,
				       BYTES_PER_SAMPLE);
	if (ret) {
		return -EIO;
	}

	ret = no_os_cb_write(iio_dev_data->buffer->buf,
			     adc_sample,
			     BYTES_PER_SAMPLE);
	if (ret) {
		return -EIO;
	}

	return 0;
}

/**
 * @brief Read buffer data corresponding to AD7091R IIO device.
 * @param [in, out] iio_dev_data - Device descriptor.
 * @return Number of samples read.
 */
static int32_t ad7091r_iio_submit_samples(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t timeout = BUF_READ_TIMEOUT;
	uint32_t sample_index = 0;
	ad7091r_conversion_flag = false;
	uint8_t adc_sample[2] = {0};
	uint16_t local_tx_data = 0;
	uint32_t spirxdma_ndtr;

	if (!iio_dev_data) {
		return -EINVAL;
	}

	nb_of_samples = iio_dev_data->buffer->size / BYTES_PER_SAMPLE;
	global_nb_of_samples = nb_of_samples;
	global_iio_dev_data = iio_dev_data;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

#if (INTERFACE_MODE == SPI_INTERRUPT)
	/* Start data capture */
	ret = ad7091r_adc_start_data_capture();
	if (ret) {
		return ret;
	}

	while (sample_index < nb_of_samples) {
		/* Check for status of conversion flag */
		while (!ad7091r_conversion_flag && timeout > 0) {
			timeout--;
		}

		if (timeout == 0) {
			return -ETIMEDOUT;
		}

		ad7091r_conversion_flag = false;

		/* Read data over spi interface (in continuous read mode) */
		ret = no_os_spi_write_and_read(ad7091r_dev_desc->spi_desc,
					       adc_sample,
					       BYTES_PER_SAMPLE);
		if (ret) {
			return -EIO;
		}

		ret = no_os_cb_write(iio_dev_data->buffer->buf,
				     adc_sample,
				     BYTES_PER_SAMPLE);
		if (ret) {
			return -EIO;
		}

		sample_index++;
		memset(adc_sample, 0, BYTES_PER_SAMPLE);
	}

	/* Stop data capture */
	ret = ad7091r_adc_stop_data_capture();
	if (ret) {
		return ret;
	}
#else //SPI_DMA
	/* STM32 SPI Descriptor */
	struct stm32_spi_desc* sdesc = ad7091r_dev_desc->spi_desc->extra;

	/* SPI Message */
	struct no_os_spi_msg ad7091r_spi_msg;

	nb_of_samples *= BYTES_PER_SAMPLE;

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	ret = no_os_cb_prepare_async_write(iio_dev_data->buffer->buf,
					   nb_of_samples, &buff_start_addr, &data_read);
	if (ret) {
		return ret;
	}

	if (!dma_config_updated) {
		/* Cap SPI RX DMA NDTR to MAX_DMA_NDTR. */
		spirxdma_ndtr = no_os_min(MAX_DMA_NDTR, nb_of_samples);
		rxdma_ndtr = spirxdma_ndtr;

		/* Register half complete callback, for ping-pong buffers implementation. */
		HAL_DMA_RegisterCallback(&hdma_spi1_rx,
					 HAL_DMA_XFER_HALFCPLT_CB_ID,
					 halfcmplt_callback);

		/* Update the SPI message */
		ad7091r_spi_msg.tx_buff = (uint8_t*)local_tx_data;
		ad7091r_spi_msg.rx_buff = (uint8_t*)local_buf;
		ad7091r_spi_msg.bytes_number = spirxdma_ndtr;

		ret = no_os_spi_transfer_dma_async(ad7091r_dev_desc->spi_desc, &ad7091r_spi_msg,
						   1, receivecomplete_callback, NULL);
		if (ret) {
			return ret;
		}

		no_os_pwm_disable(sdesc->pwm_desc); // CS PWM
		htim2.Instance->CNT = 0;
		htim1.Instance->CNT = 0;
		dma_config_updated = true;

		/* Configure CS and Tx trigger timer parameters */
		tim8_config();
		tim2_config();
	}

	if (nb_of_samples == rxdma_ndtr) {
		dma_cycle_count = 1;
	} else {
		dma_cycle_count = ((nb_of_samples) / rxdma_ndtr) + 1;
	}

	/* Update buffer indices */
	update_buff(local_buf, buff_start_addr);

	/* Enable Timers */
	stm32_timer_enable();

	/* Check for status of conversion flag */
	while (!ad7091r_conversion_flag && timeout > 0) {
		timeout--;
	}

	if (timeout == 0) {
		return -ETIMEDOUT;
	}

	dma_config_updated = false;
	no_os_cb_end_async_write(iio_dev_data->buffer->buf);

#else //CONTINUOUS_MODE
	if (!dma_config_updated) {
		ret = no_os_cb_prepare_async_write(iio_dev_data->buffer->buf,
						   nb_of_samples, &buff_start_addr, &data_read);
		if (ret) {
			return ret;
		}
		ad7091r_spi_msg.tx_buff = (uint8_t*)local_tx_data;
		ad7091r_spi_msg.rx_buff = (uint8_t*)buff_start_addr;
		ad7091r_spi_msg.bytes_number = nb_of_samples;

		ret = no_os_spi_transfer_dma_async(ad7091r_dev_desc->spi_desc, &ad7091r_spi_msg,
						   1, receivecomplete_callback, NULL);
		if (ret) {
			return ret;
		}
		no_os_pwm_disable(sdesc->pwm_desc); // CS PWM
		htim2.Instance->CNT = 0;
		htim1.Instance->CNT = 0;
		dma_config_updated = true;
		/* Configure Tx trigger timer parameters */
		tim8_config();
		tim2_config();

		/* Enable timers */
		stm32_timer_enable();
	}
#endif //DATA_CAPTURE MODE
#endif //INTERFACE_MODE

	return 0;
}

/*!
 * @brief	Read the debug register value.
 * @param	dev[in, out]- Pointer to IIO device instance.
 * @param	reg[in]- Register address to read from.
 * @param	readval[out]- Pointer to variable to read data into.
 * @return	0 in case of success, negative value otherwise.
 */
static int32_t ad7091r_iio_debug_reg_read(void *dev,
		uint32_t reg,
		uint32_t *readval)
{
	int32_t ret;

	if (!readval || (reg > REGISTER_MAX_VAL)) {
		return -EINVAL;
	}

	ret = ad7091r8_spi_reg_read(ad7091r_dev_desc, reg, (uint16_t *)readval);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Write the debug register value.
 * @param	dev[in, out]- Pointer to IIO device instance.
 * @param	reg[in]- Register address to write data into.
 * @param	writeval[out]- Pointer to variable to write data into.
 * @return	0 in case of success, negative value otherwise.
 */
static int32_t ad7091r_iio_debug_reg_write(void *dev,
		uint32_t reg,
		uint32_t writeval)
{
	int32_t ret;

	if (reg > REGISTER_MAX_VAL) {
		return -EINVAL;
	}

	ret = ad7091r8_spi_reg_write(ad7091r_dev_desc, reg, writeval);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
* @brief	Init for reading/writing and parametrization of an
* 			AD7091R IIO device.
* @param 	desc[in,out] - IIO device descriptor.
* @return	0 in case of success, negative error code otherwise.
*/
static int ad7091r_iio_param_init(struct iio_device **desc,
				  uint8_t dev_indx)
{
	struct iio_device *ad7091r_iio_inst;

	if (!desc) {
		return -EINVAL;
	}

	ad7091r_iio_inst = calloc(1, sizeof(struct iio_device));
	if (!ad7091r_iio_inst) {
		return -ENOMEM;
	}

	ad7091r_iio_inst->num_ch = NO_OS_ARRAY_SIZE(ad7091r_iio_channels[dev_indx]);
	ad7091r_iio_inst->channels = ad7091r_iio_channels[dev_indx];
	ad7091r_iio_inst->attributes = ad7091r_iio_global_attributes[dev_indx];
	ad7091r_iio_inst->debug_attributes = NULL;

	ad7091r_iio_inst->submit = ad7091r_iio_submit_samples;
	ad7091r_iio_inst->pre_enable = ad7091r_iio_prepare_transfer;
	ad7091r_iio_inst->post_disable = ad7091r_iio_end_transfer;
	ad7091r_iio_inst->debug_reg_read = ad7091r_iio_debug_reg_read;
	ad7091r_iio_inst->debug_reg_write = ad7091r_iio_debug_reg_write;
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE) \
	&& (INTERFACE_MODE == SPI_INTERRUPT)
	ad7091r_iio_inst->trigger_handler = ad7091r_trigger_handler;
#endif

	*desc = ad7091r_iio_inst;

	return 0;
}

/**
 * @brief	Initialization of AD7091r IIO hardware trigger specific parameters
 * @param 	desc[in,out] - IIO hardware trigger descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int ad7091r_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	int ret;
	struct iio_hw_trig_init_param ad7091r_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad7091r_hw_trig_init_params.irq_id = TRIGGER_INT_ID;
	ad7091r_hw_trig_init_params.name = AD7091R_IIO_TRIGGER_NAME;
	ad7091r_hw_trig_init_params.irq_trig_lvl =
		NO_OS_IRQ_EDGE_FALLING;
	ad7091r_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad7091r_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad7091r_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad7091r_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad7091r_hw_trig_init_params.iio_desc = ad7091r_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad7091r_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/**
 * @brief	Initialize the IIO interface for AD7091R IIO device.
 * @return	0 in case of success, negative error code otherwise.
 */
int ad7091r_iio_init()
{
	int ret;

	/* IIO device descriptor */
	struct iio_device *ad7091r_iio_dev[NUM_OF_IIO_DEVICES];

	/* Initialize the system peripherals */
	ret = init_system();
	if (ret) {
		return ret;
	}

	/* Read context attributes */
	ret = get_iio_context_attributes(&iio_init_params.ctx_attrs,
					 &iio_init_params.nb_ctx_attr,
					 eeprom_desc,
					 HW_MEZZANINE_NAME,
					 STR(HW_CARRIER_NAME),
					 &hw_mezzanine_is_valid);
	if (ret) {
		return ret;
	}

	iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
	iio_device_init_params[0].raw_buf = adc_data_buffer;
	iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;

	if (hw_mezzanine_is_valid) {

		/* Initialize AD7091R no-os device driver interface */
		ret = ad7091r8_init(&ad7091r_dev_desc, &ad7091r_init_params);
		if (ret) {
			return ret;
		}

		if (ad7091r_dev_desc->sleep_mode == AD7091R8_SLEEP_MODE_1) {
			is_int_ref = true;
		}

		/* Initialize the AD7091R IIO app specific parameters */
		ret = ad7091r_iio_param_init(&ad7091r_iio_dev[0], 0);
		if (ret) {
			return ret;
		}
		iio_init_params.nb_devs++;

		/* AD7091R IIO device init parameters */
		iio_device_init_params[0].dev = ad7091r_dev_desc;
		iio_device_init_params[0].dev_descriptor = ad7091r_iio_dev[0];
#if (INTERFACE_MODE == SPI_INTERRUPT) && \
	(DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		iio_device_init_params[0].trigger_id = "trigger0";
		iio_init_params.nb_trigs++;
#endif
	}

	/* Initialize the IIO interface */
	iio_init_params.devs = iio_device_init_params;
	iio_init_params.uart_desc = uart_iio_com_desc;
	ret = iio_init(&ad7091r_iio_desc, &iio_init_params);
	if (ret) {
		return ret;
	}

#if (INTERFACE_MODE == SPI_INTERRUPT) && \
	(DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = ad7091r_iio_trigger_param_init(&ad7091r_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	/* Initialize the PWM interface */
	ret = init_pwm_trigger();
	if (ret) {
		return ret;
	}

	/* Reinitialize the CNV pin as GPIO output (non-PWM) */
	ret = ad7091r_reconfig_conv(ad7091r_dev_desc, CNV_PWM);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief 	Run the AD7091R IIO event handler.
 * @return	none.
 * @details	This function monitors the new IIO client event.
 */
void ad7091r_iio_event_handler(void)
{
	iio_step(ad7091r_iio_desc);
}