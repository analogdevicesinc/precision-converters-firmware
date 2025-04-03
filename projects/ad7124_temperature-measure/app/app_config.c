/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module
 *   @details This module contains the configurations needed for IIO application
********************************************************************************
 * Copyright (c) 2025 Analog Devices, Inc.
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

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* UART init parameters structure */
struct no_os_uart_init_param uart_init_params = {
	.device_id = 0,
	.baud_rate = 230400,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
	.irq_id = UART_IRQ_ID,
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	.asynchronous_rx = false,
	.platform_ops = &uart_ops,
	.extra = &uart_extra_init_params
#endif
};

/* Designated SPI Initialization Structure */
struct no_os_spi_init_param	spi_init_params = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = MAX_SPI_CLK, // Max SPI Speed
	.chip_select =  SPI_CSB, // Chip Select pin
	.mode = NO_OS_SPI_MODE_3, // CPOL = 1, CPHA =1
	.extra = &spi_init_extra_params,  // SPI extra configurations
	.platform_ops = &spi_ops
};

