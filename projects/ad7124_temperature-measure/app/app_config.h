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
/********************** Macros and Constants Definition **********************/
/******************************************************************************/

/* List of platforms supported */
#define	MBED_PLATFORM		1
#define STM32_PLATFORM      2

/* Supported AD7124 devices (One selected at a time, default is AD7124-4) */
//#define		DEV_AD7124_4

#if defined(DEV_AD7124_4)
#define ACTIVE_DEVICE	"AD7124-4"
#elif defined(DEV_AD7124_8)
#define ACTIVE_DEVICE	"AD7124-8"
#else
#define ACTIVE_DEVICE	"AD7124-8"
#warning "No active device selected. AD7124-8 is assumed as default"
#endif
/* Select the active platform  */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"
#define spi_init_extra_params	mbed_spi_extra_init_params
#define spi_ops mbed_spi_ops
#else
#include "app_config_stm32.h"
#define spi_init_extra_params  stm32_spi_extra_init_params
#define uart_extra_init_params 	stm32_uart_extra_init_params
#define spi_ops stm32_spi_ops
#define uart_ops stm32_uart_ops
#endif

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

extern struct no_os_uart_init_param uart_init_params;
extern struct no_os_spi_init_param spi_init_params;

#endif //_APP_CONFIG_H_
