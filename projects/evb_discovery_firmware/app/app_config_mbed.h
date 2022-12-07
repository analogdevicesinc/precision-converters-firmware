/***************************************************************************//**
 *   @file    app_config_mbed.h
 *   @brief   Header file for Mbed platform configurations
********************************************************************************
 * Copyright (c) 2022-23 Analog Devices, Inc.
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

#include "mbed_uart.h"
#include "mbed_i2c.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Pin mapping of SDP-K1 w.r.t Arduino connector */
#define I2C_SCL			ARDUINO_UNO_D15
#define I2C_SDA			ARDUINO_UNO_D14

/* Common pin mapping on SDP-K1 */
#define UART_TX			CONSOLE_TX
#define	UART_RX			CONSOLE_RX

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct mbed_uart_init_param mbed_uart_extra_init_params;
extern struct mbed_uart_init_param mbed_vcom_extra_init_params;
extern struct mbed_i2c_init_param mbed_i2c_extra_init_params;

#endif /* APP_CONFIG_MBED_H_ */
