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
 * @return	0 in case of success, negative error code otherwise
 */
int ad579x_reconfig_ldac(struct ad5791_dev *device)
{
	int ret;

	ret = no_os_gpio_remove(device->gpio_ldac);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_get(&device->gpio_ldac, &ad579x_init_params.gpio_ldac);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(device->gpio_ldac, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	return 0;
}