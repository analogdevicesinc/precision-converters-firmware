/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for ad559xr IIO firmware application
******************************************************************************
* Copyright (c) 2026 Analog Devices, Inc.
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
#include "no_os_pwm.h"
#include "common_macros.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* List of data capture modes for ad559xr device */
#define CONTINUOUS_DATA_CAPTURE		0
#define BURST_DATA_CAPTURE			1

/* Select the active platform (default is STM32) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

/* Select the ADC data capture mode
 * Note: Only Burst data Capture is supported for AD5593R */
#if !defined(DATA_CAPTURE_MODE)
#define DATA_CAPTURE_MODE BURST_DATA_CAPTURE
#endif

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#define CONSOLE_STDIO_PORT_AVAILABLE
#else
#error "No/Invalid active platform selected"
#endif

/* Hardware Mezzanine name for the interposer board */
#define HW_MEZZANINE_NAME	"PMD-ARD-INT-LCZ"

/* ADC Resolution */
#define ADC_RESOLUTION		12

/* ADC max count (full scale value) for unipolar inputs */
#define ADC_MAX_COUNT_UNIPOLAR	(uint32_t)((1 << ADC_RESOLUTION) - 1)

/* ADC max count (full scale value) for bipolar inputs */
#define ADC_MAX_COUNT_BIPOLAR	(uint32_t)(1 << (ADC_RESOLUTION-1))

#define DEFAULT_VREF  2.5
#define ADC_MAX_COUNT (uint32_t)(1<<ADC_RESOLUTION)

/* Additional bit macros */
#define BYTES_PER_SAMPLE        2
#define STORAGE_BITS            (BYTES_PER_SAMPLE) * 8

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* Used to form a VCOM serial number */
#define	FIRMWARE_NAME	"ad559xr_iio"

/****** Macros used to form a VCOM serial number ******/
#if !defined(PLATFORM_NAME)
#define PLATFORM_NAME	HW_CARRIER_NAME
#endif

//#define SYSTEM_CONFIG_DISABLED // Uncomment this to disable system config
#if !defined(SYSTEM_CONFIG_DISABLED)
#define SYSTEM_CONFIG_ENABLED
#else
/* Change the device name here
 * to either AD5592R or AD5593R */
#define ACTIVE_DEVICE_NAME AD5593R
#endif

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

/* Uncomment this if you want to enable only the GPIO Subsytem */
//#define ONLY_GPIO_SUBSYSTEM

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
//#define USE_SDRAM  	// Uncomment to use SDRAM as data buffer

/* Slave Address for AD5593R */
#define AD5593R_A0_STATE 0
#define AD5593R_I2C(x) (0x10 | (x & 0x01))

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
extern struct no_os_uart_desc *uart_desc;
extern struct no_os_eeprom_desc *eeprom_desc;
extern struct no_os_pwm_desc *pwm_desc;
extern struct no_os_uart_desc *uart_console_stdio_desc;
int32_t init_system(void);
int32_t ad559xr_trigger_handler(struct iio_device_data *iio_dev_data);

#endif //APP_CONFIG_H
