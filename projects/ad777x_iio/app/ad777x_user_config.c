/*************************************************************************//**
 *   @file   ad777x_user_config.c
 *   @brief  Default user configurations file for AD777x device
******************************************************************************
* Copyright (c) 2022 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad777x_user_config.h"
#include "no_os_gpio.h"
#include "app_config.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* AD777x Init Parameters */
ad7779_init_param ad777x_init_params = {
	.spi_init = {
		.max_speed_hz = 25000000,
		.chip_select = SPI_CSB,
		.mode = NO_OS_SPI_MODE_0,
		.extra  = &spi_extra_init_params,
		.platform_ops = &spi_platform_ops,
		.device_id  = SPI_DEVICE_ID,
	},
	.gpio_reset = {
		.number = GPIO_RESET_PIN,
		.port = GPIO_RESET_PORT,
		.extra = &gpio_reset_extra_init_params,
		.platform_ops = &gpio_platform_ops
	},
	.gpio_mode0 = {
		.number = GPIO_MODE0_PIN,
		.port = GPIO_MODE0_PORT,
		.extra = &gpio_mode0_extra_init_params,
		.platform_ops = &gpio_platform_ops
	},
	.gpio_mode1 = {
		.number = GPIO_MODE1_PIN,
		.port = GPIO_MODE1_PORT,
		.extra = &gpio_mode1_extra_init_params,
		.platform_ops = &gpio_platform_ops
	},
	.gpio_mode2 = {
		.number = GPIO_MODE2_PIN,
		.port = GPIO_MODE2_PORT,
		.extra = &gpio_mode2_extra_init_params,
		.platform_ops = &gpio_platform_ops
	},
	.gpio_mode3 = {
		.number = GPIO_MODE3_PIN,
		.port = GPIO_MODE3_PORT,
		.extra = &gpio_mode3_extra_init_params,
		.platform_ops = &gpio_platform_ops
	},
	.gpio_dclk0 = {
		.number = GPIO_DCLK0_PIN,
		.port = GPIO_DCLK0_PORT,
		.extra = &gpio_dclk0_extra_init_params,
		.platform_ops = &gpio_platform_ops
	},
	.gpio_dclk1 = {
		.number = GPIO_DCLK1_PIN,
		.port = GPIO_DCLK1_PORT,
		.extra = &gpio_dclk1_extra_init_params,
		.platform_ops = &gpio_platform_ops
	},
	.gpio_dclk2 = {
		.number = GPIO_DCLK2_PIN,
		.port = GPIO_DCLK2_PORT,
		.extra = &gpio_dclk2_extra_init_params,
		.platform_ops = &gpio_platform_ops
	},
	.gpio_sync_in = {
		.number = GPIO_SYNC_IN_PIN,
		.port = GPIO_SYNC_PORT,
		.extra = &gpio_sync_in_extra_init_params,
		.platform_ops = &gpio_platform_ops
	},
	.gpio_convst_sar =  {
		.number = GPIO_CONVST_SAR_PIN,
		.port = GPIO_CONVST_PORT,
		.extra = &gpio_convst_sar_extra_init_params,
		.platform_ops = &gpio_platform_ops
	},
	.ctrl_mode = AD7779_SPI_CTRL,
	.spi_crc_en = AD7779_DISABLE,
	.state =  {
		AD7779_ENABLE,
		AD7779_ENABLE,
		AD7779_ENABLE,
		AD7779_ENABLE,
		AD7779_ENABLE,
		AD7779_ENABLE,
		AD7779_ENABLE,
		AD7779_ENABLE
	},
	.gain = {
		AD7779_GAIN_1,
		AD7779_GAIN_1,
		AD7779_GAIN_1,
		AD7779_GAIN_1,
		AD7779_GAIN_1,
		AD7779_GAIN_1,
		AD7779_GAIN_1,
		AD7779_GAIN_1
	},
	.gain_corr = {
		AD777x_GAIN_CORR,
		AD777x_GAIN_CORR,
		AD777x_GAIN_CORR,
		AD777x_GAIN_CORR,
		AD777x_GAIN_CORR,
		AD777x_GAIN_CORR,
		AD777x_GAIN_CORR,
		AD777x_GAIN_CORR
	},
	.dec_rate_int = AD777x_DEC_RATE_INT,
	.dec_rate_dec = AD777x_DEC_RATE_DEC,
	.ref_type = AD7779_INT_REF,
	.pwr_mode = AD777x_POWER_MODE,
	.dclk_div = AD7779_DCLK_DIV_1, /* Using maximum DCLK */
	.ref_buf_op_mode = {AD7779_REF_BUF_DISABLED, AD7779_REF_BUF_DISABLED},
	.sinc5_state = AD7779_DISABLE
};
