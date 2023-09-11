/***************************************************************************//**
 *   @file    app_config_mbed.h
 *   @brief   Header file for Mbed platform configurations
********************************************************************************
 * Copyright (c) 2023 Analog Devices, Inc.
 * Copyright (c) 2023 BayLibre, SAS.
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
#include "mbed_gpio_irq.h"
#include "mbed_spi.h"
#include "mbed_pwm.h"
#include "mbed_gpio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
#define SPI_CSB		ARDUINO_UNO_D10
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_SCK		ARDUINO_UNO_D13
#define PWM_TRIGGER     ARDUINO_UNO_D3

#define GPIO_A0		ARDUINO_UNO_D0
#define GPIO_A1		ARDUINO_UNO_D1
#define GPIO_RES0	ARDUINO_UNO_D5
#define GPIO_RES1	ARDUINO_UNO_D6
#define GPIO_SAMPLE	ARDUINO_UNO_D4

/* UART Common Pin Mapping on SDP-K1 */
#define UART_TX		CONSOLE_TX
#define	UART_RX		CONSOLE_RX

/* Define a sampling rate for a given setup.
 * This is used to find the time period to trigger a periodic conversion event.
 * Currently the value was experimentally found by testing the firmware on SDP-K1
 * controller board @20Mhz SPI clock, with fly wires to breakout board.
 * This can vary from board to board, the exact maximum value was not determined
 * as 16k seems reasonable for this setup.
 * */
#define SAMPLING_RATE                                   (16000)
#define CONV_TRIGGER_PERIOD_NSEC                (((float)(1.0 / SAMPLING_RATE) * 1000000) * 1000)
#define CONV_TRIGGER_DUTY_CYCLE_NSEC    (CONV_TRIGGER_PERIOD_NSEC / 2)

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct mbed_uart_init_param mbed_uart_extra_init_params;
extern struct mbed_uart_init_param mbed_vcom_extra_init_params;
extern struct mbed_spi_init_param mbed_spi_extra_init_params;
extern struct mbed_gpio_init_param mbed_trigger_gpio_extra_init_params;
extern struct mbed_gpio_irq_init_param mbed_trigger_gpio_irq_init_params;
extern struct mbed_pwm_init_param mbed_pwm_extra_init_params;

#endif /* APP_CONFIG_MBED_H_ */
