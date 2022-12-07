/***************************************************************************//*
 * @file    app_config_mbed.h
 * @brief   Header file for Mbed platform configurations
******************************************************************************
 * Copyright (c) 2021-23 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/

#ifndef _APP_CONFIG_MBED_H_
#define _APP_CONFIG_MBED_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <PinNames.h>

#include "no_os_gpio.h"
#include "mbed_uart.h"
#include "mbed_irq.h"
#include "mbed_gpio_irq.h"
#include "mbed_spi.h"
#include "mbed_gpio.h"

#if defined(TARGET_SDP_K1)
#include "sdram_sdpk1.h"
#endif

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

// Pin mapping of AD7768-1 with arduino
#define SPI_CSB			ARDUINO_UNO_D10
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_SCK			ARDUINO_UNO_D13

// Conversion over interrupt
#define CONV_MON		ARDUINO_UNO_D2

/* Common pin mapping */
#define UART_TX		CONSOLE_TX
#define	UART_RX		CONSOLE_RX

/* Define the max possible sampling frequency (or output data) rate for AD77681 (in SPS).
 * This is also used to find the time period to trigger a periodic conversion event.
 * Note: Max possible ODR is 64KSPS for continuous data capture on IIO Client.
 * This is derived by capturing data from the firmware using the SDP-K1 controller board
 * @22.5Mhz SPI clock. The max possible ODR can vary from board to board and
 * data continuity is not guaranteed above this ODR on IIO oscilloscope */

/* AD77681 default internal clock frequency (MCLK = 16.384 Mhz) */
#define AD77681_MCLK (16384)

/* AD77681 decimation rate */
#define AD77681_DECIMATION_RATE	(32U)

/* AD77681 default mclk_div value */
#define AD77681_DEFAULT_MCLK_DIV (8)

/* AD77681 ODR conversion */
#define AD77681_ODR_CONV_SCALER	(AD77681_DECIMATION_RATE * AD77681_DEFAULT_MCLK_DIV)

/* AD77681 default sampling frequency */
#define AD77681_DEFAULT_SAMPLING_FREQ	((AD77681_MCLK * 1000) / AD77681_ODR_CONV_SCALER)

/******************************************************************************/
/********************* Public/Extern Declarations *****************************/
/******************************************************************************/
extern struct mbed_gpio_irq_init_param mbed_trigger_gpio_irq_init_params;
extern struct mbed_gpio_init_param mbed_trigger_gpio_extra_init_params;
extern struct mbed_uart_init_param mbed_uart_extra_init_params;
extern struct mbed_uart_init_param mbed_vcom_extra_init_params;
extern struct mbed_spi_init_param mbed_spi_extra_init_params;

#endif /* _APP_CONFIG_MBED_H_ */
