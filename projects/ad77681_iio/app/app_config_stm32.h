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

#include "stm32_uart.h"
#include "stm32_spi.h"
#include "stm32_i2c.h"
#include "stm32_gpio.h"
#include "stm32_irq.h"
#include "stm32_gpio_irq.h"
#include "main.h"
#include "stm32_uart_stdio.h"
#include "stm32_usb_uart.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Note: The SDP-K1 board with the STM32F469NI MCU has been used
* for developing the firmware. The below parameters will change depending
* on the controller used. */
#define HW_CARRIER_NAME		SDP-K1

/* STM32 SPI Specific parameters */
#define SPI_DEVICE_ID  1 // SPI1
#define SPI_CS_PORT	 0 // GPIO Port A
#define SPI_CSB  15 // PA_15

/* STM32 UART specific parameters */
#define APP_UART_HANDLE  &huart5
#define APP_UART_USB_HANDLE  hUsbDeviceHS

/* UART Device ID */
#define UART_IRQ_ID  UART5_IRQn

/* I2C Device ID */
#define I2C_DEVICE_ID  1 // I2C1

/* RDY Specific Port D and Pin 12 */
#define CONV_MON  7 // PG_7
#define CONV_MON_PORT  6 // PORTG

#define TRIGGER_INT_ID 7
#define TRIGGER_GPIO_PORT 6
#define TRIGGER_GPIO_PIN  7

/* Priority for Ready Interrupt */
#define CONV_GPIO_PRIORITY 1

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

extern USBD_HandleTypeDef	APP_UART_USB_HANDLE;
extern UART_HandleTypeDef huart5;

extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
extern struct stm32_gpio_init_param stm32_trigger_gpio_extra_init_params;
extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;

extern void stm32_system_init(void);

#endif // APP_CONFIG_STM32_H_
