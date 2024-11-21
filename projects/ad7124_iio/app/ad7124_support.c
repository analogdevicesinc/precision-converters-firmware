/*****************************************************************************
  *@file  ad7124_support.c
  *@brief Provides useful support functions for the AD7124 NoOS driver
******************************************************************************
* Copyright (c) 2023-24 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

/*****************************************************************************/
/***************************** Include Files *********************************/
/*****************************************************************************/

#include "ad7124_support.h"
#include  "no_os_gpio.h"
#include "app_config.h"

/*****************************************************************************/
/********************* Macros and Constants Definition ***********************/
/*****************************************************************************/

/*****************************************************************************/
/******************** Variables and User Defined Data Types ******************/
/*****************************************************************************/

/*****************************************************************************/
/************************** Functions Declaration ****************************/
/*****************************************************************************/

/*****************************************************************************/
/************************** Functions Definition *****************************/
/*****************************************************************************/

/**
 * @brief Get the polarity of input channel
 * @param dev[in] - AD7124 device instance
 * @param chn[in] - Input channel
 * @param polarity[in,out] - Channel polarity
 * @return 0 in case of success, negative error code otherwise
 */
int ad7124_get_polarity(struct ad7124_dev *dev,
			uint8_t chn,
			enum ad7124_input_polarity *polarity)
{
	uint8_t setup_id;
	setup_id = dev->chan_map[chn].setup_sel;

	if (!dev || !polarity) {
		return -EINVAL;
	}

	if (dev->setups[setup_id].bi_unipolar) {
		*polarity = AD7124_BIPOLAR;
	} else {
		*polarity = AD7124_UNIPOLAR;
	}

	return 0;
}

/**
 * @brief Perform Single Conversion
 * @param device[in] - AD7124 Device Descriptor
 * @param id[in] - Channel ID (number) requested
 * @param adc_raw_data[out] ADC Raw Value
 * @return 0 in case of success, negative error code otherwise.
 */
int ad7124_single_read(struct ad7124_dev* device,
		       uint8_t id,
		       int32_t *adc_raw_data)
{
	int ret;

	/* Enable the requested channel */
	ret = ad7124_set_channel_status(device, id, true);
	if (ret) {
		return ret;
	}

	/* Set Mode to Single Conversion */
	ret = ad7124_set_adc_mode(device, AD7124_SINGLE);
	if (ret) {
		return ret;
	}

	/* Wait for Conversion completion */
	ret = ad7124_wait_for_conv_ready(device, AD7124_CONV_TIMEOUT);
	if (ret) {
		return ret;
	}

	/* Read the data register */
	ret = ad7124_read_data(device, adc_raw_data);
	if (ret) {
		return ret;
	}

	/* Disable the current channel */
	return ad7124_set_channel_status(device, id, false);
}

/**
 * @brief Read ADC Converted data
 * @param dev[in] - The AD7124 Device descriptor
 * @param sd_adc_code[in,out] - Converted Sample
 * @return 0 in case of success, negative error code otherwise
 */
int ad7124_read_converted_data(struct ad7124_dev *dev, uint32_t *sd_adc_code)
{
	uint8_t buff[3] = { 0 };
	int ret;

	if (!dev || !sd_adc_code) {
		return -EINVAL;
	}

	/* Read the SPI data */
	ret = no_os_spi_write_and_read(dev->spi_desc,
				       buff,
				       sizeof(buff));
	if (ret) {
		return ret;
	}

	*sd_adc_code = no_os_get_unaligned_be24(buff);

	/* After reading CS must be held low until all the bits are transferred. */
	ret = no_os_gpio_set_value(csb_gpio, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Enable/Disable continuous read mode
 * @param device[in] - The AD7124 Device descriptor
 * @param cont_read_en[in] - Continuous read enable status (True/False)
 * @return 0 in case of success, negative error code otherwise
 */
int ad7124_enable_cont_read(struct ad7124_dev *device, bool cont_read_en)
{
	int ret;
	int16_t value;

	if (!device) {
		return -EINVAL;
	}

	if (cont_read_en) {
		value = AD7124_ADC_CTRL_REG_CONT_READ;
	} else {
		value = 0x0U;
	}

	ret = ad7124_reg_write_msk(device,
				   AD7124_ADC_CTRL_REG,
				   value,
				   AD7124_ADC_CTRL_REG_CONT_READ);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Function to prepare the ADC for data capture
 * @param ad7124_dev_inst[in] - AD7124 device instance
 * @return 0 in case of success, negative error code otherwise
 */
int ad7124_trigger_data_capture(struct ad7124_dev *ad7124_dev_inst)
{
	int ret;

	/* Set ADC to Continuous conversion mode */
	ret = ad7124_set_adc_mode(ad7124_dev_inst, AD7124_CONTINUOUS);
	if (ret) {
		return ret;
	}

	/* Enable Continuous read operation */
	ret = ad7124_enable_cont_read(ad7124_dev_inst, true);
	if (ret) {
		return ret;
	}

	/* Pull the cs line low to detect the EOC bit during data capture */
	ret = no_os_gpio_set_value(csb_gpio, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Function to stop data capture
 * @param ad7124_dev_inst[in] - AD7124 device instance
 * @return 0 in case of success, negative error code otherwise
 */
int ad7124_stop_data_capture(struct ad7124_dev *ad7124_dev_inst)
{
	int ret;
	int32_t adc_raw_data;
	uint8_t rdy_value = NO_OS_GPIO_HIGH;
	uint32_t timeout = AD7124_CONV_TIMEOUT;

	/* Read the data register when RDY is low to exit continuous read mode */
	do {
		ret = no_os_gpio_get_value(rdy_gpio, &rdy_value);
		if (ret) {
			return ret;
		}
		timeout--;
	} while ((rdy_value != NO_OS_GPIO_LOW) && (timeout > 0));

	if (timeout == 0) {
		return -ETIMEDOUT;
	}

	ret = ad7124_read_data(ad7124_dev_inst, &adc_raw_data);
	if (ret) {
		return ret;
	}

	/* Disable continous read mode */
	ret = ad7124_enable_cont_read(ad7124_dev_inst, false);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Get the 3db cutoff frequency.
 * @param ad7124_dev_inst[in] - AD7124 device instance.
 * @param chn[in] - Input channel.
 * @param frequency[in,out] - Frequency.
 * @return 0 in case of success, negative error code otherwise.
 */
int ad7124_get_3db_frequency(struct ad7124_dev *ad7124_dev_inst,
			     uint8_t chn,
			     uint16_t *frequency)
{
	uint32_t reg_temp;
	uint32_t odr;
	int ret;

	odr = (uint32_t)ad7124_get_odr(ad7124_dev_inst, chn);

	ret = ad7124_read_register2(ad7124_dev_inst,
				    (AD7124_FILT0_REG + ad7124_dev_inst->chan_map[chn].setup_sel),
				    &reg_temp);
	if (ret) {
		return ret;
	}

	*frequency = (reg_temp >> 21) & 0x7;
	switch (*frequency) {
	case 0:
	case 4:
		*frequency = odr * 262 / 1000;
		break;
	case 2:
	case 5:
		*frequency = odr * 230 / 1000;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief Set the 3db cutoff frequency.
 * @param ad7124_dev_inst[in] - AD7124 device instance.
 * @param chn[in] - Input channel.
 * @param frequency[in] - Frequency
 * @return 0 in case of success, negative error code otherwise.
 */
int ad7124_set_3db_frequency(struct ad7124_dev *ad7124_dev_inst,
			     uint8_t chn,
			     uint16_t frequency)
{
	int32_t sinc4_3db_odr;
	int32_t sinc3_3db_odr;
	int16_t new_filter;
	int32_t new_odr;
	uint32_t reg_temp;
	int ret;

	sinc4_3db_odr = (frequency * 1000) / 230;
	sinc3_3db_odr = (frequency * 1000) / 262;

	if (sinc4_3db_odr > sinc3_3db_odr) {
		new_filter = 2;
		new_odr = sinc3_3db_odr;
	} else {
		new_filter = 0;
		new_odr = sinc4_3db_odr;
	}

	ret = ad7124_read_register2(ad7124_dev_inst,
				    (AD7124_FILT0_REG + ad7124_dev_inst->chan_map[chn].setup_sel),
				    &reg_temp);
	if (ret) {
		return ret;
	}

	reg_temp &= ~AD7124_FILT_REG_FILTER(~0);
	reg_temp |= AD7124_FILT_REG_FILTER(new_filter);

	ret = ad7124_write_register2(ad7124_dev_inst,
				     (AD7124_FILT0_REG + ad7124_dev_inst->chan_map[chn].setup_sel),
				     reg_temp);
	if (ret) {
		return ret;
	}

	ret = ad7124_set_odr(ad7124_dev_inst, (float)new_odr, chn);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Update Sampling rate when more than one channel is enabled.
 * @param ad7124_dev_inst[in] - AD7124 device instance.
 * @param frequency - Updated Frequency.
 * @return 0 in case of success, negative error code otherwise.
 */
int ad7124_update_sampling_rate(struct ad7124_dev *ad7124_dev_inst,
				uint16_t *frequency)
{
	uint16_t filt_coeff;
	float fclk;
	int ret;
	uint32_t reg_temp = 0;
	uint8_t fs_value;
	uint8_t filter;

	if (!ad7124_dev_inst || !frequency) {
		return -EINVAL;
	}

	/* Get the clock frequency*/
	ret = ad7124_fclk_get(ad7124_dev_inst, &fclk);
	if (ret) {
		return ret;
	}

	/* Get the Filter Type */
	ret = ad7124_read_register2(ad7124_dev_inst,
				    (AD7124_Filter_0),
				    &reg_temp);
	if (ret) {
		return ret;
	}

	filter = (reg_temp >> 21) & 0x7;
	/* Get the filter coefficient based on filter type */
	switch (filter) {
	case 0:
		filt_coeff = 4;
		break;
	case 4:
		if (ad7124_dev_inst->power_mode == 0) {
			filt_coeff = 11;
		} else {
			filt_coeff = 19;
		}
		break;
	case 2:
		filt_coeff = 3;
		break;
	case 5:
		if (ad7124_dev_inst->power_mode == 0) {
			filt_coeff = 10;
		} else {
			filt_coeff = 18;
		}
		break;

	default:
		return -EINVAL;
	}

	/* Get the FS value */
	fs_value = reg_temp & AD7124_FILT_REG_FS(0x7FF);

	/* Frequency here is calculated as 1 / tsettle and 30 is the dead time
	 * The Dead Time is dependent on FS value of the channels enabled
	 * More Details can be found in the sequencer section in Data Sheet. */
	*frequency = 1 / (((32 * filt_coeff * fs_value) + 30) / fclk);

	return 0;
}