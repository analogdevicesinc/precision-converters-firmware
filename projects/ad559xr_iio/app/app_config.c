/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module
 *   @details This module contains the configurations needed for ad559xr
 *            IIO application firmware
********************************************************************************
 * Copyright (c) 2026 Analog Devices, Inc.
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
#include "no_os_gpio.h"

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
#if defined(USE_VIRTUAL_COM_PORT)
	.platform_ops = &vcom_ops,
	.extra = &vcom_extra_init_params,
#else
#if defined(CONSOLE_STDIO_PORT_AVAILABLE)
	.platform_ops = &uart_ops,
	.extra = &uart_extra_init_params,
#endif
#endif
};

/* UART init parameters for console comm port */
struct no_os_uart_init_param uart_console_stdio_init_params = {
	.device_id = 0,
	.asynchronous_rx = true,
	.baud_rate = IIO_UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
	.irq_id = UART_IRQ_ID,
#if defined(USE_VIRTUAL_COM_PORT)
	/* If virtual com port is primary IIO comm port, use physical port for stdio
	 * console. Applications which does not support VCOM, should not satisfy this
	 * condition */
	.platform_ops = &uart_ops,
	.extra = &uart_extra_init_params
#else
	/* Applications which uses phy COM port as primary IIO comm port,
	 * can use VCOM as console stdio port.
	 * Else, alternative phy com port can be used for console stdio ops if available */
	.platform_ops = &vcom_ops,
	.extra = &vcom_extra_init_params
#endif
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
	.device_id = I2C_DEVICE_ID,
	.platform_ops = &eeprom_24xx32a_ops,
	.extra = &eeprom_extra_init_params
};

/* PWM init parameters */
static struct no_os_pwm_init_param pwm_init_params = {
	.id = TIMER1_ID,
	.period_ns = PERIOD_IN_NS,
	.duty_cycle_ns = DUTY_CYCLE_IN_NS,
	.irq_id = PWM_TIM_IRQ_ID,
	.pwm_callback = ad559xr_trigger_handler,
	.extra = &stm32_pwm_extra_init_params,
	.platform_ops = &pwm_ops
};

/* UART descriptor */
struct no_os_uart_desc *uart_desc;

/* Console Stdio Desc */
struct no_os_uart_desc *uart_console_stdio_desc;

/* EEPROM descriptor */
struct no_os_eeprom_desc *eeprom_desc;

/* PWM Descriptor */
struct no_os_pwm_desc *pwm_desc;

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
 * @brief 	Initialize the PWM
 * @return	0 in case of success, negative error code otherwise.
 * @details	This function initialize the PWM
 */
static int32_t init_pwm(void)
{
	int32_t ret;

	/* Initialize the PWM interface */
	ret = no_os_pwm_init(&pwm_desc, &pwm_init_params);
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

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	stm32_system_init();
#endif

	ret = init_uart();
	if (ret) {
		return ret;
	}

	ret = init_pwm();
	if (ret) {
		return ret;
	}

#if defined(USE_SDRAM)
	ret = sdram_init();
	if (ret) {
		return ret;
	}
#endif

	/* EEPROM Init */
	ret = eeprom_init(&eeprom_desc, &eeprom_init_params);
	if (ret) {
		return ret;
	}

	return 0;
}
