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
#include "stm32_gpio_irq.h"
#include "main.h"
#include "stm32_usb_uart.h"
#include "app_config.h"
#include "stm32_irq.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Note: The SDP-K1 board with the STM32F469NI MCU has been used
* for developing the firmware. The below parameters will change depending
* on the controller used. */
#define TARGET_NAME		SDP-K1

//#define ARDUINO
/* SPI Pins on SDP-K1-Arduino Interface */
#ifdef ARDUINO
#define SPI_DEVICE_ID		1 // SPI1
#define SPI_CS_PORT	        0  // GPIO Port A
#define SPI_CSB				15 // PA_15
#define I2C_DEVICE_ID       1 // I2C1
#else // Default- SDP_120 Interface
/* SPI Pins on SDP-K1-SDP-120 Interface */
#define SPI_DEVICE_ID		5 // SPI5
#define SPI_CS_PORT	        1  // GPIO Port B
#define SPI_CSB				9 // PA_15
#define I2C_DEVICE_ID       3 // I2C3
#endif

/* STM32 UART specific parameters */
#define UART_IRQ_ID			UART5_IRQn
#define UART_ID             5
#define APP_UART_HANDLE     &huart5
#define APP_UART_USB_HANDLE		hUsbDeviceHS
#define USB_IRQ_ID          OTG_HS_IRQn

/* RDY specific Port and Pin */
#define RDY_PORT	6 // Port G
#define RDY_PIN    11 // PG_11

/* Priority for Ready Interrupt */
#define RDY_GPIO_PRIORITY 1

/* Max SPI Clock Speed */
#define MAX_SPI_SCLK            11250000

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

extern UART_HandleTypeDef huart5;
extern USBD_HandleTypeDef	APP_UART_USB_HANDLE;

extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;
extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
extern struct stm32_uart_desc *uart_extra_params;

extern void stm32_system_init(void);
#endif // APP_CONFIG_STM32_H_
