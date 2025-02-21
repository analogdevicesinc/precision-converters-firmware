/***************************************************************************//**
 * @file    app_config_stm32.c
 * @brief   STM32 Specific configuration files for AD590 Console Application
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
#include "adi_console_menu.h"

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
#if defined(SDP_120)
	MX_SPI5_Init();
#else
	MX_SPI1_Init();
#endif
	MX_UART5_Init();
}

/*!
 * @brief   Determines if the Escape key was pressed
 * @return  key press status
 */
int32_t check_escape_key_pressed()
{
	int32_t wasPressed = 0;
	int32_t ret;
	uint8_t data;
	struct stm32_uart_desc *sud = uart_desc->extra;
	struct no_os_irq_init_param nvic_ip = {
		.platform_ops = &stm32_irq_ops,
		.extra = sud->huart,
	};

	/* Make UART ready for new reception */
	sud->huart->RxState = HAL_UART_STATE_READY;

	ret = no_os_irq_ctrl_init(&sud->nvic, &nvic_ip);
	if (ret) {
		return ret;
	}

	ret = no_os_irq_enable(sud->nvic, uart_desc->irq_id);
	if (ret) {
		return ret;
	}

	HAL_UART_Receive_IT(sud->huart, (uint8_t *)&data, 1);

	if (data == ESCAPE_KEY_CODE) {
		wasPressed = 1;
	}

	HAL_UART_AbortReceive_IT(sud->huart);

	return wasPressed;
}
