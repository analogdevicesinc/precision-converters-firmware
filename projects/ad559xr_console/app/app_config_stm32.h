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
#include "stm32_gpio.h"
#include "stm32_i2c.h"
#include "stm32_uart_stdio.h"
#include "main.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Note: The SDP-K1 board with the STM32F469NI MCU has been used
* for developing the firmware. The below parameters will change depending
* on the controller used. */

//#define ARDUINO

/* SPI Pins on SDP-K1-Arduino Interface */
#ifdef ARDUINO
#define SPI_DEVICE_ID	1 // SPI1
#define SPI_CS_PORT	    0  // GPIO Port A
#define SPI_CSB			15 // PA_15
#define I2C_DEVICE_ID   1 // I2C1
#else // Default- SDP_120 Interface
/* SPI Pins on SDP-K1-SDP-120 Interface */
#define SPI_DEVICE_ID	5 // SPI5
#define SPI_CS_PORT	    1  // GPIO Port B
#define SPI_CSB			9 // PA_15
#define I2C_DEVICE_ID   3 // I2C3
#endif

#define I2C_TIMING          0 // (Unused)

/* STM32 UART specific parameters */
#define APP_UART_HANDLE    &huart5
#define UART_IRQ_ID		UART5_IRQn

#define GAIN_PIN  11 // PG_11
#define GAIN_PORT  6 // GPIO PORT G

#define RESET_PIN  15  // PB_15
#define RESET_PORT  1  // GPIO Port B

#define LDAC_PIN 10  // PG_10
#define LDAC_PORT 6 // GPIO PORT G

#define ADDR0_PIN 10 // PA_10
#define ADDR0_PORT 0 // GPIO PORT A

/* platform ops */
#define spi_ops stm32_spi_ops
#define uart_ops stm32_uart_ops
#define gpio_ops stm32_gpio_ops
#define i2c_ops stm32_i2c_ops

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct no_os_uart_desc *uart_desc;
extern UART_HandleTypeDef huart5;

extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;

extern void stm32_system_init(void);

#endif // APP_CONFIG_STM32_H_
