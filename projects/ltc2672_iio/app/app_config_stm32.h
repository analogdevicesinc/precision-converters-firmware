/***************************************************************************//**
 *   @file    app_config_stm32.h
 *   @brief   Header file for STM32 platform configurations
********************************************************************************
 * Copyright (c) 2024 Analog Devices, Inc.
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

#include <stdint.h>
#include "stm32_uart.h"
#include "stm32_spi.h"
#include "stm32_gpio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Note: The SDP-K1 board with the STM32F469NI MCU has been used
 * for developing the firmware. The below parameters will change depending
 * on the controller used. */
#define TARGET_NAME SDP_K1

/* STM32 UART specific parameters */
#define APP_UART_HANDLE &huart5
#define UART_IRQ_ID UART5_IRQn

/* STM32 SPI Specific parameters */
#define SPI_DEVICE_ID       1  //SPI1
#define SPI_CSB             15 //PA15
#define STM32_SPI_CS_PORT   0  //PORTA

/* Peripheral IDs (Unused) */
#define UART_ID 0

/* Redefine the init params structure mapping wrt platform */
#define spi_extra_init_params stm32_spi_init_params
#define uart_extra_init_params stm32_uart_init_params

/* Redefine platform ops mapping wrt STM32 platform */
#define gpio_ops stm32_gpio_ops
#define spi_ops stm32_spi_ops
#define uart_ops stm32_uart_ops

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct stm32_uart_init_param stm32_uart_init_params;
extern struct stm32_spi_init_param stm32_spi_init_params;

extern UART_HandleTypeDef huart5;

void stm32_system_init(void);

#endif /* APP_CONFIG_STM32_H_ */
