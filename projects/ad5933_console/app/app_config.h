/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file of AD5933 firmware example program
******************************************************************************
* Copyright (c) 2019, 2022, 2025 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

#include <stdint.h>
#include "ad5933.h"

#define	MBED_PLATFORM		1
#define STM32_PLATFORM      2

/* Select the Active Platform */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"
#define spi_init_extra_params	mbed_spi_extra_init_params
#define spi_ops mbed_spi_ops
#define i2c_init_extra_params   mbed_i2c_extra_init_params
#else
#include "app_config_stm32.h"
#define spi_init_extra_params  stm32_spi_extra_init_params
#define spi_ops stm32_spi_ops
#define uart_extra_init_params 	stm32_uart_extra_init_params
#define i2c_init_extra_params   stm32_i2c_extra_init_params
#endif

#endif //_APP_CONFIG_H_
