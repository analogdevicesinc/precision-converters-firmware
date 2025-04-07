/***************************************************************************//**
 *   @file    app_config_stm32.c
 *   @brief   Application configurations module for STM32 platform
********************************************************************************
 * Copyright (c) 2024 Analog Devices, Inc.
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
struct stm32_uart_init_param stm32_uart_extra_init_params = {
	.huart = APP_UART_HANDLE
};

/* I2C STM32 Platform Specific Init Parameters */
struct stm32_i2c_init_param stm32_i2c_extra_init_params = {
	.i2c_timing = I2C_TIMING
};

/* STM32 GPIO IRQ specific parameters */
struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params = {
	.port_nb = GPIO_TRIGGER_INT_PORT
};

/* SPI STM32 Platform Specific Init Parameters */
struct stm32_spi_init_param stm32_spi_extra_init_params = {
	.chip_select_port = STM32_SPI_CS_PORT,
	.get_input_clock = HAL_RCC_GetPCLK2Freq
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_clear_gpio_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* VCOM Init Parameter */
struct stm32_usb_uart_init_param stm32_vcom_extra_init_params = {
	.husbdevice = &APP_UART_USB_HANDLE
};

/* LDAC pin STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_ldac_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* LDAC pin STM32 GPIO in PWM alternate function mode specific parameters */
struct stm32_gpio_init_param stm32_pwm_ldac_gpio_init_params = {
	.mode = GPIO_MODE_AF_PP,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
	.alternate = GPIO_AF2_TIM4
};

/* Reset pin STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_gpio_reset_init_params = {
	.mode = GPIO_MODE_OUTPUT_PP
};

/* STM32 LDAC PWM specific parameters */
struct stm32_pwm_init_param stm32_pwm_extra_init_params = {
	.htimer = &LDAC_PWM_HANDLE,
	.prescaler = LDAC_PWM_PRESCALER,
	.timer_autoreload = true,
	.mode = TIM_OC_PWM1,
	.timer_chn = LDAC_PWM_CHANNEL,
	.get_timer_clock = HAL_RCC_GetPCLK2Freq,
	.clock_divider = LDAC_PWM_CLK_DIVIDER
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
	MX_I2C1_Init();
	MX_UART5_Init();
	MX_TIM4_Init();
	MX_USB_DEVICE_Init();
}

