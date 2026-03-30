/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module
 *   @details This module contains the configurations needed for IIO application
********************************************************************************
 * Copyright (c) 2024, 2026 Analog Devices, Inc.
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
#include "no_os_uart.h"
#include "no_os_irq.h"
#include "no_os_gpio.h"
#include "common.h"
#include "no_os_pwm.h"
#include "ad4692_iio.h"
#include "no_os_eeprom.h"
#include "no_os_delay.h"
#include "ad4692_user_config.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* UART init parameters for IIO comm port */
struct no_os_uart_init_param uart_iio_comm_init_params = {
	.device_id = UART_MODULE,
	.asynchronous_rx = true,
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

#if defined(CONSOLE_STDIO_PORT_AVAILABLE)
/* UART init parameters for console comm port */
struct no_os_uart_init_param uart_console_stdio_init_params = {
	.device_id = UART_MODULE,
	.asynchronous_rx = true,
	.irq_id = CONSOLE_IRQ,
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
	/* Applications which uses phy COM port as primary IIO comm port,
	 * can use VCOM as console stdio port provided it is available.
	 * Else, alternative phy com port can be used for console stdio ops if available */
	.platform_ops = &vcom_ops,
	.extra = &vcom_extra_init_params
#endif
};
#endif

/* External interrupt init parameters */
struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = TRIGGER_INT_ID,
	.platform_ops = &trigger_gpio_irq_ops,
	.extra = &trigger_gpio_irq_extra_params
};

/* CNV GPIO Init params */
struct no_os_gpio_init_param cnv_gpio_init_param = {
	.port = CNV_PORT_NUM,
	.number = CNV_PIN_NUM,
	.platform_ops = &gpio_ops,
	.extra = &bsy_extra_init_params
};

/* External interrupt callback descriptor */
static struct no_os_callback_desc ext_int_callback_desc = {
	.callback = ad4692_data_capture_callback,
	.event = NO_OS_EVT_GPIO,
	.peripheral = NO_OS_GPIO_IRQ,
};

/* I2C init parameters */
static struct no_os_i2c_init_param no_os_i2c_init_params = {
	.device_id = I2C_DEV_ID,
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
	.device_id = I2C_DEV_ID,
	.platform_ops = &eeprom_24xx32a_ops,
	.extra = &eeprom_extra_init_params
};

/* SPI Burst PWM GPIO Parameters */
static struct no_os_gpio_init_param spi_burst_gpio_init_params = {
	.port = SPI_BURST_PWM_PORT,
	.number = SPI_BURST_PWM_ID,
	.platform_ops = &gpio_ops,
	.extra = &spi_burst_extra_init_params
};

/* PWM init parameters for SPI Burst PWM pulses */
struct no_os_pwm_init_param pwm_spi_burst_init = {
	.id = SPI_BURST_TIMER_ID,
	.duty_cycle_ns = 50,
	.platform_ops = &pwm_ops,
	.pwm_gpio = &spi_burst_gpio_init_params,
	.extra = &pwm_spi_burst_extra_init_params
};

/* Trigger interrupt descriptor */
struct no_os_irq_ctrl_desc *trigger_irq_desc;

/* IIO Comm Descriptor */
struct no_os_uart_desc *uart_iio_com_desc;

/* Console Descriptor */
struct no_os_uart_desc *uart_console_stdio_desc;

/* CNV GPIO descriptor */
struct no_os_gpio_desc *cnv_gpio_desc;

/* EEPROM descriptor */
struct no_os_eeprom_desc* eeprom_desc;

/* DMA Init params */
struct no_os_dma_init_param ad4692_dma_init_param = {
	.id = 0,
	.num_ch = AD4692_DMA_NUM_CHANNELS,
	.platform_ops = (struct no_os_dma_platform_ops *)&dma_ops,
	.sg_handler = (void (*)(void *))ad4692_spi_dma_rx_cplt_callback,
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

/* Chip Select GPIO init parameters */
struct no_os_gpio_init_param csb_gpio_init_param = {
	.port = SPI_CS_PORT_NUM,
	.number = SPI_CSB,
	.pull = NO_OS_PULL_NONE,
	.platform_ops = &gpio_ops,
	.extra = &csb_gpio_extra_init_params
};

/* Tx trigger descriptor */
struct no_os_pwm_desc *tx_trigger_desc;

/* SPI Burst PWM Descriptor */
struct no_os_pwm_desc *spi_burst_pwm_desc = NULL;

/* Chip Select GPIO descriptor */
struct no_os_gpio_desc* csb_gpio_desc;

/******************************************************************************/
/************************ Functions Prototypes ********************************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief 	Initialize the UART peripheral.
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

	no_os_uart_stdio(uart_console_stdio_desc);
#endif

	return 0;
}

/**
 * @brief Initialize the GPIO peripheral.
 * @return 0 in case of success, negative error code otherwise.
 */
int init_gpio(void)
{
	int ret;

	ret = no_os_gpio_get(&cnv_gpio_desc, &cnv_gpio_init_param);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(cnv_gpio_desc, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

	if (ad4692_interface_mode == SPI_DMA) {
		ret = no_os_gpio_get(&csb_gpio_desc, &csb_gpio_init_param);
		if (ret) {
			return ret;
		}

		ret = no_os_gpio_direction_output(csb_gpio_desc, NO_OS_GPIO_LOW);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Initialize the IRQ controller.
 * @return 0 in case of success, negative error code otherwise.
 * @details This function initialize the interrupts for system peripherals.
 */
int32_t init_interrupt(void)
{
	int32_t ret;

	if (ad4692_interface_mode == SPI_INTR) {
		/* Configure the IRQ parameters according to the chosen mode */
		if (ad4692_init_params.mode == AD4692_SPI_BURST) {
			trigger_gpio_irq_params.irq_ctrl_id = SPI_BURST_PWM_ID;
			stm32_gpio_irq_extra_init_params.port_nb = SPI_BURST_PWM_PORT;

		} else {
			trigger_gpio_irq_params.irq_ctrl_id = TRIGGER_INT_ID;
			stm32_gpio_irq_extra_init_params.port_nb = BSY_PORT_NUM;
		}

		/* Init interrupt controller for external interrupt */
		ret = no_os_irq_ctrl_init(&trigger_irq_desc, &trigger_gpio_irq_params);
		if (ret) {
			return ret;
		}

		if (ad4692_data_capture_mode == BURST) {
			/* The BSY pin has been tied as the interrupt source to sense the
			 * End of Conversion. The registered callback function is responsible
			 * of reading the raw samples via the SPI bus */
			ret = no_os_irq_register_callback(trigger_irq_desc,
							  TRIGGER_INT_ID,
							  &ext_int_callback_desc);
			if (ret) {
				return ret;
			}

			ret = no_os_irq_trigger_level_set(trigger_irq_desc,
							  TRIGGER_INT_ID,
							  NO_OS_IRQ_EDGE_FALLING);
			if (ret) {
				return ret;
			}
		}
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
		/* Configure the GPIO Interrupt priority to be lesser than that of the UART priority.
		 * This is done to ensure that the UART reception is not blocked by the GPIO interrupt
		 * instances */
		ret = no_os_irq_set_priority(trigger_irq_desc, TRIGGER_INT_ID,
					     GPIO_IRQ_PRIORITY);
		if (ret) {
			return ret;
		}
#endif
	}
	return 0;
}

/**
 * @brief Initialize CNV PWM
 * @return 0 in case of success, negative error code otherwise.
 */
int init_pwm(void)
{
	int ret;

	if (ad4692_init_params.mode == AD4692_SPI_BURST) {
		/* Initialize timer for SPI Burst PWM */
		stm32_tim4_init();

		ret = no_os_pwm_init(&spi_burst_pwm_desc, &pwm_spi_burst_init);
		if (ret) {
			return ret;
		}
	}

	/* Free any previously allocated conv PWM descriptor before reinit.
	 * For CNV_CLOCK/CNV_BURST modes, ad4692_init() already allocated
	 * conv_desc via ad4692_pwm_init(). */
	if (ad4692_dev->conv_desc) {
		no_os_pwm_remove(ad4692_dev->conv_desc);
		ad4692_dev->conv_desc = NULL;
	}

	ret = no_os_pwm_init(&ad4692_dev->conv_desc,
			     ad4692_init_params.conv_param);
	if (ret) {
		return ret;
	}

	/* Stop timer */
	ad4692_stop_timer();

	return 0;
}

/**
 * @brief 	Initialize Tx Trigger Timer
 * @return	0 in case of success, negative error code otherwise
 */
int32_t tx_trigger_init(void)
{
	int ret;

	if (ad4692_interface_mode == SPI_DMA) {
		/* Free previous tx_trigger_desc before reinit */
		if (tx_trigger_desc) {
			no_os_pwm_remove(tx_trigger_desc);
			tx_trigger_desc = NULL;
		}

		ret = no_os_pwm_init(&tx_trigger_desc, &tx_trigger_init_param);
		if (ret) {
			return ret;
		}
	}

	tim8_init();

	return 0;
}


/**
 * @brief Initializing system peripherals
 * @return 0 in case of success, negative error code otherwise.
 * @details This function initializes system peripherals for the application
 */
int32_t init_system(void)
{
	int ret;

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	stm32_system_init();
#endif

	ret = init_uart();
	if (ret) {
		return ret;
	}

	/* Delay to ensure board has been powered up */
	no_os_mdelay(2000);

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
