/***************************************************************************//*
 * @file    ad5710r_support.c
 * @brief   AD5710r No-OS driver support file
******************************************************************************
 * Copyright (c) 2024-25 Analog Devices, Inc. All Rights Reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad5710r_support.h"
#include "ad5710r_user_config.h"
#include "no_os_error.h"
#include "app_config.h"

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
 * @brief	SPI read from device using a mask
 * @param 	desc[in] - AD5710R device instance
 * @param 	addr[in] - Register address
 * @param	mask[in] - Mask for a specific register field
 * @param	val[in,out] - Register value
 * @return	0 in case of success, negative error code otherwise
 */
int ad5710r_spi_read_mask(struct ad5710r_desc *desc,
			  uint32_t addr,
			  uint32_t mask,
			  uint16_t *val)
{
	int ret;
	uint16_t data;

	if (!desc || !val) {
		return -EINVAL;
	}

	ret = ad5710r_reg_read(desc, addr, &data);
	if (ret) {
		return ret;
	}

	*val = no_os_field_get(mask, (uint32_t)data);

	return ret;
}

/**
 * @brief	Reconfigure LDAC pin as either GPIO output or PWM based on the pin_state value
 * @param 	device[in] - AD3530r device instance
 * @param 	pin_state[in]  - State of conversion pin
 * @return	0 in case of success, negative error code otherwise
 */
int ad5710r_reconfig_ldac(struct ad5710r_desc* device,
			  enum ad5710r_ldac_pin_state pin_state)
{
	int ret;

	if (!device) {
		return -EINVAL;
	}

	ret = no_os_gpio_remove(device->ldac);
	if (ret) {
		return ret;
	}

	if (pin_state == AD5710R_LDAC_PWM) {
		/* Reconfigure the LDAC pin as GPIO Mode */
		ret = no_os_gpio_get(&device->ldac,
				     ad5710r_init_params.ldac_gpio_param_optional);
		if (ret) {
			return ret;
		}

		ret = no_os_gpio_direction_output(device->ldac, NO_OS_GPIO_HIGH);
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
