/***************************************************************************//**
 *   @file    app_config_mbed.h
 *   @brief   Header file for Mbed platform configurations.
********************************************************************************
 * Copyright (c) 2023-24 Analog Devices, Inc.
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
#include "mbed_uart.h"
#include "mbed_i2c.h"
#include "mbed_gpio.h"
#include "mbed_gpio_irq.h"
#include "mbed_irq.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/**
 * The ADI SDP_K1 can be used with either arduino headers
 * or the 120-pin SDP header found on ADI evaluation
 * boards. The default is the Arduino header.
 *
 * Comment the define below to enable SDP-120 header
*/
#define ARDUINO

#ifdef ARDUINO
#define SPI_CSB			ARDUINO_UNO_D10
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_SCK			ARDUINO_UNO_D13
#define I2C_SCL			ARDUINO_UNO_D15
#define I2C_SDA			ARDUINO_UNO_D14
#else // SDP-120 Connector
#define SPI_CSB	        SDP_SPI_CS_A
#define SPI_HOST_SDI	SDP_SPI_MISO
#define SPI_HOST_SDO	SDP_SPI_MOSI
#define SPI_SCK			SDP_SPI_SCK
#define I2C_SCL         SDP_I2C_SCL
#define I2C_SDA         SDP_I2C_SDA
#endif

/* UART Pins on SDP-K1 */
#define UART_TX			CONSOLE_TX
#define	UART_RX			CONSOLE_RX

/* RDY Pin */
#define RDY_PIN			ARDUINO_UNO_D8

#define TICKER_ID               TICKER_INT_ID
#define ticker_ops				mbed_irq_ops

#define RDY_PORT 0 // Unused
#define SPI_DEVICE_ID 0 // Unused
#define SPI_CS_PORT 0 // Unused

#define I2C_DEVICE_ID 0 // Unused

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct mbed_uart_init_param mbed_uart_extra_init_params;
extern struct mbed_uart_init_param mbed_vcom_extra_init_params;
extern struct mbed_spi_init_param mbed_spi_extra_init_params;
extern struct mbed_i2c_init_param mbed_i2c_extra_init_params;
extern struct mbed_gpio_irq_init_param mbed_trigger_gpio_irq_init_params;
extern struct mbed_irq_init_param mbed_ticker_int_extra_init_params;

#endif // APP_CONFIG_MBED_H_
