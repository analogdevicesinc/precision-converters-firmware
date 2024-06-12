/*************************************************************************//**
 *   @file   ad355xr_user_config.c
 *   @brief  User configuration file for AD355XR devices
******************************************************************************
* Copyright (c) 2023-2024 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad355xr_user_config.h"
#include "app_config.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* Reset initialization parameters */
struct no_os_gpio_init_param gpio_reset_init = {
	.number = RESET_PIN,
	.port = RESET_PORT,
	.platform_ops = &gpio_ops,
	.extra = &gpio_reset_extra_init_params
};

/* LDAC initialization parameters */
struct no_os_gpio_init_param gpio_ldac_init = {
	.number = LDAC_PIN,
	.port = LDAC_PORT,
	.platform_ops = &gpio_ops,
	.extra = &gpio_ldac_extra_init_params
};

/* SPI initialization parameters without software csb
 * to reduce the SPI transaction time in trigger handler.*/
struct no_os_spi_init_param spi_init_params_without_sw_csb = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = MAX_SPI_SCLK, // Max SPI Speed
	.chip_select = SPI_CSB, // Chip Select
	.mode = NO_OS_SPI_MODE_0, // CPOL = 0, CPHA = 0
	.platform_ops = &spi_ops,
	.extra = &spi_extra_init_params_without_sw_csb	// SPI extra configurations
};

/* Device initialization parameters */
struct ad3552r_init_param ad3552r_init_params = {
	.chip_id = ACTIVE_DEVICE_ID,
	.spi_param = {
		.device_id = SPI_DEVICE_ID,
		.max_speed_hz = MAX_SPI_SCLK, // Max SPI Speed
		.chip_select = SPI_CSB,   // Chip Select
		.mode = NO_OS_SPI_MODE_0, // CPOL = 0, CPHA = 0
		.platform_ops = &spi_ops,
		.extra = &spi_extra_init_params	// SPI extra configurations
	},
	.reset_gpio_param_optional = &gpio_reset_init,
	.ldac_gpio_param_optional = &gpio_ldac_init,
	.use_external_vref = false,
	.vref_out_enable = false,
	.sdo_drive_strength = AD3552R_MEDIUM_LOW_SDIO_DRIVE_STRENGTH,
	.channels = {
		{ .en = true, .fast_en = true, .range = AD3552R_CH_OUTPUT_RANGE_0__2P5V},
#if (NUMBER_OF_CHANNELS == 2)
		{ .en = true, .fast_en = true, .range = AD3552R_CH_OUTPUT_RANGE_0__2P5V},
#endif
	},
	.crc_en = false,
	.is_simultaneous =false
};
