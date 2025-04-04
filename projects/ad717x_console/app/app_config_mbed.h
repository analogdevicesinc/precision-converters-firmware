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
#include "mbed_platform_support.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/**
 * The ADI SDP_K1 can be used with either arduino headers
 * or the 120-pin SDP connector found on ADI evaluation
 * boards. The default is the SDP-120 connector.
 *
 * Uncomment the #define ARDUINO below to enable the Arduino connector for
 * EVAL-AD4114SDZ, EVAL-AD4115SDZ and the EVAL-AD4116ASDZ
*/

/* NOTE: Only EVAL-AD4114SDZ, EVAL-AD4115SDZ and EVAL-AD4116ASDZ support Arduino and SDP_120
 * interface. The other EVAL Boards (EVAL-AD4111SDZ, EVAL-AD4112SDZ,
 * EVAL-AD7172-4SDZ, EVAL-AD7172-2SDZ , EVAL-AD7173-8SDZ, EVAL-AD7175-2SDZ,
 * EVAL-AD7175-8SDZ, EVAL-AD7176-2SDZ, EVAL-AD7177-2SDZ) support only the
 * SDP-120 interface.
 */

//#define  ARDUINO

#ifdef ARDUINO
/* SPI Pins on SDP-K1-Arduino Interface */
#define SPI_CSB			ARDUINO_UNO_D10		// SPI_CS
#define SPI_HOST_SDO	ARDUINO_UNO_D11		// SPI_MOSI
#define SPI_HOST_SDI	ARDUINO_UNO_D12		// SPI_MISO
#define SPI_SCK			ARDUINO_UNO_D13		// SPI_SCK
#define I2C_SCL			ARDUINO_UNO_D15
#define I2C_SDA			ARDUINO_UNO_D14
#else // Default- SDP_120 Interface
/* SPI Pins on SDP-K1-SDP-120 Interface */
#define SPI_CSB	        SDP_SPI_CS_A
#define SPI_HOST_SDI	SDP_SPI_MISO
#define SPI_HOST_SDO	SDP_SPI_MOSI
#define SPI_SCK			SDP_SPI_SCK

#define I2C_SCL         SDP_I2C_SCL
#define I2C_SDA         SDP_I2C_SDA
#endif // ARDUINO

/* Max SPI CLK SPEED */
#define MAX_SPI_CLK 2500000

/* Common pin mappings */
#define LED_GREEN	LED3

// Unused macros
#define SPI_DEVICE_ID 0 // Unused macro
#define UART_IRQ_ID 0 // Unused macro

/* platform ops */
#define spi_ops mbed_spi_ops

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct mbed_spi_init_param	mbed_spi_extra_init_params;

#endif // APP_CONFIG_MBED_H_

