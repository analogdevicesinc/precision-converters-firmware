/***************************************************************************//**
 *   @file    app_config_mbed.h
 *   @brief   Header file for Mbed platform configurations
********************************************************************************
 * Copyright (c) 2022-2023 Analog Devices, Inc.
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
#include "mbed_i2c.h"
#include "mbed_gpio.h"
#include "mbed_gpio_irq.h"
#include "mbed_pwm.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Pin mapping of SDP-K1 w.r.t Arduino connector */
#define SPI_CSB		ARDUINO_UNO_D10
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_SCK		ARDUINO_UNO_D13

#define I2C_SCL			ARDUINO_UNO_D15
#define I2C_SDA			ARDUINO_UNO_D14

/* GPIO Pins for the Pin control mode in AD777x */
#define GPIO_RESET_PIN			ARDUINO_UNO_D2
#define GPIO_MODE0_PIN			0 // Unused
#define GPIO_MODE1_PIN			0 // Unused
#define GPIO_MODE2_PIN			0 // Unused
#define GPIO_MODE3_PIN			0 // Unused

#define GPIO_DCLK0_PIN			0 // Unused
#define GPIO_DCLK1_PIN			0 // Unused
#define GPIO_DCLK2_PIN			0 // Unused
#define GPIO_SYNC_IN_PIN		ARDUINO_UNO_D0
#define GPIO_CONVST_SAR_PIN		ARDUINO_UNO_D1

#define GPIO_DRDY_PIN			ARDUINO_UNO_D8
#define GPIO_ERROR_LED          LED1 // Red LED
#define EXT_MCLK_PIN            ARDUINO_UNO_D3

/* Port names (Unused) */
#define GPIO_DRDY_PORT      0
#define GPIO_RESET_PORT     0
#define GPIO_MODE0_PORT     0
#define GPIO_MODE1_PORT     0
#define GPIO_MODE2_PORT     0
#define GPIO_MODE3_PORT     0
#define GPIO_DCLK0_PORT     0
#define GPIO_DCLK1_PORT     0
#define GPIO_DCLK2_PORT     0
#define GPIO_CONVST_PORT    0
#define GPIO_SYNC_PORT   	0
#define GPIO_ERROR_LED_PORT 0

#define IRQ_INT_ID		GPIO_IRQ_ID1
#define DRDY_IRQ_CTRL_ID	0
#define UART_DEVICE_ID      0
#define SPI_DEVICE_ID       0
#define I2C_DEVICE_ID       0
#define MCLK_PWM_ID         0

/* Common Pin mapping of UART */
#define UART_TX		CONSOLE_TX
#define	UART_RX		CONSOLE_RX

/* Define the Sampling Frequency. It needs to be noted that the
 * maximum sampling frequency attainable in SPI Mode is
 * only 12ksps. This restriction is due to the time taken
 * by the SPI drivers to read the data of all the 8 channels
 * available on the SPI line. Occurrence of successive interrupts
 * during the SPI read time posts a restriction on the maximum
 * achievable ODR on the software */
#define AD777x_SAMPLING_FREQUENCY		12000

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct mbed_uart_init_param mbed_uart_extra_init_params;
extern struct mbed_uart_init_param mbed_vcom_extra_init_params;
extern struct mbed_spi_init_param mbed_spi_extra_init_params;
extern struct mbed_i2c_init_param mbed_i2c_extra_init_params;
extern struct mbed_gpio_init_param mbed_gpio_reset_extra_init_params;
extern struct mbed_gpio_init_param mbed_gpio_mode0_extra_init_params;
extern struct mbed_gpio_init_param mbed_gpio_mode1_extra_init_params;
extern struct mbed_gpio_init_param mbed_gpio_mode2_extra_init_params;
extern struct mbed_gpio_init_param mbed_gpio_mode3_extra_init_params;
extern struct mbed_gpio_init_param mbed_gpio_dclk0_extra_init_params;
extern struct mbed_gpio_init_param mbed_gpio_dclk1_extra_init_params;
extern struct mbed_gpio_init_param mbed_gpio_dclk2_extra_init_params;
extern struct mbed_gpio_init_param mbed_gpio_sync_in_extra_init_params;
extern struct mbed_gpio_init_param mbed_gpio_convst_sar_extra_init_params;
extern struct mbed_gpio_init_param mbed_gpio_drdy_extra_init_params;
extern struct mbed_gpio_init_param mbed_gpio_error_extra_init_params;
extern struct mbed_gpio_irq_init_param mbed_trigger_gpio_irq_init_params;
extern struct mbed_pwm_init_param mbed_pwm_init_params;
void ad777x_configure_intr_priority(void);

#endif /* APP_CONFIG_MBED_H_ */

