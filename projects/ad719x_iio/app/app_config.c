/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module
 *   @details This module contains the configurations needed for ad719x
 *            IIO application firmware
********************************************************************************
 * Copyright (c) 2021-24 Analog Devices, Inc.
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
#include "common.h"
#include "no_os_error.h"
#include "no_os_uart.h"
#include "no_os_irq.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
/* UART init parameters structure */
struct no_os_uart_init_param uart_init_params = {
	.device_id = 0,
	.baud_rate = IIO_UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
	.asynchronous_rx = true,
	.irq_id = UART_IRQ_ID,
	.platform_ops = &uart_ops,
	.extra = &uart_extra_init_params
};

/* External interrupt init parameters */
static struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = TRIGGER_INT_ID,
	.platform_ops = &trigger_gpio_irq_ops,
	.extra = &trigger_gpio_irq_extra_params
};

/* External interrupt callback descriptor */
static struct no_os_callback_desc ext_int_callback_desc = {
	.callback = burst_capture_callback,
	.ctx = NULL,
	.peripheral = NO_OS_GPIO_IRQ
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

/* External interrupt descriptor */
struct no_os_irq_ctrl_desc *trigger_irq_desc;

/* EEPROM descriptor */
struct no_os_eeprom_desc *eeprom_desc;

/******************************************************************************/
/************************ Functions Prototypes ********************************/
/******************************************************************************/
/**
 * @brief 	Initialize the UART peripheral
 * @return	0 in case of success, negative error code otherwise.
 */
static int32_t init_uart(void)
{
	int32_t ret;

	ret = no_os_uart_init(&uart_desc, &uart_init_params);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief 	Initialize the IRQ contoller
 * @return	0 in case of success, negative error code otherwise.
 * @details	This function initialize the interrupts for system peripherals
 */
static int32_t init_interrupt(void)
{
	int32_t ret;

	/* Init interrupt controller for external interrupt (for monitoring
		* conversion event on BUSY pin) */
	ret = no_os_irq_ctrl_init(&trigger_irq_desc, &trigger_gpio_irq_params);
	if (ret) {
		return ret;
	}

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	/* Register a callback function for external interrupt */
	ret = no_os_irq_register_callback(trigger_irq_desc,
					  TRIGGER_INT_ID, &ext_int_callback_desc);
	if (ret) {
		return ret;
	}

	ret = no_os_irq_trigger_level_set(trigger_irq_desc,
					  TRIGGER_INT_ID, NO_OS_IRQ_EDGE_FALLING);
	if (ret) {
		return ret;
	}

	ret = no_os_irq_disable(trigger_irq_desc, TRIGGER_INT_ID);
	if (ret) {
		return ret;
	}
#endif
	return 0;
}

/**
 * @brief 	Initializing system peripherals
 * @return	0 in case of success, negative error code otherwise.
 * @details	This function initializes system peripherals for the application
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

	ret = init_interrupt();
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
