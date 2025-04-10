/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD717x/AD411x firmware example
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

/* List of active platforms supported */
#define	MBED_PLATFORM		1
#define STM32_PLATFORM      2

/* Select the active platform  */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

// **** Note for User: ACTIVE_DEVICE selection ****//
/* Define the device type here from the list of below device type defines
 * (one at a time. Defining more than one device can result into compile error).
 * e.g. #define DEV_AD4111 -> This will make AD4111 as an ACTIVE_DEVICE.
 * The ACTIVE_DEVICE is default set to AD4111, if device type is not defined.
 * */
//#define	DEV_AD4111

#if defined(DEV_AD4111)
#define ACTIVE_DEVICE_NAME	"AD4111"
#elif defined(DEV_AD4112)
#define ACTIVE_DEVICE_NAME	"AD4112"
#elif defined(DEV_AD4114)
#define ACTIVE_DEVICE_NAME	"AD4114"
#elif defined(DEV_AD4115)
#define ACTIVE_DEVICE_NAME	"AD4115"
#elif defined(DEV_AD4116)
#define ACTIVE_DEVICE_NAME	"AD4116"
#elif defined(DEV_AD7172_2)
#define AD7172_2_INIT
#define ACTIVE_DEVICE_NAME	"AD7172-2"
#elif defined(DEV_AD7172_4)
#define AD7172_4_INIT
#define ACTIVE_DEVICE_NAME	"AD7172-4"
#elif defined(DEV_AD7173_8)
#define AD7173_8_INIT
#define ACTIVE_DEVICE_NAME	"AD7173-8"
#elif defined(DEV_AD7175_2)
#define AD7175_2_INIT
#define ACTIVE_DEVICE_NAME	"AD7175-2"
#elif defined(DEV_AD7175_8)
#define AD7175_8_INIT
#define ACTIVE_DEVICE_NAME	"AD7175-8"
#elif defined(DEV_AD7176_2)
#define AD7176_2_INIT
#define ACTIVE_DEVICE_NAME	"AD7176-2"
#elif defined(DEV_AD7177_2)
#define AD7177_2_INIT
#define ACTIVE_DEVICE_NAME	"AD7177-2"
#else
#warning No/Unsupported ADxxxxy symbol defined. AD4111 defined
#define DEV_AD4111
#define ACTIVE_DEVICE_NAME	"AD4111"
#endif

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"
#define spi_init_extra_params	mbed_spi_extra_init_params
#else
#include "app_config_stm32.h"
#define spi_init_extra_params  stm32_spi_extra_init_params
#define uart_extra_init_params 	stm32_uart_extra_init_params
#endif

/* Denominator of the scale factor to be applied while converting raw values to actual voltage */
#if  defined(DEV_AD4111) || defined(DEV_AD4112) || \
	defined(DEV_AD4114) || defined(DEV_AD4115) || defined (DEV_AD4116)
#define SCALE_FACTOR_DR			0.1
#else
#define SCALE_FACTOR_DR			1
#endif

#endif //_APP_CONFIG_H_
