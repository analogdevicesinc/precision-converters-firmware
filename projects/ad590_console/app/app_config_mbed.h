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
  The ADI SDP_K1 can be used with both arduino headers
  or the 120-pin SDP connector found on ADI evaluation
  boards. The default is the Arduino connector

  Comment the SDP_120 #define below to enable the Arduino connector
*/

//#define SDP_120

#ifdef SDP_120

#define SPI_SS		SDP_SPI_CS_A
#define SPI_MISO	SDP_SPI_MISO
#define SPI_MOSI	SDP_SPI_MOSI
#define SPI_SCK		SDP_SPI_SCK

#else // ARDUINO

#define SPI_SS		ARDUINO_UNO_D10
#define SPI_MOSI	ARDUINO_UNO_D11
#define SPI_MISO	ARDUINO_UNO_D12
#define SPI_SCK		ARDUINO_UNO_D13

#endif

/* Max SPI Clk Speed */
#define MAX_SPI_CLK 2000000

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

