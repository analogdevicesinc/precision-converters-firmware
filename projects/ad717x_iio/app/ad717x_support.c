/*************************************************************************//**
 *   @file   ad717x_support.c
 *   @brief  Support file for AD717X device configurations
******************************************************************************
* Copyright (c) 2022 Analog Devices, Inc.
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
	uint8_t buffer_index;

	if (!adc_data) {
		return -EINVAL;
	}

	ret = no_os_spi_write_and_read(p_ad717x_dev_inst->spi_desc,
				       buffer,
				       sizeof(buffer));
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_set_value(csb_gpio, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

	for (buffer_index = 0; buffer_index < sizeof(buffer); buffer_index++) {
		adc_raw <<= 8;
		adc_raw += buffer[buffer_index];
	}

	*adc_data = adc_raw;

	return ret;
}
