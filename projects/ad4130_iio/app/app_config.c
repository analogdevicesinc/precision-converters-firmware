/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module
 *   @details This module contains the configurations needed for IIO application
********************************************************************************
 * Copyright (c) 2020-2023, 2025 Analog Devices, Inc.
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
#include "common.h"
#include "no_os_error.h"
#include "no_os_uart.h"
#include "no_os_irq.h"
#include "no_os_gpio.h"
#include "no_os_i2c.h"
#include "no_os_eeprom.h"
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
#include "pl_gui_events.h"
#include "pl_gui_views.h"
#endif

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/* This value is calculated for SDP-K1 eval board (STM32F469NI MCU)
 * at 180Mhz core clock frequency */
#define EEPROM_OPS_START_DELAY		0xfffff

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* UART init parameters */
struct no_os_uart_init_param uart_init_params = {
	.device_id = UART_MODULE,
	.baud_rate = IIO_UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
	.asynchronous_rx = true,
	.irq_id = UART_IRQ,
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
	.device_id = UART_MODULE,
	.asynchronous_rx = false,
	.baud_rate = IIO_UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
#if defined(USE_VIRTUAL_COM_PORT)
	/* If virtual com port is primary IIO comm port, use physical port for stdio
	 * console. Applicatios which does not support VCOM, should not satisfy this
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

/* SPI initialization parameters */
struct no_os_spi_init_param spi_init_params = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = 10000000,		// Max SPI Speed
	.chip_select = SPI_CSB,			// Chip Select
	.mode = NO_OS_SPI_MODE_3,		// CPOL = 1, CPHA = 1
	.platform_ops = &spi_ops,
	.extra = &spi_extra_init_params	// SPI extra configurations
};

/* Trigger GPIO init parameters */
struct no_os_gpio_init_param trigger_gpio_param = {
	.port = TRIGGER_GPIO_PORT,
	.number = TRIGGER_GPIO_PIN,
	.pull = NO_OS_PULL_NONE,
	.platform_ops = &trigger_gpio_ops,
	.extra = &trigger_gpio_extra_init_params
};

/* Trigger GPIO IRQ parameters */
struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = TRIGGER_INT_ID,
	.platform_ops = &trigger_gpio_irq_ops,
	.extra = &trigger_gpio_irq_extra_params
};

/* External interrupt callback descriptor */
static struct no_os_callback_desc ext_int_callback_desc = {
	.callback = ad4130_fifo_event_handler,
};

/* I2C init parameters */
static struct no_os_i2c_init_param no_os_i2c_init_params = {
	.device_id = I2C_DEVICE_ID,
	.platform_ops = &i2c_ops,
	.max_speed_hz = 100000,
};

/* EEPROM init parameters */
static struct eeprom_24xx32a_init_param eeprom_extra_init_params = {
	.i2c_init = &no_os_i2c_init_params
};

/* EEPROM init parameters */
static struct no_os_eeprom_init_param eeprom_init_params = {
	.device_id = 0,
	.platform_ops = &eeprom_24xx32a_ops,
	.extra = &eeprom_extra_init_params
};

/* UART descriptor */
struct no_os_uart_desc *uart_desc;

/* UART Console Stdio descriptor */
struct no_os_uart_desc *uart_console_stdio_desc;

/* Trigger GPIO descriptor */
struct no_os_gpio_desc *trigger_gpio_desc;

/* Trigger GPIO interrupt descriptor */
struct no_os_irq_ctrl_desc *trigger_irq_desc;

/* Ticker interrupt descriptor */
struct no_os_irq_ctrl_desc *ticker_int_desc;

/* EEPROM descriptor */
struct no_os_eeprom_desc *eeprom_desc;

/******************************************************************************/
/************************ Functions Prototypes ********************************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
void lvgl_tick_callback(void *ctx)
{
	pl_gui_lvgl_tick_update(LVGL_TICK_TIME_MS);
}
#endif

/**
 * @brief Initialize the trigger GPIO and associated IRQ event
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t gpio_trigger_init(void)
{
	int32_t ret;

	/* Configure GPIO as input */
	ret = no_os_gpio_get(&trigger_gpio_desc, &trigger_gpio_param);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_input(trigger_gpio_desc);
	if (ret) {
		return ret;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	/* Init interrupt controller for external interrupt */
	ret = no_os_irq_ctrl_init(&trigger_irq_desc, &trigger_gpio_irq_params);
	if (ret) {
		return ret;
	}

	/* The UART interrupt needs to be prioritized over the GPIO (end of conversion) interrupt.
	* If not, the GPIO interrupt may occur during the period where there is a UART read happening
	* for the READBUF command. If UART interrupts are not prioritized, then it would lead to missing of
	* characters in the IIO command sent from the client. */
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_REMOTE)
	ret = no_os_irq_set_priority(trigger_irq_desc, TRIGGER_INT_ID,
				     RDY_GPIO_PRIORITY);
	if (ret) {
		return ret;
	}
#endif
#endif

#if (DATA_CAPTURE_MODE == FIFO_DATA_CAPTURE)
	/* For FIFO mode, the IIO hardware trigger is not used. The FIFO interrupt
	 * event is mapped to callback function defined in application layer */

	/* Init interrupt controller for external interrupt */
	ret = no_os_irq_ctrl_init(&trigger_irq_desc, &trigger_gpio_irq_params);
	if (ret) {
		return ret;
	}

	/* Register a callback function for external interrupt */
	ret = no_os_irq_register_callback(trigger_irq_desc,
					  TRIGGER_INT_ID,
					  &ext_int_callback_desc);
	if (ret) {
		return ret;
	}

	ret = no_os_irq_trigger_level_set(trigger_irq_desc,
					  TRIGGER_INT_ID, NO_OS_IRQ_EDGE_RISING);
	if (ret) {
		return ret;
	}

	/* Enable external interrupt */
	ret = no_os_irq_enable(trigger_irq_desc, TRIGGER_INT_ID);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief 	Initialize the UART peripheral
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t init_uart(void)
{
	int ret;

	/* Initialize the serial link for IIO communication */
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

	/* Set up the UART for standard I/O operations */
	no_os_uart_stdio(uart_console_stdio_desc);
#endif

	return 0;
}

/**
 * @brief 	Initialize the system peripherals
 * @return	0 in case of success, negative error code otherwise
 */
int32_t init_system(void)
{
	int32_t ret;

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	stm32_system_init();
#endif

	ret = init_uart();
	if (ret) {
		return ret;
	}

	ret = gpio_trigger_init();
	if (ret) {
		return ret;
	}

#if defined(USE_SDRAM)
	ret = sdram_init();
	if (ret) {
		return ret;
	}
#endif

	ret = eeprom_init(&eeprom_desc, &eeprom_init_params);
	if (ret) {
		return ret;
	}

	return 0;
}

