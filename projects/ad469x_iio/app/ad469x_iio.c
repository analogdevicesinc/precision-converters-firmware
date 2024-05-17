/***************************************************************************//**
 *   @file    ad469x_iio.c
 *   @brief   Implementation of AD469x IIO application interfaces
 *   @details This module acts as an interface for AD469x IIO application
********************************************************************************
 * Copyright (c) 2021-24 Analog Devices, Inc.
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
#include <math.h>

#include "app_config.h"
#include "ad469x_iio.h"
#include "ad469x_support.h"
#include "ad469x_user_config.h"
#include "common.h"
#include "no_os_error.h"
#include "no_os_util.h"
#include "no_os_gpio.h"
#include "no_os_pwm.h"
#include "no_os_print_log.h"
#include "iio_trigger.h"

/******** Forward declaration of getter/setter functions ********/
static int ad469x_iio_attr_get(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv);

static int ad469x_iio_attr_set(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv);

static int ad469x_iio_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int ad469x_iio_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static void ad469x_update_scale(uint8_t ref_set);

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/
#define AD469X_CHN_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = ad469x_iio_attr_get,\
		.store = ad469x_iio_attr_set\
}

#define AD469X_CHN_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = ad469x_iio_attr_available_get,\
	.store = ad469x_iio_attr_available_set\
}

#define AD469X_IIO_CH(_name, _idx) {\
	.name = _name #_idx, \
	.ch_type = IIO_VOLTAGE,\
	.ch_out = false,\
	.indexed = true,\
	.channel = _idx,\
	.scan_index = _idx,\
	.scan_type = &ad469x_iio_scan_type,\
	.attributes = ad469x_iio_ch_attributes\
}

/* ADC data buffer size */
#if defined(USE_SDRAM)
#define adc_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(32768)		// 32kbytes
static int8_t adc_data_buffer[DATA_BUFFER_SIZE];
#endif

/*	Number of IIO devices */
#define NUM_OF_IIO_DEVICES	         1

/* IIO trigger name */
#define AD469x_IIO_TRIGGER_NAME		"ad469x_iio_trigger"

#define REGISTER_MAX_VAL  	 0x017F

/* Converts pwm period in nanoseconds to sampling frequency in samples per second */
#define PWM_PERIOD_TO_FREQUENCY(x)       (1000000000.0 / x)

/* Timeout count to avoid stuck into potential infinite loop while checking
 * for new data into an acquisition buffer. The actual timeout factor is determined
 * through 'sampling_frequency' attribute of IIO app, but this period here makes sure
 * we are not stuck into a forever loop in case data capture is interrupted
 * or failed in between.
 * Note: This timeout factor is dependent upon the MCU clock frequency. Below timeout
 * is tested for SDP-K1 platform @180Mhz default core clock */
#define BUF_READ_TIMEOUT	0xffffffff

/* Scale factor for gain correction */
#define AD469X_GAIN_CORR_SCALE(x)	(float)(x) / (float)(ADC_MAX_COUNT_BIPOLAR)

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* Pointer to the struct representing the AD469x IIO device */
struct ad469x_dev *p_ad469x_dev = NULL;

/* Variable to store the sampling rate */
static uint32_t ad469x_sampling_frequency = SAMPLING_RATE;

/* IIO interface descriptor */
static struct iio_desc *p_ad469x_iio_desc;

/* AD469x IIO device descriptor */
static struct iio_device *p_ad469x_iio_dev;

/* AD469x IIO hw trigger descriptor */
static struct iio_hw_trig *ad469x_hw_trig_desc;

/* Number of active channels in any data buffer read request */
static volatile uint8_t num_of_active_channels = 0;

/* Flag for checking the end of conversion in burst mode */
volatile bool ad469x_conversion_flag = false;

/* Flag to indicate data capture status */
static volatile bool start_data_capture = false;

/* Flag to indicate conversion mode status */
static volatile bool exit_conv_mode;

/* Variable to store number of requested samples */
static uint32_t nb_of_samples;

/* Variable to store start of buffer address */
volatile uint32_t *buff_start_addr;

/* Dummy tx data for Timer DMA */
static uint8_t data_tx = 0;

/* Flag to indicate if DMA has been configured for windowed capture */
volatile bool dma_config_updated = false;

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/* EVB HW validation status */
static bool hw_mezzanine_is_valid;

/* AD469X attribute unique IDs */
enum ad469x_attribute_ids {
	ADC_RAW,
	ADC_SCALE,
	ADC_OFFSET,
	ADC_REFERENCE_SEL,
	ADC_OFFSET_CORRECTION,
	ADC_GAIN_CORRECTION,
	ADC_AIN_HIGH_Z,

	ADC_SAMPLING_FREQUENCY,
};

/* IIOD channels configurations */
struct scan_type ad469x_iio_scan_type = {
#if (DEFAULT_POLARITY_MODE == PSEUDO_BIPOLAR_MODE)
	.sign = 's',
#else
	.sign = 'u',
#endif
	.realbits = CHN_STORAGE_BITS,
	.storagebits = ADC_RESOLUTION,
	.shift = 0,
#if (INTERFACE_MODE == SPI_DMA)
	.is_big_endian = true
#else
	.is_big_endian = false
#endif
};

/* AD469X device channel attributes list */
static struct iio_attribute ad469x_iio_ch_attributes[] = {
	AD469X_CHN_ATTR("raw", ADC_RAW),
	AD469X_CHN_ATTR("scale", ADC_SCALE),
	AD469X_CHN_ATTR("offset", ADC_OFFSET),
	AD469X_CHN_ATTR("offset_correction", ADC_OFFSET_CORRECTION),
	AD469X_CHN_ATTR("gain_correction", ADC_GAIN_CORRECTION),
	AD469X_CHN_ATTR("ain_high_z", ADC_AIN_HIGH_Z),
	AD469X_CHN_AVAIL_ATTR("ain_high_z_available", ADC_AIN_HIGH_Z),
	END_ATTRIBUTES_ARRAY,
};

/* AD469X device (global) attributes list */
static struct iio_attribute ad469x_iio_global_attributes[] = {
	AD469X_CHN_ATTR("sampling_frequency", ADC_SAMPLING_FREQUENCY),
	AD469X_CHN_ATTR("reference_sel", ADC_REFERENCE_SEL),
	AD469X_CHN_AVAIL_ATTR("reference_sel_available", ADC_REFERENCE_SEL),
	END_ATTRIBUTES_ARRAY,
};

static struct iio_channel ad469x_iio_channels[] = {
	/* 16-bit ADC Pseudo Differential Input Channels (Count= 8) */
	AD469X_IIO_CH("Chn", 0),
	AD469X_IIO_CH("Chn", 1),
	AD469X_IIO_CH("Chn", 2),
	AD469X_IIO_CH("Chn", 3),
	AD469X_IIO_CH("Chn", 4),
	AD469X_IIO_CH("Chn", 5),
	AD469X_IIO_CH("Chn", 6),
	AD469X_IIO_CH("Chn", 7),
#if defined(DEV_AD4696)
	AD469X_IIO_CH("Chn", 8),
	AD469X_IIO_CH("Chn", 9),
	AD469X_IIO_CH("Chn", 10),
	AD469X_IIO_CH("Chn", 11),
	AD469X_IIO_CH("Chn", 12),
	AD469X_IIO_CH("Chn", 13),
	AD469X_IIO_CH("Chn", 14),
	AD469X_IIO_CH("Chn", 15)
#endif
};

/* Scale value per channel */
static float ad469x_attr_scale_val;

/* AD469x IIOD debug attributes list */
static struct iio_attribute ad469x_debug_attributes[] = {
	END_ATTRIBUTES_ARRAY
};

/* Permissible values for Reference */
static char *ad469x_ref_sel[] = {
	"2P5V",
	"3P0V",
	"3P3V",
	"4P096V",
	"5V",
};

/* Values for status of analog input high Z Mode */
static char *ad469x_ain_high_z[] = {
	"disable",
	"enable"
};

/* Vref Values */
static float ad469x_vref_values[] = { 2.5, 3.0, 3.3, 4.096, 5.0 };

/* Offset correction value */
static uint32_t ad469x_offset_correction = 0;

/* Gain correction value */
static uint32_t ad469x_gain_correction = 0x8000;

/* Global Pointer for IIO Device Data */
volatile struct iio_device_data* global_iio_dev_data;

/* Global variable for number of samples */
uint32_t global_nb_of_samples;

/* Global variable for data read from CB functions */
int32_t data_read;

#if (INTERFACE_MODE == SPI_DMA)
/* STM32 SPI Init params */
struct stm32_spi_init_param* spi_init_param;

/* Rx DMA channel descriptor */
struct no_os_dma_ch* rxch;

/* Tx DMA channel descriptor */
struct no_os_dma_ch* txch;
#endif

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief Set the sampling rate and get the updated value
 *	  supported by MCU platform
 * @param sampling_rate[in,out] - Sampling rate value
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad469x_update_sampling_frequency(uint32_t* sampling_rate)
{
	int32_t ret;
	uint32_t pwm_period_ns;

	if (!sampling_rate) {
		return -EINVAL;
	}

	if (*sampling_rate > SAMPLING_RATE) {
		*sampling_rate = SAMPLING_RATE;
	}

#if (INTERFACE_MODE == SPI_DMA)
	cs_init_params.period_ns = CONV_TRIGGER_PERIOD_NSEC(ad469x_sampling_frequency);
	pwm_init_params.period_ns = CONV_TRIGGER_PERIOD_NSEC(ad469x_sampling_frequency);

	/* Initialize PWM with the updated rate */
	ret = init_pwm();
	if (ret) {
		return ret;
	}

	/* Get the actual period of the PWM */
	ret = no_os_pwm_get_period(pwm_desc, &pwm_period_ns);
	if (ret) {
		return ret;
	}

	ad469x_sampling_frequency = PWM_PERIOD_TO_FREQUENCY(pwm_period_ns);
#else
#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	ret = no_os_pwm_enable(pwm_desc);
	if (ret) {
		return ret;
	}
#endif

	ret = no_os_pwm_set_period(pwm_desc,
				   CONV_TRIGGER_PERIOD_NSEC(ad469x_sampling_frequency));
	if (ret) {
		return ret;
	}

#if (INTERFACE_MODE == SPI_INTERRUPT)
	ret = no_os_pwm_set_duty_cycle(pwm_desc,
				       CONV_TRIGGER_DUTY_CYCLE_NSEC(CONV_TRIGGER_PERIOD_NSEC(
						       ad469x_sampling_frequency)));
	if (ret) {
		return ret;
	}
#else
	ret = no_os_pwm_set_period(pwm_desc,
				   CONV_TRIGGER_PERIOD_NSEC(ad469x_sampling_frequency));
	if (ret) {
		return ret;
	}
#endif

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	ret = no_os_pwm_disable(pwm_desc);
	if (ret) {
		return ret;
	}
#endif
#endif // INTERFACE_MODE


	return ret;
}

/*!
 * @brief	Getter/Setter for the raw, offset and scale attribute value
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	Number of characters read/written
 */
static int ad469x_iio_attr_get(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	uint32_t adc_data_raw = 0;
	/* In pseudo-bipolar mode the offset is determined depending upon
	* the raw adc value. Hence it has been defined as static */
	static int32_t offset = 0;
	uint32_t pwm_period;
	int32_t ret;
	enum ad469x_ref_set ref_sel;
	uint8_t offset_corr_lsb;
	uint8_t offset_corr_msb;
	uint8_t gain_corr_lsb;
	uint8_t gain_corr_msb;
	float scale_value;
	enum ad469x_ain_high_z ain_high_z;

	if (buf == NULL) {
		return -ENOMEM;
	}

	switch (priv) {
	case ADC_RAW:
		ret = ad469x_read_single_sample(p_ad469x_dev, channel->ch_num, &adc_data_raw);
		if (ret) {
			return ret;
		}
#if (DEFAULT_POLARITY_MODE == PSEUDO_BIPOLAR_MODE)
		if (adc_data_raw >= ADC_MAX_COUNT_BIPOLAR) {
			offset = -ADC_MAX_COUNT_UNIPOLAR;
		} else {
			offset = 0;
		}

		/* Apply the offset correction value */
		offset += ad469x_offset_correction;
#endif
		return snprintf(buf, len, "%d", adc_data_raw);

	case ADC_SCALE:
		scale_value = ad469x_attr_scale_val * AD469X_GAIN_CORR_SCALE(
				      ad469x_gain_correction);

		return sprintf(buf, "%0.10f", scale_value);

	case ADC_OFFSET:
		return sprintf(buf, "%d", offset);

	case ADC_OFFSET_CORRECTION:
		/* Read the offset correction register LSB */
		ret = ad469x_spi_reg_read(p_ad469x_dev,
					  AD469x_REG_OFFSET_IN(channel->ch_num),
					  &offset_corr_lsb);
		if (ret) {
			return ret;
		}

		/* Read the offset correction register MSB */
		ret = ad469x_spi_reg_read(p_ad469x_dev,
					  AD469x_REG_OFFSET_IN(channel->ch_num) + 1,
					  &offset_corr_msb);
		if (ret) {
			return ret;
		}

		ad469x_offset_correction = offset_corr_lsb + ((uint16_t)offset_corr_msb << 8);

		return sprintf(buf, "%d", ad469x_offset_correction);

	case ADC_GAIN_CORRECTION:
		/* Read the gain correction register LSB */
		ret = ad469x_spi_reg_read(p_ad469x_dev,
					  AD469x_REG_GAIN_IN(channel->ch_num),
					  &gain_corr_lsb);
		if (ret) {
			return ret;
		}

		/* Read the gain correction register MSB */
		ret = ad469x_spi_reg_read(p_ad469x_dev,
					  AD469x_REG_GAIN_IN(channel->ch_num) + 1,
					  &gain_corr_msb);
		if (ret) {
			return ret;
		}

		ad469x_gain_correction = gain_corr_lsb + ((uint16_t)gain_corr_msb << 8);

		return sprintf(buf, "%d", ad469x_gain_correction);

	case ADC_SAMPLING_FREQUENCY:
		return snprintf(buf, len, "%ld", ad469x_sampling_frequency);

	case ADC_REFERENCE_SEL:
		ret = ad469x_get_reference(device, &ref_sel);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%s", ad469x_ref_sel[ref_sel]);

	case ADC_AIN_HIGH_Z:
		ret = ad469x_get_ain_high_z_status(device, channel->ch_num, &ain_high_z);
		if (ret) {
			return ret;
		}

		return sprintf(buf, "%s", ad469x_ain_high_z[ain_high_z]);

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Setter function for AD469X attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of expected bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int ad469x_iio_attr_set(void *device,
			       char *buf,
			       uint32_t len,
			       const struct iio_ch_info *channel,
			       intptr_t priv)
{
	int ret;
	uint8_t ref_sel;
	uint8_t offset_corr_lsb;
	uint8_t offset_corr_msb;
	uint8_t gain_corr_lsb;
	uint8_t gain_corr_msb;
	uint8_t ain_high_z;

	switch (priv) {
	/****************** ADC global setters ******************/

	/* These Attributes are only read only */
	case ADC_RAW:
	case ADC_OFFSET:
	case ADC_SCALE:
		break;
	case ADC_OFFSET_CORRECTION:
		ad469x_offset_correction = no_os_str_to_uint32(buf);
		offset_corr_lsb = ad469x_offset_correction;
		offset_corr_msb = ad469x_offset_correction >> 8;

		/* Update offset register LSB */
		ret = ad469x_spi_reg_write(p_ad469x_dev,
					   AD469x_REG_OFFSET_IN(channel->ch_num),
					   offset_corr_lsb);
		if (ret) {
			return ret;
		}

		/* Update offset register MSB */
		ret = ad469x_spi_reg_write(p_ad469x_dev,
					   AD469x_REG_OFFSET_IN(channel->ch_num) + 1,
					   offset_corr_msb);
		if (ret) {
			return ret;
		}

		break;

	case ADC_GAIN_CORRECTION:
		ad469x_gain_correction = no_os_str_to_uint32(buf);
		gain_corr_lsb = ad469x_gain_correction;
		gain_corr_msb = ad469x_gain_correction >> 8;

		/* Update offset register LSB */
		ret = ad469x_spi_reg_write(p_ad469x_dev,
					   AD469x_REG_GAIN_IN(channel->ch_num),
					   gain_corr_lsb);
		if (ret) {
			return ret;
		}

		/* Update offset register MSB */
		ret = ad469x_spi_reg_write(p_ad469x_dev,
					   AD469x_REG_GAIN_IN(channel->ch_num) + 1,
					   gain_corr_msb);
		if (ret) {
			return ret;
		}

		break;

	case ADC_SAMPLING_FREQUENCY:
		ad469x_sampling_frequency = no_os_str_to_uint32(buf);

		ret = ad469x_update_sampling_frequency(ad469x_sampling_frequency);
		if (ret) {
			return ret;
		}

		return len;

	case ADC_REFERENCE_SEL:
		for (ref_sel = AD469x_2P4_2P75; ref_sel <= AD469x_4P5_5P1; ref_sel++) {
			if (!strcmp(buf, ad469x_ref_sel[ref_sel])) {
				break;
			}
		}

		/* Set the value of selected reference */
		ret = ad469x_set_reference(device, ref_sel);
		if (ret) {
			return ret;
		}

		/* Update the scale value as per selected Vref */
		ad469x_update_scale(ref_sel);

		break;

	case ADC_AIN_HIGH_Z:
		for (ain_high_z = AD469x_AIN_HIGH_Z_DISABLE;
		     ain_high_z <= AD469x_AIN_HIGH_Z_ENABLE; ain_high_z++) {
			if (!strcmp(buf, ad469x_ain_high_z[ain_high_z])) {
				break;
			}
		}

		ret = ad469x_configure_ain_high_z(device, channel->ch_num, ain_high_z);
		if (ret) {
			return ret;
		}

		break;

	default:
		return -EINVAL;
	}

	return len;
}

/*!
 * @brief	Attribute available getter function for AD469X attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int ad469x_iio_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	switch (priv) {
	case ADC_REFERENCE_SEL:
		return sprintf(buf,
			       "%s %s %s %s %s",
			       ad469x_ref_sel[0],
			       ad469x_ref_sel[1],
			       ad469x_ref_sel[2],
			       ad469x_ref_sel[3],
			       ad469x_ref_sel[4]);

	case ADC_AIN_HIGH_Z:
		return sprintf(buf,
			       "%s %s",
			       ad469x_ain_high_z[0],
			       ad469x_ain_high_z[1]);

	default:
		break;
	}

	return len;
}

/*!
 * @brief	Attribute available setter function for AD469X attributes
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of success, negative error code otherwise
 */
static int ad469x_iio_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	return len;
}

/*!
 * @brief	Read the debug register value
 * @param	dev[in, out]- Pointer to IIO device instance
 * @param	reg[in]- Register address to read from
 * @param	readval[out]- Pointer to variable to read data into
 * @return	0 in case of success, negative value otherwise
 */
static int32_t ad469x_iio_debug_reg_read(void *dev,
		uint32_t reg,
		uint32_t *readval)
{
	int32_t ret;

	if (!readval || (reg > REGISTER_MAX_VAL)) {
		return -EINVAL;
	}

	ret = ad469x_spi_reg_read(p_ad469x_dev, reg, (uint8_t *)readval);
	if (NO_OS_IS_ERR_VALUE(ret)) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Write the debug register value
 * @param	dev[in, out]- Pointer to IIO device instance
 * @param	reg[in]- Register address to write
 * @param	writeval[out]- Variable storing data to write
 * @return	0 in case of success, negative value otherwise
 */
static int32_t ad469x_iio_debug_reg_write(void *dev,
		uint32_t reg,
		uint32_t writeval)
{
	int32_t ret;

	if (reg > REGISTER_MAX_VAL) {
		return -EINVAL;
	}

	ret = ad469x_spi_reg_write(p_ad469x_dev, reg, (uint8_t)writeval);
	if (NO_OS_IS_ERR_VALUE(ret)) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Start a data capture in continuous/burst mode
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad469x_adc_start_data_capture(void)
{
	int32_t ret;
	start_data_capture = true;
	exit_conv_mode = false;

#if (INTERFACE_MODE == SPI_INTERRUPT)
	ret = no_os_pwm_enable(pwm_desc);
	if (ret) {
		return ret;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = iio_trig_enable(ad469x_hw_trig_desc);
	if (ret) {
		return ret;
	}
#else
	ret = no_os_irq_enable(trigger_irq_desc, TRIGGER_INT_ID);
	if (ret) {
		return ret;
	}
#endif
#endif

	/* Enter into conversion mode */
	ret = ad469x_enter_conversion_mode(p_ad469x_dev);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Stop a data capture from continuous/burst mode
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad469x_adc_stop_data_capture(void)
{
	int32_t ret;
	uint32_t timeout = BUF_READ_TIMEOUT;
	start_data_capture = false;
	exit_conv_mode = true;

#if (INTERFACE_MODE == SPI_INTERRUPT)
	while (!exit_conv_mode && (timeout > 0)) {
		timeout--;
	};

	if (timeout == 0) {
		/* This returns the empty buffer */
		return -EIO;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Disable the interrupt */
	ret = iio_trig_enable(ad469x_hw_trig_desc);
	if (ret) {
		return ret;
	}
#else
	ret = no_os_irq_disable(trigger_irq_desc, TRIGGER_INT_ID);
	if (ret) {
		return ret;
	}
#endif

	/* Stop Generating PWM signal */
	ret = no_os_pwm_disable(pwm_desc);
	if (ret) {
		return ret;
	}
#endif
#if (INTERFACE_MODE == SPI_DMA)
	/* Stop timers */
	stm32_timer_stop();

	/* Abort DMA Transfers */
	stm32_abort_dma_transfer();

	/* Configure CS and CNV back to GPIO Mode */
	stm32_cs_output_gpio_config(true);
	stm32_cnv_output_gpio_config(true);

	spi_init_param = ad469x_init_str.spi_init->extra;
	spi_init_param->dma_init = NULL;

	/* Init SPI Interface in normal mode (Non DMA) */
	ret = no_os_spi_init(&p_ad469x_dev->spi_desc, ad469x_init_str.spi_init);
	if (ret) {
		return ret;
	}

	/* The exit command word needs to be
	 * sent after the conversion pulse.
	 */
	ret = ad469x_trigger_conversion(p_ad469x_dev);
	if (ret) {
		return ret;
	}

	ret = ad469x_exit_conversion_mode(p_ad469x_dev);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief  Prepares the device for data transfer.
 * @param  dev[in, out]- Application descriptor.
 * @param  mask[in]- Number of bytes to transfer.
 * @return 0 in case of success, error code otherwise.
 */
static int32_t ad469x_iio_prepare_transfer(void *dev, uint32_t mask)
{
	uint32_t ch_mask = 0x1;
	uint8_t chn;
	int32_t ret;
	buf_size_updated = false;
	num_of_active_channels = 0;

	/* Reset the lower byte of the standard sequencer configuration register*/
	ret = ad469x_spi_reg_write(p_ad469x_dev,
				   AD469x_REG_SEQ_LB,
				   AD469x_SEQ_CHANNELS_RESET);
	if (ret) {
		return ret;
	}

	/* Reset the upper byte of the standard sequencer configuration register*/
	ret = ad469x_spi_reg_write(p_ad469x_dev,
				   AD469x_REG_SEQ_UB,
				   AD469x_SEQ_CHANNELS_RESET);
	if (ret) {
		return ret;
	}

	/* Write the lower byte of the channel mask to the lower byte
	* of the standard sequencer configuration register
	* */
	ret = ad469x_spi_reg_write(p_ad469x_dev,
				   AD469x_REG_SEQ_LB,
				   AD469x_SEQ_LB_CONFIG(mask));
	if (ret) {
		return ret;
	}

	/* Write the upper byte of the channel mask to the upper byte
	 * of the standard sequencer configuration register
	 * */
	ret = ad469x_spi_reg_write(p_ad469x_dev,
				   AD469x_REG_SEQ_UB,
				   AD469x_SEQ_UB_CONFIG(mask));
	if (ret) {
		return ret;
	}

	/* Updates the count of total number of active channels */
	for (chn = 0; chn < NO_OF_CHANNELS; chn++) {
		if (mask & ch_mask) {
			num_of_active_channels++;
		}
		ch_mask <<= 1;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE) \
	|| (INTERFACE_MODE == SPI_INTERRUPT)
	ret = ad469x_adc_start_data_capture();
	if (ret) {
		return ret;
	}
#endif
#if (INTERFACE_MODE == SPI_DMA)
	ret = ad469x_enter_conversion_mode(p_ad469x_dev);
	if (ret) {
		return ret;
	}

	spi_init_param = ad469x_init_str.spi_init->extra;
	spi_init_param->pwm_init = &cs_init_params;
	spi_init_param->dma_init = &ad469x_dma_init_param;

	rxch = (struct no_os_dma_ch*)no_os_calloc(1, sizeof(*rxch));
	if (!rxch)
		return -ENOMEM;

	txch = (struct no_os_dma_ch*)no_os_calloc(1, sizeof(*txch));
	if (!txch)
		return -ENOMEM;

	rxch->irq_num = Rx_DMA_IRQ_ID;
	rxch->extra = &rxdma_channel;
	txch->extra = &txdma_channel;

	spi_init_param->rxdma_ch = rxch;
	spi_init_param->txdma_ch = txch;

	/* Init SPI interface in DMA Mode */
	ret = no_os_spi_init(&p_ad469x_dev->spi_desc, ad469x_init_str.spi_init);
	if (ret) {
		return ret;
	}

	/* Configure CS and CNV in Alternate function Mode */
	stm32_cs_output_gpio_config(false);
	stm32_cnv_output_gpio_config(false);

	/* Init PWM */
	ret = init_pwm();
	if (ret) {
		return ret;
	}

	/* Configure Timer 1 parameters */
	tim1_config();
#endif

	return 0;
}

/**
 * @brief  Terminate current data transfer
 * @param  dev[in, out]- Application descriptor.
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad469x_iio_end_transfer(void *dev)
{
	int32_t ret;

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE) \
	|| (INTERFACE_MODE == SPI_DMA)

	ret = ad469x_adc_stop_data_capture();
	if (ret) {
		return ret ;
	}

	buf_size_updated = false;
	dma_config_updated = false;
#endif

	return 0;
}

/**
 * @brief Push data into IIO buffer when trigger handler IRQ is invoked
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
int32_t ad469x_trigger_handler(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint8_t adc_data[2] = { 0 };

	if (start_data_capture) {
		if (!buf_size_updated) {
			/* Update total buffer size according to bytes per scan for proper
			 * alignment of multi-channel IIO buffer data */
			iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
							   iio_dev_data->buffer->bytes_per_scan)) * iio_dev_data->buffer->bytes_per_scan;
			buf_size_updated = true;
		}

		/* Read the sample for channel which has been sampled recently */
		ret = no_os_spi_write_and_read(p_ad469x_dev->spi_desc,
					       adc_data, BYTES_PER_SAMPLE);
		if (ret) {
			return -EIO;
		}

		no_os_swap(adc_data[0], adc_data[1]);

		return no_os_cb_write(iio_dev_data->buffer->buf,
				      adc_data,
				      BYTES_PER_SAMPLE);
	} else {
		/* Enter into register mode or exit from conversion mode */
		ad469x_exit_conversion_mode(p_ad469x_dev);
		exit_conv_mode = true;
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
	ad469x_conversion_flag = true;

	if (!start_data_capture) {
		/* Enter into register mode or exit from conversion mode */
		ad469x_exit_conversion_mode(p_ad469x_dev);
		exit_conv_mode = true;
	}
}

/**
 * @brief Read buffer data corresponding to AD469x IIO device.
 * @param [in, out] iio_dev_data - Device descriptor.
 * @return Number of samples read.
 */
static int32_t ad469x_iio_submit_samples(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t timeout = BUF_READ_TIMEOUT;
	uint32_t sample_index = 0;
	uint8_t adc_sample[2] = { 0 };
	int32_t data_read;
	ad469x_conversion_flag = false;
	uint16_t local_tx_data = 0;
	nb_of_samples = iio_dev_data->buffer->size / BYTES_PER_SAMPLE;

#if (INTERFACE_MODE == SPI_DMA)
	/* STM32 SPI Descriptor */
	struct stm32_spi_desc* sdesc = p_ad469x_dev->spi_desc->extra;

	/* SPI Message */
	struct no_os_spi_msg ad469x_spi_msg = {
		.tx_buff = (uint32_t*)local_tx_data,
		.bytes_number = nb_of_samples * (BYTES_PER_SAMPLE)
	};
#endif // INTERFACE_MODE

	if (!iio_dev_data) {
		return -EINVAL;
	}

	global_nb_of_samples  = nb_of_samples;
	global_iio_dev_data = iio_dev_data;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

#if (INTERFACE_MODE == SPI_INTERRUPT)
	/* Start data capture */
	ret = ad469x_adc_start_data_capture();
	if (ret) {
		return ret;
	}

	while (sample_index < nb_of_samples) {
		/* Check for status of conversion flag */
		while (!ad469x_conversion_flag && timeout > 0) {
			timeout--;
		}

		if (timeout <= 0) {
			return -ETIMEDOUT;
		}

		ad469x_conversion_flag = false;

		/* Read data over spi interface (in continuous read mode) */
		ret = no_os_spi_write_and_read(p_ad469x_dev->spi_desc,
					       adc_sample,
					       BYTES_PER_SAMPLE);
		if (ret) {
			return -EIO;
		}

		no_os_swap(adc_sample[0], adc_sample[1]);

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
	ret = ad469x_adc_stop_data_capture();
	if (ret) {
		return ret;
	}
#else // SPI_DMA_MODE
#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	ret = no_os_cb_prepare_async_write(iio_dev_data->buffer->buf,
					   nb_of_samples * (BYTES_PER_SAMPLE), &buff_start_addr, &data_read);
	if (ret) {
		return ret;
	}

	if (!dma_config_updated) {
		ad469x_spi_msg.rx_buff = (uint32_t*)buff_start_addr;

		ret = no_os_spi_transfer_dma_async(p_ad469x_dev->spi_desc, &ad469x_spi_msg,
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
	}

	/* Enable Timers */
	stm32_timer_enable();

	while (ad469x_conversion_flag != true && timeout > 0) {
		timeout--;
	}

	if (!timeout) {
		return -EIO;
	}

	no_os_cb_end_async_write(iio_dev_data->buffer->buf);
#else
	if (!dma_config_updated) {
		ret = no_os_cb_prepare_async_write(iio_dev_data->buffer->buf,
						   nb_of_samples * (BYTES_PER_SAMPLE), &buff_start_addr, &data_read);
		if (ret) {
			return ret;
		}
		ad469x_spi_msg.rx_buff = (uint32_t*) buff_start_addr;

		ret = no_os_spi_transfer_dma_async(p_ad469x_dev->spi_desc, &ad469x_spi_msg,
						   1, NULL, NULL);
		if (ret) {
			return ret;
		}
		no_os_pwm_disable(sdesc->pwm_desc); // CS PWM
		htim2.Instance->CNT = 0;
		htim1.Instance->CNT = 0;
		dma_config_updated = true;
		/* Configure Tx trigger timer parameters */
		tim8_config();

		/* Enable timers */
		stm32_timer_enable();
	}

#endif
#endif

	return 0;
}

/*********************************************************
 *               IIO Attributes and Structures
 ********************************************************/
/**
 * @brief	Init for reading/writing and parameterization of a
 * 			AD469x IIO device
 * @param 	desc[in,out] - IIO device descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad469x_iio_init(struct iio_device **desc)
{
	struct iio_device *ad469x_iio_inst;

	ad469x_iio_inst = calloc(1, sizeof(struct iio_device));
	if (!ad469x_iio_inst) {
		return -EINVAL;
	}

	ad469x_iio_inst->num_ch = NO_OS_ARRAY_SIZE(ad469x_iio_channels);
	ad469x_iio_inst->channels = ad469x_iio_channels;
	ad469x_iio_inst->attributes = ad469x_iio_global_attributes;
	ad469x_iio_inst->debug_attributes = ad469x_debug_attributes;

	ad469x_iio_inst->submit = ad469x_iio_submit_samples;
	ad469x_iio_inst->pre_enable = ad469x_iio_prepare_transfer;
	ad469x_iio_inst->post_disable = ad469x_iio_end_transfer;
	ad469x_iio_inst->read_dev = NULL;
	ad469x_iio_inst->write_dev = NULL;
	ad469x_iio_inst->debug_reg_read = ad469x_iio_debug_reg_read;
	ad469x_iio_inst->debug_reg_write = ad469x_iio_debug_reg_write;
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ad469x_iio_inst->trigger_handler = ad469x_trigger_handler;
#endif

	*desc = ad469x_iio_inst;

	return 0;
}

/**
 * @brief	Initialization of AD469x IIO hardware trigger specific parameters
 * @param 	desc[in,out] - IIO hardware trigger descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t ad469x_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	int32_t ret;
	struct iio_hw_trig_init_param ad469x_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad469x_hw_trig_init_params.irq_id = TRIGGER_INT_ID;
	ad469x_hw_trig_init_params.name = AD469x_IIO_TRIGGER_NAME;
	ad469x_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_FALLING;
	ad469x_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad469x_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad469x_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad469x_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad469x_hw_trig_init_params.iio_desc = p_ad469x_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad469x_hw_trig_init_params);
	if (ret) {
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/**
 * @brief Release resources allocated for IIO device
 * @param desc[in] - IIO device descriptor
 * @return 0 in case of success, negative value otherwise
 */
static int32_t ad469x_iio_remove(struct iio_desc *desc)
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
 * @brief Update the scale value
 * @param ref_set[in] - Value of VREF_SET
 * @return None
 */
static void ad469x_update_scale(uint8_t ref_set)
{
	/* ADC Raw to Voltage conversion default scale factor for IIO client */
#if defined(PSEUDO_BIPOLAR_MODE)
	/* Device supports pseudo-bipolar mode only with INX- = Vref / 2 */
	ad469x_attr_scale_val = (((ad469x_vref_values[ref_set]  / 2) /
				  ADC_MAX_COUNT_BIPOLAR) * 1000);
#else
	ad469x_attr_scale_val = ((ad469x_vref_values[ref_set] / ADC_MAX_COUNT_UNIPOLAR)
				 * 1000);
#endif
}

/**
 * @brief Release resources allocated for IIO device
 * @param desc[in] - IIO device descriptor
 * @return 0 in case of success, negative value otherwise
 */
int32_t ad469x_iio_initialize(void)
{
	int32_t init_status;
	enum ad469x_ref_set ref_set; // Value of reference

#if (INTERFACE_MODE == SPI_INTERRUPT) && \
	(DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	static struct iio_trigger ad469x_iio_trig_desc = {
		.is_synchronous = true,
		.enable = NULL,
		.disable = NULL
	};

	static struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad469x_iio_trig_desc,
		.name = AD469x_IIO_TRIGGER_NAME,
	};
#endif

	/* IIO interface init parameters */
	struct iio_init_param  iio_init_params = {
		.phy_type = USE_UART,
#if (INTERFACE_MODE == SPI_INTERRUPT) && \
	(DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		.trigs = &iio_trigger_init_params,
#endif
	};

	/* IIOD init parameters */
	static struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES] = {
		{
#if (INTERFACE_MODE == SPI_INTERRUPT) && \
	(DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
			.trigger_id = "trigger0",
#endif
		}
	};

	/* Init the system peripherals */
	init_status = init_system();
	if (init_status) {
		return init_status;
	}

#if !defined(DEV_AD4696)
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
#endif
		/* Initialize AD469x device and peripheral interface */
		init_status = ad469x_init(&p_ad469x_dev, &ad469x_init_str);
		if (init_status) {
			return init_status;
		}

		/* Configures the polarity mode */
#if (DEFAULT_POLARITY_MODE == PSEUDO_BIPOLAR_MODE)
		init_status = ad469x_polarity_mode_select(p_ad469x_dev,
				AD469x_PSEUDO_BIPOLAR_MODE);
#else
		init_status = ad469x_polarity_mode_select(p_ad469x_dev,
				AD469x_UNIPOLAR_MODE);
#endif
		if (init_status) {
			return init_status;
		}

		/* Configure reference control register */
		init_status = ad469x_reference_config(p_ad469x_dev);
		if (init_status) {
			return init_status;
		}

#if !defined(DEV_AD4696)
		/* Configure the GP0 as the data ready pin */
		init_status = ad469x_set_busy(p_ad469x_dev, AD469x_busy_gp0);
		if (init_status) {
			return init_status;
		}
#endif

		/* Register and initialize the AD469x device into IIO interface */
		init_status = ad469x_iio_init(&p_ad469x_iio_dev);
		if (init_status) {
			return init_status;
		}

		/* Initialize the IIO interface */
		iio_device_init_params[0].name = ACTIVE_DEVICE_NAME;
		iio_device_init_params[0].raw_buf = adc_data_buffer;
		iio_device_init_params[0].raw_buf_len = DATA_BUFFER_SIZE;

		iio_device_init_params[0].dev = p_ad469x_dev;
		iio_device_init_params[0].dev_descriptor = p_ad469x_iio_dev;

		iio_init_params.nb_devs++;

#if (INTERFACE_MODE == SPI_INTERRUPT) && \
	(DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
		iio_init_params.nb_trigs++;
#endif

#if !defined(DEV_AD4696)
	}
#endif

	/* Initialize the IIO interface */
	iio_init_params.uart_desc = uart_iio_com_desc;
	iio_init_params.devs = iio_device_init_params;
	init_status = iio_init(&p_ad469x_iio_desc, &iio_init_params);
	if (init_status) {
		pr_err("IIO Init Failed");
		ad469x_iio_remove(p_ad469x_iio_desc);
		return -ENOSYS;
	}

#if (INTERFACE_MODE == SPI_INTERRUPT) && \
	(DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	init_status = ad469x_iio_trigger_param_init(&ad469x_hw_trig_desc);
	if (init_status) {
		return init_status;
	}
#endif

	init_status = init_pwm();
	if (init_status) {
		return init_status;
	}

	/* Read the value of Set reference */
	init_status = ad469x_get_reference(p_ad469x_dev, &ref_set);
	if (init_status) {
		return init_status;
	}

	/* Update scale */
	ad469x_update_scale(ref_set);

	return 0;
}

/**
 * @brief 	Run the ad469x IIO event handler
 * @return	None
 */
void ad469x_iio_event_handler(void)
{
	iio_step(p_ad469x_iio_desc);
}
