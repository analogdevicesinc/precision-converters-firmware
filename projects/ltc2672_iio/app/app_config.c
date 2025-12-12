/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module (platform-agnostic)
 *   @details This module performs the system configurations
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
#include "ltc2672_user_config.h"
#include "no_os_util.h"
#include "common.h"
#include "no_os_error.h"
#include "no_os_uart.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* UART init parameters for IIO comm port */
struct no_os_uart_init_param uart_iio_comm_init_params = {
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

#if defined (CONSOLE_STDIO_PORT_AVAILABLE)
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
#endif

/* Toggle PWM GPIO init parameters */
struct no_os_gpio_init_param toggle_pwm_gpio_params = {
	.port = GPIO_TGP_PORT,
	.number = GPIO_TGP_PIN,
	.platform_ops = &gpio_ops,
	.extra = &toggle_pwm_gpio_extra_params
};

/* PWM toggle init paramaters */
struct no_os_pwm_init_param toggle_pwm_init_params = {
	.id = TOGGLE_PWM_ID,
	.period_ns = FREQ_TO_NSEC(LTC2672_MAX_TOGGLE_RATE),
	.duty_cycle_ns = DUTY_CYCLE_NSEC(FREQ_TO_NSEC(LTC2672_MAX_TOGGLE_RATE)),
	.polarity = NO_OS_PWM_POLARITY_LOW,
	.platform_ops = &pwm_ops,
	.extra = &toggle_pwm_extra_init_params,
	.pwm_gpio = &toggle_pwm_gpio_params
};

/* I2C init parameters */
static struct no_os_i2c_init_param no_os_i2c_init_params = {
	.device_id = I2C_DEV_ID,
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

/* UART IIO Descriptor */
struct no_os_uart_desc *uart_iio_com_desc;

/* UART console descriptor */
struct no_os_uart_desc *uart_console_stdio_desc;

/* EEPROM descriptor */
struct no_os_eeprom_desc *eeprom_desc;

/* GPIO LDAC descriptor */
struct no_os_gpio_desc *gpio_ldac_desc;

/* GPIO Clear descriptor */
struct no_os_gpio_desc *gpio_clear_desc;

/* GPIO toggle descriptor */
struct no_os_gpio_desc *gpio_toggle_desc;

/* PWM toggle descriptor */
struct no_os_pwm_desc *toggle_pwm_desc;

/* PWM ldac descriptor */
struct no_os_pwm_desc *ldac_pwm_desc;

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

	ret = no_os_uart_init(&uart_iio_com_desc, &uart_iio_comm_init_params);
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
 * @brief 	Initialize the PWM interface.
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t init_pwm(void)
{
	int32_t ret;

	/* Initialize the PWM interface to generate toggle signal.*/
	ret = no_os_pwm_init(&toggle_pwm_desc, &toggle_pwm_init_params);
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_disable(toggle_pwm_desc);
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

	ret = init_pwm();
	if (ret) {
		return ret;
	}

	ret = eeprom_init(&eeprom_desc, &eeprom_init_params);
	if (ret) {
		return ret;
	}

	return 0;
}
