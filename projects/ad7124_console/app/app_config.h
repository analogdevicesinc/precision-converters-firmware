/*************************************************************************//**
*   @file   app_config.h
*   @brief  Configuration file of AD7124 firmware example program
******************************************************************************
* Copyright (c) 2020,2022,2025 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/* List of platforms supported */
#define	MBED_PLATFORM		1
#define STM32_PLATFORM      2

// **** Note for User on selection of Active Device ****//
/* Define the device type here from the list of below device type defines
 * (one at a time. Defining more than one device can result into compile error).
 * e.g. #define DEV_AD7142_4= -> This will make AD7124-4 as an active device.
 * The active device is default set to AD7124-4 if device type is not defined.
 * */
#define DEV_AD7124_4

/* Select the active platform  */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"
#define spi_init_extra_params	mbed_spi_extra_init_params
#else
#include "app_config_stm32.h"
#define spi_init_extra_params  stm32_spi_extra_init_params
#define uart_extra_init_params 	stm32_uart_extra_init_params
#endif

#endif //_APP_CONFIG_H_
