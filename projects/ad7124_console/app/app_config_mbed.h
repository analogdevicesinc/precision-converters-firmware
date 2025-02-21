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
#include "mbed_gpio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

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

/* Common pin mappings */
#define LED_GREEN	LED3

/* Max SPI Clock SPEED */
#define MAX_SPI_CLK 5625000

/* Unused macros */
#define SPI_DEVICE_ID 0 // Unused macro
#define LED_PORT  0 // Unused macro
#define UART_IRQ_ID 0 // Unused macro

/* platform ops */
#define spi_ops mbed_spi_ops
#define gpio_ops mbed_gpio_ops

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct mbed_spi_init_param	mbed_spi_extra_init_params;

#endif // APP_CONFIG_MBED_H_
