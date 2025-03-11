/***************************************************************************//**
 * @file    app_config_stm32.c
 * @brief   STM32 Specific configuration files for AD5770R Console Application
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

#include "no_os_error.h"
#include "app_config_stm32.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* SPI STM32 Platform Specific Init Parameters */
struct stm32_spi_init_param stm32_spi_extra_init_params = {
	.chip_select_port = SPI_CS_PORT,
	.get_input_clock = HAL_RCC_GetPCLK2Freq
};

/* STM32 UART specific parameters */
struct stm32_uart_init_param stm32_uart_extra_init_params = {
	.huart = APP_UART_HANDLE
};

/* LDAC pin STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_ldac_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH
};

/******************************************************************************/
/************************** Functions Declaration *****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definition ******************************/
/******************************************************************************/

/**
 * @brief Initialize the STM32 system peripherals
 * @return None
 */
void stm32_system_init(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
#if defined(ARDUINO)
	MX_SPI1_Init();
#else
	MX_SPI5_Init();
#endif
	MX_UART5_Init();
}

