/*************************************************************************//**
 *   @file   ad405x_user_config.c
 *   @brief  User configuration file for AD405X
******************************************************************************
* Copyright (c) 2022-2024 Analog Devices, Inc.
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
#include "ad405x_user_config.h"
#include "app_config.h"
#include "no_os_spi.h"
#include "no_os_gpio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
static struct no_os_spi_init_param spi_init_params = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = MAX_SPI_SCLK,
	.mode = NO_OS_SPI_MODE_0,
	.chip_select = SPI_CS_PIN_NUM,
	.bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST,
	.platform_ops = &spi_ops,
	.extra = &spi_extra_init_params
};

/* ad405x cnv init parameters */
static struct no_os_gpio_init_param gpio_cnv_param = {
	.port = CNV_PORT_NUM,
	.number = CNV_PIN_NUM,
	.platform_ops = &gpio_ops,
	.extra = &cnv_extra_init_params
};

/* ad405x gpio0 init parameters */
struct no_os_gpio_init_param gpio_gpio0_param = {
	.port = GP0_PORT_NUM,
	.number = GP0_PIN_NUM,
	.platform_ops = &gpio_ops,
	.extra = &gp0_extra_init_params
};

/* ad405x gpio1 init parameters */
struct no_os_gpio_init_param gpio_gpio1_param = {
	.port = GP1_PORT_NUM,
	.number = GP1_PIN_NUM,
	.platform_ops = &gpio_ops,
	.extra = &gp1_extra_init_params
};

/* Initialize the AD405X device structure */
struct ad405x_init_param ad405x_init_params = {
	.spi_init = &spi_init_params,
	.active_device = ACTIVE_DEVICE_ID,
	.gpio_cnv = &gpio_cnv_param,
	.gpio_gpio0 = &gpio_gpio0_param,
	.gpio_gpio1 = &gpio_gpio1_param,
	.rate = AD405X_2_MSPS,
	.filter_length = AD405X_LENGTH_2,
	.operation_mode = AD405X_CONFIG_MODE_OP
};
