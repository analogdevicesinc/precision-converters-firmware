/***************************************************************************//**
 *   @file    app_config_mbed.h
 *   @brief   Header file for Mbed platform configurations
********************************************************************************
 * Copyright (c) 2021-24 Analog Devices, Inc.
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
#include <PinNames.h>
#include "mbed_spi.h"
#include "mbed_uart.h"
#include "mbed_gpio.h"
#include "mbed_gpio_irq.h"
#include "mbed_irq.h"
#include "mbed_i2c.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* Pin mapping for AD719x w.r.t Arduino Headers */
#define MAX_SPI_BAUDRATE 10000000
#define SPI_CSB		     ARDUINO_UNO_D10
#define SPI_HOST_SDO	 ARDUINO_UNO_D11
#define SPI_HOST_SDI	 ARDUINO_UNO_D12
#define SPI_SCK		     ARDUINO_UNO_D13

/* Common pin mapping on SDP-K1 */
#define UART_TX		     CONSOLE_TX
#define	UART_RX		     CONSOLE_RX

/* I2C Pin connected to EEPROM Chip */
#define I2C_SCL			 ARDUINO_UNO_D15
#define I2C_SDA			 ARDUINO_UNO_D14

/* This pin is used to detect the End of Conversion
 * signal and should be connected to the MOSI (D12)
 * pin physically on the EVAL board.
 * */
#define RDY_PIN	         ARDUINO_UNO_D8

/* To explicitly have control over the CS line,
 * the firmware makes use of D9 pin on the Arduino.
 * This done by switching the LK12 header on the
 * EVAL board to position 2 */
#define CS_ARD_TWO       ARDUINO_UNO_D9

/* The SYNC pin allows the user to reset the modulator
 * without affecting any of the setup conditions on the part.
 * This allows the firmware to start gathering samples of the
 * analog input from a known point in time.
 * */
#define SYNC_PIN         ARDUINO_UNO_D4

#define I2C_DEVICE_ID    0 // Unused
#define SPI_DEVICE_ID    0 // Unused
#define SPI_CS_PORT      0 // Unused
#define SYNC_PORT        0 // Unused
#define RDY_PORT         0 // Unused

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/
extern struct mbed_uart_init_param mbed_uart_extra_init_params;
extern struct mbed_uart_init_param mbed_vcom_extra_init_params;
extern struct mbed_spi_init_param mbed_spi_extra_init_params;
extern struct mbed_i2c_init_param mbed_i2c_extra_init_params;
extern struct mbed_gpio_init_param mbed_gpio_sync_extra_init_params;
extern struct mbed_gpio_irq_init_param mbed_trigger_gpio_irq_init_params;

#endif /* APP_CONFIG_MBED_H_ */
