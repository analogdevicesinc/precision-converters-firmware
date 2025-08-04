/***************************************************************************//**
 * @file    app_config_stm32.c
 * @brief   STM32 Specific configuration files for AD7124 IIO Application
 * @details This module contains the STM32 platform specific configurations
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

#include "no_os_error.h"
#include "app_config.h"
#include "app_config_stm32.h"
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
#include "pl_gui_events.h"
#endif

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/**
 * @brief Return the peripheral frequency
 * @return Peripheral frequency in Hz
 */
uint32_t HAL_RCC_GetSysClockFreq_app()
{
	return HAL_RCC_GetPCLK2Freq();
}

/* UART STM32 Platform Specific Init Parameters */
struct stm32_uart_init_param stm32_uart_extra_init_params = {
	.huart = &APP_UART_HANDLE
};

/* SPI STM32 Platform Specific Init Parameters */
struct stm32_spi_init_param stm32_spi_extra_init_params = {
	.chip_select_port = SPI_CS_PORT,
	.get_input_clock = HAL_RCC_GetSysClockFreq_app
};

/* STM32 GPIO IRQ specific parameters */
struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params = {
	.port_nb = RDY_PORT
};

/* STM32 I2C specific parameters */
struct stm32_i2c_init_param stm32_i2c_extra_init_params = {
	.i2c_timing = I2C_TIMING
};

#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
/* LVGL tick counter */
static uint32_t lvgl_tick_counter = 0;
#endif

/******************************************************************************/
/************************** Functions Declaration *****************************/
/******************************************************************************/
void SystemClock_Config(void);
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
	MX_I2C1_Init();
	MX_GPIO_Init();
#ifdef STM32H563xx
	MX_SPI1_Init();
	MX_USART3_UART_Init();
	MX_ICACHE_Init();
#else
	MX_SPI2_Init();
	MX_USART6_UART_Init();
#endif
}

/**
 * @brief Systick Handler definition
 * @return none
 */
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
void SysTick_Handler(void)
{
	HAL_IncTick();
	/* USER CODE BEGIN SysTick_IRQn 1 */
	HAL_SYSTICK_IRQHandler();
	/* USER CODE END SysTick_IRQn 1 */
}

/**
 * @brief Systick Callback definition
 * @return none
 */
void HAL_SYSTICK_Callback(void)
{
	lvgl_tick_counter++;
	if (lvgl_tick_counter >= LVGL_TICK_TIME_MS) {
		// 5ms interval (if SysTick is 1ms)
		pl_gui_lvgl_tick_update(LVGL_TICK_TIME_MS);
		lvgl_tick_counter = 0;
	}
}
#endif
