/***************************************************************************//**
 *   @file    app_config_mbed.c
 *   @brief   Application configurations module for Mbed platform
********************************************************************************
 * Copyright (c) 2021, 2023 Analog Devices, Inc.
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

/* Note: The Interrupt ID depends on the pin name of the GPIO on which the
* ODR signal from the ADC is mapped to. The value of the below macro might need
* an update depending on the device chosen. PD_12 is the pin on SDP-K1 and
* PE_13 is the pin for Nucleo-L552ZEQ. */
#if defined (TARGET_NUCLEO_L552ZE_Q)
#define GPIO_IRQ_INTR_PRIORITY	EXTI13_IRQn
#else
#define GPIO_IRQ_INTR_PRIORITY	EXTI15_10_IRQn
#endif

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* UART Mbed platform specific init parameters */
struct mbed_uart_init_param mbed_uart_extra_init_params = {
	.uart_tx_pin = UART_TX,
	.uart_rx_pin = UART_RX,
};

/* VCOM Mbed platform specific init parameters */
struct mbed_uart_init_param mbed_vcom_extra_init_params = {
	.vendor_id = VIRTUAL_COM_PORT_VID,
	.product_id = VIRTUAL_COM_PORT_PID,
	.serial_number = VIRTUAL_COM_SERIAL_NUM
};

/* External interrupt Mbed platform specific parameters */
struct mbed_gpio_irq_init_param mbed_ext_int_extra_init_params = {
#if (AD7134_ASRC_MODE == CONTROLLER_MODE)
	.gpio_irq_pin = ODR_PIN,
#else
	.gpio_irq_pin = ODR_PIN,
#endif
};

/* SPI Mbed platform specific parameters */
struct mbed_spi_init_param mbed_spi_extra_init_params = {
	.spi_clk_pin = SPI_SCK,
	.spi_miso_pin = SPI_HOST_SDI,
	.spi_mosi_pin = SPI_HOST_SDO
};

/* PWM Mbed platform specific init parameters */
struct mbed_pwm_init_param mbed_pwm_extra_init_params = {
	.pwm_pin = ODR_PIN
};

/* I2C Mbed platform specific parameters */
struct mbed_i2c_init_param mbed_i2c_extra_init_params = {
	.i2c_sda_pin = I2C_SDA,
	.i2c_scl_pin = I2C_SCL
};

/* GPIO PDN mbed platform specific parameters */
struct mbed_gpio_init_param mbed_pdn_extra_init_params = {
	.pin_mode = 0
};

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/
/*!
 * @brief Configure the interrupt priorities
 * @return None
 */
void ad7134_configure_intr_priority(void)
{
	NVIC_SetPriority(GPIO_IRQ_INTR_PRIORITY, 1);
}