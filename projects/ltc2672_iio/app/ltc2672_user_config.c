/***************************************************************************//**
 * @file    ltc2672_user_config.c
 * @brief   User configurations for LTC2672 No-OS driver
********************************************************************************
* Copyright (c) 2023-25 Analog Devices, Inc.
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
/* LDAC GPIO parameters */
struct no_os_gpio_init_param gpio_ldac_params = {
	.port = GPIO_LDAC_PORT,
	.number = GPIO_LDAC_PIN,
	.platform_ops = &gpio_ops,
	.extra = &gpio_ldac_extra_params
};

/* Clear GPIO parameters */
struct no_os_gpio_init_param gpio_clear_params = {
	.port = GPIO_CLR_PORT,
	.number = GPIO_CLR_PIN,
	.platform_ops = &gpio_ops,
	.extra = &gpio_clear_extra_params
};

/* Toggle GPIO parameters */
struct no_os_gpio_init_param gpio_toggle_params = {
	.port = GPIO_TGP_PORT,
	.number = GPIO_TGP_PIN,
	.platform_ops = &gpio_ops,
	.extra = &gpio_toggle_extra_params
};

/* Fault GPIO parameters */
struct no_os_gpio_init_param gpio_fault_params = {
	.port = GPIO_FAULT_PORT,
	.number = GPIO_FAULT_PIN,
	.platform_ops = &gpio_ops,
	.extra = &gpio_fault_extra_params
};


/* LTC2672 No-OS driver init parameters */
struct ltc2672_init_param ltc2672_init_params = {
#if defined(DC2903A)
	.id = ACTIVE_DEVICE_ID,
#endif
	.spi_init = {
		.device_id = SPI_DEVICE_ID,
		.max_speed_hz = MAX_SPI_SCLK,
		.chip_select = SPI_CSB,
		.mode = NO_OS_SPI_MODE_0,
		.platform_ops = &spi_ops,
		.extra = &spi_extra_init_params
	},
	.gpio_clear = &gpio_clear_params,
	.gpio_tgp = &gpio_toggle_params,
	.gpio_ldac = &gpio_ldac_params,
	.gpio_fault = &gpio_fault_params
};