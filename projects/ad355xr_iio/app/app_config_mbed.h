/***************************************************************************//**
 *   @file    app_config_mbed.h
 *   @brief   Header file for Mbed platform configurations
********************************************************************************
 * Copyright (c) 2023-2024 Analog Devices, Inc.
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
#include "mbed_gpio.h"
#include "mbed_pwm.h"
#include "mbed_irq.h"
#include "mbed_gpio_irq.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Pin mapping of SDP-K1 w.r.t Arduino connector */
#define SPI_CSB		    ARDUINO_UNO_D10
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_SCK		    ARDUINO_UNO_D13

#define LDAC_PIN        ARDUINO_UNO_D9
#define RESET_PIN       ARDUINO_UNO_D8

/* Common pin mapping on SDP-K1 */
#define UART_TX			CONSOLE_TX
#define	UART_RX			CONSOLE_RX

/* Port names (Unused) */
#define LDAC_PORT				0
#define RESET_PORT              0

/* Redefine the init params structure mapping w.r.t. platform */
#define uart_extra_init_params      mbed_uart_init_params
#define vcom_extra_init_params      mbed_vcom_init_params
#define spi_extra_init_params       mbed_spi_init_params
#define spi_extra_init_params_without_sw_csb mbed_spi_init_params_without_sw_csb
#define ldac_pwm_extra_init_params       mbed_pwm_init_params
#define ext_int_extra_init_params   mbed_trigger_gpio_irq_init_params
#define gpio_ldac_extra_init_params	mbed_gpio_ldac_init_params
#define gpio_reset_extra_init_params mbed_gpio_reset_init_params
#define gpio_ops                    mbed_gpio_ops
#define spi_ops                     mbed_spi_ops
#define uart_ops                    mbed_uart_ops
#define vcom_ops                    mbed_virtual_com_ops
#define irq_platform_ops            mbed_gpio_irq_ops
#define pwm_ops                     mbed_pwm_ops
#define SPI_MODULE	                0	// Unused macro
#define trigger_gpio_handle	        0   // Unused macro
#define LDAC_PWM_ID                 0   // Unused macro
#define IRQ_CTRL_ID	                0   // Unused macro
#define SPI_DEVICE_ID	            0	// Unused macro
#define TRIGGER_INT_ID	            GPIO_IRQ_ID1
#define MAX_SPI_SCLK                22500000

/* Define the max possible sampling rate for a given platform.
 * Note: Max possible sampling rate is 45.45 KSPS.
 * This is derived by testing the firmware on SDP-K1 controller board with STM32F469NI MCU
 * using GCC and ARM compilers. The max possible sampling rate can vary from board to board */
#define MAX_SAMPLING_RATE	(45454)
#define LDAC_PWM_DUTY_CYCLE		(80)

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct mbed_uart_init_param mbed_uart_init_params;
extern struct mbed_uart_init_param mbed_vcom_init_params;
extern struct mbed_spi_init_param mbed_spi_init_params;
extern struct mbed_spi_init_param mbed_spi_init_params_without_sw_csb;
extern struct mbed_pwm_init_param mbed_pwm_init_params;
extern struct mbed_gpio_irq_init_param mbed_trigger_gpio_irq_init_params;
extern struct mbed_gpio_init_param mbed_gpio_ldac_init_params;
extern struct mbed_gpio_init_param mbed_gpio_reset_init_params;

#endif /* APP_CONFIG_MBED_H_ */
