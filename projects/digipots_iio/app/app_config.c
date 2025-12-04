/***************************************************************************//**
 * @file   app_config.c
 * @brief  Application configurations module for digipots IIO FW
********************************************************************************
Copyright 2025(c) Analog Devices, Inc.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of Analog Devices, Inc. nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. “AS IS” AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL ANALOG DEVICES, INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdbool.h>
#include "app_config.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/
#define SPI_CLOCK 1000000
#define I2C_CLOCK 100000
#define SPI_DEV_NUM 1
/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
/******************************************************************************/
/* Digipot devices SPI Mode 2 init parameters */
struct no_os_spi_init_param spi_mode2_init_params = {
	.max_speed_hz = SPI_CLOCK,
	.mode = NO_OS_SPI_MODE_2,
	.chip_select = SPI_CSB,
	.platform_ops = &spi_ops,
	.device_id = SPI_DEV_NUM,
	.extra = &spi_extra_init_params
};

/* Digipot devices SPI Mode 0 init parameters */
struct no_os_spi_init_param spi_mode0_init_params = {
	.max_speed_hz = SPI_CLOCK,
	.mode = NO_OS_SPI_MODE_0,
	.chip_select = SPI_CSB,
	.platform_ops = &spi_ops,
	.device_id = SPI_DEV_NUM,
	.extra = &spi_extra_init_params
};

/* EEPROM and Digipot devices I2C init parameters */
struct no_os_i2c_init_param i2c_init_params = {
	.device_id = I2C_ID,
	.platform_ops = &i2c_ops,
	.max_speed_hz = I2C_CLOCK
};

/* UART init parameters for IIO comm port */
struct no_os_uart_init_param uart_iio_comm_init_params = {
	.device_id = UART_ID,
	.baud_rate = IIO_UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
	.asynchronous_rx = true,
	.irq_id = APP_UART_USB_IRQ,
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
	.asynchronous_rx = true,
	.irq_id = UART_IRQ_ID,
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

/* EEPROM init parameters */
static struct eeprom_24xx32a_init_param eeprom_extra_init_params = {
	.i2c_init = &i2c_init_params
};

/* EEPROM init parameters */
static struct no_os_eeprom_init_param eeprom_init_params = {
	.device_id = 0,
	.platform_ops = &eeprom_24xx32a_ops,
	.extra = &eeprom_extra_init_params
};

/* UART IIO descriptor */
struct no_os_uart_desc *uart_iio_com_desc;

/* UART console descriptor */
struct no_os_uart_desc *uart_console_stdio_desc;

/* EEPROM descriptor */
struct no_os_eeprom_desc *eeprom_desc;

active_dpot_device oactive_dev = {
	.intf_type = DEFAULT_INTERFACE_TYPE,
	.active_device_name = DEFAULT_DEVICE_NAME,
	.active_device = DEFAULT_ACTIVE_DEVICE,
	.device_i2c_addr = DEFAULT_DEVICE_I2C_ADDR,
	.max_chns_pot = DEFAULT_NUM_CHNS_POT,
	.max_chns_linGain = DEFAULT_NUM_CHNS_LINGAIN,
	.mode  = DEFAULT_OPERATING_MODE
};
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
 * @brief 	Initialize system peripherals.
 * @return	0 in case of success, negative error code otherwise.
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
