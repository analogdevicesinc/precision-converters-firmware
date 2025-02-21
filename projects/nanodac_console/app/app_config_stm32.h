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
#define SPI_DEVICE_ID		1 // SPI1
#define SPI_CS_PORT	        0  // GPIO Port A
#define SPI_CSB				15 // PA_15

/* I2C Device ID */
#define I2C_DEVICE_ID       1 // I2C1

/* GAIN Specific Port G and Pin 11 */
#define GAIN_PIN  11 // PG_11
#define GAIN_PORT  6 // GPIO PORT G

/* RESET Specific Port B and Pin 15 */
#define RESET_PIN  15 // PB_15
#define RESET_PORT  1 // GPIO Port B

/* LDAC Specific Port G and Pin 10 */
#define LDAC_PIN 10  // PG_10
#define LDAC_PORT 6 // GPIO PORT G

/* ADDR0 Specific Port A and Pin 10 */
#define ADDR0_PIN 10 // PA_10
#define ADDR0_PORT 0 // GPIO PORT A
#else /* Default- SDP_120 Interface */
/* SPI Pins on SDP-K1-SDP-120 Interface */
#define SPI_DEVICE_ID		5 // SPI5
#define SPI_CS_PORT	        1  // GPIO Port B
#define SPI_CSB				9 // PB_9
#define I2C_DEVICE_ID       3 // I2C3
#endif

/* Define the other GPIO mapping based on the compatible EVAL board
 	 *Note: The 7-bit I2C slave address mentioned below is the default address for the
        device, set by combination of slave address bits (7:3) from the device
        datasheet and default logic level of A1 and A0 pins (bits 2:1) on the
        respective device EVAL board. For more information, refer the device
        datasheet and EVAL board manual. */

#if defined(DEV_AD5686R) || defined(DEV_AD5686) || \
    defined(DEV_AD5684R) || defined(DEV_AD5684) || \
    defined(DEV_AD5685R)
/* These devices support EVAL-AD5686RSDZ board */
#if !defined ARDUINO
#define GAIN_PIN	0 // PJ_0
#define GAIN_PORT   9 // PORTJ
#define RESET_PIN	3 // PJ_3
#define RESET_PORT  9 // PORTJ
#define LDAC_PIN	4 // PJ_4
#define LDAC_PORT   9 // PORTJ
#endif
#elif defined(DEV_AD5696R) || defined(DEV_AD5696) || \
      defined(DEV_AD5694R) || defined(DEV_AD5694) || \
      defined(DEV_AD5695R) || defined(DEV_AD5697R)
/* These devices support EVAL-AD5696RSDZ board */
#if !defined ARDUINO
#define GAIN_PIN	0 // PJ_0
#define GAIN_PORT   9 // PORTJ
#define RESET_PIN	3 // PJ_3
#define RESET_PORT  9 // PORTJ
#define LDAC_PIN	4 // PJ_4
#define LDAC_PORT   9 // PORTJ
#endif
#define I2C_SLAVE_ADDRESS	0x18
#elif defined(DEV_AD5683) || defined(DEV_AD5683R) || defined(DEV_AD5682R) || \
      defined(DEV_AD5681R)
/* These devices uses EVAL-AD5683R board */
#if !defined ARDUINO
#define GAIN_PIN	3 // PJ_3
#define GAIN_PORT   9 // PORTJ
#define RESET_PIN	1 // PJ_1
#define RESET_PORT  9 // PORTJ
#define LDAC_PIN	0 // PJ_0
#define LDAC_PORT   9 // PORTJ
#endif
#elif defined(DEV_AD5693) || defined(DEV_AD5693R) || defined(DEV_AD5692R) || \
      defined(DEV_AD5691R)
/* These devices uses EVAL-AD5693R board */
#if !defined ARDUINO
#define GAIN_PIN	3 // PJ_3
#define GAIN_PORT   9 // PORTJ
#define RESET_PIN	1 // PJ_1
#define RESET_PORT  9 // PORTJ
#define LDAC_PIN	0 // PJ_0
#define LDAC_PORT   9 // PORTJ
#endif
#define I2C_SLAVE_ADDRESS	0x98
#elif defined (DEV_AD5674R) || defined (DEV_AD5674) || \
      defined (DEV_AD5679R) || defined (DEV_AD5679) || \
      defined (DEV_AD5677R) || defined (DEV_AD5673R)
/* These devices uses EVAL-AD5679RSDZ/EVAL-AD567xRSDZ board */
#if !defined ARDUINO
#define GAIN_PIN	0 // PJ_0
#define GAIN_PORT   9 // PORTJ
#define RESET_PIN	2 // PJ_2
#define RESET_PORT  9 // PORTJ
#define LDAC_PIN	1 // PJ_1
#define LDAC_PORT   9 // PORTJ
#endif
#define I2C_SLAVE_ADDRESS	0x1E
#elif defined (DEV_AD5676R) || defined (DEV_AD5676) || \
      defined (DEV_AD5672R)
/* These devices uses EVAL-AD5676RSDZ board */
#if !defined ARDUINO
#define GAIN_PIN	3 // PJ_3
#define GAIN_PORT   9 // PORTJ
#define RESET_PIN	1 // PJ_1
#define RESET_PORT  9 // PORTJ
#define LDAC_PIN	0 // PJ_0
#define LDAC_PORT   9 // PORTJ
#endif
#elif defined (DEV_AD5671R) || defined (DEV_AD5675R)
/* These devices uses EVAL-AD5675RSDZ board */
#if !defined ARDUINO
#define GAIN_PIN	3 // PJ_3
#define GAIN_PORT   9 // PORTJ
#define RESET_PIN	1 // PJ_1
#define RESET_PORT  9 // PORTJ
#define LDAC_PIN	0 // PJ_0
#define LDAC_PORT   9 // PORTJ
#endif
#define I2C_SLAVE_ADDRESS	0x18
#else
#warning No/Unsupported EVAL board found. Using EVAL-AD5686R as default.
#if !defined ARDUINO
#define GAIN_PIN	0 //PJ_1
#define GAIN_PORT   9 // PortJ
#define RESET_PIN	3 // PJ_3
#define RESET_PORT  9 // PortJ
#define LDAC_PIN	4 // PJ_4
#define LDAC_PORT   9 // PortJ
#endif
#endif

/* Common pin mappings */
#define LED_GREEN	LED3	// PK_5
#define LED_GREEN_PORT 10   // PortK

#define I2C_TIMING          0 // (Unused)

/* STM32 UART specific parameters */
#define APP_UART_HANDLE     &huart5
#define UART_IRQ_ID			UART5_IRQn

/* platform ops */
#define uart_ops stm32_uart_ops
#define i2c_ops stm32_i2c_ops
#define gpio_ops stm32_gpio_ops
#define spi_ops stm32_spi_ops

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct no_os_uart_desc *uart_desc;
extern UART_HandleTypeDef huart5;

extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_reset_init_params;
extern struct stm32_gpio_init_param stm32_gain_gpio_init_params;
extern struct stm32_gpio_init_param stm32_gpio_ldac_init_params;

extern void stm32_system_init(void);

#endif // APP_CONFIG_STM32_H_
