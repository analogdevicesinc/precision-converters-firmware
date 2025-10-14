/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module
 *   @details This module contains the configurations needed for IIO application
********************************************************************************
 * Copyright (c) 2022-2025 Analog Devices, Inc.
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
#include "no_os_dma.h"

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
	.asynchronous_rx = true,
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
	.asynchronous_rx = true,
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

/* Trigger GPIO IRQ parameters */
struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = GP1_PIN_NUM,
	.platform_ops = &trigger_gpio_irq_ops,
	.extra = &trigger_gpio_irq_extra_params
};

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
struct no_os_pwm_init_param spi_dma_pwm_init_params = {
	.id = CNV_TIMER_ID,
	.period_ns = PWM_FREQUENCY_TO_PERIOD(SAMPLING_RATE_SPI_DMA),
	.duty_cycle_ns = PWM_FREQUENCY_TO_PERIOD(SAMPLING_RATE_SPI_DMA) - 360,
	.polarity = NO_OS_PWM_POLARITY_LOW,
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	.pwm_gpio = &pwm_gpio_params,
#endif
	.platform_ops = &pwm_ops,
	.extra = &pwm_extra_init_params
};

/* PWM init parameters for conversion pulses */
struct no_os_pwm_init_param spi_intr_pwm_init_params = {
	.id = CNV_TIMER_ID,
	.period_ns = PWM_FREQUENCY_TO_PERIOD(SAMPLING_RATE_SPI_INTR),
	.duty_cycle_ns = CONV_TRIGGER_DUTY_CYCLE_NSEC(PWM_FREQUENCY_TO_PERIOD(SAMPLING_RATE_SPI_INTR)),
	.polarity = NO_OS_PWM_POLARITY_HIGH,
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	.pwm_gpio = &pwm_gpio_params,
#endif
	.platform_ops = &pwm_ops,
	.extra = &pwm_extra_init_params
};

/* PWM init parameters for conversion pulses */
struct no_os_pwm_init_param i3c_dma_pwm_init_params = {
	.id = CNV_TIMER_ID,
	.period_ns = PWM_FREQUENCY_TO_PERIOD(SAMPLING_RATE_I3C_DMA),
	// 50ns High time.
	.duty_cycle_ns = PWM_FREQUENCY_TO_PERIOD(SAMPLING_RATE_I3C_DMA) - 50,
	.polarity = NO_OS_PWM_POLARITY_LOW,
	.platform_ops = &pwm_ops,
	.extra = &pwm_extra_init_params
};

/* PWM init parameters for conversion pulses */
struct no_os_pwm_init_param i3c_intr_pwm_init_params = {
	.id = CNV_TIMER_ID,
	.period_ns = PWM_FREQUENCY_TO_PERIOD(SAMPLING_RATE_I3C_INTR),
	// 50ns High time.
	.duty_cycle_ns = PWM_FREQUENCY_TO_PERIOD(SAMPLING_RATE_I3C_INTR) - 50,
	/*
	 * The callback is not defined since it is being created in init_pwm_irq().
	 * This is done to overcome handle mismatch in irq callback function in stm_irq.c.
	 * The #irq_id is provided to trigger a pwm in interrupt mode.
	 */
	.irq_id = CNV_PWM_TIMER_IRQ_ID,
	.platform_ops = &pwm_ops,
	.extra = &pwm_extra_init_params
};

#ifdef SPI_SUPPORT_AVAILABLE
struct no_os_gpio_init_param cs_pwm_gpio_params = {
	.port = SPI_CS_PORT_NUM,
	.number = SPI_CS_PIN_NUM,
	.platform_ops = &gpio_ops,
	.extra = &pwm_gpio_extra_init_params
};

/* PWM chip select init parameters */
struct no_os_pwm_init_param cs_init_params = {
	.id = CS_TIMER_ID,
	.period_ns = PWM_FREQUENCY_TO_PERIOD(SAMPLING_RATE_SPI_DMA),
	.duty_cycle_ns = PWM_FREQUENCY_TO_PERIOD(SAMPLING_RATE_SPI_DMA) - 360,
	.polarity = NO_OS_PWM_POLARITY_HIGH,
	.platform_ops = &pwm_ops,
	.extra = &cs_extra_init_params,
	.pwm_gpio = &cs_pwm_gpio_params
};

/* Init params for timer pwm triggering SPI TX */
struct no_os_pwm_init_param tx_trigger_init_params = {
	.id = TX_TRIGGER_TIMER_ID,
	.period_ns = 100,
	.duty_cycle_ns = 0,
	.polarity = NO_OS_PWM_POLARITY_LOW,
	.platform_ops = &pwm_ops,
	.extra = &tx_trigger_extra_init_params,
};
#endif

/* Note: Below variables are defined in the init_system_post_verification function */
/* Variable to hold the value to be configure Control Register of I3C */
uint32_t i3c_cr;

/* DMA transfer structure for I3C Control Register */
struct no_os_dma_xfer_desc i3c_cr_dma_xfer = {
	/** Source address for the data */
	.src = (uint8_t *)&i3c_cr,
	/** Destination address for the data */
	.dst = NULL, // Has to be defined after HAL I3C Initialization
	/** Transfer length in bytes */
	.length = sizeof(uint32_t),
	/** Transfer direction */
	.xfer_type = MEM_TO_MEM,
	.xfer_complete_cb = NULL,
	.xfer_complete_ctx = NULL,
	.irq_priority = 0,
	.periph = NO_OS_DMA_IRQ,
	/** User or platform defined data */
	.extra = NULL
};

#if (APP_CAPTURE_MODE == WINDOWED_DATA_CAPTURE)
/* External interrupt callback descriptor */
static struct no_os_callback_desc ext_int_callback_desc = {
	.callback = data_capture_callback,
	.ctx = NULL,
	.event = NO_OS_EVT_GPIO,
	.peripheral = NO_OS_GPIO_IRQ
};
#endif

/* I2C init parameters */
static struct no_os_i2c_init_param no_os_i2c_init_params = {
	.device_id = I2C_DEV_ID,
	.platform_ops = &i2c_ops,
	.max_speed_hz = I2C_MAX_SPEED_HZ,
#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	.extra = &i2c_extra_init_params
#else
	.extra = I2C_EXTRA_PARAM_PTR
#endif
};

/* EEPROM init parameters */
static struct eeprom_24xx32a_init_param eeprom_extra_init_params = {
	.i2c_init = &no_os_i2c_init_params
};

/* EEPROM init parameters */
struct no_os_eeprom_init_param eeprom_init_params = {
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
struct no_os_irq_ctrl_desc *pwm_irq_desc;
struct no_os_uart_desc *uart_iio_com_desc;
struct no_os_uart_desc *uart_console_stdio_desc;
struct no_os_gpio_desc *trigger_gpio_desc;
struct no_os_irq_ctrl_desc *trigger_irq_desc;

/* PWM descriptor for controlling the CS pulse.
 */
struct no_os_gpio_desc* cs_gpio_desc;
struct no_os_dma_desc *ad405x_dma_desc;
struct no_os_pwm_desc* tx_trigger_desc;
struct no_os_pwm_desc* cs_pwm_desc;

/* DMA init params for SPI RX DMA */
struct no_os_dma_init_param ad405x_dma_init_param = {
	.id = 0,
	.num_ch = AD405x_DMA_NUM_CHANNELS,
	.platform_ops = (struct no_os_dma_platform_ops *) &dma_ops,
	.sg_handler = (void *)receivecomplete_callback,
};

/******************************************************************************/
/************************ Functions Prototypes ********************************/
/******************************************************************************/
int32_t init_pwm_irq(void);

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
int32_t gpio_trigger_init(void)
{
	int32_t ret;

	/* Initialize the IRQ controller */
	ret = no_os_irq_ctrl_init(&trigger_irq_desc, &trigger_gpio_irq_params);
	if (ret) {
		return ret;
	}

#if (APP_CAPTURE_MODE == WINDOWED_DATA_CAPTURE)
	ret = no_os_irq_register_callback(trigger_irq_desc,
					  TRIGGER_INT_ID_SPI_INTR,
					  &ext_int_callback_desc);
	if (ret) {
		return ret;
	}

	ret = no_os_irq_trigger_level_set(trigger_irq_desc,
					  TRIGGER_INT_ID_SPI_INTR,
					  NO_OS_IRQ_EDGE_FALLING);
	if (ret) {
		return ret;
	}

	ret = no_os_irq_disable(trigger_irq_desc, TRIGGER_INT_ID_SPI_INTR);
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
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	stm32_config_cnv_prescalar();
#endif
#ifdef SPI_SUPPORT_AVAILABLE
	/* Initialize the PWM interface to generate conversion signal.
	 * If SPI_DMA is enabled, pwm interface is also used to
	 * trigger dummy spi tx dma transcation to fetch higher byte of
	 * 16-bit data
	 * */
	if (ad405x_interface_mode == SPI_DMA) {
		ret = no_os_pwm_init(&tx_trigger_desc, &tx_trigger_init_params);
		if (ret) {
			return ret;
		}

		ret = no_os_pwm_disable(tx_trigger_desc);
		if (ret) {
			return ret;
		}

		ret = no_os_pwm_init(&cs_pwm_desc, &cs_init_params);
		if (ret) {
			return ret;
		}

		ret = no_os_pwm_disable(cs_pwm_desc);
		if (ret) {
			return ret;
		}

		stm32_cs_output_gpio_config(true);

		ret = no_os_pwm_init(&pwm_desc, &spi_dma_pwm_init_params);
		if (ret) {
			return ret;
		}

	} else if (ad405x_interface_mode == SPI_INTR) {
		ret = no_os_pwm_init(&pwm_desc, &spi_intr_pwm_init_params);
		if (ret) {
			return ret;
		}
	}
#endif

#ifdef I3C_SUPPORT_AVAILABLE

	if (ad405x_interface_mode == I3C_DMA) {
		ret = no_os_pwm_init(&pwm_desc, &i3c_dma_pwm_init_params);
		if (ret) {
			return ret;
		}
	} else if (ad405x_interface_mode == I3C_INTR) {
		ret = no_os_pwm_init(&pwm_desc, &i3c_intr_pwm_init_params);
		if (ret) {
			return ret;
		}

		ret = init_pwm_irq();
		if (ret) {
			return ret;
		}
	}

#endif

	ret = no_os_pwm_disable(pwm_desc);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief 	DeInitialize the PWM interface.
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t deinit_pwm(void)
{
	if (tx_trigger_desc) {
		no_os_pwm_disable(tx_trigger_desc);

		no_os_pwm_remove(tx_trigger_desc);

		tx_trigger_desc = NULL;
	}

	if (cs_pwm_desc) {
		no_os_pwm_disable(cs_pwm_desc);

		no_os_pwm_remove(cs_pwm_desc);

		cs_pwm_desc = NULL;
	}

	if (pwm_desc) {
		no_os_pwm_disable(pwm_desc);

		no_os_pwm_remove(pwm_desc);

		pwm_desc = NULL;
	}

	return 0;
}

/**
 * @brief 	Initialize the PWM Completion Interrupt.
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t init_pwm_irq(void)
{
	int32_t ret;

	/* Return if the IRQ handler is already defined */
	if (pwm_irq_desc) {
		return 0;
	}

	struct no_os_irq_init_param pwm_irq_init_param = {
		.platform_ops = &stm32_irq_ops
	};
	struct no_os_callback_desc pwm_cb_desc = {
		.callback = &data_capture_callback,
		.ctx = NULL,
		.event = NO_OS_EVT_LPTIM_PWM_PULSE_FINISHED,
		.peripheral = NO_OS_LPTIM_IRQ,
		.handle = CNV_TIMER_HANDLE
	};

	ret = no_os_irq_ctrl_init(&pwm_irq_desc, &pwm_irq_init_param);
	if (ret) {
		goto error;
	}

	ret = no_os_irq_register_callback(pwm_irq_desc, CNV_PWM_TIMER_IRQ_ID,
					  &pwm_cb_desc);
	if (ret) {
		goto error_register;
	}

	ret = no_os_irq_set_priority(pwm_irq_desc, CNV_PWM_TIMER_IRQ_ID, 1);
	if (ret < 0)
		goto error_priority;

	ret = no_os_irq_enable(pwm_irq_desc, CNV_PWM_TIMER_IRQ_ID);
	if (ret) {
		goto error_enable;
	}

	return 0;
error_enable:
error_priority:
	no_os_irq_unregister_callback(pwm_irq_desc, CNV_PWM_TIMER_IRQ_ID,
				      &pwm_cb_desc);
error_register:
	no_os_irq_ctrl_remove(pwm_irq_desc);
error:
	return ret;
}

#ifdef I3C_SUPPORT_AVAILABLE
/**
 * @brief 	Initialize the DMA.
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t init_dma(void)
{
	int32_t ret;

	/* Return if the DMA descriptor is already initialized */
	if (ad405x_dma_desc)
		return 0;

	/* Initialize the DMA */
	ret = no_os_dma_init(&ad405x_dma_desc, &ad405x_dma_init_param);
	if (ret)
		return ret;

	ad405x_dma_desc->channels[0].id = (uint32_t)i3c_dma_txdma_channel.hdma;
	ad405x_dma_desc->channels[0].extra = &i3c_dma_txdma_channel;

	ad405x_dma_desc->channels[1].id = (uint32_t)i3c_dma_rxdma_channel.hdma;
	ad405x_dma_desc->channels[1].extra = &i3c_dma_rxdma_channel;
	ad405x_dma_desc->channels[1].irq_num = Rx_DMA_IRQ_ID;

	return 0;
}
#endif

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

#ifdef SPI_SUPPORT_AVAILABLE
	ret = gpio_trigger_init();
	if (ret) {
		return ret;
	}
#endif

#if defined(SDRAM_SUPPORT_AVAILABLE)
	ret = sdram_init();
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
int32_t init_system_post_verification(void)
{
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#ifdef I3C_SUPPORT_AVAILABLE
	int32_t ret;
	if (ad405x_interface_mode == I3C_DMA) {
		ret = init_dma();
		if (ret)
			return ret;
	}
#endif
	stm32_system_init_post_verification();

#endif

	return 0;
}
