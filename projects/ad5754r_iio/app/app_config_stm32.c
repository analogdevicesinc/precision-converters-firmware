/***************************************************************************//**
 * @file    app_config_stm32.c
 * @brief   Source file for STM32 platform configurations
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
#include <stdbool.h>
#include "app_config.h"
#include "app_config_stm32.h"
#include "ad5754r_iio.h"
#include "ad5754r.h"
#include "no_os_pwm.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/**
 * @brief 	Return the peripheral frequency
 * @return	Peripheral frequency in Hz
 */
uint32_t HAL_RCC_GetSysClockFreq_app()
{
	return HAL_RCC_GetPCLK2Freq();
}

/* STM32 UART specific parameters */
struct stm32_uart_init_param stm32_uart_extra_init_params = {
	.huart = APP_UART_HANDLE,
};

/* STM32 SPI specific parameters */
struct stm32_spi_init_param stm32_spi_extra_init_params = {
	.chip_select_port = SPI_CS_PORT,
	.get_input_clock = HAL_RCC_GetPCLK2Freq
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_ldac_gpio_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_clear_gpio_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 GPIO IRQ specific parameters */
struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params = {
	.port_nb = LDAC_GPIO_PORT, /* Port A */
};

/* STM32 PWM GPIO specific parameters */
struct stm32_gpio_init_param stm32_pwm_gpio_init_params = {
	.mode = GPIO_MODE_AF_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
	.alternate = GPIO_AF1_TIM1
};

/* STM32 PWM for specific parameters */
struct stm32_pwm_init_param stm32_pwm_extra_init_params = {
	.htimer = &LDAC_PWM_HANDLE,
	.prescaler = LDAC_PWM_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM1,
	.timer_chn = LDAC_PWM_CHANNEL,
	.get_timer_clock = HAL_RCC_GetPCLK2Freq,
	.clock_divider = LDAC_PWM_CLK_DIVIDER
};

/* VCOM Init Parameter */
struct stm32_usb_uart_init_param stm32_vcom_extra_init_params = {
	.husbdevice = &APP_UART_USB_HANDLE
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
	MX_UART5_Init();
	MX_I2C1_Init();
	MX_SPI1_Init();
	MX_TIM1_Init();
	MX_USB_DEVICE_Init();
}
