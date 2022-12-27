/*************************************************************************//**
 *   @file   ad4696_user_config.c
 *   @brief  User configuration file for AD4696 device
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

#include <stdint.h>
#include "app_config.h"
#include "ad4696_user_config.h"
#include "ad4696_support.h"
#include "no_os_gpio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

#define		USER_CONFIG_RANGE	(DEFAULT_CHN_RANGE * 1000)

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

struct no_os_gpio_init_param gpio_init_reset = {
	.number = RESET_PIN,
	.platform_ops = &gpio_ops,
	.extra = NULL
};

struct no_os_gpio_init_param gpio_init_convst = {
	.number = CONVST_PIN,
	.platform_ops = &gpio_ops,
	.extra = NULL
};

struct no_os_gpio_init_param gpio_init_busy = {
	.number = BUSY_PIN,
	.platform_ops = &gpio_ops,
	.extra = &bsy_gpio_extra_init_params
};

struct no_os_spi_init_param spi_init_params = {
	.max_speed_hz = 22500000,    	// Max SPI Speed
	.chip_select = SPI_CSB,    		// Chip Select
	.mode = NO_OS_SPI_MODE_3,       // CPOL = 1, CPHA = 1
	.platform_ops = &spi_ops,
	.extra = &spi_extra_init_params	// SPI extra configurations
};

/* Initialize the AD4696 device structure */
struct ad469x_init_param ad4696_init_str = {
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