/***************************************************************************//**
 * @file    app_config.c
 * @brief   Source file for the application configuration for AD717x IIO Application
********************************************************************************
* Copyright (c) 2021-23,2025 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include "no_os_uart.h"
#include "ad717x.h"
#include "app_config.h"
#include "no_os_gpio.h"
#include "no_os_irq.h"
#include "no_os_error.h"
#include "common.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* The UART Descriptor */
struct no_os_uart_desc *uart_desc;

/* UART console descriptor */
struct no_os_uart_desc *uart_console_stdio_desc;

/* GPIO descriptor for the chip select pin */
struct no_os_gpio_desc *csb_gpio;

/* GPIO descriptor for the RDY pin */
struct no_os_gpio_desc *rdy_gpio;

/* External interrupt descriptor */
struct no_os_irq_ctrl_desc *trigger_irq_desc;

/* EEPROM descriptor */
struct no_os_eeprom_desc *eeprom_desc;

/* UART Initialization Parameters */
static struct no_os_uart_init_param uart_init_params = {
	.device_id = UART_ID,
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

/* GPIO - Chip select Pin init parameters */
static struct no_os_gpio_init_param csb_init_param = {
	.number = SPI_CSB,
	.port = SPI_CS_PORT,
	.platform_ops = &csb_platform_ops,
	.extra = NULL
};

/* GPIO RDY Pin init parameters */
static struct no_os_gpio_init_param rdy_init_param = {
	.port = RDY_PORT,
	.number = RDY_PIN,
	.platform_ops = &rdy_platform_ops,
	.extra = NULL
};

/* External interrupt init parameters */
static struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = IRQ_INT_ID,
	.platform_ops = &irq_platform_ops,
	.extra = &ext_int_extra_init_params
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

/******************************************************************************/
/************************** Functions Declaration *****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definition ******************************/
/******************************************************************************/

/**
 * @brief 	Initialize the UART peripheral
 * @return	0 in case of success, negative error code
 */
static int32_t init_uart(void)
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

	no_os_uart_stdio(uart_console_stdio_desc);
#endif

	return 0;
}


/**
 * @brief Initialize the IRQ contoller
 * @return 0 in case of success, negative error code otherwise
 * @details This function initialize the interrupts for system peripherals
 */
int32_t init_interrupt(void)
{
	int32_t ret;

	do {
		/* Init interrupt controller for external interrupt */
		ret = no_os_irq_ctrl_init(&trigger_irq_desc, &trigger_gpio_irq_params);
		if (ret) {
			break;
		}

		return 0;
	} while (0);

	return ret;
}


/**
 * @brief 	Initialize the system peripherals
 * @return	- 0 in case of success, negative error code otherwise
 */
int32_t init_system(void)
{
	int32_t ret;

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	stm32_system_init();
#endif

	if (init_uart() != 0) {
		return -EINVAL;
	}

#if defined(USE_SDRAM)
	if (sdram_init() != 0) {
		return -EINVAL;
	}
#endif

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = init_interrupt();
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_get(&csb_gpio, &csb_init_param);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(csb_gpio, NO_OS_GPIO_HIGH);
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
#endif

	ret = eeprom_init(&eeprom_desc, &eeprom_init_params);
	if (ret) {
		return ret;
	}

	return 0;
}
