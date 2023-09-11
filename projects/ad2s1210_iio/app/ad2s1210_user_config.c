/***************************************************************************//**
 * @file    ad2s1210_user_config.c
 * @brief   User configurations for AD2S1210 No-OS driver
********************************************************************************
* Copyright (c) 2023 Analog Devices, Inc.
* Copyright (c) 2023 BayLibre, SAS.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad2s1210_user_config.h"
#include "app_config.h"
/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* AD2S1210 No-OS driver init parameters */
struct ad2s1210_init_param ad2s1210_init_params = {
	.spi_init = { /* TODO: move to spi_init_param */
		.max_speed_hz = 2000000,
		.mode = NO_OS_SPI_MODE_1,
		.chip_select = SPI_CSB,
		.platform_ops = &spi_ops,
		.extra = &spi_extra_init_params
	},
	.gpio_a0 = {
		.number = GPIO_A0,
		.platform_ops = &gpio_ops,
		.extra = NULL
	},
	.gpio_a1 = {
		.number = GPIO_A1,
		.platform_ops = &gpio_ops,
		.extra = NULL
	},
	.gpio_res0 = {
		.number = GPIO_RES0,
		.platform_ops = &gpio_ops,
		.extra = NULL
	},
	.gpio_res1 = {
		.number = GPIO_RES1,
		.platform_ops = &gpio_ops,
		.extra = NULL
	},
	.gpio_sample = {
		.number = GPIO_SAMPLE,
		.platform_ops = &gpio_ops,
		.extra = NULL
	},
	.resolution = AD2S1210_RESOLUTION,
	.clkin_hz = AD2S1210_FCLKIN,
};
