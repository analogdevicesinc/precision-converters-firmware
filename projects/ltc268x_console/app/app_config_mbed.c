/***************************************************************************//**
 *   @file    app_config_mbed.c
 *   @brief   Application configurations module for Mbed platform
********************************************************************************
 * Copyright (c) 2022 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdbool.h>
#include "app_config_mbed.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/
/* SPI Mbed platform specific parameters */
struct mbed_spi_init_param mbed_spi_extra_init_params = {
	.spi_clk_pin = SPI_SCK,
	.spi_miso_pin = SPI_HOST_SDI,
	.spi_mosi_pin = SPI_HOST_SDO,
	.use_sw_csb = true
};
