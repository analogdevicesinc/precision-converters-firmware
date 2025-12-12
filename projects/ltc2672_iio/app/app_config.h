/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for LTC2672 device application
******************************************************************************
* Copyright (c) 2023-25 Analog Devices, Inc.
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
#include "ltc2672.h"
#include "no_os_pwm.h"
#include "version.h"
#include "common_macros.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

#ifdef FIRMWARE_VERSION
#define GET_FIRMWARE_VERSION (FIRMWARE_VERSION)
#else
#define GET_FIRMWARE_VERSION NULL
#endif

/******************************************************************************/

/* Select the active platform (default is STM32) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

/* Support for Arduino interfaced board is enabled by default */
//#define DC2903A			// Uncomment to select Linduino interfaced board

#if defined(DC2903A)
//*** Note for user to select active device ***//
/* Define the device type here
 * (use only one define at time. Defining multiple devices gives compilation error)
 * e.g. #define DEV_LTC2672_16 will select LTC2672_16 as the active device
 * */
// #define DEV_LTC2672_16

#if defined(DEV_LTC2672_12)
#define ACTIVE_DEVICE_NAME	 "ltc2672-12"
#define DEVICE_NAME		     "DEV_LTC2672_12"
#define ACTIVE_DEVICE_ID	 LTC2672_12
#define DAC_RESOLUTION  12
#define DAC_MAX_COUNT	LTC2672_12BIT_RESO
#elif defined(DEV_LTC2672_16)
#define ACTIVE_DEVICE_NAME	 "ltc2672-16"
#define DEVICE_NAME		     "DEV_LTC2672_16"
#define ACTIVE_DEVICE_ID	 LTC2672_16
#define DAC_RESOLUTION 16
#define DAC_MAX_COUNT	LTC2672_16BIT_RESO
#else
#warning No/Unsupported ADxxxxy symbol defined. LTC2672_16 defined
#define DEV_LTC2672_16
#define ACTIVE_DEVICE_NAME	 "ltc2672-16"
#define DEVICE_NAME		     "DEV_LTC2672_16"
#define ACTIVE_DEVICE_ID	 LTC2672_16
#define DAC_RESOLUTION 16
#define DAC_MAX_COUNT	LTC2672_16BIT_RESO
#endif
#else	// For Arduino interfaced board
#define DAC_RESOLUTION		16
#define DAC_MAX_COUNT		LTC2672_16BIT_RESO
#define DEVICE_LTC2662_16	"ltc2662-16"
#define DEVICE_LTC2672_16	"ltc2672-16"
#endif

/* DAC Reference Voltage */
#define DAC_VREF  1.25

/* Full Scale Adjusted Resitor Value in kohm */
#define DAC_FSADJ_RESISTOR  20

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#define CONSOLE_STDIO_PORT_AVAILABLE
#else
#error "No/Invalid active platform selected"
#endif

#define HW_CARRIER_NAME TARGET_NAME

#define HW_MEZZANINE_NAME "DC2903A-A"

/****** Macros used to form a VCOM serial number ******/

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* Used to form a VCOM serial number */
#define	FIRMWARE_NAME	"ltc2672_iio"

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

/* Check if any serial port available for use as console stdio port */
#if defined(USE_VIRTUAL_COM_PORT)
/* If VCOM is selected, PHY com port will/should act as a console stdio port */
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif

/* PWM period and duty cycle */
#define CONV_TRIGGER_PERIOD_NSEC		(((float)(1.0 / SAMPLING_RATE) * 1000000) * 1000)
#define CONV_TRIGGER_DUTY_CYCLE_NSEC	(CONV_TRIGGER_PERIOD_NSEC / 2)

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

extern struct no_os_uart_desc *uart_iio_com_desc;
extern struct no_os_uart_desc *uart_console_stdio_desc;
extern struct no_os_eeprom_desc *eeprom_desc;
extern struct no_os_gpio_desc *gpio_ldac_desc;
extern struct no_os_gpio_desc *gpio_clear_desc;
extern struct no_os_gpio_desc *gpio_toggle_desc;
extern struct no_os_pwm_desc *toggle_pwm_desc;
extern struct no_os_pwm_desc *ldac_pwm_desc;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;
extern struct no_os_gpio_init_param toggle_pwm_gpio_params;
int32_t init_system(void);
int32_t init_pwm(void);

#endif /* APP_CONFIG_H_ */
