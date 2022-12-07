/***************************************************************************//**
 *   @file    eeprom_config.c
 *   @brief   EEPROM configuration module
 *   @details This module configures the specific EEPROM type parameters
********************************************************************************
 * Copyright (c) 2022 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdbool.h>

#include "app_config.h"
#include "eeprom_config.h"
#include "no_os_error.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* EEPROM I2C init parameters */
struct no_os_i2c_init_param no_os_i2c_init_params = {
	.device_id = 0,
	.platform_ops = &i2c_ops,
	.max_speed_hz = 100000,
	.extra = &i2c_extra_init_params
};

/* EEPROM init parameters */
struct eeprom_24xx32a_init_param eeprom_extra_init_params = {
	.i2c_init = &no_os_i2c_init_params
};

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/

/**
 * @brief 	Store the EEPROM device address
 * @param	eeprom_desc[in] - EEPROM descriptor
 * @param	dev_addr[in] - EEPROM device address
 * @return	0 in case of success, negative error code otherwise
 */
int32_t load_eeprom_dev_address(struct no_os_eeprom_desc *eeprom_desc,
				uint8_t dev_addr)
{
	struct eeprom_24xx32a_dev *eeprom_dev;

	if (!eeprom_desc) {
		return -EINVAL;
	}

	eeprom_dev = eeprom_desc->extra;

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	/* Left shift by 1 to get 7-bit address (7 MSBs)
	 * The LSB (0th bit) acts as R/W bit */
	eeprom_dev->i2c_desc->slave_address = dev_addr << 1;
#else
	eeprom_dev->i2c_desc->slave_address = dev_addr;
#endif

	return 0;
}