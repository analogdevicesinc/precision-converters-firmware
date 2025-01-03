/***************************************************************************//*
 * @file    app_config.h
 * @brief   Header file for application configurations (platform-agnostic)
******************************************************************************
 * Copyright (c) 2022-24 Analog Devices, Inc.
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

/* List of supported platforms */
#define	MBED_PLATFORM		1
#define STM32_PLATFORM		2

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/******************************************************************************/

/* Select the active platform (default is Mbed) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		MBED_PLATFORM
#endif

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"

#define HW_CARRIER_NAME		TARGET_NAME

/* Redefine the init params structure mapping w.r.t. platform */
#if defined(USE_VIRTUAL_COM_PORT)
#define uart_extra_init_params mbed_vcom_extra_init_params
#define uart_ops mbed_virtual_com_ops
#else
#define uart_extra_init_params mbed_uart_extra_init_params
#define uart_ops mbed_uart_ops
#endif
#define i2c_extra_init_params mbed_i2c_extra_init_params
#define i2c_ops mbed_i2c_ops
#elif (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"

#define uart_extra_init_params	stm32_uart_extra_init_params
#define i2c_extra_init_params stm32_i2c_extra_init_params
#define i2c_ops stm32_i2c_ops
#define uart_ops stm32_uart_ops
#else
#error "No/Invalid active platform selected"
#endif

/****** Macros used to form a VCOM serial number ******/
#define	FIRMWARE_NAME	"evb_discovery_firmware"

#if !defined(PLATFORM_NAME)
#define PLATFORM_NAME	HW_CARRIER_NAME
#endif

/* Below USB configurations (VID and PID) are owned and assigned by ADI.
 * If intended to distribute software further, use the VID and PID owned by your
 * organization */
#define VIRTUAL_COM_PORT_VID	0x0456
#define VIRTUAL_COM_PORT_PID	0xb66c
/* Serial number string is formed as: application name + platform (host) name */
#define VIRTUAL_COM_SERIAL_NUM	(FIRMWARE_NAME "_" STR(PLATFORM_NAME))

/* Default baud rate for IIO UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* Uncomment to enable the EEPROM IIO device */
//#define ENABLE_EVB_EEPROM_IIO_DEV

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct no_os_uart_desc *uart_desc;
extern struct no_os_eeprom_desc *eeprom_desc;

int32_t init_system(void);

#endif /* _APP_CONFIG_H_ */
