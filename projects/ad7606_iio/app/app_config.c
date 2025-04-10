/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module (platform-agnostic)
 *   @details This module performs the system configurations
********************************************************************************
 * Copyright (c) 2020-2023, 2025 Analog Devices, Inc.
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
#include "common.h"
#include "no_os_error.h"
#include "no_os_uart.h"
#include "no_os_gpio.h"
#include "no_os_irq.h"
#include "no_os_pwm.h"
#include "no_os_delay.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* UART init parameters */
static struct no_os_uart_init_param uart_init_params = {
	.device_id = 0,
	.baud_rate = IIO_UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	.asynchronous_rx = true,
	.irq_id =  UART_IRQ_ID,
#endif
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
	.device_id = UART_ID,
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

/* LED GPO init parameters */
static struct no_os_gpio_init_param led_gpio_init_params = {
	.number = LED_GPO,
	.port = LED_PORT,
	.platform_ops = &gpio_ops,
	.extra = NULL
};

/* Trigger GPIO init parameters */
struct no_os_gpio_init_param trigger_gpio_param = {
	.port = TRIGGER_GPIO_PORT,
	.number = TRIGGER_GPIO_PIN,
	.pull = NO_OS_PULL_NONE,
	.platform_ops = &trigger_gpio_ops,
	.extra = &trigger_gpio_extra_init_params
};

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
/* PWM GPIO init parameters */
static struct no_os_gpio_init_param pwm_gpio_init_params = {
	.number = PWM_TRIGGER,
	.port =  TRIGGER_GPIO_PORT,
	.platform_ops = &gpio_ops,
	.extra = &pwm_gpio_extra_init_params
};
#endif

/* Trigger GPIO IRQ parameters */
struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = IRQ_INT_ID,
	.platform_ops = &trigger_gpio_irq_ops,
	.extra = &trigger_gpio_irq_extra_params
};

/* PWM init parameters */
static struct no_os_pwm_init_param pwm_init_params = {
	.id = PWM_ID,
	.period_ns = CONV_TRIGGER_PERIOD_NSEC,			// PWM period in nsec
	.duty_cycle_ns = CONV_TRIGGER_DUTY_CYCLE_NSEC,	// PWM duty cycle in nsec
	.extra = &pwm_extra_init_params,
	.platform_ops = &pwm_ops,
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	.pwm_gpio = &pwm_gpio_init_params
#endif
};

/* LED GPO descriptor */
struct no_os_gpio_desc *led_gpio_desc;

/* UART descriptor */
struct no_os_uart_desc *uart_desc;
struct no_os_uart_desc *uart_console_stdio_desc;

/* Trigger GPIO descriptor */
struct no_os_gpio_desc *trigger_gpio_desc;

/* Trigger GPIO interrupt descriptor */
struct no_os_irq_ctrl_desc *trigger_irq_desc;

/* PWM descriptor */
struct no_os_pwm_desc *pwm_desc;

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/

/**
 * @brief 	Initialize the GPIOs
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t init_gpio(void)
{
	int32_t ret;

	/* Initialize the LED GPO */
	ret = no_os_gpio_get_optional(&led_gpio_desc, &led_gpio_init_params);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(led_gpio_desc, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief 	Initialize the UART peripheral
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t init_uart(void)
{
	int32_t ret;

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

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	no_os_uart_stdio(uart_console_stdio_desc);
#endif
#endif

	return 0;
}

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

	/* Init interrupt controller for external interrupt */
	ret = no_os_irq_ctrl_init(&trigger_irq_desc, &trigger_gpio_irq_params);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief 	Initialize the PWM trigger contoller
 * @return	0 in case of success, negative error code otherwise
 */
int32_t init_pwm_trigger(void)
{
	int32_t ret;

	/* Initialize the PWM interface to generate PWM signal
	 * on conversion trigger event pin */
	ret = no_os_pwm_init(&pwm_desc, &pwm_init_params);
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_enable(pwm_desc);
	if (ret) {
		return ret;
	}

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

	ret = init_gpio();
	if (ret) {
		return ret;
	}

	ret = init_uart();
	if (ret) {
		return ret;
	}

#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	ret = gpio_trigger_init();
	if (ret) {
		return ret;
	}
#endif

#if defined(USE_SDRAM)
	ret = sdram_init();
	if (ret) {
		return ret;
	}
#endif

	return 0;
}
