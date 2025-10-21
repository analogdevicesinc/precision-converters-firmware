/***************************************************************************//**
 *   @file    app_config_stm32.h
 *   @brief   Header file for STM32 platform configurations
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
#include <stdint.h>

#include "stm32_i2c.h"
#include "stm32_irq.h"
#include "stm32_gpio_irq.h"
#include "stm32_spi.h"
#include "stm32_gpio.h"
#include "stm32_uart.h"
#ifdef STM32F469xx
#include "stm32_usb_uart.h"
#include "stm32_uart_stdio.h"
#endif

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* Select FS scaler value for the default user config mode.
 * This is not a max FS value that can be set into device but rather a value to
 * achieve max approximate ODR in the firmware for a given platform/setup.
 * Max ODR is derived by testing the firmware on SDP-K1 and DISCO-F769 controller board
 * @10Mhz SPI clock. The max possible ODR can vary from board to board and
 * data continuity is not guaranteed above this ODR on IIO client */
#define FS_CONFIG_VALUE		1	// ODR = 2.4KSPS

#ifdef STM32F469xx
/* Note: The SDP-K1 board with the STM32F469NI MCU has been used
* for developing the firmware. The below parameters will change depending
* on the controller used. */
#define TARGET_NAME       SDP_K1

/* Pin mapping for AD4052 w.r.t Arduino Headers */
/* STM32 I2C specific parameters */
#define I2C_DEVICE_ID      1    // I2C1

/* STM32 UART Specific parameters */
#define UART_MODULE        5    // UART5
#define UART_IRQ           UART5_IRQn
#define APP_UART_HANDLE    huart5
#define APP_UART_USB_HANDLE	 hUsbDeviceHS

/* STM32 SPI Specific parameters */
#define SPI_DEVICE_ID       1   // SPI1
#define SPI_CSB             15  // PA_15
#define SPI_CS_PORT_NUM     0   // PORTA

/* STM32 GPIO Specific parameter */
#define CNV_PIN_NUM         7   // PG_7
#define CNV_PORT_NUM        6   // PORTG
#define LED_GPIO_PORT       10 // PK_7
#define LED_GPIO            7  // PORTK

/* Priority of the RDY Interrupt */
#define RDY_GPIO_PRIORITY 1

#else
/* The below configurations are specific to STM32769NI MCU on Disco-F769NI Board. */
#define HW_CARRIER_NAME		DISCO-F769NI

/* STM32 SPI Specific parameters */
#define SPI_DEVICE_ID		2 // SPI1
#define SPI_CS_PORT_NUM	0  // GPIO Port D
#define SPI_CSB				11 // PA_11

/* STM32 UART specific parameters */
#define APP_UART_HANDLE 	huart6

/* UART Device ID */
#define UART_IRQ			USART6_IRQn
#define UART_MODULE        6    // UART5

/* RDY specific Port and Pin */
#define CNV_PORT_NUM    9
#define CNV_PIN_NUM         1 // PJ_1

#define I2C_DEVICE_ID           1 // I2C1

/* I2C timing register value for standard mode of operation
 * Check here for more understanding on I2C timing register
 * configuration: TODO*/
#define I2C_TIMING				0x40912732

/* Ticker for Pocket Lab */
#define LVGL_TICK_TIME_US	5000
#define LVGL_TICK_TIME_MS	(LVGL_TICK_TIME_US / 1000)

#endif

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern UART_HandleTypeDef APP_UART_HANDLE;
#ifdef STM32F469xx
extern USBD_HandleTypeDef	APP_UART_USB_HANDLE;
#endif

extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
extern struct stm32_gpio_init_param stm32_trigger_gpio_extra_init_params;
extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;

void stm32_system_init(void);

#endif /* APP_CONFIG_STM32_H_ */
