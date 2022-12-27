/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module
 *   @details This module contains the configurations needed for IIO application
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
#include "no_os_error.h"
#include "no_os_gpio.h"
#include "no_os_irq.h"
#include "no_os_pwm.h"
#include "no_os_uart.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
/* UART init parameters structure */
struct no_os_uart_init_param uart_init_params = {
	.device_id = NULL,
	.baud_rate = IIO_UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
	.extra = &uart_extra_init_params
};

/* External interrupt init parameters */
struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = 0,
	.platform_ops = &trigger_gpio_irq_ops,
	.extra = &trigger_gpio_irq_extra_params
};

/* External interrupt callback descriptor */
static struct no_os_callback_desc ext_int_callback_desc = {
	.callback = burst_capture_callback,
	.ctx = NULL,
	.peripheral = NO_OS_GPIO_IRQ
};

/* PWM init parameters */
static struct no_os_pwm_init_param pwm_init_params = {
	.id = 0,
	.period_ns = CONV_TRIGGER_PERIOD_NSEC(DEFAULT_SAMPLING_RATE),            // PWM period in nsec
	.duty_cycle_ns = CONV_TRIGGER_DUTY_CYCLE_NSEC(DEFAULT_SAMPLING_RATE),  	// PWM duty cycle in nsec
	.extra = &pwm_extra_init_params
};

/* External interrupt descriptor */
struct no_os_irq_ctrl_desc *trigger_irq_desc;

/* UART console output descriptor */
struct no_os_uart_desc *uart_desc;

/* PWM descriptor */
struct no_os_pwm_desc *pwm_desc;

/******************************************************************************/
/************************ Functions Prototypes ********************************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/
/**
 * @brief 	Initialize the UART peripheral.
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
 * @brief 	Initialize the IRQ controller.
 * @return	0 in case of success, negative error code otherwise.
 * @details	This function initialize the interrupts for system peripherals.
 */
static int32_t init_interrupt(void)
{
	int32_t ret;

	/* Init interrupt controller for external interrupt */
	ret = no_os_irq_ctrl_init(&trigger_irq_desc, &trigger_gpio_irq_params);
	if (ret) {
		return ret;
	}

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	/* The BSY pin has been tied as the interrupt source to sense the
	 * End of Conversion. The registered callback function is responsible
	 * of reading the raw samples via the SPI bus */
	ret = no_os_irq_register_callback(trigger_irq_desc,
					  TRIGGER_INT_ID,
					  &ext_int_callback_desc);
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
 * @brief 	Initialize the PWM interface.
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t init_pwm(void)
{
	int32_t ret;

	/* Initialize the PWM interface to generate PWM signal
	 * for recurring function calls */
	ret = no_os_pwm_init(&pwm_desc, &pwm_init_params);
	if (ret) {
		return ret;
	}

	/* Disable the PWM interface.*/
	ret = no_os_pwm_disable(pwm_desc);
	if (ret) {
		return ret;
	}

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
	return 0;
}