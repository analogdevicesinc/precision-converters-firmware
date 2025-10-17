/***************************************************************************//**
 *   @file app_config_stm32.h
 *   @brief Header file for STM32 platform configurations
********************************************************************************
 * Copyright (c) 2024-25 Analog Devices, Inc.
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
#include "no_os_irq.h"
#include "main.h"
/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
#ifdef STM32H563xx
/* The below configurations are specific to STM32H563ZIT6 MCU on NUCLEO-H563ZI Board. */
#define HW_CARRIER_NAME		NUCLEO-H563ZI

/* STM32 SPI Specific parameters */
#define SPI_DEVICE_ID		1 // SPI1
#define SPI_CS_PORT	3  // GPIO Port D
#define SPI_CSB				14 // PD_14

/* STM32 UART specific parameters */
#define APP_UART_HANDLE 	huart3

/* UART Device ID */
#define UART_IRQ_ID			USART3_IRQn

/* RDY specific Port and Pin */
#define RDY_PORT	5
#define RDY_PIN 3

#define RDY_GPIO_PRIORITY 1

#define I2C_DEVICE_ID           1 // I2C1

/* I2C timing register value for standard mode of operation
 * Check here for more understanding on I2C timing register
 * configuration: https://wiki.analog.com/resources/no-os/drivers/i2c */
#define I2C_TIMING				0x00000E14

#else
/* The below configurations are specific to STM32769NI MCU on Disco-F769NI Board. */
#define HW_CARRIER_NAME		DISCO-F769NI

/* STM32 SPI Specific parameters */
#define SPI_DEVICE_ID       2 // SPI1
#define SPI_CS_PORT         0  // GPIO Port D
#define SPI_CSB             11 // PA_11

/* STM32 UART specific parameters */
#define APP_UART_HANDLE 	huart6

/* UART Device ID */
#define UART_IRQ_ID			USART6_IRQn

/* RDY specific Port and Pin */
#define RDY_PORT           9 // GPIO Port J
#define RDY_PIN            4 // PJ_4

#define I2C_DEVICE_ID       1 // I2C1

/* I2C timing register value for standard mode of operation
 * Check here for more understanding on I2C timing register
 * configuration: TODO*/
#define I2C_TIMING      0x40912732

/* Ticker for Pocket Lab */
#define LVGL_TICK_TIME_US       5000
#define LVGL_TICK_TIME_MS       (LVGL_TICK_TIME_US / 1000)

#endif
/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
extern UART_HandleTypeDef APP_UART_HANDLE;

extern void stm32_system_init();

#endif // APP_CONFIG_STM32_H_
