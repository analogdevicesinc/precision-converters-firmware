/***************************************************************************//**
 *   @file    app_config_stm32.c
 *   @brief   Application configurations module for STM32 platform
********************************************************************************
 * Copyright (c) 2023-2024 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "app_config_stm32.h"
#include "no_os_error.h"
#include "no_os_util.h"
/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* UART STM32 Platform Specific Init Parameters */
struct stm32_uart_init_param stm32_uart_init_params = {
	.huart = APP_UART_HANDLE
};

/* VCOM STM32 Platform Specific Init Parameters */
struct stm32_usb_uart_init_param stm32_vcom_extra_init_params = {
	.husbdevice = &APP_UART_USB_HANDLE
};

/* SPI STM32 Platform Specific Init Parameters */
struct stm32_spi_init_param stm32_spi_init_params = {
	.chip_select_port = STM32_SPI_CS_PORT,
	.get_input_clock = HAL_RCC_GetPCLK2Freq
};

/* SPI STM32 Platform Specific Init Parameters */
struct stm32_gpio_init_param stm32_reset_gpio_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/* SPI STM32 Platform Specific Init Parameters */
struct stm32_gpio_init_param stm32_wp_gpio_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/* SPI STM32 Platform Specific Init Parameters */
struct stm32_gpio_init_param stm32_lrdac_gpio_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/* SPI STM32 Platform Specific Init Parameters */
struct stm32_gpio_init_param stm32_dis_gpio_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/* SPI STM32 Platform Specific Init Parameters */
struct stm32_gpio_init_param stm32_indep_gpio_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/

/**
 * @brief 	Initialize the STM32 system peripherals
 * @return	None
 */
void stm32_system_init(void)
{
	HAL_Init();
	SystemClock_Config();
	MX_GPIO_Init();
	MX_SPI1_Init();
	MX_UART5_Init();
	MX_USB_DEVICE_Init();
}
