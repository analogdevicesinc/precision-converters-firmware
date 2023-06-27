/***************************************************************************//**
 * @file    app_config.c
 * @brief   Source file for the application configuration for AD717x IIO Application
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
	.device_id = NULL,
	.baud_rate = IIO_UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
	.platform_ops = &uart_ops,
	.extra = &uart_extra_init_params
};

/* GPIO - Chip select Pin init parameters */
static struct no_os_gpio_init_param csb_init_param = {
	.number = SPI_CSB,
	.platform_ops = &csb_platform_ops,
	.extra = NULL
};

/* GPIO RDY Pin init parameters */
static struct no_os_gpio_init_param rdy_init_param = {
	.number = RDY_PIN,
	.platform_ops = &rdy_platform_ops,
	.extra = NULL
};

/* External interrupt init parameters */
static struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = 0,
	.platform_ops = &irq_platform_ops,
	.extra = &ext_int_extra_init_params
};

/* I2C init parameters */
static struct no_os_i2c_init_param no_os_i2c_init_params = {
	.device_id = 0,
	.platform_ops = &i2c_ops,
	.max_speed_hz = 100000,
	.extra = &i2c_extra_init_params
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
	return no_os_uart_init(&uart_desc, &uart_init_params);
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
