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
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
#include "pl_gui_events.h"
#endif

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/************************** Note **************************/
/* Most of the configurations specific to SPI_DMA implementation
 * such as clock configuration, timer master or slave mode,
 * has been done through the auto generated initialization code.
 * */

/* STM32 UART specific parameters */
struct stm32_uart_init_param stm32_uart_extra_init_params = {
	.huart = &APP_UART_HANDLE,
};

/* STM32 SPI specific parameters */
struct stm32_spi_init_param stm32_spi_extra_init_params = {
	.chip_select_port = SPI_CS_PORT_NUM,
	.get_input_clock = HAL_RCC_GetPCLK2Freq
};

/* STM32 GPIO specific parameters */
struct stm32_gpio_init_param stm32_trigger_gpio_extra_init_params = {
	.mode = GPIO_MODE_INPUT,
	.speed = GPIO_SPEED_FREQ_VERY_HIGH,
};

/* STM32 GPIO IRQ specific parameters */
struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params = {
	.port_nb = CNV_PORT_NUM /* Port B */
};

#ifdef STM32F469xx
/* VCOM Init Parameter */
struct stm32_usb_uart_init_param stm32_vcom_extra_init_params = {
	.husbdevice = &APP_UART_USB_HANDLE
};
#endif

#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
/* LVGL tick counter */
static uint32_t lvgl_tick_counter = 0;
#endif

/******************************************************************************/
/************************** Functions Declaration *****************************/
/******************************************************************************/


/******************************************************************************/
/************************** Functions Definition ******************************/
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
	MX_I2C1_Init();
#ifdef  STM32F469xx
	MX_UART5_Init();
	MX_SPI1_Init();
	MX_USB_DEVICE_Init();
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
	if (lvgl_tick_counter >= 5) {
		// 5ms interval (if SysTick is 1ms)
		pl_gui_lvgl_tick_update(LVGL_TICK_TIME_MS); // or LVGL_TICK_TIME_MS
		lvgl_tick_counter = 0;
	}
}
#endif

