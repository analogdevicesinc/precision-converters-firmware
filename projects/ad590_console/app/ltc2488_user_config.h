/*************************************************************************//**
 *   @file   ltc2488_user_config.h
 *   @brief  Header for ltc2488 user configuration file
******************************************************************************
* Copyright (c) 2021-22,2025 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef LTC2488_USER_CONFIG_H_
#define LTC2488_USER_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* List of platforms supported */
#define	MBED_PLATFORM		1
#define STM32_PLATFORM      2

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

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
extern struct ltc2488_dev_init ltc2488_init_str;

#endif
