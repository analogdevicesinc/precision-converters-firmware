/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module (platform-agnostic)
 *   @details This module performs the system configurations
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
#include "common.h"
#include "no_os_error.h"
#include "no_os_uart.h"
#include "no_os_gpio.h"
#include "no_os_irq.h"
#include "no_os_pwm.h"
#include "no_os_i2c.h"
#include "no_os_delay.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* UART init parameters */
static struct no_os_uart_init_param uart_init_params = {
	.device_id = NULL,
	.baud_rate = IIO_UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
	.platform_ops = &uart_ops,
	.extra = &uart_extra_init_params
};

/* LED GPO init parameters */
static struct no_os_gpio_init_param led_gpio_init_params = {
	.number = LED_GPO,
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

/* Trigger GPIO IRQ parameters */
struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = 0,
	.platform_ops = &trigger_gpio_irq_ops,
	.extra = &trigger_gpio_irq_extra_params
};

/* PWM init parameters */
static struct no_os_pwm_init_param pwm_init_params = {
	.id = 0,
	.period_ns = CONV_TRIGGER_PERIOD_NSEC,			// PWM period in nsec
	.duty_cycle_ns = CONV_TRIGGER_DUTY_CYCLE_NSEC,	// PWM duty cycle in nsec
	.extra = &pwm_extra_init_params
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

/* LED GPO descriptor */
struct no_os_gpio_desc *led_gpio_desc;

/* UART descriptor */
struct no_os_uart_desc *uart_desc;

/* Trigger GPIO descriptor */
struct no_os_gpio_desc *trigger_gpio_desc;

/* Trigger GPIO interrupt descriptor */
struct no_os_irq_ctrl_desc *trigger_irq_desc;

/* PWM descriptor */
struct no_os_pwm_desc *pwm_desc;

/* EEPROM descriptor */
struct no_os_eeprom_desc *eeprom_desc;

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
	return no_os_uart_init(&uart_desc, &uart_init_params);
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

	ret = eeprom_init(&eeprom_desc, &eeprom_init_params);
	if (ret) {
		return ret;
	}

	return 0;
}
