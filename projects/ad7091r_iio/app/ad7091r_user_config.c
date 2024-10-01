/***************************************************************************//**
 * @file    ad7091r_user_config.c
 * @brief   User configurations for AD7091R No-OS driver
********************************************************************************
* Copyright (c) 2024 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad7091r_user_config.h"
#include "app_config.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* Conversion Start GPIO init parameters */
struct no_os_gpio_init_param cnvst_params = {
	.number = CNV_PIN,
	.port = CNV_PORT,
	.platform_ops = &gpio_ops,
	.extra = &cnv_gpio_extra_init_param
};

/* Reset GPIO init parameters */
struct no_os_gpio_init_param reset_params = {
	.number = RESET_PIN,
	.port = RESET_PORT,
	.platform_ops = &gpio_ops,
	.extra = &reset_gpio_extra_init_param
};

/* SPI init parameters */
struct no_os_spi_init_param spi_params = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = MAX_SPI_SCLK,
	.chip_select = SPI_CSB,
	.mode = NO_OS_SPI_MODE_0, //CPOL=0, CPHA=0
	.platform_ops = &spi_ops,
	.extra = &spi_extra_init_params
};

/* AD7091R No-OS driver init parameters */
struct ad7091r8_init_param ad7091r_init_params = {
	.device_id = ACTIVE_DEVICE_ID,
	.spi_init = &spi_params,
	.vref_mv = ADC_VREF_MV,
	.gpio_convst = &cnvst_params,
	.gpio_reset = &reset_params,
	.sleep_mode = AD7091R8_SLEEP_MODE_1 //Sleep mode off, internal ref on
};