/*!
 *****************************************************************************
  @file:  app_config.h
  @brief: AD5592R/AD5593R device selection. Pin mappings.
  @details:
 -----------------------------------------------------------------------------
 Copyright (c) 2020, 2022, 2025 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
*****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

#include <stdint.h>

/******************************************************************************/
/************************* Macros & Constant Definitions ***************************/
/******************************************************************************/

#define	MBED_PLATFORM		1
#define STM32_PLATFORM      2

/* Select the active platform  */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

// Supported Devices
#define DEV_AD5592R 0
#define DEV_AD5593R 1

#define AD5593R_A0_STATE 0

// **** Note for User: ACTIVE_DEVICE selection ****
// Define the device type here from the list of below device type defines
// e.g. #define ACTIVE_DEVICE ID_AD5593R -> This will set AD5593R as an ACTIVE_DEVICE.
// The ACTIVE_DEVICE is default set to AD5592R, if device type is not defined.

#if !defined(ACTIVE_DEVICE)
#define ACTIVE_DEVICE  DEV_AD5592R
#endif

#define NUM_CHANNELS 8

#define AD5593R_I2C (0x10 | (AD5593R_A0_STATE & 0x01))

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"
#define spi_init_extra_params	mbed_spi_extra_init_params
#define i2c_init_extra_params   mbed_i2c_extra_init_params
#else
#include "app_config_stm32.h"
#define spi_init_extra_params  stm32_spi_extra_init_params
#define i2c_init_extra_params   stm32_i2c_extra_init_params
#define uart_extra_init_params 	stm32_uart_extra_init_params
#endif

#endif //_APP_CONFIG_H_
