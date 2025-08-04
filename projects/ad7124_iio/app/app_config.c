/***************************************************************************//**
 * @file    app_config.c
 * @brief   Source file for the application configuration for AD7124 IIO Application
********************************************************************************
* Copyright (c) 2023-25 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "app_config.h"
#include "no_os_uart.h"
#include "no_os_error.h"
#include "no_os_eeprom.h"
#include "common.h"
#include "no_os_irq.h"
#include "no_os_gpio.h"
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
#include "pl_gui_events.h"
#include "pl_gui_views.h"
#endif

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/* This value is calculated for SDP-K1 eval board (STM32F469NI MCU)
 * at 180Mhz core clock frequency */
#define EEPROM_OPS_START_DELAY		0xfffff

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* UART descriptor */
struct no_os_uart_desc *uart_desc;

/* UART console descriptor */
struct no_os_uart_desc *uart_console_stdio_desc;

/* EEPROM descriptor */
struct no_os_eeprom_desc *eeprom_desc;

/* GPIO descriptor for the chip select pin */
struct no_os_gpio_desc *csb_gpio;

/* GPIO descriptor for the RDY pin */
struct no_os_gpio_desc *rdy_gpio;

/* External interrupt descriptor */
struct no_os_irq_ctrl_desc *trigger_irq_desc;

/* Ticker interrupt callback descriptor */
static struct no_os_callback_desc ticker_int_callback_desc = {
	.callback = lvgl_tick_callback,
};

/* UART Initialization Parameters */
struct no_os_uart_init_param uart_init_params = {
	.device_id = 0,
	.baud_rate = IIO_UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
	.asynchronous_rx = true,
	.irq_id = UART_IRQ_ID,
#if defined(USE_VIRTUAL_COM_PORT)
	.platform_ops = &vcom_ops,
	.extra = &vcom_extra_init_params
#else
	.platform_ops = &uart_ops,
	.extra = &uart_extra_init_params
#endif
};

/* UART init parameters for console comm port */
struct no_os_uart_init_param uart_console_stdio_init_params = {
	.device_id = 0,
	.baud_rate = IIO_UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
#if defined(USE_VIRTUAL_COM_PORT)
	/* If virtual com port is primary IIO comm port, use physical port for stdio
	 * console. Applications which does not support VCOM, should not satisfy this
	 * condition */
	.platform_ops = &uart_ops,
	.extra = &uart_extra_init_params
#else
#if defined(CONSOLE_STDIO_PORT_AVAILABLE)
	/* Applications which uses phy COM port as primary IIO comm port,
	 * can use VCOM as console stdio port provided it is available.
	 * Else, alternative phy com port can be used for console stdio ops if available */
	.platform_ops = &vcom_ops,
	.extra = &vcom_extra_init_params
#endif
#endif
};

/* I2C init parameters */
static struct no_os_i2c_init_param no_os_i2c_init_params = {
	.device_id = I2C_DEVICE_ID,
	.platform_ops = &i2c_ops,
	.max_speed_hz = 100000,
	.extra = &i2c_extra_init_params
};

static struct eeprom_24xx32a_init_param eeprom_extra_init_params = {
	.i2c_init = &no_os_i2c_init_params
};

/* EEPROM init parameters */
static struct no_os_eeprom_init_param eeprom_init_params = {
	.device_id = I2C_DEVICE_ID,
	.platform_ops = &eeprom_24xx32a_ops,
	.extra = &eeprom_extra_init_params
};

#if(ACTIVE_PLATFORM == MBED_PLATFORM)
/* Ticker interrupt init parameters */
static struct no_os_irq_init_param ticker_int_init_params = {
	.irq_ctrl_id = 0,
	.platform_ops = &ticker_ops,
	.extra = &ticker_int_extra_init_params
};
#endif

/* GPIO RDY Pin init parameters */
static struct no_os_gpio_init_param rdy_init_param = {
	.number = RDY_PIN,
	.port = RDY_PORT,
	.platform_ops = &gpio_platform_ops,
};

/* GPIO - Chip select Pin init parameters */
static struct no_os_gpio_init_param csb_init_param = {
	.number = SPI_CSB,
	.port = SPI_CS_PORT,
	.platform_ops = &gpio_platform_ops,
	.extra = NULL
};

/* External interrupt init parameters */
static struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = IRQ_INT_ID,
	.platform_ops = &irq_platform_ops,
	.extra = &ext_int_extra_init_params
};

/* External interrupt callback descriptor */
static struct no_os_callback_desc ext_int_callback_desc = {
	.callback = data_capture_callback,
	.event = NO_OS_EVT_GPIO,
	.peripheral = NO_OS_GPIO_IRQ,
};

/* Ticker interrupt descriptor */
struct no_os_irq_ctrl_desc *ticker_int_desc;

/******************************************************************************/
/************************** Functions Declaration *****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definition ******************************/
/******************************************************************************/

/**
 * @brief 	Initialize the UART peripheral
 * @return	0 in case of success, negative error code otherwise
 */
static int init_uart(void)
{
	int ret;

	ret = no_os_uart_init(&uart_desc, &uart_init_params);
	if (ret) {
		return ret;
	}

#if defined(CONSOLE_STDIO_PORT_AVAILABLE)
	/* Initialize the serial link for console stdio communication */
	ret = no_os_uart_init(&uart_console_stdio_desc,
			      &uart_console_stdio_init_params);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief Initialize the IRQ contoller
 * @return 0 in case of success, negative error code otherwise
 * @details This function initialize the interrupts for system peripherals
 */
int init_interrupt(void)
{
	int ret;

	/* Init interrupt controller for external interrupt */
	ret = no_os_irq_ctrl_init(&trigger_irq_desc, &trigger_gpio_irq_params);
	if (ret) {
		return ret;
	}

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	ret = no_os_irq_register_callback(trigger_irq_desc,
					  IRQ_INT_ID,
					  &ext_int_callback_desc);
	if (ret) {
		return ret;
	}

	ret = no_os_irq_trigger_level_set(trigger_irq_desc,
					  IRQ_INT_ID,
					  NO_OS_IRQ_EDGE_FALLING);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief 	Initialize the system peripherals
 * @return	0 in case of success, negative error code otherwise
 */
int init_system(void)
{
	int ret;

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	stm32_system_init();
#endif

	ret = init_uart();
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_get(&csb_gpio, &csb_init_param);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_get(&rdy_gpio, &rdy_init_param);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_input(rdy_gpio);
	if (ret) {
		return ret;
	}

	ret = eeprom_init(&eeprom_desc, &eeprom_init_params);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(csb_gpio, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	ret = init_interrupt();
	if (ret) {
		return ret;
	}

	return 0;
}
