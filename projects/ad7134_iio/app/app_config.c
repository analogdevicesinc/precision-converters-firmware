/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module
 *   @details This module contains the configurations needed for AD7134 IIO
 *            application
********************************************************************************
 * Copyright (c) 2021, 2023-24 Analog Devices, Inc.
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
#include "no_os_pwm.h"
#include "no_os_tdm.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* UART init parameters for IIO comm port */
struct no_os_uart_init_param uart_iio_comm_init_params = {
	.device_id = UART_DEVICE_ID,
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
	.device_id = UART_DEVICE_ID,
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

/* External interrupt init parameters */
static struct no_os_irq_init_param ext_int_init_params = {
	.irq_ctrl_id = IRQ_INT_ID,
	.platform_ops = &trigger_gpio_irq_ops,
	.extra = &ext_int_extra_init_params
};

#if (INTERFACE_MODE != TDM_MODE)
/* PWM init parameters */
static struct no_os_pwm_init_param pwm_init_params = {
	.period_ns = CONV_TRIGGER_PERIOD_NSEC,			// PWM period in nsec
	.duty_cycle_ns = CONV_TRIGGER_DUTY_CYCLE_NSEC,	// PWM duty cycle in nsec
	.extra = &pwm_extra_init_params,
	.platform_ops = &pwm_ops
};
#elif (INTERFACE_MODE == TDM_MODE)
struct no_os_tdm_init_param tdm_init_param = {
	.mode = NO_OS_TDM_SLAVE_RX,		// AD7134 acts as a controller and the STM board acts as a target
	.data_size = TDM_DATA_SIZE,			// 16 Bit data transfer mode
	.data_offset = 0,
	.data_lsb_first = false,
	.slots_per_frame = TDM_SLOTS_PER_FRAME,		// Each slot to hold the data of each channel
	.fs_active_low = true,
	.fs_active_length = TDM_FS_ACTIVE_LENGTH,
	.fs_lastbit = false,
	.rising_edge_sampling = false,
	.irq_id = DMA_IRQ_ID,
	.rx_complete_callback = ad7134_dma_rx_cplt,
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
	.rx_half_complete_callback = ad7134_dma_rx_half_cplt,
#endif
	.extra = &tdm_extra_init_params,
	.platform_ops = &stm32_tdm_platform_ops
};

/* TDM Descriptor */
struct no_os_tdm_desc *ad7134_tdm_desc;
#endif // INTERFACE_MODE

/* Define the GPIO init parameter structure for DCLK pin */
static struct no_os_gpio_init_param dclk_init_param = {
	.number = DCLK_PIN,
	.platform_ops = &gpio_ops,
	.extra = NULL
};

/* Define the GPIO init parameter structure for ODR pin */
static struct no_os_gpio_init_param odr_init_param = {
	.number = ODR_PIN,
	.platform_ops = &gpio_ops,
	.extra = NULL
};

/* Define the GPIO init parameter structure for DOUT0 pin */
static struct no_os_gpio_init_param dout0_init_param = {
	.number = DOUT0_PIN,
	.platform_ops = &gpio_ops,
	.extra = NULL
};

/* Define the GPIO init parameter structure for DOUT0 pin */
static struct no_os_gpio_init_param dout1_init_param = {
	.number = DOUT1_PIN,
	.platform_ops = &gpio_ops,
	.extra = NULL
};

/* Define the GPIO init parameter structure for PDN Pin */
struct no_os_gpio_init_param pdn_init_param = {
	.number = PDN_PIN,
	.port = PDN_PORT,
	.extra = &gpio_pdn_extra_init_params,
	.platform_ops = &gpio_ops
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
	.device_id = I2C_DEVICE_ID,
	.platform_ops = &eeprom_24xx32a_ops,
	.extra = &eeprom_extra_init_params
};

/* Descriptor for the data capture GPIOs */
static struct no_os_gpio_desc *dclk_desc = NULL;
static struct no_os_gpio_desc *odr_desc = NULL;
static struct no_os_gpio_desc *dout0_desc = NULL;
static struct no_os_gpio_desc *dout1_desc = NULL;

/* EEPROM descriptor */
struct no_os_eeprom_desc *eeprom_desc;

/* External interrupt descriptor */
struct no_os_irq_ctrl_desc *external_int_desc;

/* PWM descriptor */
struct no_os_pwm_desc *pwm_desc;

/* UART IIO Descriptor */
struct no_os_uart_desc *uart_iio_com_desc;

/* UART console descriptor */
struct no_os_uart_desc *uart_console_stdio_desc;

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
#if (ACTIVE_PLATFORM == MBED_PLATFORM)
	do {
		/* Create a new descriptor for DCLK GPIO */
		if (no_os_gpio_get(&dclk_desc, &dclk_init_param) != 0) {
			break;
		}

		/* Set DCLK direction as input */
#if (AD7134_ASRC_MODE == CONTROLLER_MODE)
		if (no_os_gpio_direction_input(dclk_desc) != 0) {
#else
		if (no_os_gpio_direction_output(dclk_desc, NO_OS_GPIO_LOW) != 0) {
#endif
			break;
		}

		/* Create a new descriptor for ODR GPIO */
		if (no_os_gpio_get(&odr_desc, &odr_init_param) != 0) {
			break;
		}

		/* Set ODR direction as input */
#if (AD7134_ASRC_MODE == CONTROLLER_MODE)
		if (no_os_gpio_direction_input(odr_desc) != 0) {
#else
		if (no_os_gpio_direction_output(odr_desc, NO_OS_GPIO_LOW) != 0) {
#endif
			break;
		}

		/* Create a new descriptor for DOUT0 GPIO */
		if (no_os_gpio_get(&dout0_desc, &dout0_init_param) != 0) {
			break;
		}

		/* Set DOUT0 direction as input */
		if (no_os_gpio_direction_input(dout0_desc) != 0) {
			break;
		}

		/* Create a new descriptor for DOUT1 GPIO */
		if (no_os_gpio_get(&dout1_desc, &dout1_init_param) != 0) {
			break;
		}

		/* Set DOUT1 direction as input */
		if (no_os_gpio_direction_input(dout1_desc) != 0) {
			break;
		}

		return 0;
	}
	while (0);

	return -EINVAL;
#endif

	return 0;
}


/**
 * @brief 	Initialize the IRQ contoller
 * @return	0 in case of success, negative error code otherwise
 * @details	This function initialize the interrupts used by application
 */
static int32_t init_interrupt(void)
{
	do {
		/* Init interrupt controller for external interrupt (for monitoring
		 * conversion event on BUSY pin) */
		if (no_os_irq_ctrl_init(&external_int_desc, &ext_int_init_params) != 0) {
			break;
		}

		return 0;
	} while (0);

	return -EINVAL;

}


/**
 * @brief 	Initialize the PWM contoller
 * @return	0 in case of success, negative error code otherwise
 */
int32_t init_pwm(void)
{
	do {
#if (ACTIVE_PLATFORM == MBED_PLATFORM)
		/* Initialize the PWM interface to generate PWM signal
		 * on conversion trigger event pin */
		if (no_os_pwm_init(&pwm_desc, &pwm_init_params) != 0) {
			break;
		}

		if (no_os_pwm_enable(pwm_desc) != 0) {
			break;
		}
#endif //ACTIVE_PLATFORM

		return 0;
	} while (0);

	return -EINVAL;
}


/**
 * @brief 	Initialize the TDM peripheral
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t init_tdm(void)
{
#if (INTERFACE_MODE == TDM_MODE)
	if (no_os_tdm_init(&ad7134_tdm_desc, &tdm_init_param) != 0) {
		return -EINVAL;
	}
#endif // ACTIVE_PLATFORM

	return 0;
}

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

	if (init_uart() != 0) {
		return -EINVAL;
	}

	if (init_gpio() != 0) {
		return -EINVAL;
	}

	if (init_interrupt() != 0) {
		return -EINVAL;
	}

#if defined(USE_SDRAM)
	if (sdram_init() != 0) {
		return -EINVAL;
	}
#endif
	if (init_tdm () != 0) {
		return -EINVAL;
	}

	ret = eeprom_init(&eeprom_desc, &eeprom_init_params);
	if (ret) {
		return ret;
	}

	return 0;
}
