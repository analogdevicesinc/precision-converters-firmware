/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD717x and AD411x IIO firmware application
******************************************************************************
* Copyright (c) 2021-23,2025-26 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include <common_macros.h>
#include "no_os_gpio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* List of data capture modes for AD717x device */
#define CONTINUOUS_DATA_CAPTURE		0
#define BURST_DATA_CAPTURE			1

/* List of board interface for AD717x device */
#define SDP_120_INTERFACE			0
#define ARDUINO_INTERFACE			1

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/* Select the interface mode (default is SDP_120 interface) */
#if !defined(EVB_INTERFACE)
#define EVB_INTERFACE	ARDUINO_INTERFACE
#endif

/* Select the active platform */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif // ACTIVE_PLATFORM

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

/* Enable register map support for AD717x family devices.
 * AD411x devices do not require these macros. */
#define AD7172_2_INIT
#define AD7172_4_INIT
#define AD7173_8_INIT
#define AD7175_2_INIT
#define AD7175_8_INIT
#define AD7176_2_INIT
#define AD7177_2_INIT

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#define HW_CARRIER_NAME		    	TARGET_NAME
/* Redefine the init params structure mapping w.r.t. platform */
#define uart_extra_init_params        stm32_uart_extra_init_params
#define spi_extra_init_params         stm32_spi_extra_init_params
#define vcom_extra_init_params  stm32_vcom_extra_init_params
#define uart_extra_init_params        stm32_uart_extra_init_params
#define ext_int_extra_init_params stm32_trigger_gpio_irq_init_params
#define uart_ops stm32_uart_ops
#define vcom_ops  stm32_usb_uart_ops
#define irq_platform_ops stm32_gpio_irq_ops
#define csb_platform_ops stm32_gpio_ops
#define rdy_platform_ops stm32_gpio_ops
#define spi_platform_ops stm32_spi_ops
#define irq_ops          stm32_irq_ops
#define i2c_ops		stm32_i2c_ops
#define trigger_gpio_irq_ops stm32_gpio_irq_ops
#define trigger_gpio_handle 0	// Unused macro
#define IRQ_INT_ID  RDY_PIN
#else
#error "No/Invalid active platform selected"
#endif

/* VCOM Serial number definition */
#define	FIRMWARE_NAME	"ad717x_iio"

#if !defined(PLATFORM_NAME)
#define PLATFORM_NAME	HW_CARRIER_NAME
#endif

/* Select the ADC data capture mode (default is CC mode) */
#if !defined(DATA_CAPTURE_MODE)
#define DATA_CAPTURE_MODE	CONTINUOUS_DATA_CAPTURE
#endif

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
//#define USE_SDRAM	// Uncomment to use SDRAM for data buffer

/* ADC Reference Voltage in volts */
#define AD717X_INTERNAL_REFERENCE	2.5
#define AD717x_EXTERNAL_REFERENCE	2.5
#define AD717X_AVDD_AVSS_REFERENCE	2.5

/* Baud Rate for IIO Application */
#define IIO_UART_BAUD_RATE		(230400)

#define CONSOLE_STDIO_PORT_AVAILABLE

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

extern struct no_os_uart_desc *uart_desc;

extern struct no_os_gpio_desc *csb_gpio;

extern struct no_os_gpio_desc *rdy_gpio;

extern struct no_os_irq_ctrl_desc *trigger_irq_desc;

extern struct no_os_eeprom_desc *eeprom_desc;

extern struct no_os_uart_desc *uart_console_stdio_desc;

int32_t init_system(void);

#endif // APP_CONFIG_H

