/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module
 *   @details This module contains the configurations needed for IIO application
********************************************************************************
 * Copyright (c) 2023-25 Analog Devices, Inc.
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
#include "no_os_error.h"
#include "no_os_delay.h"
#include "no_os_gpio.h"
#include "no_os_spi.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/*************************** Variables and User Defined Types *****************/
/******************************************************************************/
/* Configuration SPI Init Parameters */
struct no_os_spi_init_param config_spi_init_params = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = SPI_CFG_SPEED,
	.mode = NO_OS_SPI_MODE_3,
	.chip_select = SPI_CSB,
	.bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST,
	.platform_ops = &spi_ops,
	.extra = &config_spi_extra_init_params
};

/* Data SPI Init Parameters */
struct no_os_spi_init_param data_spi_init_params = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = SPI_DATA_SPEED,
	.mode = NO_OS_SPI_MODE_3,
	.chip_select = SPI_DCS_CSB,
	.bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST,
	.platform_ops = &spi_ops,
	.extra = &data_spi_extra_init_params
};

#ifdef USE_QUAD_SPI
/* Data QSPI Init Parameters */
struct no_os_spi_init_param qspi_init_params = {
	.device_id = QSPI_DEVICE_ID,
	.max_speed_hz = QSPI_SPEED,
	.chip_select = 0,
	.mode = NO_OS_SPI_MODE_3,
	.bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST,
	.lanes = NO_OS_SPI_QUAD_LANE,
	.platform_ops = &xspi_ops,
	.extra = &data_qspi_extra_init_params
};
#endif

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
	 * console. Applications which does not support VCOM, should not satisfy this
	 * condition */
	.platform_ops = &uart_ops,
	.extra = &uart_extra_init_params,
#else
#if defined(CONSOLE_STDIO_PORT_AVAILABLE)
	/* Applications which uses phy COM port as primary IIO comm port,
	 * can use VCOM as console stdio port provided it is available.
	 * Else, alternative phy com port can be used for console stdio ops if available */
	.platform_ops = &vcom_ops,
	.extra = &vcom_extra_init_params,
#endif
#endif
};

/* XTAL_OSC_EN GPIO */
struct no_os_gpio_init_param gpio_xtal_osc_en_init_params = {
	.port = GPIO_XTAL_OSC_EN_PORT,
	.number = GPIO_XTAL_OSC_EN,
	.platform_ops = &gpio_ops,
	.extra = &gpio_xtal_osc_en_extra_init_params,
};

/* GP1 GPIO */
struct no_os_gpio_init_param gpio_gp1_init_params = {
	.port = GPIO_GP1_PORT,
	.number = GPIO_GP1,
	.platform_ops = &gpio_ops,
	.extra = &gpio_gp1_extra_init_params,
};

/* GP2 GPIO */
struct no_os_gpio_init_param gpio_gp2_init_params = {
	.port = GPIO_GP2_PORT,
	.number = GPIO_GP2,
	.platform_ops = &gpio_ops,
	.extra = &gpio_gp2_extra_init_params,
};

/* GP3 GPIO */
struct no_os_gpio_init_param gpio_gp3_init_params = {
	.port = GPIO_GP3_PORT,
	.number = GPIO_GP3,
	.platform_ops = &gpio_ops,
	.extra = &gpio_gp3_extra_init_params,
};

/* 40M Oscillator Enable GPIO */
struct no_os_gpio_init_param gpio_40m_osc_init_params = {
	.port = GPIO_OSC_EN_40M_PORT,
	.number = GPIO_OSC_EN_40M,
	.platform_ops = &gpio_ops,
	.extra = &gpio_40m_osc_extra_init_params,
};

/* 20M Oscillator Enable GPIO */
struct no_os_gpio_init_param gpio_20m_osc_init_params = {
	.port = GPIO_OSC_EN_20M_PORT,
	.number = GPIO_OSC_EN_20M,
	.platform_ops = &gpio_ops,
	.extra = &gpio_20m_osc_extra_init_params,
};

/* 10M Oscillator Enable GPIO */
struct no_os_gpio_init_param gpio_10m_osc_init_params = {
	.port = GPIO_OSC_EN_10M_PORT,
	.number = GPIO_OSC_EN_10M,
	.platform_ops = &gpio_ops,
	.extra = &gpio_10m_osc_extra_init_params,
};

/* AFE CTRL GPIO */
struct no_os_gpio_init_param gpio_afe_ctrl_init_params = {
	.port = GPIO_AFE_CTRL_PORT,
	.number = GPIO_AFE_CTRL,
	.platform_ops = &gpio_ops,
	.extra = &gpio_afe_ctrl_extra_init_params,
};

/* I2C init parameters */
struct no_os_i2c_init_param no_os_i2c_init_params = {
	.device_id = I2C_DEVICE_ID,
	.platform_ops = &i2c_ops,
	.max_speed_hz = 100000,
	.extra = &i2c_extra_init_params,
};

/* EEPROM extra init parameters */
struct eeprom_24xx32a_init_param eeprom_extra_init_params = {
	.i2c_init = &no_os_i2c_init_params
};

/* EEPROM init parameters */
struct no_os_eeprom_init_param eeprom_init_params = {
	.device_id = I2C_DEVICE_ID,
	.platform_ops = &eeprom_24xx32a_ops,
	.extra = &eeprom_extra_init_params
};

/* UART IIO Comm descriptor */
struct no_os_uart_desc *uart_iio_comm_desc;

#if defined(CONSOLE_STDIO_PORT_AVAILABLE)
/* Console UART descriptor */
struct no_os_uart_desc *uart_console_stdio_desc;
#endif

/* AFE CTRL GPIO Descriptor */
struct no_os_gpio_desc *gpio_afe_ctrl_desc;

/* GP1 GPIO Descriptor */
struct no_os_gpio_desc *gpio_gp1_desc;

/* GP2 GPIO Descriptor */
struct no_os_gpio_desc *gpio_gp2_desc;

/* GP3 GPIO Descriptor */
struct no_os_gpio_desc *gpio_gp3_desc;

/* XTAL_OSC_EN GPIO Descriptor */
struct no_os_gpio_desc *gpio_xtal_osc_en_desc;

/* 40M Oscillator Enable GPIO Descriptor */
struct no_os_gpio_desc *gpio_osc_en_40m_desc;

/* 20M Oscillator Enable GPIO Descriptor */
struct no_os_gpio_desc *gpio_osc_en_20m_desc;

/* 10M Oscillator Enable GPIO Descriptor */
struct no_os_gpio_desc *gpio_osc_en_10m_desc;

#ifdef USE_QUAD_SPI
/* QSPI descriptor */
struct no_os_spi_desc *quad_spi_desc;
#endif

/* EEPROM descriptor */
struct no_os_eeprom_desc *eeprom_desc;

/******************************************************************************/
/************************ Functions Prototypes ********************************/
/******************************************************************************/
#ifdef USE_QUAD_SPI
/**
 * @brief Initialize the QSPI peripheral
 * @return 0 in case of success else negative error code.
 */
static int32_t init_qspi()
{
	int32_t ret;

	/* Initialize QSPI descriptor */
	ret = no_os_spi_init(&quad_spi_desc, &qspi_init_params);
	if (ret) {
		return ret;
	}

	return 0;
}
#endif

/**
 * @brief Initialize the UART peripheral
 * @return 0 in case of success else negative error code.
 */
static int32_t init_uart()
{
	int32_t ret;

	/* Initialize IIO UART port */
	ret = no_os_uart_init(&uart_iio_comm_desc, &uart_iio_comm_init_params);
	if (ret) {
		goto err_uart_init;
	}

#if defined(CONSOLE_STDIO_PORT_AVAILABLE)
	/* Initialize the serial link for console stdio communication */
	ret = no_os_uart_init(&uart_console_stdio_desc,
			      &uart_console_stdio_init_params);
	if (ret) {
		goto err_uart_init;
	}
#endif

	return 0;

err_uart_init:
	/* If failure, remove uart descriptors */
	no_os_uart_remove(uart_iio_comm_desc);

#if defined(CONSOLE_STDIO_PORT_AVAILABLE)
	no_os_uart_remove(uart_console_stdio_desc);
#endif

	return ret;
}

/**
 * @brief Initialize the GPIOs.
 * @return 0 in case of success else negative error code.
 */
static int32_t init_gpio()
{
	int32_t ret;

	/********** XTAL_OSC_EN GPIO **********/
	/* Initialize GPIO descriptor */
	ret = no_os_gpio_get(&gpio_xtal_osc_en_desc, &gpio_xtal_osc_en_init_params);
	if (ret) {
		goto err_gpio_init;
	}

	/* Set the GPIO direction as output */
	ret = no_os_gpio_direction_output(gpio_xtal_osc_en_desc, NO_OS_GPIO_HIGH);
	if (ret) {
		goto err_gpio_init;
	}

	/*********** GP1 GPIO ***********/
	/* Initialize GPIO descriptor */
	ret = no_os_gpio_get(&gpio_gp1_desc, &gpio_gp1_init_params);
	if (ret) {
		goto err_gpio_init;
	}

	/* Set the GPIO direction as output */
	ret = no_os_gpio_direction_output(gpio_gp1_desc, NO_OS_GPIO_LOW);
	if (ret) {
		goto err_gpio_init;
	}

	/********** GP2 GPIO **********/
	/* Initialize GPIO descriptor */
	ret = no_os_gpio_get(&gpio_gp2_desc, &gpio_gp2_init_params);
	if (ret) {
		goto err_gpio_init;
	}

	/* Set the GPIO direction as input */
	ret = no_os_gpio_direction_input(gpio_gp2_desc);
	if (ret) {
		goto err_gpio_init;
	}

	/********** GP3 GPIO **********/
	/* Initialize GPIO descriptor */
	ret = no_os_gpio_get(&gpio_gp3_desc, &gpio_gp3_init_params);
	if (ret) {
		goto err_gpio_init;
	}

	/* Set the GPIO direction as input */
	ret = no_os_gpio_direction_input(gpio_gp3_desc);
	if (ret) {
		goto err_gpio_init;
	}

	/********** 40M Oscillator Enable GPIO **********/
	/* Initialize GPIO descriptor */
	ret = no_os_gpio_get(&gpio_osc_en_40m_desc, &gpio_40m_osc_init_params);
	if (ret) {
		goto err_gpio_init;
	}

	/* Set the GPIO direction as output */
	ret = no_os_gpio_direction_output(gpio_osc_en_40m_desc, OSC_40M_DEFAULT_STATE);
	if (ret) {
		goto err_gpio_init;
	}

	/********** 20M Oscillator Enable GPIO **********/
	/* Initialize GPIO descriptor */
	ret = no_os_gpio_get(&gpio_osc_en_20m_desc, &gpio_20m_osc_init_params);
	if (ret) {
		goto err_gpio_init;
	}

	/* Set the GPIO direction as output */
	ret = no_os_gpio_direction_output(gpio_osc_en_20m_desc, OSC_20M_DEFAULT_STATE);
	if (ret) {
		goto err_gpio_init;
	}

	/********** 10M Oscillator Enable GPIO **********/
	/* Initialize GPIO descriptor */
	ret = no_os_gpio_get(&gpio_osc_en_10m_desc, &gpio_10m_osc_init_params);
	if (ret) {
		goto err_gpio_init;
	}

	/* Set the GPIO direction as output */
	ret = no_os_gpio_direction_output(gpio_osc_en_10m_desc, OSC_10M_DEFAULT_STATE);
	if (ret) {
		goto err_gpio_init;
	}

	/********** AFE CTRL GPIO ***********/
	/* Initialize GPIO descriptor */
	ret = no_os_gpio_get(&gpio_afe_ctrl_desc, &gpio_afe_ctrl_init_params);
	if (ret) {
		goto err_gpio_init;
	}

	/* Set the GPIO direction as output */
	ret = no_os_gpio_direction_output(gpio_afe_ctrl_desc, NO_OS_GPIO_HIGH);
	if (ret) {
		goto err_gpio_init;
	}

	return 0;

err_gpio_init:
	no_os_gpio_remove(gpio_afe_ctrl_desc);
	no_os_gpio_remove(gpio_gp1_desc);
	no_os_gpio_remove(gpio_gp2_desc);
	no_os_gpio_remove(gpio_gp3_desc);
	no_os_gpio_remove(gpio_xtal_osc_en_desc);
	no_os_gpio_remove(gpio_osc_en_40m_desc);
	no_os_gpio_remove(gpio_osc_en_20m_desc);
	no_os_gpio_remove(gpio_osc_en_10m_desc);

	return ret;
}

/**
 * @brief 	Initializing system peripherals
 * @return	0 in case of success, negative error code otherwise.
 * @details	This function initializes system peripherals for the application
 */
int32_t init_system()
{
	int32_t ret;

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	stm32_system_init();
#endif

	/* Initialize UART */
	ret = init_uart();
	if (ret) {
		return ret;
	}

	/* Initialize GPIO */
	ret = init_gpio();
	if (ret) {
		return ret;
	}

#ifdef USE_QUAD_SPI
	/* Initialize QSPI */
	ret = init_qspi();
	if (ret) {
		return ret;
	}
#endif

	/* Initialize EEPROM */
	ret = eeprom_init(&eeprom_desc, &eeprom_init_params);
	if (ret) {
		return ret;
	}

#if defined(USE_SDRAM)
	/* Initialize SDRAM */
	ret = sdram_init();
	if (ret) {
		return ret;
	}
#endif

	return 0;
}