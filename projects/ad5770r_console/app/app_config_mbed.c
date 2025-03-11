/***************************************************************************//**
 * @file    app_config_mbed.c
 * @brief   Source file for the mbed configuration for AD5770r Console Application
********************************************************************************
* Copyright (c) 2025 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "app_config.h"
#include "app_config_mbed.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* SPI MBED Platform Specific Init Parameters */
struct mbed_spi_init_param mbed_spi_extra_init_params = {
	.spi_clk_pin = SPI_SCK,
	.spi_miso_pin = SPI_HOST_SDI,
	.spi_mosi_pin = SPI_HOST_SDO,
};

/* GPIO Mbed platform specific init parameters */
struct mbed_gpio_init_param hw_ldacb_extra_init_params = {
	.pin_mode = 0	// NA
};

/******************************************************************************/
/************************** Functions Declaration *****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definition ******************************/
/******************************************************************************/
