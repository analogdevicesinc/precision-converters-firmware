/***************************************************************************//**
 *   @file    app_config_mbed.c
 *   @brief   Application configurations module for Mbed platform
********************************************************************************
 * Copyright (c) 2021-22 Analog Devices, Inc.
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
#if defined (USE_PHY_COM_PORT)
	.uart_tx_pin = UART_TX,
	.uart_rx_pin = UART_RX,
	.virtual_com_enable = false
#else
	.virtual_com_enable = true,
	.vendor_id = VIRTUAL_COM_PORT_VID,
	.product_id = VIRTUAL_COM_PORT_PID,
	.serial_number = VIRTUAL_COM_SERIAL_NUM
#endif
};

/* External interrupt Mbed platform specific parameters */
struct mbed_gpio_irq_init_param mbed_trigger_gpio_irq_init_params = {
	.gpio_irq_pin = EXT_TRIGGER_PIN
};

/* SPI Mbed platform specific parameters */
struct mbed_spi_init_param mbed_spi_extra_init_params = {
	.spi_clk_pin = SPI_SCK,
	.spi_miso_pin = SPI_HOST_SDI,
	.spi_mosi_pin = SPI_HOST_SDO,
	.use_sw_csb = false
};

/* PWM Mbed platform specific parameters */
struct mbed_pwm_init_param mbed_pwm_extra_init_params = {
	.pwm_pin = PWM_PIN
};

/* GPIO Mbed platform specific parameters */
struct mbed_gpio_init_param mbed_gpio_bsy_extra_init_params = {
	.pin_mode = PullNone
};