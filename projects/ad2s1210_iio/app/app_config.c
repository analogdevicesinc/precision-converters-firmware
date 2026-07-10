/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module (platform-agnostic)
 *   @details This module performs the system configurations
********************************************************************************
 * Copyright (c) 2023, 2026 Analog Devices, Inc.
 * Copyright (c) 2023 BayLibre, SAS.
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

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
/* PWM GPIO init parameters */
static struct no_os_gpio_init_param pwm_gpio_init_params = {
	.number = TRIGGER_INT_ID,
	.port =  GPIO_TRIGGER_INT_PORT,
	.platform_ops = &gpio_ops,
	.extra = &stm32_pwm_gpio_init_params
};
#endif


/* PWM init parameters */
struct no_os_pwm_init_param pwm_init_params = {
	.id = PWM_ID,
	.period_ns = CONV_TRIGGER_PERIOD_NSEC,
	.duty_cycle_ns = CONV_TRIGGER_DUTY_CYCLE_NSEC,
	.platform_ops = &pwm_ops,
	.extra = &pwm_extra_init_params,
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	.pwm_gpio = &pwm_gpio_init_params
#endif
};

/* Trigger GPIO IRQ parameters */
struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = TRIGGER_GPIO_IRQ_CTRL_ID,
	.platform_ops = &trigger_gpio_irq_ops,
	.extra = &trigger_gpio_irq_extra_params
};

/* UART init parameters */
struct no_os_uart_init_param uart_init_params = {
	.device_id = UART_ID,
	.asynchronous_rx = true,
	.baud_rate = IIO_UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	.asynchronous_rx = true,
	.irq_id = UART_IRQ_ID,
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
	.asynchronous_rx = true,
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

/* I2C init parameters */
struct no_os_i2c_init_param no_os_i2c_init_params = {
	.device_id = I2C_DEVICE_ID,
	.platform_ops = &i2c_ops,
	.max_speed_hz = 100000,
	.extra = &i2c_extra_init_params,
};

/* EEPROM extra init parameters */
struct eeprom_24xx32a_init_param eeprom_extra_init_params = {
	.i2c_init = &no_os_i2c_init_params
};

/* EEPROM init parameters */
struct no_os_eeprom_init_param eeprom_init_params = {
	.device_id = I2C_DEVICE_ID,
	.platform_ops = &eeprom_24xx32a_ops,
	.extra = &eeprom_extra_init_params
};

/* UART descriptor */
struct no_os_uart_desc *uart_desc;

/* UART console descriptor */
struct no_os_uart_desc *uart_console_stdio_desc;

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
 * @brief Initialize the IRQ contoller
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t gpio_trigger_init(void)
{
	int32_t ret;

	/* Initialize the IRQ controller */
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
	 * on conversion trigger event pin
	 */
	ret = no_os_pwm_init(&pwm_desc, &pwm_init_params);
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

	/* Delay added for peripheral initialization */
	no_os_mdelay(2000);

	/* Initialize EEPROM */
	ret = eeprom_init(&eeprom_desc, &eeprom_init_params);
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
