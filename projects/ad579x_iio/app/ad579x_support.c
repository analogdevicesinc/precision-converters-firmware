/***************************************************************************//*
 * @file    ad579x_support.c
 * @brief   AD579x No-OS driver support file
******************************************************************************
 * Copyright (c) 2023 Analog Devices, Inc. All Rights Reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad579x_support.h"
#include "ad579x_user_config.h"
#include "app_config.h"
#include "no_os_pwm.h"

/******************************************************************************/
/********************* Macros and Constants Definitions ***********************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/

/**
 * @brief	Reconfigure LDAC pin as GPIO output
 * @param 	device[in] - AD579x device instance
 * @param 	pin_state[in]  - State of conversion pin
 * @return	0 in case of success, negative error code otherwise
 */
int ad579x_reconfig_ldac(struct ad5791_dev *device,
			 enum ad579x_ldac_pin_state pin_state)
{
	int ret;

	if (!device) {
		return -EINVAL;
	}

	ret = no_os_gpio_remove(device->gpio_ldac);
	if (ret) {
		return ret;
	}

	if (pin_state == AD579x_LDAC_GPIO_OUTPUT) {
		/* Reconfigure the LDAC pin as GPIO Mode */
		ret = no_os_gpio_get(&device->gpio_ldac,
				     &ad579x_init_params.gpio_ldac);
		if (ret) {
			return ret;
		}

		ret = no_os_gpio_direction_output(device->gpio_ldac, NO_OS_GPIO_HIGH);
		if (ret) {
			return ret;
		}
	} else {
		/* Reconfigure the LDAC pin as Alternate Function Mode (for PWM) */
		ret = no_os_gpio_get(&pwm_desc->pwm_gpio, pwm_init_params.pwm_gpio);
		if (ret) {
			return ret;
		}
	}

	return 0;
}
