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

#include "mbed_uart.h"
#include "mbed_spi.h"
#include "mbed_pwm.h"
#include "mbed_gpio_irq.h"
#include "mbed_gpio.h"
#include "mbed_pwm.h"
#include "mbed_i2c.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Pin mapping for AD469x w.r.t Arduino Headers */
#define SPI_CSB		    ARDUINO_UNO_D10
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_SCK		    ARDUINO_UNO_D13

/* Common pin mapping on SDP-K1 */
#define UART_TX		    CONSOLE_TX
#define	UART_RX		    CONSOLE_RX

#define I2C_SCL			ARDUINO_UNO_D15
#define I2C_SDA			ARDUINO_UNO_D14

#define DIGI_GOOD_PIN   ARDUINO_UNO_D6

/* Define the max possible sampling (or output data) rate for a given platform.
 * This is also used to find the time period to trigger a periodic conversion event.
 * Note: Max possible ODR is 62.5KSPS per channel for continuous data capture on
 * IIO client. This is derived by testing the firmware on SDP-K1 controller board
 * @22Mhz SPI clock. The max possible ODR can vary from board to board and
 * data continuity is not guaranteed above this ODR on IIO oscilloscope */
#define SAMPLING_RATE			        (62500)
#define CONV_TRIGGER_PERIOD_NSEC(x)		(((float)(1.0 / x) * 1000000) * 1000)
#define CONV_TRIGGER_DUTY_CYCLE_NSEC(x)	(CONV_TRIGGER_PERIOD_NSEC(x) / 10)

/* Redefine the init params structure mapping w.r.t. Mbed platform */
#define gpio_ops                    mbed_gpio_ops
#define spi_ops		                mbed_spi_ops
#define i2c_ops                     mbed_i2c_ops
#define uart_ops                    mbed_uart_ops
#define vcom_ops                    mbed_virtual_com_ops
#define pwm_ops                     mbed_pwm_ops
#define gpio_ops                    mbed_gpio_ops
#define spi_ops                     mbed_spi_ops
#define trigger_gpio_irq_ops        mbed_gpio_irq_ops
#define trigger_gpio_handle         0    // Unused macro
#define TRIGGER_GPIO_PORT           0    // Unused macro
#define TRIGGER_GPIO_PIN            ARDUINO_UNO_D2
#define TIMER1_ID                   0    // Unused macro
#define TIMER2_ID                   0    // Unused macro
#define TRIGGER_INT_ID	            0    // Unused macro
#define I2C_DEV_ID                  0    // Unused macro
#define UART_MODULE                 0    // Unused macro
#define UART_IRQ                    0    // Unused macro
#define SPI_DEVICE_ID               0    // Unused macro
#define SPI_CS_PIN_NUM              ARDUINO_UNO_D10
#define CNV_PIN_NUM                 ARDUINO_UNO_D6
#define CNV_PORT_NUM                0    // Unused macro
#define GP0_PIN_NUM                 0    // Unused macro
#define GP0_PORT_NUM                0    // Unused macro
#define GP1_PIN_NUM                 0    // Unused macro
#define GP1_PORT_NUM                0    // Unused macro
#define BSY_PIN_NUM                 ARDUINO_UNO_D9
#define BSY_PORT_NUM                0    // Unused macro
#define RESET_PIN_NUM               ARDUINO_UNO_D4
#define RESET_PORT_NUM              0    // Unused macro
#define MAX_SPI_SCLK                22500000

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct mbed_gpio_init_param mbed_gpio_bsy_extra_init_params;
extern struct mbed_gpio_irq_init_param mbed_trigger_gpio_irq_init_params;
extern struct mbed_uart_init_param mbed_uart_extra_init_params;
extern struct mbed_uart_init_param mbed_vcom_extra_init_params;
extern struct mbed_spi_init_param mbed_spi_extra_init_params;
extern struct mbed_pwm_init_param mbed_pwm_extra_init_params;
extern struct mbed_i2c_init_param mbed_i2c_extra_init_params;
extern struct mbed_gpio_init_param mbed_gpio_reset_extra_init_params;
extern struct mbed_gpio_init_param mbed_gpio_cnv_extra_init_params;

#endif /* APP_CONFIG_MBED_H_ */