/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module
 *   @details This module contains the configurations needed for IIO application
********************************************************************************
 * Copyright (c) 2021-25 Analog Devices, Inc.
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
#include "no_os_irq.h"
#include "no_os_gpio.h"
#include "no_os_i2c.h"
#include "no_os_eeprom.h"
#include "no_os_tdm.h"
#include "no_os_pwm.h"
#include "no_os_delay.h"
#if (ACTIVE_IIO_CLIENT == IIO_CLIENT_LOCAL)
#include "pl_gui_events.h"
#include "pl_gui_views.h"
#endif

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* UART init parameters */
struct no_os_uart_init_param uart_init_params = {
	.device_id = NULL,
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

/* LDAC GPO init parameters.
 * NOTE: DIG_AUX_2 is used only as SYNC in case of AD4190 */
struct no_os_gpio_init_param gpio_init_ldac_n = {
	.number = DIG_AUX_2,
	.port = DIG_AUX_2_PORT,
	.platform_ops = &gpio_ops,
	.extra = &gpio_dig_aux2_extra_init_params
};

/* RDY GPO init parameters */
struct no_os_gpio_init_param gpio_init_rdy = {
	.number = DIG_AUX_1,
	.port = DIG_AUX_1_PORT,
	.platform_ops = &gpio_ops,
	.extra = &gpio_dig_aux1_extra_init_params
};

/* SYNC GPO init parameters */
struct no_os_gpio_init_param gpio_init_sync_inb = {
	.number = SYNC_INB,
	.port = SYNC_INB_PORT,
	.platform_ops = &gpio_ops,
	.extra = &gpio_sync_inb_extra_init_params
};

#if (INTERFACE_MODE == SPI_INTERRUPT_MODE)
/* Trigger GPIO init parameters */
struct no_os_gpio_init_param trigger_gpio_param = {
	.port = TRIGGER_GPIO_PORT,
	.number = TRIGGER_GPIO_PIN,
	.pull = NO_OS_PULL_NONE,
	.platform_ops = &gpio_ops,
	.extra = &trigger_gpio_extra_init_params
};
#endif

#if (INTERFACE_MODE != SPI_DMA_MODE)
/* Trigger GPIO IRQ parameters */
struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = TRIGGER_GPIO_IRQ_CTRL_ID,
	.platform_ops = &trigger_gpio_irq_ops,
	.extra = &trigger_gpio_irq_extra_params
};
#endif

#if (INTERFACE_MODE == TDM_MODE)
struct no_os_tdm_init_param tdm_init_param = {
	.mode = NO_OS_TDM_SLAVE_RX,
	.data_size = TDM_DATA_SIZE,
	.data_offset = 0,
	.data_lsb_first = false,
	.slots_per_frame = TDM_SLOTS_PER_FRAME,
	.fs_active_low = true,
	.fs_active_length = TDM_FS_ACTIVE_LENGTH,
	.fs_lastbit = false,
	.rising_edge_sampling = false,
	.irq_id = DMA_IRQ_ID,
	.rx_complete_callback = ad4170_dma_rx_cplt,
	.active_slots = (1 << TDM_SLOTS_PER_FRAME) - 1,
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	.rx_half_complete_callback = ad4170_dma_rx_half_cplt,
#endif
	.extra = &tdm_extra_init_params,
	.platform_ops = &tdm_ops
};

/* TDM Descriptor */
struct no_os_tdm_desc *ad4170_tdm_desc;
#endif // INTERFACE_MODE

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
	.device_id = I2C_DEVICE_ID,
	.platform_ops = &eeprom_24xx32a_ops,
	.extra = &eeprom_extra_init_params
};

#if (INTERFACE_MODE == SPI_DMA_MODE)
/* DMA Init params */
struct no_os_dma_init_param ad4170_dma_init_param = {
	.id = 0,
	.num_ch = AD469x_DMA_NUM_CHANNELS,
	.platform_ops = &dma_ops,
	.sg_handler = ad4170_spi_dma_rx_cplt_callback,
};

/* Tx Trigger Init params */
struct no_os_pwm_init_param tx_trigger_init_param = {
	.id = TX_TRIGGER_TIMER_ID,
	.period_ns = TX_TRIGGER_PERIOD,
	.duty_cycle_ns = TX_TRIGGER_DUTY_RATIO,
	.polarity = NO_OS_PWM_POLARITY_HIGH,
	.platform_ops = &pwm_ops,
	.extra = &tx_trigger_extra_init_params,
};

/* Tx trigger descriptor */
struct no_os_pwm_desc *tx_trigger_desc;
#endif

#if (INTERFACE_MODE == TDM_MODE) || (INTERFACE_MODE == SPI_DMA_MODE)
/* Chip Select GPIO init parameters */
struct no_os_gpio_init_param csb_gpio_init_param = {
	.port = CSB_GPIO_PORT,
	.number = SPI_CSB,
	.pull = NO_OS_PULL_NONE,
	.platform_ops = &gpio_ops,
	.extra = &csb_gpio_extra_init_params
};
#endif

/* LED GPO descriptor */
struct no_os_gpio_desc *led_gpio_desc = NULL;

/* UART descriptor */
struct no_os_uart_desc *uart_desc;

/* Trigger GPIO descriptor */
struct no_os_gpio_desc *trigger_gpio_desc;

/* Trigger GPIO interrupt descriptor */
struct no_os_irq_ctrl_desc *trigger_irq_desc;

/* Ticker interrupt descriptor */
struct no_os_irq_ctrl_desc *ticker_int_desc;

/* EEPROM descriptor */
struct no_os_eeprom_desc *eeprom_desc;

/* Chip Select GPIO descriptor */
struct no_os_gpio_desc* csb_gpio_desc;

/******************************************************************************/
/************************ Functions Prototypes ********************************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief 	Initialize the GPIOs
 * @return	0 in case of success, negative error code otherwise
 * @details	This function initialize the GPIOs used by application
 */
static int32_t init_gpio(void)
{
	int32_t ret;

#if (INTERFACE_MODE == TDM_MODE) || (INTERFACE_MODE == SPI_DMA_MODE)
	/* Initialize the Chip select pin as a GPIO */
	ret = no_os_gpio_get_optional(&csb_gpio_desc, &csb_gpio_init_param);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(csb_gpio_desc, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}
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

#if (INTERFACE_MODE != SPI_DMA_MODE)
#if (INTERFACE_MODE == SPI_INTERRUPT_MODE)
	/* Configure GPIO as input */
	ret = no_os_gpio_get(&trigger_gpio_desc, &trigger_gpio_param);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_input(trigger_gpio_desc);
	if (ret) {
		return ret;
	}
#endif // INTERFACE_MODE

	/* Init interrupt controller for external interrupt */
	ret = no_os_irq_ctrl_init(&trigger_irq_desc, &trigger_gpio_irq_params);
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
	return no_os_uart_init(&uart_desc, &uart_init_params);
}

/**
 * @brief 	Initialize the IRQ contoller
 * @return	0 in case of success, negative error code otherwise
 * @details	This function initialize the interrupts for system peripherals
 */
static int32_t init_interrupt(void)
{
	int32_t ret;

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	/* Init interrupt controller for Ticker interrupt */
	ret = no_os_irq_ctrl_init(&ticker_int_desc, &ticker_int_init_params);
	if (ret) {
		return ret;
	}

	/* Register a callback function for Ticker interrupt */
	ret = no_os_irq_register_callback(ticker_int_desc,
					  TICKER_ID,
					  &ticker_int_callback_desc);
	if (ret) {
		return ret;
	}

	/* Enable Ticker interrupt */
	ret = no_os_irq_enable(ticker_int_desc, TICKER_ID);
	if (ret) {
		return ret;
	}
#endif // ACTIVE_PLATFORM

	return 0;
}

/**
 * @brief 	Initialize the TDM peripheral
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t init_tdm(void)
{
#if (INTERFACE_MODE == TDM_MODE)
	if (no_os_tdm_init(&ad4170_tdm_desc, &tdm_init_param) != 0) {
		return -EINVAL;
	}
#endif // INTERFACE_MODE

	return 0;
}

/**
 * @brief 	Initialize Tx Trigger Timer
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t tx_trigger_init(void)
{
	int ret;

#if (INTERFACE_MODE == SPI_DMA_MODE)
	ret = no_os_pwm_init(&tx_trigger_desc, &tx_trigger_init_param);
	if (ret) {
		return ret;
	}

	tim8_init(tx_trigger_desc);
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
#endif

	ret = init_gpio();
	if (ret) {
		return ret;
	}

	ret = init_uart();
	if (ret) {
		return ret;
	}

	/* Delay to ensure that the peripherals are initialized */
	no_os_mdelay(5000);

	ret = gpio_trigger_init();
	if (ret) {
		return ret;
	}

	ret = init_interrupt();
	if (ret) {
		return ret;
	}

#if (INTERFACE_MODE == TDM_MODE)
	ret = init_tdm();
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

	ret = tx_trigger_init();
	if (ret) {
		return ret;
	}

	return 0;
}
