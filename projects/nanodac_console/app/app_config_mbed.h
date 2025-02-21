/***************************************************************************//**
 *   @file    app_config_mbed.h
 *   @brief   Header file for Mbed platform configurations.
********************************************************************************
 * Copyright (c) 2025 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef APP_CONFIG_MBED_H_
#define APP_CONFIG_MBED_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <PinNames.h>
#include "mbed_spi.h"
#include "mbed_i2c.h"
#include "mbed_gpio.h"
#include "mbed_platform_support.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/**
  The ADI SDP_K1 can be used with either arduino headers
  or the 120-pin SDP connector found on ADI evaluation
  boards. The default is the SDP-120 connector.

  Uncomment the ARDUINO #define to enable the ARDUINO connector
*/
//#define  ARDUINO

// Pin mapping of nanoDAC+ with SDP-120 way or Arduino connectors
#ifdef ARDUINO
#define I2C_SCL		ARDUINO_UNO_D15
#define I2C_SDA		ARDUINO_UNO_D14

#define SPI_CSB			ARDUINO_UNO_D10
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_SCK			ARDUINO_UNO_D13

#define GAIN_PIN	ARDUINO_UNO_D8
#define RESET_PIN	ARDUINO_UNO_D9
#define LDAC_PIN	ARDUINO_UNO_D7
#define ADDR0_PIN	ARDUINO_UNO_D6
#else
#define I2C_SCL		SDP_I2C_SCL		// PH_7
#define I2C_SDA		SDP_I2C_SDA		// PC_9

#define SPI_CSB		    SDP_SPI_CS_A	// PB_9
#define SPI_HOST_SDO	SDP_SPI_MOSI	// PF_9
#define SPI_HOST_SDI	SDP_SPI_MISO	// PF_8
#define SPI_SCK		    SDP_SPI_SCK		// PH_6
#endif

// Define the other GPIO mapping based on the compatible EVAL board
// *Note: The 7-bit I2C slave address mentioned below is the default address for the
//        device, set by combination of slave address bits (7:3) from the device
//        datasheet and default logic level of A1 and A0 pins (bits 2:1) on the
//        respective device EVAL board. For more information, refer the device
//        datasheet and EVAL board manual.

#if defined(DEV_AD5686R) || defined(DEV_AD5686) || \
    defined(DEV_AD5684R) || defined(DEV_AD5684) || \
    defined(DEV_AD5685R)
// These devices support EVAL-AD5686RSDZ board
#if !defined ARDUINO
#define GAIN_PIN	SDP_GPIO_0
#define RESET_PIN	SDP_GPIO_2
#define LDAC_PIN	SDP_GPIO_3
#endif
#elif defined(DEV_AD5696R) || defined(DEV_AD5696) || \
      defined(DEV_AD5694R) || defined(DEV_AD5694) || \
      defined(DEV_AD5695R) || defined(DEV_AD5697R)
// These devices support EVAL-AD5696RSDZ board
#if !defined ARDUINO
#define GAIN_PIN	SDP_GPIO_0
#define RESET_PIN	SDP_GPIO_2
#define LDAC_PIN	SDP_GPIO_3
#endif
#define I2C_SLAVE_ADDRESS	0x18
#elif defined(DEV_AD5683) || defined(DEV_AD5683R) || defined(DEV_AD5682R) || \
      defined(DEV_AD5681R)
// These devices uses EVAL-AD5683R board
#if !defined ARDUINO
#define GAIN_PIN	SDP_GPIO_2
#define RESET_PIN	SDP_GPIO_1
#define LDAC_PIN	SDP_GPIO_0
#endif
#elif defined(DEV_AD5693) || defined(DEV_AD5693R) || defined(DEV_AD5692R) || \
      defined(DEV_AD5691R)
// These devices uses EVAL-AD5693R board
#if !defined ARDUINO
#define GAIN_PIN	SDP_GPIO_2
#define RESET_PIN	SDP_GPIO_1
#define LDAC_PIN	SDP_GPIO_0
#endif
#define I2C_SLAVE_ADDRESS	0x98
#elif defined (DEV_AD5674R) || defined (DEV_AD5674) || \
      defined (DEV_AD5679R) || defined (DEV_AD5679) || \
      defined (DEV_AD5677R) || defined (DEV_AD5673R)
// These devices uses EVAL-AD5679RSDZ/EVAL-AD567xRSDZ board
#if !defined ARDUINO
#define GAIN_PIN	SDP_GPIO_0
#define RESET_PIN	SDP_GPIO_2
#define LDAC_PIN	SDP_GPIO_1
#endif
#define I2C_SLAVE_ADDRESS	0x1E
#elif defined (DEV_AD5676R) || defined (DEV_AD5676) || \
      defined (DEV_AD5672R)
// These devices uses EVAL-AD5676RSDZ board
#if !defined ARDUINO
#define GAIN_PIN	SDP_GPIO_2
#define RESET_PIN	SDP_GPIO_1
#define LDAC_PIN	SDP_GPIO_0
#endif
#elif defined (DEV_AD5671R) || defined (DEV_AD5675R)
// These devices uses EVAL-AD5675RSDZ board
#if !defined ARDUINO
#define GAIN_PIN	SDP_GPIO_2
#define RESET_PIN	SDP_GPIO_1
#define LDAC_PIN	SDP_GPIO_0
#endif
#define I2C_SLAVE_ADDRESS	0x18
#else
#warning No/Unsupported EVAL board found. Using EVAL-AD5686R as default.
#if !defined ARDUINO
#define GAIN_PIN	SDP_GPIO_0
#define RESET_PIN	SDP_GPIO_2
#define LDAC_PIN	SDP_GPIO_3
#endif
#endif

// Common pin mappings
#define LED_GREEN	LED3	// PK_5

// Unused macros
#define SPI_DEVICE_ID 0 // Unused macro
#define I2C_DEVICE_ID 0 // Unused macro
#define UART_IRQ_ID 0 // Unused macro
#define RESET_PORT 0 // Unused macro
#define LDAC_PORT 0 // Unused macro
#define GAIN_PORT 0 // Unused macro

/* platform ops */
#define spi_ops mbed_spi_ops
#define i2c_ops mbed_i2c_ops
#define gpio_ops mbed_gpio_ops

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct mbed_spi_init_param  mbed_spi_extra_init_params;
extern struct mbed_gpio_init_param mbed_gpio_reset_init_params;
extern struct mbed_gpio_init_param mbed_gain_gpio_init_params;
extern struct mbed_gpio_init_param mbed_gpio_ldac_init_params;
extern struct mbed_i2c_init_param  mbed_i2c_extra_init_params;

#endif // APP_CONFIG_MBED_H_

