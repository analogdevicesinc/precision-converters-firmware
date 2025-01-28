/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module
 *   @details This module contains the configurations needed for IIO application
********************************************************************************
 * Copyright (c) 2022-25 Analog Devices, Inc.
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

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
/* UART init parameters for IIO comm port */
struct no_os_uart_init_param uart_iio_comm_init_params = {
	.device_id = UART_ID,
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

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
/* PWM GPIO init parameters */
static struct no_os_gpio_init_param pwm_gpio_init_params = {
	.number = PWM_GPIO_PIN,
	.port = PWM_GPIO_PORT,
	.platform_ops = &gpio_ops,
	.extra = &gpio_pwm_extra_init_params
};
#endif

/* Trigger GPIO IRQ parameters */
struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = IRQ_CTRL_ID,
	.platform_ops = &trigger_gpio_irq_ops,
	.extra = &trigger_gpio_irq_extra_params
};

/* PWM init parameters */
struct no_os_pwm_init_param pwm_init_params = {
	.id = LDAC_PWM_ID,
	.period_ns = CONV_TRIGGER_PERIOD_NSEC(MAX_SAMPLING_RATE),
#if (INTERFACE_MODE == SPI_DMA)
	.duty_cycle_ns = LDAC_PULSE_WIDTH_NS,
	.polarity = NO_OS_PWM_POLARITY_LOW,
#else
	.duty_cycle_ns = CONV_TRIGGER_DUTY_CYCLE_NSEC(MAX_SAMPLING_RATE, LDAC_PWM_DUTY_CYCLE_PERCENT),
#endif
	.platform_ops = &pwm_ops,
	.extra = &pwm_extra_init_params,
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	.pwm_gpio = &pwm_gpio_init_params
#endif
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

#if (INTERFACE_MODE == SPI_DMA)
/* Tx Trigger Init params */
struct no_os_pwm_init_param tx_trigger_init_params = {
	.id = TIMER8_ID,
	.period_ns = TX_TRIGGER_PERIOD,
	.duty_cycle_ns = TX_TRIGGER_DUTY_CYCLE_NS,
	.polarity = NO_OS_PWM_POLARITY_LOW,
	.platform_ops = &pwm_ops,
	.extra = &tx_trigger_extra_init_params,
};

/* Chip Select GPIO init parameters */
struct no_os_gpio_init_param csb_gpio_init_param = {
	.port = STM32_SPI_CS_PORT,
	.number = SPI_CSB,
	.pull = NO_OS_PULL_NONE,
	.platform_ops = &gpio_ops,
	.extra = &csb_gpio_extra_init_params
};

/* DMA descriptor */
struct no_os_dma_desc* ad7091r_dma_desc;

/* Tx Trigger PWM descriptor */
struct no_os_pwm_desc* tx_trigger_desc;

/* DMA Init params */
struct no_os_dma_init_param ad3530r_dma_init_param = {
	.id = 0,
	.num_ch = DMA_NUM_CHANNELS,
	.platform_ops = &dma_ops,
	.sg_handler = (void (*)(void *))receivecomplete_callback
};
#endif

/* PWM descriptor */
struct no_os_pwm_desc *pwm_desc;

/* UART IIO descriptor */
struct no_os_uart_desc *uart_iio_com_desc;

/* UART console descriptor */
struct no_os_uart_desc *uart_console_stdio_desc;

/* Trigger GPIO descriptor */
struct no_os_gpio_desc *trigger_gpio_desc;

/* Trigger GPIO interrupt descriptor */
struct no_os_irq_ctrl_desc *trigger_irq_desc;

/* EEPROM descriptor */
struct no_os_eeprom_desc *eeprom_desc;

/* Chip Select GPIO descriptor */
struct no_os_gpio_desc* csb_gpio_desc;

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
 * @brief 	Initialize the GPIOs
 * @return	0 in case of success, negative error code otherwise
 * @details	This function initialize the GPIOs used by application
 */
static int32_t init_gpio(void)
{
	int32_t ret;

#if (INTERFACE_MODE == SPI_DMA)
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
 * @brief 	Initialize the trigger GPIO and associated IRQ event
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t gpio_trigger_Init(void)
{
	int32_t ret;

	/* Initialize the IRQ controller */
	ret = no_os_irq_ctrl_init(&trigger_irq_desc, &trigger_gpio_irq_params);
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

	return 0;
}

/**
 * @brief 	Initialize the PWM interface.
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t init_pwm(void)
{
	int32_t ret;

	/* Initialize the PWM interface to generate PWM signal
	 * for recurring function calls */
	ret = no_os_pwm_init(&pwm_desc, &pwm_init_params);
	if (ret) {
		return ret;
	}

	/* Disable the PWM interface.*/
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

	ret = init_gpio();
	if (ret) {
		return ret;
	}

	ret = gpio_trigger_Init();
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
