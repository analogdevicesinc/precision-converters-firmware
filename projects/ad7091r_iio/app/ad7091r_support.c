/***************************************************************************/ /**
 * @file    ad7091r_support.c
 * @brief   AD7091R No-OS driver support functionality
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

#include "ad7091r_support.h"
#include "ad7091r_user_config.h"
#include "app_config.h"
#include "no_os_error.h"

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief	Reconfigure Conversion pin from GPIO output to PWM and vice versa
 * @param 	device[in] - AD7091r device instance
 * @param 	pin_state[in]  - Current state of conversion pin
 * @return	0 in case of success, negative error code otherwise
 */
int ad7091r_reconfig_conv(struct ad7091r8_dev *device,
			  enum ad7091r_conv_pin_state pin_state)
{
	int ret;

	if (!device) {
		return -EINVAL;
	}

	ret = no_os_gpio_remove(device->gpio_convst);
	if (ret) {
		return ret;
	}

	if (pin_state == CNV_PWM) {
		ret = no_os_gpio_get(&device->gpio_convst, ad7091r_init_params.gpio_convst);
		if (ret) {
			return ret;
		}

		ret = no_os_gpio_direction_output(device->gpio_convst, NO_OS_GPIO_LOW);
		if (ret) {
			return ret;
		}
	} else {
		ret = no_os_gpio_get(&pwm_desc->pwm_gpio, pwm_init_params.pwm_gpio);
		if (ret) {
			return ret;
		}
	}

	return 0;
}