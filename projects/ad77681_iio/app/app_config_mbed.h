/***************************************************************************//*
 * @file    app_config_mbed.h
 * @brief   Header file for Mbed platform configurations
******************************************************************************
 * Copyright (c) 2021-23, 2025 Analog Devices, Inc.
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
#include "mbed_i2c.h"
#include "mbed_gpio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

// Pin mapping of AD7768-1 with arduino
#define SPI_CSB			ARDUINO_UNO_D10
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_SCK			ARDUINO_UNO_D13

#define I2C_SCL			ARDUINO_UNO_D15
#define I2C_SDA			ARDUINO_UNO_D14

// Conversion over interrupt
#define CONV_MON		ARDUINO_UNO_D2

/* Common pin mapping */
#define UART_TX		CONSOLE_TX
#define	UART_RX		CONSOLE_RX

/* Unused macros */
#define SPI_DEVICE_ID 0

/******************************************************************************/
/********************* Public/Extern Declarations *****************************/
/******************************************************************************/
extern struct mbed_gpio_irq_init_param mbed_trigger_gpio_irq_init_params;
extern struct mbed_gpio_init_param mbed_trigger_gpio_extra_init_params;
extern struct mbed_uart_init_param mbed_uart_extra_init_params;
extern struct mbed_uart_init_param mbed_vcom_extra_init_params;
extern struct mbed_spi_init_param mbed_spi_extra_init_params;
extern struct mbed_i2c_init_param mbed_i2c_extra_init_params;

#endif /* _APP_CONFIG_MBED_H_ */
