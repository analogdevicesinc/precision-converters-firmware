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

// Pin mapping of with SDP-120 or Arduino connectors
#ifdef ARDUINO
#define I2C_SCL		 ARDUINO_UNO_D15	// I2C_SCL
#define I2C_SDA		 ARDUINO_UNO_D14	// I2C_SDA

#define SPI_CSB	      ARDUINO_UNO_D10	// SPI_CS
#define SPI_HOST_SDO  ARDUINO_UNO_D11	// SPI_MOSI
#define SPI_HOST_SDI  ARDUINO_UNO_D12	// SPI_MISO
#define SPI_SCK		  ARDUINO_UNO_D13	// SPI_SCK

#define GAIN_PIN	  ARDUINO_UNO_D8
#define RESET_PIN	  ARDUINO_UNO_D9
#define LDAC_PIN	  ARDUINO_UNO_D7
#define ADDR0_PIN	  ARDUINO_UNO_D6
#else
// SDP-120 connector
#define I2C_SCL		 SDP_I2C_SCL	// PH_7
#define I2C_SDA		 SDP_I2C_SDA	// PC_9

#define SPI_CSB		    SDP_SPI_CS_A // PB_9
#define SPI_HOST_SDI 	SDP_SPI_MISO // PF_8
#define SPI_HOST_SDO	SDP_SPI_MOSI // PF_9
#define SPI_SCK		    SDP_SPI_SCK	 // PH_6
#endif

// Unused macros
#define SPI_DEVICE_ID 0 // Unused macro
#define I2C_DEVICE_ID 0 // Unused macro
#define UART_IRQ_ID 0 // Unused macro

/* platform ops */
#define spi_ops mbed_spi_ops
#define i2c_ops mbed_i2c_ops

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct mbed_spi_init_param	mbed_spi_extra_init_params;
extern struct mbed_i2c_init_param mbed_i2c_extra_init_params;

#endif // APP_CONFIG_MBED_H_

