/*************************************************************************//**
 *   @file   ad7191_support.c
 *   @brief  Support file for AD7191 device configurations
******************************************************************************
* Copyright (c) 2024 Analog Devices, Inc.
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
#include "ad7191_support.h"
#include "no_os_error.h"

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/**
 * @brief Initialize the GPIOs
 * @param device[in]     - The device structure.
 * @param init_param[in] - The structure that contains the device initial
 * 		       		       parameters.
 *
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad7191_init_gpio(struct ad7191_dev **device,
			 struct ad7191_init_param init_param)
{
	int ret;
	struct ad7191_dev *dev;

	dev = (struct ad7191_dev *)no_os_malloc(sizeof(*dev));
	if (!dev) {
		return -ENOMEM;
	}

	ret = no_os_gpio_get(&dev->odr1_gpio, init_param.odr1_gpio);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_input(dev->odr1_gpio);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_get(&dev->odr2_gpio, init_param.odr2_gpio);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_input(dev->odr2_gpio);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_get(&dev->pga1_gpio, init_param.pga1_gpio);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_input(dev->pga1_gpio);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_get(&dev->pga2_gpio, init_param.pga2_gpio);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_input(dev->pga2_gpio);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_get(&dev->csb_gpio, init_param.csb_gpio);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(dev->csb_gpio, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_get(&dev->rdy_gpio, init_param.rdy_gpio);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_input(dev->rdy_gpio);
	if (ret) {
		return ret;
	}

	*device = dev;

	return 0;
}
