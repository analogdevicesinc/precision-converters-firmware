/*************************************************************************//**
 *   @file   ad3530r_user_config.c
 *   @brief  User configuration file for AD3530R
******************************************************************************
* Copyright (c) 2022-25 Analog Devices, Inc.
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
#include "ad3530r_user_config.h"
#include "app_config.h"
#include "no_os_spi.h"
#include "no_os_gpio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* Reset GPIO init parameters */
struct no_os_gpio_init_param gpio_reset_init = {
	.number = RESET_PIN,
	.port = RESET_PORT,
	.platform_ops = &gpio_ops,
	.extra = &gpio_reset_extra_init_params
};

/* LDAC GPIO init parameters */
struct no_os_gpio_init_param gpio_ldac_init = {
	.number = LDAC_PIN,
	.port = LDAC_PORT,
	.platform_ops = &gpio_ops,
	.extra = &gpio_ldac_extra_init_params
};

/* SPI init parameters */
struct no_os_spi_init_param spi_init_params = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = MAX_SPI_SCLK,   // Max SPI Speed
	.chip_select = SPI_CSB,    		// Chip Select
	.mode = NO_OS_SPI_MODE_3,		// CPOL = 1, CPHA = 1
	.platform_ops = &spi_ops,
	.extra = &spi_extra_init_params,// SPI extra configurations
};

/* Device Initialization Parameters */
struct ad3530r_init_param ad3530r_init_params = {
	.chip_id = AD3530R_ID,
	.spi_param = &spi_init_params,
	.spi_cfg = {
		.stream_mode_length = 0,
		.addr_asc = 0,
		.single_instr = 1, // Currently only single instruction mode is supported
		.short_instr = 0,
		.stream_length_keep_value = 0
	},
	/* If set, reset is done with RESET pin, otherwise it will be soft */
	.reset_gpio_param_optional = &gpio_reset_init,
	/* If set, input register are used and LDAC pulse is sent */
	.ldac_gpio_param_optional = &gpio_ldac_init,
	/* If set, use external Vref */
	.vref_enable = AD3530R_EXTERNAL_VREF_PIN_INPUT,
	.chn_op_mode = {
		AD3530R_CH_OPERATING_MODE_3, AD3530R_CH_OPERATING_MODE_3,
		AD3530R_CH_OPERATING_MODE_3, AD3530R_CH_OPERATING_MODE_3,
		AD3530R_CH_OPERATING_MODE_3, AD3530R_CH_OPERATING_MODE_3,
		AD3530R_CH_OPERATING_MODE_3, AD3530R_CH_OPERATING_MODE_3
	},
	.range = AD3530R_CH_OUTPUT_RANGE_0_VREF,
	.hw_ldac_mask = 0xff,
	.sw_ldac_mask = 0xff,
	/* Set to enable CRC */
	.crc_en = false
};
