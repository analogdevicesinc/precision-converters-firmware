/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module
 *   @details This module contains the configurations needed for IIO application
********************************************************************************
 * Copyright (c) 2023-2024 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "app_config.h"
#include "ad355xr_user_config.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* UART init parameters for IIO comm port */
struct no_os_uart_init_param uart_iio_comm_init_params = {
	.device_id = 0,
	.asynchronous_rx = false,
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
	.device_id = 0,
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

/* PWM init parameters for ldac */
static struct no_os_pwm_init_param ldac_pwm_init_params = {
	.id = LDAC_PWM_ID,
	.period_ns = CONV_PERIOD_NSEC(MAX_SAMPLING_RATE),
	.duty_cycle_ns = CONV_DUTY_CYCLE_NSEC(MAX_SAMPLING_RATE, LDAC_PWM_DUTY_CYCLE),
	.platform_ops = &pwm_ops,
	.extra = &ldac_pwm_extra_init_params,
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	.pwm_gpio = &gpio_ldac_init
#endif
};

#if (INTERFACE_MODE == SPI_DMA)
/* gpio pin for timer output which will stop spi dma transfer */
struct no_os_gpio_init_param spi_dma_tx_stop_pwm_gpio_init = {
	.number = SPI_DMA_TX_STOP_PWM_GPIO_PIN,
	.port = SPI_DMA_TX_STOP_PWM_GPIO_PORT,
	.platform_ops = &gpio_ops,
	.extra = &spi_dma_tx_stop_pwm_gpio_extra_init_params
};

/* PWM init parameters for timer which will stop spi dma transfer */
static struct no_os_pwm_init_param spi_dma_tx_stop_pwm_init_params = {
	.id = SPI_DMA_TX_STOP_PWM_ID,
	.period_ns = CONV_PERIOD_NSEC(MAX_SAMPLING_RATE),
	.duty_cycle_ns = CONV_DUTY_CYCLE_NSEC(MAX_SAMPLING_RATE, LDAC_PWM_DUTY_CYCLE),
	.platform_ops = &pwm_ops,
	.extra = &spi_dma_tx_stop_pwm_extra_init_params,
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	.pwm_gpio = &spi_dma_tx_stop_pwm_gpio_init
#endif
};
#endif

/* External interrupt init parameters used for ldac */
static struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = IRQ_CTRL_ID,
	.platform_ops = &irq_platform_ops,
	.extra = &ext_int_extra_init_params
};

/* External interrupt callback descriptor used for ldac */
static struct no_os_callback_desc ext_int_callback_desc = {
	.callback = ldac_pos_edge_detect_callback,
	.event = NO_OS_EVT_GPIO,
	.peripheral = NO_OS_GPIO_IRQ,
	.handle = trigger_gpio_handle
};

/* UART descriptor for IIO communication */
struct no_os_uart_desc *uart_iio_com_desc;

/* UART descriptor for console communication */
struct no_os_uart_desc *uart_console_stdio_desc;

/* PWM descriptor for ldac */
struct no_os_pwm_desc *ldac_pwm_desc;

/* PWM descriptor to stops SPI DMA transfer */
struct no_os_pwm_desc* spi_dma_tx_stop_pwm_desc;

/* External interrupt descriptor */
struct no_os_irq_ctrl_desc *trigger_irq_desc;

/******************************************************************************/
/************************ Functions Prototypes ********************************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/
/**
 * @brief 	Initialize the PWM trigger contoller for ldac
 * @return	0 in case of success, negative error code otherwise
 */
int32_t init_ldac_pwm_trigger(void)
{
	int32_t ret;

	ret = no_os_pwm_init(&ldac_pwm_desc, &ldac_pwm_init_params);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief 	Initialize the timer in pwm mode which used to stops spi dma transfer
 * @return	0 in case of success, negative error code otherwise
 */
int32_t init_spi_dma_tx_stop_pwm(void)
{
	int32_t ret;

#if (INTERFACE_MODE == SPI_DMA)
	ret = no_os_pwm_init(&spi_dma_tx_stop_pwm_desc,
			     &spi_dma_tx_stop_pwm_init_params);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief Interrupt Service Routine to monitor ldac positive edge.
 * @param ctx[in] - Callback context (unused)
 * @return None
 * @note this function is unused. we are handling ldac interrupt
 * directly from EXTI IRQHandler.
 */
void ldac_pos_edge_detect_callback(void* ctx)
{

}

/**
 * @brief Initialize the IRQ contoller
 * @return 0 in case of success, negative error code otherwise
 */
int32_t init_interrupt(void)
{
	int32_t ret;

	/* Initialize interrupt controller for external interrupt tied to ldac pin */
	ret = no_os_irq_ctrl_init(&trigger_irq_desc, &trigger_gpio_irq_params);
	if (ret) {
		return ret;
	}

#if (INTERFACE_MODE == SPI_DMA)
	/* register the callback for gpio interrupt tied to ldac pin */
	ret = no_os_irq_register_callback(trigger_irq_desc, IRQ_CTRL_ID,
					  &ext_int_callback_desc);
	if (ret) {
		return ret;
	}

	ret = no_os_irq_trigger_level_set(trigger_irq_desc, IRQ_CTRL_ID,
					  NO_OS_IRQ_EDGE_RISING);
	if (ret) {
		return ret;
	}
#endif

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
#endif

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
#if (INTERFACE_MODE == SPI_DMA)
	ret = init_spi_dma_tx_stop_pwm();
	if (ret) {
		return ret;
	}
#endif
#endif

	ret = init_uart();
	if (ret) {
		return ret;
	}

	ret = init_interrupt();
	if (ret) {
		return ret;
	}

	/* Lowering the LDAC GPIO interrupt priority than uart because some
	 * charecters of iio command are missing when both LDAC GPIO interrupt
	 * and uart interrupt occurs at same time.*/
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	ret = no_os_irq_set_priority(trigger_irq_desc, IRQ_CTRL_ID, LDAC_GPIO_PRIORITY);
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
