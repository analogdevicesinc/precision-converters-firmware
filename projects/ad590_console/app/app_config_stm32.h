/***************************************************************************//**
 *   @file app_config_stm32.h
 *   @brief Header file for STM32 platform configurations
********************************************************************************
 * Copyright (c) 2025 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef APP_CONFIG_STM32_H_
#define APP_CONFIG_STM32_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "stm32_spi.h"
#include "stm32_uart.h"
#include "stm32_irq.h"
#include "stm32_uart_stdio.h"
#include "main.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Note: The SDP-K1 board with the STM32F469NI MCU has been used
* for developing the firmware. The below parameters will change depending
* on the controller used. */

/**
  The ADI SDP_K1 can be used with both arduino headers
  or the 120-pin SDP connector found on ADI evaluation
  boards. The default is the Arduino connector

  Comment the SDP_120 #define below to enable the Arduino connector
*/

//#define SDP_120

#ifdef SDP_120

/* STM32 SPI Specific parameters */
#define SPI_DEVICE_ID		5 // SPI5
#define SPI_CS_PORT	1  // GPIO Port B
#define SPI_SS				9 // PB_9
#else
#define SPI_DEVICE_ID		1 // SPI1
#define SPI_CS_PORT	0  // GPIO Port A
#define SPI_SS				15 // PA_15
#endif

/* STM32 UART specific parameters */
#define APP_UART_HANDLE     &huart5
#define UART_IRQ_ID			UART5_IRQn

/* Max SPI Clock Speed */
#define MAX_SPI_CLK 2000000

/* platform ops */
#define spi_ops stm32_spi_ops
#define uart_ops stm32_uart_ops

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct no_os_uart_desc *uart_desc;
extern UART_HandleTypeDef huart5;

extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;

extern void stm32_system_init(void);
extern int32_t check_escape_key_pressed();

#endif // APP_CONFIG_STM32_H_
