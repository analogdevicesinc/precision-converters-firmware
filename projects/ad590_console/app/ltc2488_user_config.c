/*************************************************************************//**
 *   @file   ltc2488_user_config.c
 *   @brief  User configuration file for LTC2488 device
******************************************************************************
* Copyright (c) 2021-22,2025 Analog Devices, Inc.
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

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* Initialize the LTC2488 device structure */
struct ltc2488_dev_init ltc2488_init_str = {
	// Define SPI init parameters structure
	.spi_init = {
		.device_id = SPI_DEVICE_ID,
		.max_speed_hz = MAX_SPI_CLK,        // Max SPI Speed
		.chip_select = SPI_SS,          // Chip Select
		.mode = NO_OS_SPI_MODE_0,             // CPOL = 0, CPHA = 0
		.extra = &spi_init_extra_params,	// SPI extra configurations
		.platform_ops = &spi_ops
	}
};
