/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file of nanodac firmware example program
******************************************************************************
* Copyright (c) 2020, 2022 , 2025 Analog Devices, Inc.
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
#include <common_macros.h>

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/* Select the Active Platform */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

// **** Note for User: ACTIVE_DEVICE selection ****
// Define the device type here from the list of below device type defines
// e.g. #define DEV_AD5677R -> This will make AD5677R as an ACTIVE_DEVICE.
// The ACTIVE_DEVICE is default set to AD5686, if device type is not defined.

//#define DEV_AD5677R

#if defined(DEV_AD5671R)
#define ACTIVE_DEVICE		ID_AD5671R
#define ACTIVE_DEVICE_NAME	"AD5671R"
#elif defined(DEV_AD5672R)
#define ACTIVE_DEVICE		ID_AD5672R
#define ACTIVE_DEVICE_NAME	"AD5672R"
#elif defined(DEV_AD5673R)
#define ACTIVE_DEVICE		ID_AD5673R
#define ACTIVE_DEVICE_NAME	"AD5673R"
#elif defined(DEV_AD5674)
#define ACTIVE_DEVICE		ID_AD5674
#define ACTIVE_DEVICE_NAME	"AD5674"
#elif defined(DEV_AD5674R)
#define ACTIVE_DEVICE		ID_AD5674R
#define ACTIVE_DEVICE_NAME	"AD5674R"
#elif defined(DEV_AD5675R)
#define ACTIVE_DEVICE		ID_AD5675R
#define ACTIVE_DEVICE_NAME	"AD5675R"
#elif defined(DEV_AD5676)
#define ACTIVE_DEVICE		ID_AD5676
#define ACTIVE_DEVICE_NAME	"AD5676"
#elif defined(DEV_AD5676R)
#define ACTIVE_DEVICE		ID_AD5676R
#define ACTIVE_DEVICE_NAME	"AD5676R"
#elif defined(DEV_AD5677R)
#define ACTIVE_DEVICE		ID_AD5677R
#define ACTIVE_DEVICE_NAME	"AD5677R"
#elif defined(DEV_AD5679)
#define ACTIVE_DEVICE		ID_AD5679
#define ACTIVE_DEVICE_NAME	"AD5679"
#elif defined(DEV_AD5679R)
#define ACTIVE_DEVICE		ID_AD5679R
#define ACTIVE_DEVICE_NAME	"AD5679R"
#elif defined(DEV_AD5686)
#define ACTIVE_DEVICE		ID_AD5686
#define ACTIVE_DEVICE_NAME	"AD5686"
#elif defined(DEV_AD5684R)
#define ACTIVE_DEVICE		ID_AD5684R
#define ACTIVE_DEVICE_NAME	"AD5684R"
#elif defined(DEV_AD5685R)
#define ACTIVE_DEVICE		ID_AD5685R
#define ACTIVE_DEVICE_NAME	"AD5685R"
#elif defined(DEV_AD5686R)
#define ACTIVE_DEVICE		ID_AD5686R
#define ACTIVE_DEVICE_NAME	"AD5686R"
#elif defined(DEV_AD5687)
#define ACTIVE_DEVICE		ID_AD5687
#define ACTIVE_DEVICE_NAME	"AD5687"
#elif defined(DEV_AD5687R)
#define ACTIVE_DEVICE		ID_AD5687R
#define ACTIVE_DEVICE_NAME	"AD5687R"
#elif defined(DEV_AD5689)
#define ACTIVE_DEVICE		ID_AD5689
#define ACTIVE_DEVICE_NAME	"AD5689"
#elif defined(DEV_AD5689R)
#define ACTIVE_DEVICE		ID_AD5689R
#define ACTIVE_DEVICE_NAME	"AD5689R"
#elif defined(DEV_AD5697R)
#define ACTIVE_DEVICE		ID_AD5697R
#define ACTIVE_DEVICE_NAME	"AD5697R"
#elif defined(DEV_AD5694)
#define ACTIVE_DEVICE		ID_AD5694
#define ACTIVE_DEVICE_NAME	"AD5694"
#elif defined(DEV_AD5694R)
#define ACTIVE_DEVICE		ID_AD5694R
#define ACTIVE_DEVICE_NAME	"AD5694R"
#elif defined(DEV_AD5695R)
#define ACTIVE_DEVICE		ID_AD5695R
#define ACTIVE_DEVICE_NAME	"AD5695R"
#elif defined(DEV_AD5696)
#define ACTIVE_DEVICE		ID_AD5696
#define ACTIVE_DEVICE_NAME	"AD5696"
#elif defined(DEV_AD5696R)
#define ACTIVE_DEVICE		ID_AD5696R
#define ACTIVE_DEVICE_NAME	"AD5696R"
#elif defined(DEV_AD5681R)
#define ACTIVE_DEVICE		ID_AD5681R
#define ACTIVE_DEVICE_NAME	"AD5681R"
#elif defined(DEV_AD5682R)
#define ACTIVE_DEVICE		ID_AD5682R
#define ACTIVE_DEVICE_NAME	"AD5682R"
#elif defined(DEV_AD5683R)
#define ACTIVE_DEVICE		ID_AD5683R
#define ACTIVE_DEVICE_NAME	"AD5683R"
#elif defined(DEV_AD5683)
#define ACTIVE_DEVICE		ID_AD5683
#define ACTIVE_DEVICE_NAME	"AD5683"
#elif defined(DEV_AD5691R)
#define ACTIVE_DEVICE		ID_AD5691R
#define ACTIVE_DEVICE_NAME	"AD5691R"
#elif defined(DEV_AD5692R)
#define ACTIVE_DEVICE		ID_AD5692R
#define ACTIVE_DEVICE_NAME	"AD5692R"
#elif defined(DEV_AD5693R)
#define ACTIVE_DEVICE		ID_AD5693R
#define ACTIVE_DEVICE_NAME	"AD5693R"
#elif defined(DEV_AD5693)
#define ACTIVE_DEVICE		ID_AD5693
#define ACTIVE_DEVICE_NAME	"AD5693"
#else
#warning No/Unsupported ADxxxxy symbol defined. AD5686R defined
#define DEV_AD5686R
#define ACTIVE_DEVICE		ID_AD5686R
#define ACTIVE_DEVICE_NAME	"AD5686R"
#endif


#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#define spi_init_extra_params  stm32_spi_extra_init_params
#define uart_extra_init_params 	stm32_uart_extra_init_params
#define i2c_init_extra_params   stm32_i2c_extra_init_params
#define reset_gpio_extra_init_params stm32_gpio_reset_init_params
#define ldac_gpio_extra_init_params stm32_gpio_ldac_init_params
#define gain_gpio_extra_init_params stm32_gain_gpio_init_params
#endif

#endif //_APP_CONFIG_H_
