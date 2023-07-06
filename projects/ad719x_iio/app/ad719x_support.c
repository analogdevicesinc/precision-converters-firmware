/*************************************************************************//**
 *   @file   ad719x_support.c
 *   @brief  Support file for AD719X device configurations
******************************************************************************
* Copyright (c) 2021-22 Analog Devices, Inc.
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
#include "ad719x_support.h"
#include "ad719x_user_config.h"
#include "ad719x_iio.h"
#include "no_os_error.h"

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
struct no_os_gpio_desc *gpio_cs = NULL;

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/
/*!
 * @brief	Function to initialize the cs pin gpio.
 * @return	0 in case of SUCCESS, negative error code otherwise.
 */
int32_t ad719x_gpio_cs_init(void)
{
	int32_t ret;

	ret = no_os_gpio_get(&gpio_cs, &gpio_cs_init);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(gpio_cs, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	return 0;
}

/*!
 * @brief	Configures the device for noise and 50hz test.
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t ad719x_noise_config(void)
{
	int32_t ret;

	/* Disable Chop */
	ret = ad719x_set_masked_register_value(p_ad719x_dev_inst,
					       AD719X_REG_CONF, AD719X_CONF_CHOP, 0, 3);
	if (ret) {
		return ret;
	}

	/* Reference Select */
	ret = ad719x_set_masked_register_value(p_ad719x_dev_inst,
					       AD719X_REG_CONF, AD719X_CONF_REFSEL, 0, 3);
	if (ret) {
		return ret;
	}

#if (ACTIVE_MODE == FAST_50HZ_TEST)
	ret = ad719x_set_masked_register_value(p_ad719x_dev_inst, AD719X_REG_MODE,
					       AD719X_MODE_AVG(AD719X_AVG_BY_16), AD719X_AVG_BY_16, 3);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

