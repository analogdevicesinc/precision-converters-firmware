/***************************************************************************//**
 *   @file    app_config_stm32.c
 *   @brief   Application configurations module for STM32 platform
********************************************************************************
 * Copyright (c) 2024-25 Analog Devices, Inc.
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
#include "usb_device.h"

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

/* VCOM STM32 Platform Specific parameters */
struct stm32_usb_uart_init_param stm32_vcom_extra_init_params = {
	.husbdevice = &hUsbDeviceHS,
};

/* SPI STM32 Platform Specific Init Parameters */
struct stm32_spi_init_param stm32_spi_init_params = {
	.chip_select_port = STM32_SPI_CS_PORT,
	.get_input_clock = HAL_RCC_GetPCLK2Freq
};

/* STM32 LDAC GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_ldac_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 Clear GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_clear_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 Toggle GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_toggle_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 Fault GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_fault_params = {
	.mode = GPIO_MODE_INPUT,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_toggle_pwm_gpio_params = {
	.mode = GPIO_MODE_AF_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
	.alternate = GPIO_AF1_TIM1
};

/* STM32 PWM toggle parameters */
struct stm32_pwm_init_param stm32_toggle_pwm_init_params = {
	.htimer = &TOGGLE_PWM_HANDLE,
	.prescaler = TOGGLE_PWM_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM2,
	.timer_chn = TOGGLE_PWM_CHANNEL,
	.complementary_channel = false,
	.get_timer_clock = HAL_RCC_GetPCLK2Freq,
	.clock_divider = TOGGLE_PWM_CLK_MULTIPLIER
};

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

void SystemClock_Config(void);

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
	MX_TIM1_Init();
	MX_USB_DEVICE_Init();
}