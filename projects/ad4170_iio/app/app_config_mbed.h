/***************************************************************************//**
 *   @file    app_config_mbed.h
 *   @brief   Header file for Mbed platform configurations
********************************************************************************
 * Copyright (c) 2021-23 Analog Devices, Inc.
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

#include "mbed_gpio.h"
#include "mbed_uart.h"
#include "mbed_irq.h"
#include "mbed_gpio_irq.h"
#include "mbed_spi.h"
#include "mbed_i2c.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

#define HW_CARRIER_NAME		STR(TARGET_NAME)

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

#define SYNC_INB		SDP_GPIO_1
#define DIG_AUX_1		SDP_GPIO_0
#define DIG_AUX_2		SDP_GPIO_2
#else
/* Pin mapping of SDP-K1 w.r.t Arduino connector */
#define SPI_CSB			ARDUINO_UNO_D10
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_SCK			ARDUINO_UNO_D13
#define I2C_SCL			ARDUINO_UNO_D15
#define I2C_SDA			ARDUINO_UNO_D14

#define SYNC_INB		ARDUINO_UNO_D4
#define DIG_AUX_1		ARDUINO_UNO_D2
#define DIG_AUX_2		ARDUINO_UNO_D7
#endif

#define I2C_DEVICE_ID   0

/* GPIO Port (Unused) */
#define DIG_AUX_1_PORT	0
#define DIG_AUX_2_PORT	0
#define SYNC_INB_PORT	0

/* Common pin mapping on SDP-K1 */
#define UART_TX			CONSOLE_TX
#define	UART_RX			CONSOLE_RX
#define LED_GPO			LED3

/* Time period for periodic ticker interrupt (in usec) */
#define TICKER_INTERRUPT_PERIOD_uSEC	(50000)

/* LVGL tick time period for Mbed platform */
#define LVGL_TICK_TIME_US	5000
#define LVGL_TICK_TIME_MS	(LVGL_TICK_TIME_US / 1000)

/* Note: The below macro and the type of digital filter chosen together
 * decides the output data rate to be configured for the device.
 * Filter configuration can be modified by changing the macro "AD4170_FILTER_CONFIG"
 * in the respective user configuration header file.
 * Please refer to the datasheet for more details on the other filter configurations.
 * It has to be noted that this is not the maximum ODR permissible by the device, but
 * a value specific to the SDP-K1 platform tested with a 10MHz SPI clock. The maximum
 * ODR might vary across platforms and data continuity is not guaranteed above this ODR
 * on the IIO Client */
#define FS_CONFIG_VALUE		16	// ODR = ~32KSPS (per channel) with Sinc5 average filter

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct mbed_irq_init_param mbed_ticker_int_extra_init_params;
extern struct mbed_gpio_irq_init_param mbed_trigger_gpio_irq_init_params;
extern struct mbed_gpio_init_param mbed_dig_aux1_gpio_extra_init_params;
extern struct mbed_gpio_init_param mbed_dig_aux2_gpio_extra_init_params;
extern struct mbed_gpio_init_param mbed_sync_inb_gpio_extra_init_params;
extern struct mbed_gpio_init_param mbed_trigger_gpio_extra_init_params;
extern struct mbed_uart_init_param mbed_uart_extra_init_params;
extern struct mbed_uart_init_param mbed_vcom_extra_init_params;
extern struct mbed_spi_init_param mbed_spi_extra_init_params;
extern struct mbed_i2c_init_param mbed_i2c_extra_init_params;

void lvgl_tick_callback(void *ctx);

#endif /* APP_CONFIG_MBED_H_ */
