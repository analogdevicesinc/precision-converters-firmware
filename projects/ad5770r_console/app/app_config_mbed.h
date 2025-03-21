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

// Pin mapping of AD5770R with SDP-K1/Arduino
#ifdef ARDUINO
#define SPI_CSB			ARDUINO_UNO_D10
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_SCK			ARDUINO_UNO_D13
#define HW_LDACB		ARDUINO_UNO_D2
#else
#define SPI_CSB		    SDP_SPI_CS_A	// PB_9
#define SPI_HOST_SDO	SDP_SPI_MOSI	// PF_9
#define SPI_HOST_SDI	SDP_SPI_MISO	// PF_8
#define SPI_SCK		    SDP_SPI_SCK		// PH_6
#define HW_LDACB        SDP_GPIO_0      // PJ_0
#endif

/* Max SPI Clk Speed */
#define MAX_SPI_CLK 2500000

// Unused macros
#define SPI_DEVICE_ID 0 // Unused macro
#define HW_LDACB_PORT 0 // Unused macro
#define UART_IRQ_ID 0 // Unused macro

/* platform ops */
#define spi_ops mbed_spi_ops
#define gpio_ops mbed_gpio_ops

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct mbed_spi_init_param	mbed_spi_extra_init_params;
extern struct mbed_gpio_init_param mbed_gpio_ldac_init_params;

#endif // APP_CONFIG_MBED_H_

