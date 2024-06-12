/***************************************************************************//**
 *   @file    ad355xr_iio.c
 *   @brief   AD355XR IIO application interface module
********************************************************************************
 * Copyright (c) 2023-2024 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "app_config.h"
#include "ad355xr_iio.h"
#include "ad355xr_regs.h"
#include "ad355xr_user_config.h"
#include "ad355xr_support.h"
#include "no_os_error.h"
#include "no_os_util.h"
#include "no_os_gpio.h"
#include "iio_trigger.h"

/******** Forward declaration of getter/setter functions ********/
static int ad355xr_iio_attr_get(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv);

static int ad355xr_iio_attr_set(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv);

static int ad355xr_iio_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

static int ad355xr_iio_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv);

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

#define AD355XR_CHN_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = ad355xr_iio_attr_get,\
		.store = ad355xr_iio_attr_set\
}

#define AD355XR_CHN_AVAIL_ATTR(_name, _priv) {\
	.name = _name,\
	.priv = _priv,\
	.show = ad355xr_iio_attr_available_get,\
	.store = ad355xr_iio_attr_available_set\
}

#define AD355XR_CH(_name, _idx, _type) {\
	.name = _name, \
	.ch_type = _type,\
	.ch_out = true,\
	.indexed = true,\
	.channel = _idx,\
	.scan_index = _idx,\
	.scan_type = &ad355xr_iio_scan_type,\
	.attributes = ad355xr_iio_ch_attributes\
}

/*	Number of IIO devices */
#define NUM_OF_IIO_DEVICES	1

/* IIO trigger name */
#define AD355XR_IIO_TRIGGER_NAME "ad355xr_iio_trigger"

/* Bytes per sample. This count should divide the total 256 bytes into 'n' equivalent
 * DAC samples as IIO library requests only 256 bytes of data at a time in a given
 * data read query.
 * For 1 to 8-bit DAC, bytes per sample = 1 (2^0)
 * For 9 to 16-bit DAC, bytes per sample = 2 (2^1)
 * For 17 to 32-bit DAC, bytes per sample = 4 (2^2)
 **/
#define	BYTES_PER_SAMPLE	sizeof(uint16_t)	// For DAC resolution of 16-bits

/* Number of data storage bits (needed for IIO client to send buffer of data) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

/* DAC data buffer size */
#if defined(USE_SDRAM)
#define dac_data_buffer				SDRAM_START_ADDRESS
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#define DATA_BUFFER_SIZE			(32768)		// 32kbytes
static int8_t dac_data_buffer[DATA_BUFFER_SIZE] = { 0 };
#endif

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* Pointer to the structure representing the AD355xr IIO device */
struct ad3552r_desc *ad355xr_dev_inst = NULL;

/* ad355xr IIO interface descriptor */
static struct iio_desc *ad355xr_iio_desc;

/* AD355XR IIO hw trigger descriptor */
static struct iio_hw_trig *ad355xr_hw_trig_desc;

/* variable to store AD355XR sampling rate */
static int32_t ad355xr_sampling_rate = MAX_SAMPLING_RATE;

/* channel mask */
static uint32_t channel_mask;

/* Enabled channel number */
static uint8_t channel_num;

/* Number of channels enabled */
static uint8_t num_channels_en;

/* Flag to indicate if SPI DMA enabled */
volatile static bool spi_dma_enabled = false;

/* IIO attributes ID */
enum ad355xr_attribute_id {
	DAC_RAW,
	DAC_SCALE,
	DAC_OFFSET,
	DAC_CH_ENABLE,
	DAC_CH_OUTPUT_RANGE,
	DAC_CH_MODE,
	DAC_VREF_VOLTAGE,
	DAC_SIMULTANEOUS_UPDATE,
	DAC_SAMPLING_FREQUENCY
};

/* IIO channels scan structure */
static struct scan_type ad355xr_iio_scan_type = {
	.sign = 'u',
	.realbits = DAC_RESOLUTION,
	.storagebits = CHN_STORAGE_BITS,
	.shift = 0,
	.is_big_endian = true
};

/* Channel output ranges */
static char *ad355xr_ch_output_range[] = {
	"output_range_0v_2.5v",
	"output_range_0v_5v",
	"output_range_0v_10v",
	"output_range_-5v_5v",
#if defined(DEV_AD3551R) || defined(DEV_AD3552R)
	"output_range_-10v_10v"
#else
	"output_range_-2.5v_7.5v"
#endif
};

/* Channel modes */
static char *ad355xr_ch_mode[] = {
	"fast_mode",
	"precision_mode"
};

/* DAC voltage references */
static char *ad355xr_vref_voltage[] = {
	"internal_vref_pin_floating",
	"internal_vref_pin_2.5v",
	"external_vref_pin_input"
};

/* IIO channels attributes list */
static struct iio_attribute ad355xr_iio_ch_attributes[] = {
	AD355XR_CHN_ATTR("raw", DAC_RAW),
	AD355XR_CHN_ATTR("scale", DAC_SCALE),
	AD355XR_CHN_ATTR("offset", DAC_OFFSET),
	AD355XR_CHN_ATTR("en", DAC_CH_ENABLE),
	AD355XR_CHN_ATTR("output_range", DAC_CH_OUTPUT_RANGE),
	AD355XR_CHN_AVAIL_ATTR("output_range_available", DAC_CH_OUTPUT_RANGE),
#if !defined(DEV_AD3542R_12)
	AD355XR_CHN_ATTR("mode", DAC_CH_MODE),
	AD355XR_CHN_AVAIL_ATTR("mode_available", DAC_CH_MODE),
#endif
	END_ATTRIBUTES_ARRAY
};

/* IIO global attributes list */
static struct iio_attribute ad355xr_iio_global_attributes[] = {
	AD355XR_CHN_ATTR("vref_voltage", DAC_VREF_VOLTAGE),
	AD355XR_CHN_AVAIL_ATTR("vref_voltage_available", DAC_VREF_VOLTAGE),
#if !defined(DEV_AD3541R) && !defined(DEV_AD3551R)
	AD355XR_CHN_ATTR("simultaneous_update", DAC_SIMULTANEOUS_UPDATE),
#endif
	AD355XR_CHN_ATTR("sampling_frequency", DAC_SAMPLING_FREQUENCY),
	END_ATTRIBUTES_ARRAY
};

/* IIO channels info */
static struct iio_channel ad355xr_iio_channels[] = {
	AD355XR_CH("Chn0", 0, IIO_VOLTAGE),
#if (NUMBER_OF_CHANNELS == 2)
	AD355XR_CH("Chn1", 1, IIO_VOLTAGE),
#endif
};

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief	Set the pwm frequency supported by MCU platform and
			update frequency variable
 * @param 	frequency[in,out] - frequency in hz
 * @param 	duty_cycle[in] - duty cycle value in percentage
 * @param	pwm_desc[in] - pwm descriptor
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t set_pwm_frequency(struct no_os_pwm_desc* pwm_desc,
				 uint32_t* frequency, uint32_t duty_cycle)
{
	uint32_t pwm_period_ns;
	int32_t ret;

	if (!frequency || !pwm_desc) {
		return -EINVAL;
	}

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	/* Enable PWM to get the PWM period (explicitly done for Mbed
	 * platform as it needs pwm to be enabled to update pwm period) */
	ret = no_os_pwm_enable(pwm_desc);
	if (ret) {
		return ret;
	}
#endif

	ret = no_os_pwm_set_period(pwm_desc, CONV_PERIOD_NSEC(*frequency));
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_set_duty_cycle(pwm_desc,
				       CONV_DUTY_CYCLE_NSEC(*frequency, duty_cycle));
	if (ret) {
		return ret;
	}

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	ret = no_os_pwm_disable(pwm_desc);
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
	*frequency = CONV_FREQUENCY_HZ(pwm_period_ns);

	return 0;
}

/*!
 * @brief	Getter functions for AD355XR attributes
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in,out]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	0 in case of success, negative error code otherwise
 */
static int ad355xr_iio_attr_get(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv)
{
	int32_t values[2];
	int32_t ret;
	uint16_t read_back_value;

	switch (priv) {
	case DAC_RAW:
		ret = ad3552r_get_ch_value(ad355xr_dev_inst, AD3552R_CH_CODE, channel->ch_num,
					   &read_back_value);
		if (ret) {
			return ret;
		}
		return sprintf(buf, "%u", read_back_value);

	case DAC_SCALE:
		ret = ad3552r_get_scale(ad355xr_dev_inst, channel->ch_num, &values[0],
					&values[1]);
		if (ret) {
			return ret;
		}
		return iio_format_value(buf, len, IIO_VAL_INT_PLUS_MICRO, 2, values);

	case DAC_OFFSET:
		ret = ad3552r_get_offset(ad355xr_dev_inst, channel->ch_num, &values[0],
					 &values[1]);
		if (ret) {
			return ret;
		}
		return iio_format_value(buf, len, IIO_VAL_INT_PLUS_MICRO, 2, values);

	case DAC_CH_ENABLE:
		ret = ad3552r_get_ch_value(ad355xr_dev_inst, AD3552R_CH_DAC_POWERDOWN,
					   channel->ch_num, &read_back_value);
		if (ret) {
			return ret;
		}
		return sprintf(buf, "%u", !read_back_value);

	case DAC_CH_OUTPUT_RANGE:
		ret = ad3552r_get_ch_value(ad355xr_dev_inst, AD3552R_CH_OUTPUT_RANGE_SEL,
					   channel->ch_num, &read_back_value);
		if (ret) {
			return ret;
		}
		return sprintf(buf, "%s", ad355xr_ch_output_range[read_back_value]);

	case DAC_CH_MODE:
		ret = ad3552r_get_ch_value(ad355xr_dev_inst, AD3552R_CH_FAST_EN,
					   channel->ch_num, &read_back_value);
		if (ret) {
			return ret;
		}

		if (read_back_value) {
			return sprintf(buf, "%s", ad355xr_ch_mode[0]);
		} else {
			return sprintf(buf, "%s", ad355xr_ch_mode[1]);
		}

	case DAC_VREF_VOLTAGE:
		ret = ad3552r_get_dev_value(ad355xr_dev_inst, AD3552R_VREF_SELECT,
					    &read_back_value);
		if (ret) {
			return ret;
		}
		return sprintf(buf, "%s", ad355xr_vref_voltage[read_back_value]);

	case DAC_SIMULTANEOUS_UPDATE:
		return sprintf(buf, "%u", ad355xr_dev_inst->is_simultaneous);

	case DAC_SAMPLING_FREQUENCY:
		return sprintf(buf, "%u", ad355xr_sampling_rate);

	default:
		return -EINVAL;
	}
}

/*!
 * @brief	Setter functions for AD355XR attributes
 * @param	device[in]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - Input channel
 * @param	priv[in] - Attribute private ID
 * @return	0 in case of success, negative error code otherwise
 */
static int ad355xr_iio_attr_set(void *device,
				char *buf,
				uint32_t len,
				const struct iio_ch_info *channel,
				intptr_t priv)
{
	int32_t ret, count;
	uint32_t write_value, value;

	write_value = no_os_str_to_uint32(buf);

	switch (priv) {
	case DAC_RAW:
		if (write_value > 0xFFFF) {
			return -EINVAL;
		}

		return ad3552r_set_ch_value(ad355xr_dev_inst, AD3552R_CH_CODE, channel->ch_num,
					    write_value);

	case DAC_SCALE:
	case DAC_OFFSET:
		break;

	case DAC_CH_ENABLE:
		if (!write_value) {
			if (ad355xr_dev_inst->chip_id == AD3541R_ID
			    || ad355xr_dev_inst->chip_id == AD3542R_ID) {
				ret = ad3552r_set_ch_value(ad355xr_dev_inst,
							   AD3552R_CH_AMPLIFIER_POWERDOWN,
							   channel->ch_num,
							   !write_value);
				if (ret) {
					return ret;
				}
			}
			ret = ad3552r_set_ch_value(ad355xr_dev_inst,
						   AD3552R_CH_DAC_POWERDOWN,
						   channel->ch_num,
						   !write_value);
			if (ret) {
				return ret;
			}
		} else {
			ret = ad3552r_set_ch_value(ad355xr_dev_inst,
						   AD3552R_CH_DAC_POWERDOWN,
						   channel->ch_num,
						   !write_value);
			if (ret) {
				return ret;
			}

			if (ad355xr_dev_inst->chip_id == AD3541R_ID
			    || ad355xr_dev_inst->chip_id == AD3542R_ID) {
				ret = ad3552r_set_ch_value(ad355xr_dev_inst,
							   AD3552R_CH_AMPLIFIER_POWERDOWN,
							   channel->ch_num,
							   !write_value);
				if (ret) {
					return ret;
				}
			}
		}
		break;

	case DAC_CH_OUTPUT_RANGE:
		for (count = 0; count <= AD3552R_CH_OUTPUT_RANGE_NEG_10__10V; ++count) {
			if (!strncmp(buf, ad355xr_ch_output_range[count], strlen(buf))) {
				break;
			}
		}
		return ad3552r_set_ch_value(ad355xr_dev_inst, AD3552R_CH_OUTPUT_RANGE_SEL,
					    channel->ch_num, count);

	case DAC_CH_MODE:
		if (!strncmp(buf, ad355xr_ch_mode[0], strlen(buf))) {
			value = 1;
		} else {
			value = 0;
		}
		return ad3552r_set_ch_value(ad355xr_dev_inst, AD3552R_CH_FAST_EN,
					    channel->ch_num, value);

	case DAC_VREF_VOLTAGE:
		for (count = 0; count <= AD3552R_EXTERNAL_VREF_PIN_INPUT; count++) {
			if (!strncmp(buf, ad355xr_vref_voltage[count], strlen(buf))) {
				break;
			}
		}
		return ad3552r_set_dev_value(ad355xr_dev_inst, AD3552R_VREF_SELECT, count);

	case DAC_SIMULTANEOUS_UPDATE:
		ad355xr_dev_inst->is_simultaneous = write_value;
		return ad3552r_simulatneous_update_enable(ad355xr_dev_inst);

	case DAC_SAMPLING_FREQUENCY:
		if (write_value > MAX_SAMPLING_RATE) {
			write_value = MAX_SAMPLING_RATE;
		}
		ad355xr_sampling_rate = write_value;
		ret = set_pwm_frequency(ldac_pwm_desc, &ad355xr_sampling_rate,
					LDAC_PWM_DUTY_CYCLE);
		if (ret) {
			return ret;
		}
		break;

	default:
		return -EINVAL;
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
static int ad355xr_iio_attr_available_get(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	switch (priv) {
	case DAC_CH_OUTPUT_RANGE:
		return sprintf(buf, "%s %s %s %s %s", ad355xr_ch_output_range[0],
			       ad355xr_ch_output_range[1], ad355xr_ch_output_range[2],
			       ad355xr_ch_output_range[3], ad355xr_ch_output_range[4]);

	case DAC_CH_MODE:
		return sprintf(buf, "%s %s", ad355xr_ch_mode[0], ad355xr_ch_mode[1]);

	case DAC_VREF_VOLTAGE:
		return sprintf(buf, "%s %s %s", ad355xr_vref_voltage[0],
			       ad355xr_vref_voltage[1], ad355xr_vref_voltage[2]);

	default:
		return -EINVAL;
	}
}

/*!
 * @brief	Attribute available setter function
 * @param	device[in, out]- Pointer to IIO device instance
 * @param	buf[in]- IIO input data buffer
 * @param	len[in]- Number of input bytes
 * @param	channel[in] - input channel
 * @param	priv[in] - Attribute private ID
 * @return	len in case of SUCCESS, negative error code otherwise
 */
static int ad355xr_iio_attr_available_set(void *device,
		char *buf,
		uint32_t len,
		const struct iio_ch_info *channel,
		intptr_t priv)
{
	return len;
}

/*!
 * @brief	Search the debug register address in registers array and check for validity
 * @param	addr[in]- Register address to search for
 * @return	base register address index in case of SUCCESS, negative error code otherwise
 */
static int32_t debug_reg_validate(uint32_t addr)
{
	uint32_t current_index;
	uint8_t reg_len;

	reg_len = ad3552r_reg_len(addr);
	for (current_index = 0;
	     ad355xr_regs[current_index] <= AD3552R_REG_ADDR_CH_INPUT_24B(1);
	     current_index++) {
		if (addr == ad355xr_regs[current_index]) {
			if (reg_len > 1) {
				return -EINVAL;
			} else {
				return current_index;
			}
		} else if (addr < ad355xr_regs[current_index]) {
			if (addr == ad355xr_regs[current_index] - reg_len + 1) {
				return current_index;
			} else {
				return -EINVAL;
			}
		}
	}
	return -EINVAL;
}

/**
 * @brief Read the debug register value
 * @param dev[in,out] - Pointer to IIO device descriptor
 * @param reg[in]- Address of the register to be read
 * @param readval[out]- Pointer to the register data variable
 * @return 0 in case of success, negative error code otherwise
 */

static int32_t ad355xr_iio_debug_reg_read(void *dev,
		uint32_t reg,
		uint32_t *readval)
{
	int32_t ret, base_addr_index;
	uint16_t read_value;
	uint8_t  base_addr;

	if (!readval || reg > AD3552R_REG_ADDR_CH_INPUT_24B(1)) {
		return -EINVAL;
	}

	base_addr_index = debug_reg_validate(reg);
	if (base_addr_index < 0) {
		return -EINVAL;
	}

	base_addr = ad355xr_regs[base_addr_index];
	ret = ad3552r_read_reg(ad355xr_dev_inst, base_addr, &read_value);
	if (ret) {
		return ret;
	}
	*readval = read_value & 0xFFFF;

	return 0;
}

/**
 * @brief Write value to the debug register
 * @param dev[in,out] Pointer to IIO device descriptor
 * @param reg[in] Address of the register where the data is to be written
 * @param writeval[in] Pointer to the register data variable
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad355xr_iio_debug_reg_write(void *dev,
		uint32_t reg,
		uint32_t writeval)
{
	int32_t ret, base_addr_index;
	uint8_t base_addr;

	base_addr_index = debug_reg_validate(reg);
	if (base_addr_index < 0) {
		return -EINVAL;
	}

	base_addr = ad355xr_regs[base_addr_index];
	ret = ad3552r_write_reg(ad355xr_dev_inst, base_addr, writeval);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Prepare for DAC data push
 * @param dev[in] - IIO device instance
 * @param chn_mask[in] - Channels select mask
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad355xr_iio_prepare_transfer(void *dev,
		uint32_t chn_mask)
{
	int32_t ret;

	if (!dev) {
		return -EINVAL;
	}

	channel_mask = chn_mask;

	/* Getting enabled channel number, If there is any one channel enabled.
	 * when both channel enabled, both the dac channels are updated in single
	 * SPI transaction.*/
	if (channel_mask == 0x1) {
		channel_num = 0;
		num_channels_en = 1;
	} else if (channel_mask == 0x2) {
		channel_num = 1;
		num_channels_en = 1;
	} else {
		num_channels_en = 2;
	}

#if (INTERFACE_MODE != SPI_DMA)
	/* Returning error when both channels are enabled and both are in different mode */
	if (channel_mask == AD3552R_MASK_ALL_CH &&
	    ad355xr_dev_inst->ch_data[0].fast_en != ad355xr_dev_inst->ch_data[1].fast_en) {
		return -EINVAL;
	}

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	ret = no_os_spi_remove(ad355xr_dev_inst->spi);
	if (ret) {
		return ret;
	}

	/* Initialize SPI without software csb to reduce SPI transaction time in trigger handler */
	ret = no_os_spi_init(&ad355xr_dev_inst->spi, &spi_init_params_without_sw_csb);
	if (ret) {
		return ret;
	}
#endif

	ret = iio_trig_enable(ad355xr_hw_trig_desc);
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_enable(ldac_pwm_desc);
	if (ret) {
		return ret;
	}
#else
	/* Returning error when enabled channel is in precision mode */
	if ((channel_mask == AD3552R_MASK_ALL_CH
	     && (!(ad355xr_dev_inst->ch_data[0].fast_en)
		 || !(ad355xr_dev_inst->ch_data[1].fast_en))) || (num_channels_en == 1
				 && !(ad355xr_dev_inst->ch_data[channel_num].fast_en))) {
		return -EINVAL;
	}

	ret = set_pwm_frequency(spi_dma_tx_stop_pwm_desc,
				&spi_dma_tx_stop_pwm_frquency[num_channels_en - 1],
				SPI_DMA_TX_STOP_PWM_DUTY_CYCLE);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief Perform tasks before end of current data transfer
 * @param dev[in] - IIO device instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad355xr_iio_end_transfer(void *dev)
{
	int32_t ret;

	if (!dev) {
		return -EINVAL;
	}

#if (INTERFACE_MODE == SPI_INTERRUPT)
	ret = iio_trig_disable(ad355xr_hw_trig_desc);
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_disable(ldac_pwm_desc);
	if (ret) {
		return ret;
	}

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	ret = no_os_spi_remove(ad355xr_dev_inst->spi);
	if (ret) {
		return ret;
	}

	/* Reinitialize SPI with default parameter */
	ret = no_os_spi_init(&ad355xr_dev_inst->spi, &ad3552r_init_params.spi_param);
	if (ret) {
		return ret;
	}
#endif
#else
	ret = no_os_irq_disable(trigger_irq_desc, TRIGGER_INT_ID);
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_disable(ldac_pwm_desc);
	if (ret) {
		return ret;
	}

	ret = stm32_spi_dma_disable(ad355xr_dev_inst->spi->extra);
	if (ret) {
		return ret;
	}

	spi_dma_enabled = false;
#endif

	return 0;
}

/**
 * @brief Read requested number of DAC codes from client from IIO buffer
 * @param iio_dev_data[in] - Pointer to IIO device data structure
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad355xr_iio_submit_buffer(struct iio_device_data* iio_dev_data)
{
	int32_t ret;
	uint16_t num_of_bytes_transfer;
	uint8_t start_addr;

	if (!iio_dev_data) {
		return -EINVAL;
	}

#if (INTERFACE_MODE == SPI_DMA)
	if (!spi_dma_enabled) {
		/* only fast mode is supported */
		num_of_bytes_transfer = num_channels_en * BYTES_PER_SAMPLE;

		if (num_channels_en == 2) {
			start_addr = ad3552r_get_code_reg_addr(1, 0,
							       ad355xr_dev_inst->ch_data[0].fast_en);
		} else {
			start_addr = ad3552r_get_code_reg_addr(channel_num, 0,
							       ad355xr_dev_inst->ch_data[0].fast_en);
		}

		ret = ad3552r_write_reg(ad355xr_dev_inst, AD3552R_REG_ADDR_STREAM_MODE,
					num_of_bytes_transfer);
		if (ret) {
			return ret;
		}

		ret = stm32_spi_dma_enable(ad355xr_dev_inst->spi->extra, iio_dev_data,
					   num_of_bytes_transfer, start_addr);
		if (ret) {
			return ret;
		}

		ret = no_os_irq_enable(trigger_irq_desc, TRIGGER_INT_ID);
		if (ret) {
			return ret;
		}

		ret = no_os_pwm_enable(ldac_pwm_desc);
		if (ret) {
			return ret;
		}

		spi_dma_enabled = true;
	}
#endif

	return 0;
}

/**
 * @brief Pops data from IIO buffer and writes into DAC when
 *		  trigger handler IRQ is invoked
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad355xr_trigger_handler(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint16_t dac_code[2] = { 0 };

	ret = iio_buffer_pop_scan(iio_dev_data->buffer, &dac_code);
	if (ret) {
		return ret;
	}

	if (channel_mask == AD3552R_MASK_ALL_CH) {
		return ad355xr_write_one_sample_all_ch(ad355xr_dev_inst, dac_code);
	} else {
		return ad355xr_write_one_sample_one_ch(ad355xr_dev_inst, dac_code, channel_num);
	}

}

/**
 * @brief Initialization of AD355XR IIO hardware trigger specific parameters
 * @param desc[in,out] - IIO hardware trigger descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad355xr_iio_trigger_param_init(struct iio_hw_trig **desc)
{
	struct iio_hw_trig_init_param ad355xr_hw_trig_init_params;
	struct iio_hw_trig *hw_trig_desc;
	int32_t ret;

	if (!desc) {
		return -EINVAL;
	}

	hw_trig_desc = calloc(1, sizeof(struct iio_hw_trig));
	if (!hw_trig_desc) {
		return -ENOMEM;
	}

	ad355xr_hw_trig_init_params.irq_id = TRIGGER_INT_ID;
	ad355xr_hw_trig_init_params.name = AD355XR_IIO_TRIGGER_NAME;
	ad355xr_hw_trig_init_params.irq_trig_lvl = NO_OS_IRQ_EDGE_RISING;
	ad355xr_hw_trig_init_params.irq_ctrl = trigger_irq_desc;
	ad355xr_hw_trig_init_params.cb_info.event = NO_OS_EVT_GPIO;
	ad355xr_hw_trig_init_params.cb_info.peripheral = NO_OS_GPIO_IRQ;
	ad355xr_hw_trig_init_params.cb_info.handle = trigger_gpio_handle;
	ad355xr_hw_trig_init_params.iio_desc = ad355xr_iio_desc;

	/* Initialize hardware trigger */
	ret = iio_hw_trig_init(&hw_trig_desc, &ad355xr_hw_trig_init_params);
	if (ret) {
		free(hw_trig_desc);
		return ret;
	}

	*desc = hw_trig_desc;

	return 0;
}

/**
* @brief	Initialization of AD355XR specific IIO parameters
* @param 	desc[in,out] - IIO device descriptor
* @return	0 in case of success, negative error code otherwise
*/
static int32_t ad355xr_iio_param_init(struct iio_device **desc)
{
	struct iio_device *iio_ad355xr_inst;

	if (!desc) {
		return -EINVAL;
	}

	iio_ad355xr_inst = calloc(1, sizeof(struct iio_device));
	if (!iio_ad355xr_inst) {
		return -ENOMEM;
	}

	iio_ad355xr_inst->num_ch = NO_OS_ARRAY_SIZE(ad355xr_iio_channels);
	iio_ad355xr_inst->channels = ad355xr_iio_channels;
	iio_ad355xr_inst->attributes = ad355xr_iio_global_attributes;
	iio_ad355xr_inst->pre_enable = ad355xr_iio_prepare_transfer;
	iio_ad355xr_inst->post_disable = ad355xr_iio_end_transfer;
	iio_ad355xr_inst->debug_reg_read = ad355xr_iio_debug_reg_read;
	iio_ad355xr_inst->debug_reg_write = ad355xr_iio_debug_reg_write;
#if (INTERFACE_MODE == SPI_INTERRUPT)
	iio_ad355xr_inst->trigger_handler = ad355xr_trigger_handler;
#else
	iio_ad355xr_inst->submit = ad355xr_iio_submit_buffer;
#endif
	*desc = iio_ad355xr_inst;

	return 0;
}

/**
 * @brief	Initialize the IIO interface for AD355XR IIO device
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad355xr_iio_initialize(void)
{
	int32_t ret;

	/* IIO device descriptor */
	struct iio_device *iio_ad355xr_dev;

#if (INTERFACE_MODE != SPI_DMA)
	static struct iio_trigger ad355xr_iio_trig_desc = {
		.is_synchronous = true,
	};

	/* IIO trigger init parameters */
	static struct iio_trigger_init iio_trigger_init_params = {
		.descriptor = &ad355xr_iio_trig_desc,
		.name = AD355XR_IIO_TRIGGER_NAME,
	};
#endif

	/* IIO interface init parameters */
	static struct iio_init_param iio_init_params = {
		.phy_type = USE_UART,
#if (INTERFACE_MODE != SPI_DMA)
		.trigs = &iio_trigger_init_params
#endif
	};

	/* IIOD init parameters */
	static struct iio_device_init iio_device_init_params[NUM_OF_IIO_DEVICES] = {
		{
			.name = ACTIVE_DEVICE_NAME,
			.raw_buf = dac_data_buffer,
			.raw_buf_len = DATA_BUFFER_SIZE,
#if (INTERFACE_MODE != SPI_DMA)
			.trigger_id = "trigger0"
#endif
		}
	};

	/* Initialize the system peripherals */
	ret = init_system();
	if (ret) {
		return ret;
	}

	/* Initialize AD355XR no-os device driver interface */
	ret = ad3552r_init(&ad355xr_dev_inst, &ad3552r_init_params);
	if (ret) {
		return ret;
	}

	/* Initialize the AD355XR IIO app specific parameters */
	ret = ad355xr_iio_param_init(&iio_ad355xr_dev);
	if (ret) {
		return ret;
	}

	iio_device_init_params[0].dev = ad355xr_dev_inst;
	iio_device_init_params[0].dev_descriptor = iio_ad355xr_dev;
#if (INTERFACE_MODE != SPI_DMA)
	iio_init_params.nb_trigs++;
#endif
	iio_init_params.nb_devs++;

	/* Initialize the IIO interface */
	iio_init_params.devs = iio_device_init_params;
	iio_init_params.uart_desc = uart_iio_com_desc;
	ret = iio_init(&ad355xr_iio_desc, &iio_init_params);
	if (ret) {
		return ret;
	}

#if (INTERFACE_MODE != SPI_DMA)
	/* Initialize the AD355XR IIO trigger specific parameters */
	ret = ad355xr_iio_trigger_param_init(&ad355xr_hw_trig_desc);
	if (ret) {
		return ret;
	}
#endif

	/* Initialize the PWM trigger source for ldac */
	ret = init_ldac_pwm_trigger();
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief 	Run the AD355XR IIO event handler
 * @return	none
 * @details	This function monitors the new IIO client event
 */
void ad355xr_iio_event_handler(void)
{
	iio_step(ad355xr_iio_desc);
}
