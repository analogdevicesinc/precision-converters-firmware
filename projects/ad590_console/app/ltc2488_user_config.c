/*************************************************************************//**
 *   @file   ltc2488_user_config.c
 *   @brief  User configuration file for LTC2488 device
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
#include "ltc2488.h"
#include "ltc2488_user_config.h"
#include "mbed_spi.h"

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* Define SPI extra init parameters structure */
struct mbed_spi_init_param spi_init_extra_params = {
	.spi_clk_pin = SPI_SCK,
	.spi_miso_pin = SPI_MISO,
	.spi_mosi_pin = SPI_MOSI
};

/* Initialize the LTC2488 device structure */
struct ltc2488_dev_init ltc2488_init_str = {
	// Define SPI init parameters structure
	.spi_init = {
		.max_speed_hz = 2000000,        // Max SPI Speed
		.chip_select = SPI_SS,          // Chip Select
		.mode = NO_OS_SPI_MODE_0,             // CPOL = 0, CPHA = 0
		.extra = &spi_init_extra_params,	// SPI extra configurations
		.platform_ops = &mbed_spi_ops
	}
};
