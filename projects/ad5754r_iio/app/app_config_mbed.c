/***************************************************************************//**
 *   @file    app_config_mbed.c
 *   @brief   Application configurations module for Mbed platform
********************************************************************************
 * Copyright (c) 2024 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdbool.h>
#include "app_config_mbed.h"
#include "app_config.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* Primary UART Mbed platform specific init parameters */
struct mbed_uart_init_param mbed_uart_extra_init_params = {
	.uart_tx_pin = UART_TX,
	.uart_rx_pin = UART_RX,
#if defined(USE_PHY_COM_PORT)
	.is_console_stdio_port = false
#else
	.is_console_stdio_port = true
#endif
};

/* VCOM Mbed platform specific init parameters */
struct mbed_uart_init_param mbed_vcom_extra_init_params = {
	.vendor_id = VIRTUAL_COM_PORT_VID,
	.product_id = VIRTUAL_COM_PORT_PID,
	.serial_number = VIRTUAL_COM_SERIAL_NUM,
#if defined(USE_VIRTUAL_COM_PORT)
	.is_console_stdio_port = false
#else
	.is_console_stdio_port = true
#endif
};

/* SPI Mbed platform specific parameters */
struct mbed_spi_init_param mbed_spi_extra_init_params = {
	.spi_clk_pin = SPI_SCK,
	.spi_miso_pin = SPI_HOST_SDI,
	.spi_mosi_pin = SPI_HOST_SDO,
	.use_sw_csb = false
};

/* I2C Mbed platform specific parameters */
struct mbed_i2c_init_param mbed_i2c_extra_init_params = {
	.i2c_sda_pin = I2C_SDA,
	.i2c_scl_pin = I2C_SCL
};

/* LDAC gpio Mbed platform specific parameters */
struct mbed_gpio_init_param mbed_ldac_gpio_init_params = {
	.pin_mode = 0
};

/* Clear gpio Mbed platform specific parameters */
struct mbed_gpio_init_param mbed_clear_gpio_init_params = {
	.pin_mode = 0
};

/* GPIO Trigger Mbed platform specific init parameters */
struct mbed_gpio_irq_init_param mbed_trigger_gpio_irq_init_params = {
	.gpio_irq_pin = PWM_TRIGGER,
};

/* PWM Mbed platform specific init parameters */
struct mbed_pwm_init_param mbed_pwm_extra_init_params = {
	.pwm_pin = PWM_TRIGGER
};

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/