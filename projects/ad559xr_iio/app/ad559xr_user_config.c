/*************************************************************************//**
 *   @file   ad559xr_user_config.c
 *   @brief  User configuration file for ad559xr device
******************************************************************************
* Copyright (c) 2026 Analog Devices, Inc.
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
#include "app_config.h"
#include "ad559xr_user_config.h"

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
/* Define SPI init parameters structure for ad5592r */
struct no_os_spi_init_param spi_init_params = {
	.max_speed_hz = MAX_SPI_BAUDRATE, // Max SPI Speed
	.chip_select = SPI_CSB, // Chip Select
	.device_id = SPI_DEVICE_ID,
	.mode = NO_OS_SPI_MODE_2, // CPOL = 1, CPHA = 0
	.platform_ops = &spi_ops,
	.extra = &spi_extra_init_params	 // SPI extra configurations
};

/* Define I2C init parameters structure for ad5593r */
struct no_os_i2c_init_param i2c_user_params = {
	.max_speed_hz = I2C_BAUDRATE,
	.slave_address = AD5593R_I2C(AD5593R_A0_STATE),
	.platform_ops = &i2c_ops,
	.device_id = I2C_DEVICE_ID
};

/* Initialize the ad559xr device structure */
struct ad5592r_init_param ad5592r_init_params = {
	.spi_init = &spi_init_params,
	.i2c_init = &i2c_user_params,
	.int_ref = true,
	.adc_range = ZERO_TO_VREF,
	.dac_range = ZERO_TO_VREF,
	.channel_modes = {
		CH_MODE_UNUSED,
		CH_MODE_UNUSED,
		CH_MODE_UNUSED,
		CH_MODE_UNUSED,
		CH_MODE_UNUSED,
		CH_MODE_UNUSED,
		CH_MODE_UNUSED,
		CH_MODE_UNUSED
	},
	.adc_buf = true,
};
