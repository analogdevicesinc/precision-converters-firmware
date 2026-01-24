/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module
 *   @details This module contains the configurations needed for IIO application
********************************************************************************
 * Copyright (c) 2024-2026 Analog Devices, Inc.
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
#include "ad5706r_iio.h"
#include "ad5706r_user_config.h"
#include "no_os_delay.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
/* UART init parameters for IIO comm port */
struct no_os_uart_init_param uart_iio_comm_init_params = {
	.device_id = UART_MODULE,
	.baud_rate = IIO_UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
	.asynchronous_rx = true,
	.irq_id = UART_IRQ,
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
#endif

/* Address AD0 GPIO parameters */
struct no_os_gpio_init_param gpio_ad0_params = {
	.port = AD0_PORT,
	.number = GPIO_AD0,
	.platform_ops = &gpio_ops,
	.extra = &gpio_ad0_extra_params
};

/* Address AD1 GPIO parameters */
struct no_os_gpio_init_param gpio_ad1_params = {
	.port = AD1_PORT,
	.number = GPIO_AD1,
	.platform_ops = &gpio_ops,
	.extra = &gpio_ad1_extra_params
};

/* LDAC/TG GPIO parameters */
struct no_os_gpio_init_param gpio_ldac_tg_params = {
	.port = LDAC_TG_PORT,
	.number = GPIO_LDAC_TG,
	.platform_ops = &gpio_ops,
	.extra = &gpio_ldac_tg_extra_params
};

/* Shutdown GPIO parameters */
struct no_os_gpio_init_param gpio_shutdown_params = {
	.port = SHUTDOWN_PORT,
	.number = GPIO_SHUTDOWN,
	.platform_ops = &gpio_ops,
	.extra = &gpio_shutdown_extra_params
};

/* LDAC PWM GPIO init parameters */
struct no_os_gpio_init_param ldac_pwm_gpio_params = {
	.port = LDAC_TG_PORT,
	.number = GPIO_LDAC_TG,
	.platform_ops = &gpio_ops,
	.extra = &ldac_pwm_gpio_extra_init_params
};

/* DAC Update PWM GPIO init parameters */
struct no_os_gpio_init_param dac_update_pwm_gpio_params = {
	.port = DAC_UPDATE_PORT,
	.number = GPIO_DAC_UPDATE,
	.platform_ops = &gpio_ops,
	.extra = &dac_update_pwm_gpio_extra_init_params
};

/* PWM init parameters for LDAC pulses */
struct no_os_pwm_init_param ldac_pwm_init_params = {
	.id = PWM_TIMER_ID,
	.period_ns = FREQ_TO_NSEC(AD5706_MAX_UPDATE_RATE),
	.duty_cycle_ns = LDAC_DUTY_CYCLE_NSEC(FREQ_TO_NSEC(AD5706_MAX_UPDATE_RATE)),
	.polarity = NO_OS_PWM_POLARITY_HIGH,
	.pwm_gpio = &ldac_pwm_gpio_params,
	.platform_ops = &pwm_ops,
	.extra = &ldac_pwm_extra_init_params
};

/* PWM init parameters for LDAC pulses */
struct no_os_pwm_init_param dac_update_pwm_init_params = {
	.id = DAC_UPDATE_TIMER_ID,
	.period_ns = FREQ_TO_NSEC(AD5706_MAX_UPDATE_RATE),
	.duty_cycle_ns = DAC_UPDATE_DUTY_CYCLE_NSEC(FREQ_TO_NSEC(AD5706_MAX_UPDATE_RATE)),
	.polarity = NO_OS_PWM_POLARITY_HIGH,
	.pwm_gpio = &dac_update_pwm_gpio_params,
	.platform_ops = &pwm_ops,
	.extra = &dac_update_pwm_extra_init_params
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
	.device_id = 0,
	.platform_ops = &eeprom_24xx32a_ops,
	.extra = &eeprom_extra_init_params
};

#if (INTERFACE_MODE == SPI_DMA)
/* Tx Trigger Init params */
struct no_os_pwm_init_param tx_trigger_init_params = {
	.id = TIMER8_ID,
	.period_ns = TX_TRIGGER_PERIOD,
	.duty_cycle_ns = TX_TRIGGER_DUTY_RATIO,
	.polarity = NO_OS_PWM_POLARITY_LOW,
	.platform_ops = &pwm_ops,
	.extra = &tx_trigger_extra_init_params,
};

/* Chip Select GPIO init parameters */
struct no_os_gpio_init_param csb_gpio_init_param = {
	.port = SPI_CS_PORT_NUM,
	.number = SPI_CSB,
	.pull = NO_OS_PULL_NONE,
	.platform_ops = &gpio_ops,
	.extra = &csb_gpio_extra_init_params,
};

/* PWM chip select init paramaters */
struct no_os_pwm_init_param cs_init_params = {
	.id = CS_TIMER_ID,
	.period_ns = FREQ_TO_NSEC(AD5706_MAX_UPDATE_RATE),
	.duty_cycle_ns = DAC_UPDATE_DUTY_CYCLE_NSEC(FREQ_TO_NSEC(AD5706_MAX_UPDATE_RATE)),
	.polarity = NO_OS_PWM_POLARITY_LOW,
	.platform_ops = &pwm_ops,
	.extra = &cs_extra_init_params,
	.pwm_gpio = &csb_gpio_init_param
};

/* DMA Init params */
struct no_os_dma_init_param ad5706r_dma_init_param = {
	.id = 0,
	.num_ch = AD5706_DMA_NUM_CHANNELS,
	.platform_ops = (struct no_os_dma_platform_ops *)&dma_ops,
	.sg_handler = (void (*)(void *))ad5706r_rx_cplt_callback,
};

/* Tx Trgiger PWM descriptor */
struct no_os_pwm_desc* tx_trigger_desc;
#else
/* Trigger GPIO IRQ parameters */
struct no_os_irq_init_param trigger_gpio_irq_params = {
	.irq_ctrl_id = TRIGGER_INT_ID,
	.platform_ops = &trigger_gpio_irq_ops,
	.extra = &trigger_gpio_irq_extra_params
};

/* Trigger GPIO interrupt descriptor */
struct no_os_irq_ctrl_desc *trigger_irq_desc;
#endif

/* UART IIO Comm descriptor */
struct no_os_uart_desc *uart_iio_comm_desc;

#if defined(CONSOLE_STDIO_PORT_AVAILABLE)
/* Console descriptor */
struct no_os_uart_desc *uart_console_stdio_desc;
#endif

/* GPIO AD0 descriptor */
struct no_os_gpio_desc *gpio_ad0_desc;

/* GPIO AD1 descriptor */
struct no_os_gpio_desc *gpio_ad1_desc;

/* GPIO LDAC descriptor */
struct no_os_gpio_desc *gpio_ldac_tg_desc;

/* GPIO Shutdown descriptor */
struct no_os_gpio_desc *gpio_shutdown_desc;

/* LDAC PWM descriptor */
struct no_os_pwm_desc *ldac_pwm_desc;

/* DAC Update PWM descriptor */
struct no_os_pwm_desc *dac_update_pwm_desc;

/* EEPROM descriptor */
struct no_os_eeprom_desc *eeprom_desc;

#if (INTERFACE_MODE == SPI_DMA)

#else

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

	/* Initialize the PWM interface to generate LDAC signal.*/
	ret = no_os_pwm_init(&ldac_pwm_desc, &ldac_pwm_init_params);
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_disable(ldac_pwm_desc);
	if (ret) {
		return ret;
	}

	/* Initialize the PWM interface to generate DAC Update signal.*/
	ret = no_os_pwm_init(&dac_update_pwm_desc, &dac_update_pwm_init_params);
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_disable(dac_update_pwm_desc);
	if (ret) {
		return ret;
	}


	return 0;
}

/**
 * @brief 	Initialize the GPIOs.
 * @return	0 in case of success, negative error code otherwise.
 */
static int32_t init_gpio(void)
{
	int32_t ret;

	/* Get the AD0 gpio */
	ret = no_os_gpio_get(&gpio_ad0_desc, &gpio_ad0_params);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(gpio_ad0_desc, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

	/* Get the AD1 gpio */
	ret = no_os_gpio_get(&gpio_ad1_desc, &gpio_ad1_params);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(gpio_ad1_desc, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

	/* Get the LDAC/TG gpio */
	ret = no_os_gpio_get(&gpio_ldac_tg_desc, &gpio_ldac_tg_params);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(gpio_ldac_tg_desc, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	/* Get the shutdown gpio */
	ret = no_os_gpio_get(&gpio_shutdown_desc, &gpio_shutdown_params);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_direction_output(gpio_shutdown_desc, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief 	Initialize the trigger GPIO and associated IRQ event
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t gpio_trigger_init(void)
{
#if (INTERFACE_MODE == SPI_INTERRUPT)
	int ret;

	/* Initialize the IRQ controller */
	ret = no_os_irq_ctrl_init(&trigger_irq_desc, &trigger_gpio_irq_params);
	if (ret) {
		return ret;
	}

	/* Lowering the LDAC GPIO interrupt priority than uart because some
	 * charecters of iio command are missing when both LDAC GPIO interrupt
	 * and uart interrupt occurs at same time.*/
	ret = no_os_irq_set_priority(trigger_irq_desc, TRIGGER_INT_ID,
				     DAC_UPDATE_GPIO_PRIORITY);
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
