/***************************************************************************//**
 *   @file app_config_stm32.h
 *   @brief Header file for STM32 platform configurations
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

#include "stm32_uart.h"
#include "stm32_spi.h"
#include "stm32_i2c.h"
#include "stm32_gpio.h"
#include "stm32_irq.h"
#include "stm32_gpio_irq.h"
#include "main.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Note: The SDP-K1 board with the STM32F469NI MCU has been used
* for developing the firmware. The below parameters will change depending
* on the controller used. */
#define HW_CARRIER_NAME		SDP-K1

/* STM32 SPI Specific parameters */
#define MAX_SPI_BAUDRATE    5000000
#define SPI_DEVICE_ID		1 // SPI1
#define SPI_CS_PORT	        0 // GPIO Port A
#define SPI_CSB				15 // PA_15

/* STM32 UART specific parameters */
#define APP_UART_HANDLE     &huart5

/* UART Device ID */
#define UART_IRQ_ID			UART5_IRQn

/* I2C Device ID */
#define I2C_DEVICE_ID           1 // I2C1

/* ODR1 Specific Port C and Pin 1 */
#define ODR1_PIN                1 // PC_1
#define ODR1_PORT               2 // Port C

/* ODR2 Specific Port A and Pin 6 */
#define ODR2_PIN                6 // PA_6
#define ODR2_PORT               0 // Port A

/* PGA1 Specific Port G and Pin 11 */
#define PGA1_PIN                11 // PG_11
#define PGA1_PORT               6

/* PGA2 Specific Port G and Pin 10 */
#define PGA2_PIN                10 // PG_10
#define PGA2_PORT               6

/* CLKSEL Specific Port G and Pin 7 */
#define CLKSEL_PIN              7  // PG_7
#define CLKSEL_PORT             6

/* CHAN Specific Port A and Pin 2 */
#define CHAN_PIN                2 // PA_2
#define CHAN_PORT               0

/* RDY Specific Port D and Pin 12 */
#define RDY_PIN                12 // PD_12
#define RDY_PORT               3

/* Priority for Ready Interrupt */
#define RDY_GPIO_PRIORITY 1
/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart5;
extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
extern void stm32_system_init(void);

#endif // APP_CONFIG_STM32_H_
