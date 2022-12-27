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

#include "mbed_uart.h"
#include "mbed_spi.h"
#include "mbed_pwm.h"
#include "mbed_gpio_irq.h"
#include "mbed_gpio.h"
#include "mbed_pwm.h"

#if defined(TARGET_SDP_K1)
#include "sdram_sdpk1.h"
#endif

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Pin mapping for AD4696 w.r.t Arduino Headers */
#define SPI_CSB		    ARDUINO_UNO_D10
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_SCK		    ARDUINO_UNO_D13

/* Common pin mapping on SDP-K1 */
#define UART_TX		    CONSOLE_TX
#define	UART_RX		    CONSOLE_RX

/* Pins used to trigger, reset and/or read a new (periodic) conversion event */
#define RESET_PIN	    ARDUINO_UNO_D5
#define BUSY_PIN	    ARDUINO_UNO_D7
#define CONVST_PIN	    ARDUINO_UNO_D6
#define PWM_PIN         ARDUINO_UNO_D6

/* Define the max possible sampling (or output data) rate for a given platform.
 * This is also used to find the time period to trigger a periodic conversion event.
 * Note: Max possible ODR is 62.5KSPS per channel for continuous data capture on
 * IIO client. This is derived by testing the firmware on SDP-K1 controller board
 * @22Mhz SPI clock. The max possible ODR can vary from board to board and
 * data continuity is not guaranteed above this ODR on IIO oscilloscope */
#define DEFAULT_SAMPLING_RATE			(62500)
#define CONV_TRIGGER_PERIOD_NSEC(x)		(((float)(1.0 / x) * 1000000) * 1000)
#define CONV_TRIGGER_DUTY_CYCLE_NSEC(x)	(CONV_TRIGGER_PERIOD_NSEC(x) / 10)

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct mbed_gpio_init_param mbed_gpio_bsy_extra_init_params;
extern struct mbed_gpio_irq_init_param mbed_trigger_gpio_irq_init_params;
extern struct mbed_uart_init_param mbed_uart_extra_init_params;
extern struct mbed_spi_init_param mbed_spi_extra_init_params;
extern struct mbed_pwm_init_param mbed_pwm_extra_init_params;

#endif /* APP_CONFIG_MBED_H_ */