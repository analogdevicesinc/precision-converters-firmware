/***************************************************************************//**
 * @file    ad7689_user_config.c
 * @brief   User configurations for AD7689 No-OS driver
********************************************************************************
* Copyright (c) 2021-22 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad7689_user_config.h"
#include "app_config.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* AD7689 No-OS driver init parameters */
struct ad7689_init_param ad7689_init_params = {
	.id = ACTIVE_DEVICE,

	.config = {
		.incc = ADC_INPUT_TYPE_CFG,
		.inx = 0,
		.bw = AD7689_BW_FULL,
		.ref = ADC_REF_VOLTAGE_CFG,
		.seq = AD7689_SEQ_DISABLE,
		.rb = false
	},

	.spi_init = {
		.max_speed_hz = 22500000,
		.mode = NO_OS_SPI_MODE_0,
		.chip_select = SPI_CSB,
		.platform_ops = &spi_ops,
		.extra = &spi_extra_init_params
	}
};
