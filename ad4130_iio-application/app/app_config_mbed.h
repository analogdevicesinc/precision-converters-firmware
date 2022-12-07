/***************************************************************************//**
 *   @file    app_config_mbed.h
 *   @brief   Header file for Mbed platform configurations
********************************************************************************
 * Copyright (c) 2021-22 Analog Devices, Inc.
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

#include "no_os_gpio.h"
#include "mbed_uart.h"
#include "mbed_irq.h"
#include "mbed_gpio_irq.h"
#include "mbed_spi.h"
#include "mbed_gpio.h"
#include "mbed_i2c.h"

#if defined(TARGET_SDP_K1)
#include "sdram_sdpk1.h"
#endif

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Select b/w Arduino or SDP-120 pin header (default is Arduino) */
//#define  SDP_120

#ifdef SDP_120
/* Pin mapping of SDP-K1 w.r.t SDP-120 connector */
#define SPI_CSB			SDP_SPI_CS_A
#define SPI_HOST_SDO	SDP_SPI_MOSI
#define SPI_HOST_SDI	SDP_SPI_MISO
#define SPI_SCK			SDP_SPI_SCK
#define I2C_SCL			SDP_I2C_SCL
#define I2C_SDA			SDP_I2C_SDA

#define MBED_CONV_MON	SDP_GPIO_1
#else
/* Pin mapping of SDP-K1 w.r.t Arduino connector */
#define SPI_CSB			ARDUINO_UNO_D10
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_SCK			ARDUINO_UNO_D13
#define I2C_SCL			ARDUINO_UNO_D15
#define I2C_SDA			ARDUINO_UNO_D14

#define MBED_CONV_MON	ARDUINO_UNO_D2
#endif

#if defined(AD4130_WLCSP_PACKAGE_TYPE)
#define	CONV_MON	MBED_CONV_MON
#else
#define	CONV_MON	ARDUINO_UNO_D2		// Conversion interrupt source pin (The selected interrupt source pin 
// e.g. MCLK or GPIO needs to be tied to D2 pin on Arduino header).
// The selection of interrupt source is done in 'iio_data_capture_init' function
#endif

/* Common pin mapping on SDP-K1 */
#define UART_TX			CONSOLE_TX
#define	UART_RX			CONSOLE_RX
#define LED_GPO			LED3

/* Select FS scaler value for the default user config mode.
 * This is not a max FS value that can be set into device but rather a value to
 * achieve max approximate ODR in the firmware for a given platform/setup.
 * Max ODR is derived by testing the firmware on SDP-K1 controller board
 * @10Mhz SPI clock. The max possible ODR can vary from board to board and
 * data continuity is not guaranteed above this ODR on IIO client */
#define FS_CONFIG_VALUE		1	// ODR = 2.4KSPS

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct mbed_gpio_irq_init_param mbed_trigger_gpio_irq_init_params;
extern struct mbed_gpio_init_param mbed_trigger_gpio_extra_init_params;
extern struct mbed_uart_init_param mbed_uart_extra_init_params;
extern struct mbed_spi_init_param mbed_spi_extra_init_params;
extern struct mbed_i2c_init_param mbed_i2c_extra_init_params;

#endif /* APP_CONFIG_MBED_H_ */
