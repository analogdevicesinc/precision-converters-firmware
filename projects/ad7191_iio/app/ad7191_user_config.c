/*************************************************************************//**
 *   @file   ad7191_user_config.c
 *   @brief  User configuration file for AD7191 device
******************************************************************************
* Copyright (c) 2024 Analog Devices, Inc.
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
#include "ad7191_user_config.h"

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
/* Define SPI init parameters structure for AD7191 */
struct no_os_spi_init_param spi_init_params = {
	.max_speed_hz = MAX_SPI_BAUDRATE, // Max SPI Speed
	.chip_select = SPI_CSB, // Chip Select
	.device_id = SPI_DEVICE_ID,
	.mode = NO_OS_SPI_MODE_3, // CPOL = 1, CPHA = 1
	.platform_ops = &spi_ops,
	.extra = &spi_extra_init_params	 // SPI extra configurations
};

/* GPIO - ODR1 Pin init parameters */
struct no_os_gpio_init_param odr1_init_params = {
	.number = ODR1_PIN,
	.port = ODR1_PORT,
	.platform_ops = &gpio_ops,
};

/* GPIO - ODR2 Pin init parameters */
static struct no_os_gpio_init_param odr2_init_params = {
	.number = ODR2_PIN,
	.port = ODR2_PORT,
	.platform_ops = &gpio_ops,
};

/* GPIO - PGA1 Pin init parameters */
static struct no_os_gpio_init_param pga1_init_params = {
	.number = PGA1_PIN,
	.port = PGA1_PORT,
	.platform_ops = &gpio_ops,
};

/* GPIO - PGA2 Pin init parameters */
static struct no_os_gpio_init_param pga2_init_params = {
	.number = PGA2_PIN,
	.port = PGA2_PORT,
	.platform_ops = &gpio_ops,
};

/* GPIO - CS Pin init parameters */
static struct no_os_gpio_init_param csb_gpio_init_params = {
	.number = SPI_CSB,
	.port = SPI_CS_PORT,
	.platform_ops = &gpio_ops,
};

/* GPIO - RDY Pin init parameters */
static struct no_os_gpio_init_param rdy_gpio_init_params = {
	.number = RDY_PIN,
	.port = RDY_PORT,
	.platform_ops = &gpio_ops,
};

/* Initialize the AD7191 device structure */
struct ad7191_init_param ad7191_init_params = {
	.spi_init = &spi_init_params,
	.odr1_gpio = &odr1_init_params,
	.odr2_gpio = &odr2_init_params,
	.pga1_gpio = &pga1_init_params,
	.pga2_gpio = &pga2_init_params,
	.csb_gpio = &csb_gpio_init_params,
	.rdy_gpio = &rdy_gpio_init_params
};
