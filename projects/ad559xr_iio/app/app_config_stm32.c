/***************************************************************************//**
 * @file    app_config_stm32.c
 * @brief   STM32 Specific configuration files for AD559xr IIO Application
 * @details This module contains the STM32 platform specific configurations
********************************************************************************
* Copyright (c) 2026 Analog Devices, Inc.
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

/* UART STM32 Platform Specific Init Parameters */
struct stm32_uart_init_param stm32_uart_extra_init_params = {
	.huart = APP_UART_HANDLE
};

/* SPI STM32 Platform Specific Init Parameters */
struct stm32_spi_init_param stm32_spi_extra_init_params = {
	.chip_select_port = SPI_CS_PORT,
	.get_input_clock = HAL_RCC_GetPCLK2Freq
};

/* VCOM Init Parameters */
struct stm32_usb_uart_init_param stm32_vcom_extra_init_params = {
	.husbdevice = &APP_UART_USB_HANDLE
};

/* STM32 PWM specific parameters */
struct stm32_pwm_init_param stm32_pwm_extra_init_params = {
	.htimer = APP_TIM_HANDLE,
	.prescaler = TIMER_1_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM2,
	.timer_chn = TIMER_CHANNEL_3,
	.get_timer_clock = HAL_RCC_GetPCLK1Freq,
	.clock_divider = TIMER_1_CLK_DIVIDER
};

/* PWM extra params */
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
struct stm32_pwm_desc *pwm_extra_params;
#endif

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
	MX_SPI1_Init();
	MX_UART5_Init();
	MX_I2C1_Init();
	MX_TIM1_Init();
	/* Initialize USB device */
	MX_USB_DEVICE_Init();
}
