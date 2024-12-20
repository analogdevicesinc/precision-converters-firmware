/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module
 *   @details This module contains the configurations needed for IIO application
********************************************************************************
 * Copyright (c) 2022-2024 Analog Devices, Inc.
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
#include "ad405x_iio.h"
#include "ad405x_user_config.h"
#include "no_os_delay.h"
#include "no_os_i2c.h"

#if (INTERFACE_MODE == SPI_DMA)
#include "no_os_dma.h"
#include "stm32_dma.h"
#endif

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
/* UART init parameters for IIO comm port */
struct no_os_uart_init_param uart_iio_comm_init_params = {
	.device_id = UART_MODULE,
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	.asynchronous_rx = false,
#endif
	.baud_rate = IIO_UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
	.irq_id = UART_IRQ,
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
	.device_id = UART_MODULE,
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

#if (INTERFACE_MODE == SPI_DMA)
/* Trigger GPIO IRQ parameters */
struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = TRIGGER_INT_ID,
	.platform_ops = &stm32_irq_ops
};
#else
/* Trigger GPIO IRQ parameters */
struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = GP1_PIN_NUM,
	.platform_ops = &trigger_gpio_irq_ops,
	.extra = &trigger_gpio_irq_extra_params
};
#endif

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
/* PWM GPIO init parameters */
struct no_os_gpio_init_param pwm_gpio_params = {
	.port = CNV_PORT_NUM,
	.number = CNV_PIN_NUM,
	.platform_ops = &gpio_ops,
	.extra = &pwm_gpio_extra_init_params
};
#endif

/* PWM init parameters for conversion pulses */
struct no_os_pwm_init_param pwm_init_params = {
	.id = TIMER1_ID,
	.period_ns = CONV_TRIGGER_PERIOD_NSEC(SAMPLING_RATE),
	.duty_cycle_ns = CONV_TRIGGER_PERIOD_NSEC(SAMPLING_RATE) - 360,
#if (INTERFACE_MODE == SPI_DMA)
	.polarity = NO_OS_PWM_POLARITY_LOW,
#else
	.duty_cycle_ns = CONV_TRIGGER_DUTY_CYCLE_NSEC(CONV_TRIGGER_PERIOD_NSEC(SAMPLING_RATE)),
#endif
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	.pwm_gpio = &pwm_gpio_params,
#endif
	.platform_ops = &pwm_ops,
	.extra = &pwm_extra_init_params
};

#if (INTERFACE_MODE == SPI_DMA)
struct no_os_gpio_init_param cs_pwm_gpio_params = {
	.port = STM32_SPI_CS_PORT_NUM,
	.number = SPI_CS_PIN_NUM,
	.platform_ops = &gpio_ops,
	.extra = &pwm_gpio_extra_init_params
};

/* PWM chip select init parameters */
struct no_os_pwm_init_param cs_init_params = {
	.id = TIMER2_ID,
	.period_ns = CONV_TRIGGER_PERIOD_NSEC(SAMPLING_RATE),
	.duty_cycle_ns = CONV_TRIGGER_PERIOD_NSEC(SAMPLING_RATE) - 360,
	.polarity = NO_OS_PWM_POLARITY_HIGH,
	.platform_ops = &pwm_ops,
	.extra = &cs_extra_init_params,
	.pwm_gpio = &cs_pwm_gpio_params
};

/* Init params for timer pwm triggering SPI TX */
struct no_os_pwm_init_param tx_trigger_init_params = {
	.id = TIMER8_ID,
	.period_ns = 100,
	.duty_cycle_ns = 0,
	.polarity = NO_OS_PWM_POLARITY_LOW,
	.platform_ops = &pwm_ops,
	.extra = &tx_trigger_extra_init_params,
};
#endif

/* External interrupt callback descriptor */
static struct no_os_callback_desc ext_int_callback_desc = {
	.callback = data_capture_callback,
	.ctx = NULL,
	.event = NO_OS_EVT_GPIO,
	.peripheral = NO_OS_GPIO_IRQ
};

/* I2C init parameters */
static struct no_os_i2c_init_param no_os_i2c_init_params = {
	.device_id = I2C_DEV_ID,
	.platform_ops = &i2c_ops,
	.max_speed_hz = 100000,
#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	.extra = &i2c_extra_init_params
#endif
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

/* Note: When in SPI_DMA mode, the pwm_desc here not only
 * generates the conversion pulses, but also triggers
 * the dummy spi tx dma (8 bits) transaction for fetching
 * the higher byte of 16-bit sampled data.
 * */
struct no_os_pwm_desc *pwm_desc;
struct no_os_uart_desc *uart_iio_com_desc;
struct no_os_uart_desc *uart_console_stdio_desc;
struct no_os_gpio_desc *trigger_gpio_desc;
struct no_os_irq_ctrl_desc *trigger_irq_desc;
struct no_os_eeprom_desc *eeprom_desc;

#if (INTERFACE_MODE == SPI_DMA)
/* PWM descriptor for controlling the CS pulse.
 */
struct no_os_pwm_desc *cs_desc;
struct no_os_gpio_desc* cs_gpio_desc;
struct no_os_dma_desc *ad405x_dma_desc;
struct no_os_pwm_desc* tx_trigger_desc;

/* DMA init params for SPI RX DMA */
struct no_os_dma_init_param ad405x_dma_init_param = {
	.id = 0,
	.num_ch = AD405x_DMA_NUM_CHANNELS,
	.platform_ops = &dma_ops,
	.sg_handler = (void *)receivecomplete_callback,
};

#endif

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

	/* Initialize the serial link for IIO communication */
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

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	no_os_uart_stdio(uart_console_stdio_desc);
#endif
#endif

	return 0;
}

/**
 * @brief 	Initialize the trigger GPIO and associated IRQ event
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t gpio_trigger_init(void)
{
	int32_t ret;

	/* Initialize the IRQ controller */
	ret = no_os_irq_ctrl_init(&trigger_irq_desc, &trigger_gpio_irq_params);
	if (ret) {
		return ret;
	}

#if (APP_CAPTURE_MODE == WINDOWED_DATA_CAPTURE) && (INTERFACE_MODE == SPI_INTERRUPT)
	ret = no_os_irq_register_callback(trigger_irq_desc, TRIGGER_INT_ID,
					  &ext_int_callback_desc);
	if (ret) {
		return ret;
	}

	ret = no_os_irq_trigger_level_set(trigger_irq_desc, TRIGGER_INT_ID,
					  NO_OS_IRQ_EDGE_FALLING);
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

	/* Initialize the PWM interface to generate conversion signal.
	 * If SPI_DMA is enabled, pwm interface is also used to
	 * trigger dummy spi tx dma transcation to fetch higher byte of
	 * 16-bit data
	 * */
	ret = no_os_pwm_init(&pwm_desc, &pwm_init_params);
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_disable(pwm_desc);
	if (ret) {
		return ret;
	}

#if (INTERFACE_MODE == SPI_DMA)
	ret = no_os_pwm_init(&tx_trigger_desc, &tx_trigger_init_params);
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_disable(tx_trigger_desc);
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

	ret = gpio_trigger_init();
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
