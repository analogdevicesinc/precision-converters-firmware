/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module (platform-agnostic)
 *   @details This module performs the system configurations
********************************************************************************
 * Copyright (c) 2024 Analog Devices, Inc.
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
#include "no_os_util.h"
#include "common.h"
#include "no_os_error.h"
#include "no_os_uart.h"
#include "no_os_i2c.h"
#include "no_os_eeprom.h"
#include "no_os_irq.h"
#include "no_os_pwm.h"
#include "ad7091r_support.h"

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

/* External interrupt init parameters */
struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = BSY_PIN,
	.platform_ops = &trigger_gpio_irq_ops,
	.extra = &trigger_gpio_irq_extra_params
};

/* External interrupt callback descriptor */
static struct no_os_callback_desc ext_int_callback_desc = {
	.callback = burst_capture_callback,
	.ctx = NULL,
	.event = NO_OS_EVT_GPIO,
	.peripheral = NO_OS_GPIO_IRQ
};

/* I2C init parameters */
static struct no_os_i2c_init_param no_os_i2c_init_params = {
	.device_id = I2C_DEVICE_ID,
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

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
/* PWM GPIO init parameters */
struct no_os_gpio_init_param pwm_gpio_params = {
	.port = CNV_PORT,
	.number = CNV_PIN,
	.platform_ops = &gpio_ops,
	.extra = &pwm_gpio_extra_init_params
};
#endif

/* PWM init parameters for conversion pulses */
struct no_os_pwm_init_param pwm_init_params = {
	.id = CNV_PWM_ID,
	.period_ns = CONV_TRIGGER_PERIOD_NSEC(MAX_SAMPLING_RATE),
	.duty_cycle_ns = CONV_TRIGGER_PERIOD_NSEC(MAX_SAMPLING_RATE) - PWM_DUTY_CYCLE_NSEC,
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	.polarity = NO_OS_PWM_POLARITY_LOW,
	.pwm_gpio = &pwm_gpio_params,
#endif
	.platform_ops = &pwm_ops,
	.extra = &pwm_extra_init_params,
};

#if (INTERFACE_MODE == SPI_DMA)
/* PWM descriptor for controlling the CS pulse. */
struct no_os_pwm_desc* cs_desc;

/* CS GPIO Init params */
struct no_os_gpio_init_param cs_pwm_gpio_params = {
	.port = SPI_CS_PORT_NUM,
	.number = SPI_CS_PIN_NUM,
	.platform_ops = &gpio_ops,
	.extra = &cs_pwm_gpio_extra_init_params
};

/* PWM chip select init paramaters */
struct no_os_pwm_init_param cs_init_params = {
	.id = CS_TIMER_ID,
	.period_ns = CONV_TRIGGER_PERIOD_NSEC(MAX_SAMPLING_RATE),
	.duty_cycle_ns = CHIP_SELECT_DUTY_CYCLE_NS,
	.polarity = NO_OS_PWM_POLARITY_HIGH,
	.platform_ops = &pwm_ops,
	.extra = &cs_extra_init_params,
	.pwm_gpio = &cs_pwm_gpio_params
};

/* Tx Trigger Init params */
struct no_os_pwm_init_param tx_trigger_init_params = {
	.id = TIMER8_ID,
	.period_ns = TX_TRIGGER_PERIOD,
	.duty_cycle_ns = TX_TRIGGER_DUTY_CYCLE_NS,
	.polarity = NO_OS_PWM_POLARITY_LOW,
	.platform_ops = &pwm_ops,
	.extra = &tx_trigger_extra_init_params,
};

/* DMA descriptor */
struct no_os_dma_desc* ad7091r_dma_desc;

/* Tx Trigger PWM descriptor */
struct no_os_pwm_desc* tx_trigger_desc;

/* DMA Init params */
struct no_os_dma_init_param ad7091r_dma_init_param = {
	.id = 0,
	.num_ch = AD7091R_DMA_NUM_CHANNELS,
	.platform_ops = &dma_ops,
	.sg_handler = receivecomplete_callback,
};
#endif

/* UART IIO Descriptor */
struct no_os_uart_desc *uart_iio_com_desc;

/* UART console descriptor */
struct no_os_uart_desc *uart_console_stdio_desc;

/* PWM descriptor */
struct no_os_pwm_desc *pwm_desc;

/* EEPROM descriptor */
struct no_os_eeprom_desc *eeprom_desc;

/* Trigger IRQ descriptor */
struct no_os_irq_ctrl_desc *trigger_irq_desc;

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
static int init_uart(void)
{
	int ret;

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

	/* Set up the UART for standard I/O operations */
	no_os_uart_stdio(uart_console_stdio_desc);
#endif

	return 0;
}

/**
 * @brief 	Initialize the IRQ controller.
 * @return	0 in case of success, negative error code otherwise.
 * @details	This function initialize the interrupts for system peripherals.
 */
static int init_interrupt(void)
{
	int ret;

	/* Init interrupt controller for external interrupt */
	ret = no_os_irq_ctrl_init(&trigger_irq_desc, &trigger_gpio_irq_params);
	if (ret) {
		return ret;
	}

#if (DATA_CAPTURE_MODE == BURST_DATA_CAPTURE)
	/* The BSY pin has been tied as the interrupt source to sense the
	 * End of Conversion. The registered callback function is responsible
	 * for monitoring the end of conversion during data capture */
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
#endif

	return 0;
}

/**
 * @brief 	Initialize the PWM trigger controller.
 * @return	0 in case of success, negative error code otherwise.
 */
int init_pwm_trigger(void)
{
	int ret;
	uint32_t pwm_period_ns;
	uint32_t dcycle_ns;

	/* Initialize the PWM interface to generate conversion signal.*/
	ret = no_os_pwm_init(&pwm_desc, &pwm_init_params);
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_disable(pwm_desc);
	if (ret) {
		return ret;
	}

#if (INTERFACE_MODE == SPI_DMA)
	/* Initialize the TX Triggger PWM interface. */
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
 * @brief 	Initialize the system peripherals
 * @return	0 in case of success, negative error code otherwise
 */
int init_system(void)
{
	int32_t ret;

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	stm32_system_init();
#endif

	ret = init_uart();
	if (ret) {
		return ret;
	}

#if (INTERFACE_MODE == SPI_INTERRUPT)
	ret = init_interrupt();
	if (ret) {
		return ret;
	}
#endif

	/* Lowering the BSY GPIO interrupt priority than uart because some
	* charecters of iio command are missing when both BSY GPIO interrupt
	* and uart interrupt occurs at same time.*/
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	configure_intr_priority();
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