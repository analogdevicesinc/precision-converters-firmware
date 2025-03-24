/***************************************************************************//**
 *   @file    app_config_mbed.h
 *   @brief   Header file for Mbed platform configurations
********************************************************************************
 * Copyright (c) 2024, 2025 Analog Devices, Inc.
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
#include "mbed_spi.h"
#include "mbed_i2c.h"
#include "mbed_pwm.h"
#include "mbed_gpio.h"
#include "mbed_gpio_irq.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Pin mapping of SDP-K1 w.r.t Arduino connector */
#define SPI_CSB		    ARDUINO_UNO_D10
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_SCK		    ARDUINO_UNO_D13

#define I2C_SCL			ARDUINO_UNO_D15
#define I2C_SDA			ARDUINO_UNO_D14

/* Console pin mapping on SDP-K1 */
#define UART_TX			CONSOLE_TX
#define	UART_RX			CONSOLE_RX

/* LDAC and CLR pin mapping on SDP-K1*/
#define LDAC_GPIO		ARDUINO_UNO_D6
#define CLEAR_GPIO		ARDUINO_UNO_D4

#define PWM_TRIGGER		LDAC_GPIO

/* Redefine platform ops mapping wrt Mbed platform */
#define gpio_ops mbed_gpio_ops
#define spi_ops mbed_spi_ops
#define i2c_ops mbed_i2c_ops
#define uart_ops mbed_uart_ops
#define vcom_ops mbed_virtual_com_ops
#define pwm_ops mbed_pwm_ops
#define trigger_gpio_irq_ops mbed_gpio_irq_ops

/* Unused macros */
#define SPI_DEVICE_ID   0
#define I2C_DEV_ID      0
#define LDAC_PWM_ID     0
#define CLEAR_GPIO_PORT 0
#define LDAC_GPIO_PORT  0
#define LDAC_PWM_ID     0
#define IRQ_CTRL_ID     0
#define UART_ID         0

/* Max spi clk speed */
#define MAX_SPI_CLK 12500000

/* Define the max possible sampling (or update) rate for a given platform.
 * Note: Max possible update rate is 58.82 KSPS per channel on IIO client.
 * This is derived by testing the firmware on SDP-K1 controller board with STM32F469NI MCU
 * using GCC and ARM compilers. The max possible update rate can vary from board to board and
 * data continuity is not guaranteed above this update rate */
#define MAX_SAMPLING_RATE					(uint32_t)(58823)

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct mbed_uart_init_param mbed_uart_extra_init_params;
extern struct mbed_uart_init_param mbed_vcom_extra_init_params;
extern struct mbed_spi_init_param mbed_spi_extra_init_params;
extern struct mbed_i2c_init_param mbed_i2c_extra_init_params;
extern struct mbed_gpio_init_param mbed_ldac_gpio_init_params;
extern struct mbed_gpio_init_param mbed_clear_gpio_init_params;
extern struct mbed_gpio_irq_init_param mbed_trigger_gpio_irq_init_params;
extern struct mbed_pwm_init_param mbed_pwm_extra_init_params;

#endif /* APP_CONFIG_MBED_H_ */