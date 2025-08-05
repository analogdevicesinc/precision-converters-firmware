/*****************************************************************************
 *   @file   app_config.h
 *
 *   @brief  Configuration file of AD5770R firmware example program
******************************************************************************
 *
Copyright (c) 2020-2022,2025 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.
By using this software you agree to the terms of the associated
Analog Devices Software License Agreement.
 ******************************************************************************/

#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

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

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#define spi_init_extra_params  stm32_spi_extra_init_params
#define uart_extra_init_params 	stm32_uart_extra_init_params
#define hw_ldacb_extra_init_params stm32_gpio_ldac_init_params
#endif

#endif /* APP_CONFIG_H_ */
