/***************************************************************************//**
 *   @file    app_config_mbed.h
 *   @brief   Header file for Mbed platform configurations
********************************************************************************
 * Copyright (c) 2021-23, 2025 Analog Devices, Inc.
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
#include "mbed_irq.h"
#include "mbed_gpio_irq.h"
#include "mbed_spi.h"
#include "mbed_i2c.h"
#include "mbed_pwm.h"
#include "mbed_gpio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Pin mapping of SDP-K1 w.r.t Arduino connector */
#define SPI_CSB			ARDUINO_UNO_D10
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_SCK			ARDUINO_UNO_D13

#define I2C_SCL			ARDUINO_UNO_D15
#define I2C_SDA			ARDUINO_UNO_D14

/* Common pin mapping on SDP-K1 */
#define UART_TX			CONSOLE_TX
#define	UART_RX			CONSOLE_RX
#define LED_GPO			LED3

/* Pin mapping w.r.t. target */
#define	OSR0_PIN		ARDUINO_UNO_D1
#define	OSR1_PIN		ARDUINO_UNO_D2
#define	OSR2_PIN		ARDUINO_UNO_D4
#define RESET_PIN		ARDUINO_UNO_D5
#define CONVST_PIN		ARDUINO_UNO_D6
#define BUSY_PIN		ARDUINO_UNO_D7
#define RANGE_PIN		ARDUINO_UNO_D8
#define STDBY_PIN		ARDUINO_UNO_D9

/* Pins used to trigger and/or read a new (periodic) conversion event */
#define PWM_TRIGGER		ARDUINO_UNO_D3
#define INT_EVENT		ARDUINO_UNO_D3

/* Unused macros */
#define SPI_DEVICE_ID 0
#define PWM_ID 0
#define UART_ID 0
#define LED_PORT 0
#define OSR0_PORT 0
#define OSR1_PORT 0
#define OSR2_PORT 0
#define RESET_PORT 0
#define CONVST_PORT 0
#define BUSY_PORT 0
#define RANGE_PORT 0
#define STDBY_PORT 0

/* Define the max possible sampling (or output data) rate for a given platform.
 * This is also used to find the time period to trigger a periodic conversion event.
 * Note: Max possible ODR is 16KSPS per channel for continuous data capture on
 * IIO client. This is derived by testing the firmware on SDP-K1 controller board
 * @22Mhz SPI clock. The max possible ODR can vary from board to board and
 * data continuity is not guaranteed above this ODR on IIO oscilloscope */
#define SAMPLING_RATE					(16000)
#define CONV_TRIGGER_PERIOD_NSEC		(((float)(1.0 / SAMPLING_RATE) * 1000000) * 1000)
#define CONV_TRIGGER_DUTY_CYCLE_NSEC	(CONV_TRIGGER_PERIOD_NSEC / 2)

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct mbed_gpio_irq_init_param mbed_trigger_gpio_irq_init_params;
extern struct mbed_gpio_init_param mbed_trigger_gpio_extra_init_params;
extern struct mbed_gpio_init_param mbed_reset_gpio_extra_init_params;
extern struct mbed_gpio_init_param mbed_convst_gpio_extra_init_params;
extern struct mbed_gpio_init_param mbed_busy_gpio_extra_init_params;
extern struct mbed_gpio_init_param mbed_osr0_gpio_extra_init_params;
extern struct mbed_gpio_init_param mbed_osr1_gpio_extra_init_params;
extern struct mbed_gpio_init_param mbed_osr2_gpio_extra_init_params;
extern struct mbed_gpio_init_param mbed_range_gpio_extra_init_params;
extern struct mbed_gpio_init_param mbed_stdby_gpio_extra_init_params;
extern struct mbed_pwm_init_param mbed_pwm_extra_init_params;
extern struct mbed_uart_init_param mbed_uart_extra_init_params;
extern struct mbed_uart_init_param mbed_vcom_extra_init_params;
extern struct mbed_spi_init_param mbed_spi_extra_init_params;
extern struct mbed_i2c_init_param mbed_i2c_extra_init_params;

#endif /* APP_CONFIG_MBED_H_ */

