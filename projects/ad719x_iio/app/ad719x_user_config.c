/*************************************************************************//**
 *   @file   ad719x_user_config.c
 *   @brief  User configuration file for AD719X device
******************************************************************************
* Copyright (c) 2021-22,2024 Analog Devices, Inc.
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
#include "ad719x_user_config.h"

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
/* Define SPI init parameters structure for AD719X */
struct no_os_spi_init_param spi_init_params = {
	.max_speed_hz = 10000000, // Max SPI Speed
	.chip_select = SPI_CSB, // Chip Select
	.mode = NO_OS_SPI_MODE_3, // CPOL = 1, CPHA = 1
	.platform_ops = &spi_ops,
	.extra = &spi_extra_init_params	 // SPI extra configurations
};

/* Define GPIO cs pin init parameters structure */
struct no_os_gpio_init_param gpio_cs_init = {
	.number = SPI_CSB,
	.platform_ops = &mbed_gpio_ops,
	.extra = NULL
};

/* Define GPIO miso pin init parameters structure */
struct no_os_gpio_init_param gpio_miso_init = {
	.number = RDY_PIN,
	.platform_ops = &gpio_ops,
	.extra = NULL
};

/* Define GPIO sync pin init parameters structure */
struct no_os_gpio_init_param gpio_sync_init = {
	.number = SYNC_PIN,
	.platform_ops = &gpio_ops,
	.extra = &gpio_sync_init_params
};

/* Initialize the AD7193 device structure */
struct ad719x_init_param ad719x_init_params = {
	.spi_init = &spi_init_params,
	.gpio_miso = &gpio_miso_init,
	.sync_pin = &gpio_sync_init,
	.current_polarity = POLARITY_CONFIG,
	.current_gain = DEFAULT_GAIN,
	.data_rate_code = DATA_OUTPUT_RATE_BITS,
	.operating_mode = AD719X_MODE_IDLE,
	.clock_source = AD719X_INT_CLK_4_92_MHZ,
	.input_mode = INPUT_CONFIG,
	.buffer = true,
	.bpdsw_mode = false,
#if defined(DEV_AD7190)
	.chip_id = AD7190
#elif defined(DEV_AD7192)
	.chip_id = AD7192
#elif defined(DEV_AD7193)
	.chip_id = AD7193
#elif defined(DEV_AD7194)
	.chip_id = AD7194
#elif defined(DEV_AD7195)
	.chip_id = AD7195
#endif
};