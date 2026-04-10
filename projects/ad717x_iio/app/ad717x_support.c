/*************************************************************************//**
 *   @file   ad717x_support.c
 *   @brief  Support file for AD717X device configurations
******************************************************************************
* Copyright (c) 2022, 2026 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <string.h>

#include "ad717x_support.h"
#include "ad717x_iio.h"
#include "no_os_error.h"
#include "app_config.h"
#include "no_os_irq.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

#define AD717x_CONV_TIMEOUT	10000

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief Write filter order to FILTCON register
 * @param device[in] - AD717x device instance
 * @param filter_order[in] - Filter order (sinc5_sinc1 or sinc3)
 * @param setup_id[in] - Setup ID (0-7)
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad717x_write_filter_order(ad717x_dev *device,
				  enum ad717x_order filter_order,
				  uint8_t setup_id)
{
	ad717x_st_reg *filtcon_reg;

	if (!device || setup_id >= AD717x_MAX_SETUPS) {
		return -EINVAL;
	}

	/* Get FILTCON register */
	filtcon_reg = AD717X_GetReg(device, AD717X_FILTCON0_REG + setup_id);
	if (!filtcon_reg) {
		return -EINVAL;
	}

	/* Clear filter order bits and set new value */
	if (filter_order == sinc3) {
		filtcon_reg->value |= AD717X_FILT_CONF_REG_SINC3_MAP;
	} else {
		filtcon_reg->value &= (~AD717X_FILT_CONF_REG_SINC3_MAP);
	}

	/* Write register */
	if (AD717X_WriteRegister(device, AD717X_FILTCON0_REG + setup_id) < 0) {
		return -EIO;
	}
	device->filter_configuration[setup_id].oder = filter_order;

	return 0;
}

/**
 * @brief Write post filter (enhanced filter) settings to FILTCON register
 * @param device[in] - AD717x device instance
 * @param enable[in] - Enable post filter
 * @param post_filter[in] - Post filter type
 * @param setup_id[in] - Setup ID (0-7)
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad717x_write_post_filter(ad717x_dev *device,
				 bool enable,
				 enum ad717x_enhfilt post_filter,
				 uint8_t setup_id)
{
	ad717x_st_reg *filtcon_reg;

	if (!device || setup_id >= AD717x_MAX_SETUPS) {
		return -EINVAL;
	}

	/* Get FILTCON register */
	filtcon_reg = AD717X_GetReg(device, AD717X_FILTCON0_REG + setup_id);
	if (!filtcon_reg) {
		return -EINVAL;
	}

	/* Clear post filter bits */
	filtcon_reg->value &= ~(AD717X_FILT_CONF_REG_ENHFILTEN |
				AD717X_FILT_CONF_REG_ENHFILT_MSK);

	/* Set new values */
	if (enable) {
		filtcon_reg->value |= AD717X_FILT_CONF_REG_ENHFILTEN;
		filtcon_reg->value |= AD717X_FILT_CONF_REG_ENHFILT(post_filter);
	}

	/* Write register */
	if (AD717X_WriteRegister(device, AD717X_FILTCON0_REG + setup_id) < 0) {
		return -EIO;
	}
	device->filter_configuration[setup_id].enhfilten = enable;
	device->filter_configuration[setup_id].enhfilt = post_filter;

	return 0;
}

/**
 * @brief Enable/Disable continuous read mode
 * @param device[in] - The AD717x Device descriptor
 * @param cont_read_en[in] - True in case of enable CONT_READ/ False in case of disable
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad717x_enable_cont_read(ad717x_dev *device, bool cont_read_en)
{
	ad717x_st_reg *ifmode_reg;
	int32_t ret;

	/* Retrieve the IFMODE Register */
	ifmode_reg = AD717X_GetReg(p_ad717x_dev_inst, AD717X_IFMODE_REG);
	if (!ifmode_reg) {
		return -EINVAL;
	}

	if (cont_read_en) {
		ifmode_reg->value |= (AD717X_IFMODE_REG_CONT_READ);
	} else {
		ifmode_reg->value &= ~(AD717X_IFMODE_REG_CONT_READ);
	}

	ret = AD717X_WriteRegister(p_ad717x_dev_inst, AD717X_IFMODE_REG);
	if (ret) {
		return ret;
	}

	return 0;
}


/*!
 * @brief Read ADC raw data for recently sampled channel
 * @param adc_data[out] - Pointer to adc data read variable
 * @return 0 in case of success, negative error code otherwise
 * @note This function is intended to call from the conversion end trigger
 *	 event. Therefore, this function should just read raw ADC data
 *	 without further monitoring conversion end event
 */
int32_t ad717x_adc_read_converted_sample(uint32_t *adc_data)
{
	uint32_t adc_raw = 0;
	int32_t ret;
	uint8_t buffer[3] = { 0, 0, 0 };
	uint8_t data_size;
	uint8_t buffer_index;

	if (!adc_data) {
		return -EINVAL;
	}

	/* AD4113 has a 16-bit data register (2 bytes); all other devices use 24-bit (3 bytes) */
	if (p_ad717x_dev_inst->active_device == ID_AD4113) {
		data_size = 2;
	} else {
		data_size = 3;
	}

	ret = no_os_spi_write_and_read(p_ad717x_dev_inst->spi_desc,
				       buffer,
				       data_size);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_set_value(csb_gpio, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

	for (buffer_index = 0; buffer_index < data_size; buffer_index++) {
		adc_raw <<= 8;
		adc_raw += buffer[buffer_index];
	}

	*adc_data = adc_raw;

	return ret;
}
