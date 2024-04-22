/***************************************************************************//**
 * @file    ltc2672_user_config.c
 * @brief   User configurations for LTC2672 No-OS driver
********************************************************************************
* Copyright (c) 2023-24 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ltc2672_user_config.h"
#include "app_config.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* LTC2672 No-OS driver init parameters */
struct ltc2672_init_param ltc2672_init_params = {
	.id = ACTIVE_DEVICE_ID,
	.spi_init = {
		.device_id = SPI_DEVICE_ID,
		.max_speed_hz = 5625000,
		.chip_select = SPI_CSB,
		.mode = NO_OS_SPI_MODE_0,
		.platform_ops = &spi_ops,
		.extra = &spi_extra_init_params
	},
};