/***************************************************************************//*
 * @file    app_config.h
 * @brief   Configuration file for AD77681 IIO firmware application
 * @details
******************************************************************************
 * Copyright (c) 2021-23, 2025 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/

#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* List of supported platforms*/
#define	MBED_PLATFORM		1
#define STM32_PLATFORM      2

/* List of supported data capture modes for an application */
#define BURST_DATA_CAPTURE			0
#define CONTINUOUS_DATA_CAPTURE		1

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/* Select the active platform (default is Mbed) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

/* Select the ADC data capture mode (default is CC mode) */
#if !defined(DATA_CAPTURE_MODE)
#define DATA_CAPTURE_MODE	CONTINUOUS_DATA_CAPTURE
#endif

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

/* Name of active device */
#define ACTIVE_DEVICE_NAME	"ad7768-1"

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"

#define HW_CARRIER_NAME		TARGET_NAME

/* Redefine the init params structure mapping w.r.t. platform */
#define ext_int_extra_init_params mbed_ext_int_extra_init_params
#define vcom_extra_init_params mbed_vcom_extra_init_params
#define uart_extra_init_params mbed_uart_extra_init_params
#define spi_extra_init_params mbed_spi_extra_init_params
#define i2c_extra_init_params mbed_i2c_extra_init_params
#define trigger_gpio_irq_extra_params mbed_trigger_gpio_irq_init_params
#define trigger_gpio_extra_init_params mbed_trigger_gpio_extra_init_params
#define trigger_gpio_ops mbed_gpio_ops
#define irq_ops		mbed_gpio_irq_ops
#define gpio_ops	mbed_gpio_ops
#define spi_ops		mbed_spi_ops
#define i2c_ops mbed_i2c_ops
#define trigger_gpio_irq_ops mbed_gpio_irq_ops
#define vcom_ops mbed_virtual_com_ops
#define uart_ops mbed_uart_ops
#define trigger_gpio_handle 0	// Unused macro
#define TRIGGER_GPIO_PORT 0  // Unused macro
#define TRIGGER_GPIO_PIN  CONV_MON
#define TRIGGER_INT_ID	GPIO_IRQ_ID1
#elif (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
/* Redefine the init params structure mapping w.r.t. platform */
#define vcom_extra_init_params stm32_vcom_extra_init_params
#define vcom_ops stm32_usb_uart_ops
#define uart_extra_init_params stm32_uart_extra_init_params
#define uart_ops stm32_uart_ops
#define spi_extra_init_params stm32_spi_extra_init_params
#define i2c_extra_init_params stm32_i2c_extra_init_params
#define trigger_gpio_irq_extra_params stm32_trigger_gpio_irq_init_params
#define trigger_gpio_extra_init_params stm32_trigger_gpio_extra_init_params
#define trigger_gpio_ops stm32_gpio_ops
#define irq_ops		stm32_gpio_irq_ops
#define gpio_ops	stm32_gpio_ops
#define spi_ops		stm32_spi_ops
#define i2c_ops stm32_i2c_ops
#define trigger_gpio_irq_ops stm32_gpio_irq_ops
#define trigger_gpio_handle 0	// Unused macro
#else
#error "No/Invalid active platform selected"
#endif

/* ADC resolution for active device */
#define ADC_RESOLUTION		24

/****** Macros used to form a VCOM serial number ******/
#define	FIRMWARE_NAME	"ad77681_iio"

#define DEVICE_NAME		"DEV_AD77681"

#if !defined(PLATFORM_NAME)
#define PLATFORM_NAME	HW_CARRIER_NAME
#endif

/* Below USB configurations (VID and PID) are owned and assigned by ADI.
 * If intended to distribute software further, use the VID and PID owned by your
 * organization */
#define VIRTUAL_COM_PORT_VID	0x0456
#define VIRTUAL_COM_PORT_PID	0xb66c
/* Serial number string is formed as: application name + device (target) name + platform (host) name */
#define VIRTUAL_COM_SERIAL_NUM	(FIRMWARE_NAME "_" DEVICE_NAME "_" STR(PLATFORM_NAME))

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE (230400)

/* Check if any serial port available for use as console stdio port */
#if defined(USE_PHY_COM_PORT)
/* If PHY com is selected, VCOM or alternate PHY com port can act as a console stdio port */
#if (ACTIVE_PLATFORM == MBED_PLATFORM || ACTIVE_PLATFORM == STM32_PLATFORM)
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif
#else
/* If VCOM is selected, PHY com port will/should act as a console stdio port */
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif

/* Define the max possible sampling frequency (or output data) rate for AD77681 (in SPS).
 * This is also used to find the time period to trigger a periodic conversion event.
 * Note: Max possible ODR is 64KSPS for continuous data capture on IIO Client.
 * This is derived by capturing data from the firmware using the SDP-K1 controller board
 * @22.5Mhz SPI clock. The max possible ODR can vary from board to board and
 * data continuity is not guaranteed above this ODR on IIO oscilloscope */

/* AD77681 default internal clock frequency (MCLK = 16.384 Mhz) */
#define AD77681_MCLK (16384)

/* AD77681 decimation rate */
#define AD77681_DECIMATION_RATE	(32U)

/* AD77681 default mclk_div value */
#define AD77681_DEFAULT_MCLK_DIV (8)

/* AD77681 ODR conversion */
#define AD77681_ODR_CONV_SCALER	(AD77681_DECIMATION_RATE * AD77681_DEFAULT_MCLK_DIV)

/* AD77681 default sampling frequency */
#define AD77681_DEFAULT_SAMPLING_FREQ	((AD77681_MCLK * 1000) / AD77681_ODR_CONV_SCALER)

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
//#define USE_SDRAM		// Uncomment to use SDRAM for data buffer

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

int32_t init_system(void);

extern struct no_os_uart_desc *uart_desc;
extern struct no_os_gpio_desc *trigger_gpio_desc;
extern struct no_os_spi_init_param spi_init_params;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;

#endif /* _APP_CONFIG_H_ */
