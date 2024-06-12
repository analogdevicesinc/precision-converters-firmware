/***************************************************************************//**
 *   @file    app_config_mbed.c
 *   @brief   Application configurations module for Mbed platform
********************************************************************************
 * Copyright (c) 2023-2024 Analog Devices, Inc.
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

/* UART Mbed platform specific init parameters */
struct mbed_uart_init_param mbed_uart_init_params = {
	.uart_tx_pin = UART_TX,
	.uart_rx_pin = UART_RX,
#if defined(USE_PHY_COM_PORT)
	.is_console_stdio_port = false
#else
	.is_console_stdio_port = true
#endif
};

/* VCOM Mbed platform specific init parameters */
struct mbed_uart_init_param mbed_vcom_init_params = {
	.vendor_id = VIRTUAL_COM_PORT_VID,
	.product_id = VIRTUAL_COM_PORT_PID,
	.serial_number = VIRTUAL_COM_SERIAL_NUM,
#if defined(USE_VIRTUAL_COM_PORT)
	.is_console_stdio_port = false
#else
	.is_console_stdio_port = true
#endif
};

/* SPI Mbed platform specific init parameters */
struct mbed_spi_init_param mbed_spi_init_params = {
	.spi_clk_pin = SPI_SCK,
	.spi_miso_pin = SPI_HOST_SDI,
	.spi_mosi_pin = SPI_HOST_SDO,
	/* sw_csb is used to support spi tranfer function */
	.use_sw_csb = true
};

/* SPI Mbed platform specific init parameters without software csb */
struct mbed_spi_init_param mbed_spi_init_params_without_sw_csb = {
	.spi_clk_pin = SPI_SCK,
	.spi_miso_pin = SPI_HOST_SDI,
	.spi_mosi_pin = SPI_HOST_SDO,
	.use_sw_csb = false
};

/* PWM Mbed platform specific init parameters */
struct mbed_pwm_init_param mbed_pwm_init_params = {
	.pwm_pin = LDAC_PIN
};

/* External interrupt, Mbed platform specific parameters */
struct mbed_gpio_irq_init_param mbed_trigger_gpio_irq_init_params = {
	.gpio_irq_pin = LDAC_PIN
};

/* GPIO Mbed platform specific init parameters */
struct mbed_gpio_init_param mbed_gpio_ldac_init_params = {
	.pin_mode = 0
};

/* GPIO Mbed platform specific init parameters */
struct mbed_gpio_init_param mbed_gpio_reset_init_params = {
	.pin_mode = 0
};

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/
