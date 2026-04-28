/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module
 *   @details This module contains the configurations needed for IIO application
********************************************************************************
 * Copyright (c) 2026 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include "app_config.h"
#include "common.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
/* SPI Init parameters */
struct no_os_spi_init_param spi_init_params = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = SPI_SPEED,
	.mode = NO_OS_SPI_MODE_3,
	.chip_select = SPI_CSB,
	.bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST,
	.platform_ops = &spi_ops,
	.extra = &spi_extra_init_params
};

/* I2C init parameters */
struct no_os_i2c_init_param i2c_init_params = {
	.device_id = I2C_DEVICE_ID,
	.platform_ops = &i2c_ops,
	.max_speed_hz = 100000,
	.extra = &i2c_extra_init_params
};

/* UART init parameters for IIO comm port */
struct no_os_uart_init_param uart_iio_comm_init_params = {
	.device_id = UART_DEVICE_ID,
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
	.platform_ops = &uart_ops,
	.extra = &uart_extra_init_params,
#endif
};

#if defined(CONSOLE_STDIO_PORT_AVAILABLE)
/* UART init parameters for console comm port */
struct no_os_uart_init_param uart_console_stdio_init_params = {
	.device_id = UART_DEVICE_ID,
	.asynchronous_rx = true,
	.irq_id = UART_IRQ_ID,
	.baud_rate = IIO_UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
#if defined(USE_VIRTUAL_COM_PORT)
	/* If virtual com port is primary IIO comm port, use physical port for stdio
	 * console. Applications which does not support VCOM, should not satisfy this
	 * condition */
	.platform_ops = &uart_ops,
	.extra = &uart_extra_init_params,
#else
	/* Applications which uses phy COM port as primary IIO comm port,
	 * can use VCOM as console stdio port provided it is available.
	 * Else, alternative phy com port can be used for console stdio ops if available */
	.platform_ops = &vcom_ops,
	.extra = &vcom_extra_init_params,
#endif
};
#endif

/* CLEAR_N GPIO Init parameters */
struct no_os_gpio_init_param gpio_clear_n_init_params = {
	.port = GPIO_CLEAR_N_PORT,
	.number = GPIO_CLEAR_N,
	.platform_ops = &gpio_ops,
	.extra = &gpio_output_extra_init_params,
};

/* RESET_N GPIO Init parameters */
struct no_os_gpio_init_param gpio_reset_n_init_params = {
	.port = GPIO_RESET_N_PORT,
	.number = GPIO_RESET_N,
	.platform_ops = &gpio_ops,
	.extra = &gpio_output_extra_init_params,
};

/* ALARM_N GPIO Init parameters */
struct no_os_gpio_init_param gpio_alarm_n_init_params = {
	.port = GPIO_ALARM_N_PORT,
	.number = GPIO_ALARM_N,
	.platform_ops = &gpio_ops,
	.extra = &gpio_input_extra_init_params,
};

/* MD_ADDR0 GPIO Init parameters */
struct no_os_gpio_init_param gpio_md_addr0_init_params = {
	.port = GPIO_MD_ADDR0_PORT,
	.number = GPIO_MD_ADDR0,
	.platform_ops = &gpio_ops,
	.extra = &gpio_output_extra_init_params,
};

/* MD_ADDR1 GPIO Init parameters */
struct no_os_gpio_init_param gpio_md_addr1_init_params = {
	.port = GPIO_MD_ADDR1_PORT,
	.number = GPIO_MD_ADDR1,
	.platform_ops = &gpio_ops,
	.extra = &gpio_output_extra_init_params,
};

/* LDAC_TGPx GPIO Init parameters.
 * Default will be in output state Whenever required, GPIO (TGP0) will be
 * reinitialized with PWM extra init params for timer output
 */
struct no_os_gpio_init_param gpio_ldac_tgpx_init_params[NUM_TGPx] = {
	{
		.port = GPIO_LDAC_TOGGLE0_PORT,
		.number = GPIO_LDAC_TOGGLE0,
		.pull = NO_OS_PULL_DOWN,
		.platform_ops = &gpio_ops,
		.extra = &gpio_output_extra_init_params,
	},
	{
		.port = GPIO_LDAC_TOGGLE1_PORT,
		.number = GPIO_LDAC_TOGGLE1,
		.pull = NO_OS_PULL_DOWN,
		.platform_ops = &gpio_ops,
		.extra = &gpio_output_extra_init_params,
	},
	{
		.port = GPIO_LDAC_TOGGLE2_PORT,
		.number = GPIO_LDAC_TOGGLE2,
		.pull = NO_OS_PULL_DOWN,
		.platform_ops = &gpio_ops,
		.extra = &gpio_output_extra_init_params,
	},
	{
		.port = GPIO_LDAC_TOGGLE3_PORT,
		.number = GPIO_LDAC_TOGGLE3,
		.pull = NO_OS_PULL_DOWN,
		.platform_ops = &gpio_ops,
		.extra = &gpio_output_extra_init_params,
	},
};

/* IRQ Trigger Init parameters */
#if (INTERFACE_MODE == SPI_INTERRUPT)
struct no_os_irq_init_param irq_iio_trigger_init_params = {
	.irq_ctrl_id = IRQ_IIO_TRIGGER_ID,
	.platform_ops = &irq_ops,
};
#endif

/* EEPROM Init parameters */
struct eeprom_24xx32a_init_param eeprom_extra_init_params = {
	.i2c_init = &i2c_init_params
};

/* EEPROM Init parameters */
struct no_os_eeprom_init_param eeprom_init_params = {
	.device_id = 0,
	.platform_ops = &eeprom_24xx32a_ops,
	.extra = &eeprom_extra_init_params
};

/* EEPROM descriptor */
struct no_os_eeprom_desc *eeprom_desc;

/* IIO UART Descriptor */
struct no_os_uart_desc *uart_iio_comm_desc;

#if defined(CONSOLE_STDIO_PORT_AVAILABLE)
/* Console UART Descriptor */
struct no_os_uart_desc *uart_console_stdio_desc = NULL;
#endif

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/
/**
 * @brief 	Set prescaler for timer.
 * @param 	desc[in] - The PWM descriptor.
 * @param 	prescaler[in] - Prescaler to be set.
 * @return 	0 in case of success else negative error code.
 */
int32_t set_timer_prescaler(struct no_os_pwm_desc *desc, uint32_t prescaler)
{
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	struct stm32_pwm_desc *sdesc = ((struct stm32_pwm_desc *)desc->extra);

	sdesc->prescaler = prescaler;
	((TIM_HandleTypeDef *)sdesc->htimer)->Instance->PSC = prescaler;

	return 0;
#else
	return -ENOTSUP;
#endif
}
/**
 * @brief	Initialize the UART peripheral
 * @return	0 in case of success else negative error code.
 */
static int32_t uart_init(void)
{
	int32_t ret;

	/* Initialize the serial link for IIO communication */
	ret = no_os_uart_init(&uart_iio_comm_desc, &uart_iio_comm_init_params);
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
 * @brief 	Initializing system peripherals
 * @return	0 in case of success, negative error code otherwise.
 * @details	This function initializes system peripherals for the application
 */
int32_t init_system(void)
{
	int32_t ret;

	/* Initialize the system */
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	ret = stm32_init_system();
	if (ret) {
		return ret;
	}
#endif

	/* Initialize UART */
	ret = uart_init();
	if (ret) {
		return ret;
	}

	/* Initialize EEPROM */
	ret = eeprom_init(&eeprom_desc, &eeprom_init_params);
	if (ret)
		return ret;

#ifdef USE_SDRAM
	/* Initialize the SDRAM */
	ret = sdram_init();
	if (ret) {
		return ret;
	}
#endif

	return ret;
}
