/***************************************************************************//**
 *   @file    app_config_mbed.c
 *   @brief   Application configurations module for Mbed platform
********************************************************************************
 * Copyright (c) 2021-23 Analog Devices, Inc.
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
#include "app_config.h"
#include "app_config_mbed.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* UART Mbed platform specific init parameters */
struct mbed_uart_init_param mbed_uart_extra_init_params = {
	.uart_tx_pin = UART_TX,
	.uart_rx_pin = UART_RX
};

/* VCOM Mbed platform specific init parameters */
struct mbed_uart_init_param mbed_vcom_extra_init_params = {
	.vendor_id = VIRTUAL_COM_PORT_VID,
	.product_id = VIRTUAL_COM_PORT_PID,
	.serial_number = VIRTUAL_COM_SERIAL_NUM
};

/* GPIO interrupt Mbed platform specific parameters */
struct mbed_gpio_irq_init_param mbed_trigger_gpio_irq_init_params = {
	.gpio_irq_pin = DIG_AUX_1,
};

struct mbed_gpio_init_param mbed_trigger_gpio_extra_init_params = {
	.pin_mode = 0	// NA
};

struct mbed_gpio_init_param mbed_dig_aux1_gpio_extra_init_params = {
	.pin_mode = 0	// NA
};

struct mbed_gpio_init_param mbed_dig_aux2_gpio_extra_init_params = {
	.pin_mode = 0	// NA
};

struct mbed_gpio_init_param mbed_sync_inb_gpio_extra_init_params = {
	.pin_mode = 0	// NA
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

/* Ticker interrupt Mbed platform specific parameters */
struct mbed_irq_init_param mbed_ticker_int_extra_init_params = {
	.int_obj_type = NULL,
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
	.ticker_period_usec = LVGL_TICK_TIME_US,
#else
	.ticker_period_usec = TICKER_INTERRUPT_PERIOD_uSEC,
#endif
};

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/
