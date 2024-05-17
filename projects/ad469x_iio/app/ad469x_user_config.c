/*************************************************************************//**
 *   @file   ad469x_user_config.c
 *   @brief  User configuration file for AD469x device
******************************************************************************
* Copyright (c) 2021-23 Analog Devices, Inc.
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

#include <stdint.h>
#include "app_config.h"
#include "ad469x_user_config.h"
#include "ad469x_support.h"
#include "no_os_gpio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

#define		USER_CONFIG_RANGE	(DEFAULT_CHN_RANGE * 1000)

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

struct no_os_gpio_init_param gpio_init_reset = {
	.port = RESET_PORT_NUM,
	.number = RESET_PIN_NUM,
	.platform_ops = &gpio_ops,
	.extra = &reset_extra_init_params
};

struct no_os_gpio_init_param gpio_init_convst = {
	.port = CNV_PORT_NUM,
	.number = CNV_PIN_NUM,
	.platform_ops = &gpio_ops,
	.extra = &cnv_extra_init_params
};

struct no_os_gpio_init_param gpio_init_busy = {
	.port = BSY_PORT_NUM,
	.number = BSY_PIN_NUM,
	.platform_ops = &gpio_ops,
	.extra = &bsy_extra_init_params
};

struct no_os_spi_init_param spi_init_params = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = 22500000,    	// Max SPI Speed
	.chip_select = SPI_CS_PIN_NUM,    		// Chip Select
	.mode = NO_OS_SPI_MODE_3,       // CPOL = 1, CPHA = 1
	.platform_ops = &spi_ops,
	.extra = &spi_extra_init_params	// SPI extra configurations
};

/* Initialize the AD469x device structure */
struct ad469x_init_param ad469x_init_str = {
	// Define SPI init parameters structure
	.spi_init = &spi_init_params,
	// Define GPIOs init parameter structure
	.gpio_resetn = &gpio_init_reset,
	.gpio_convst = &gpio_init_convst,
	.gpio_busy = &gpio_init_busy,
	.std_seq_osr = AD469x_OSR_1,
	.std_seq_pin_pairing = AD469x_INx_COM,
	.ch_sequence = AD469x_standard_seq,
	.dev_id = ACTIVE_DEVICE
};
