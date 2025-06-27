/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for ad7191 IIO firmware application
******************************************************************************
* Copyright (c) 2024 Analog Devices, Inc.
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

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* List of supported platforms*/
#define STM32_PLATFORM      1

/* List of data capture modes for AD7191 device */
#define CONTINUOUS_DATA_CAPTURE		0
#define BURST_DATA_CAPTURE			1

/* Select the active platform (default is STM32) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

/* Select the ADC data capture mode (default is CC mode) */
#if !defined(DATA_CAPTURE_MODE)
#define DATA_CAPTURE_MODE	 CONTINUOUS_DATA_CAPTURE
#endif

/* Only Physical COM Port is supported in STM32 Platform */
#if !defined(USE_PHY_COM_PORT)
#define USE_PHY_COM_PORT
#endif

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
/* Redefine the init params structure mapping wrt platform */
#define spi_extra_init_params   	stm32_spi_extra_init_params
#define uart_extra_init_params 		stm32_uart_extra_init_params
#define gpio_ops			stm32_gpio_ops
#define spi_ops            stm32_spi_ops
#define i2c_ops                     stm32_i2c_ops
#define uart_ops stm32_uart_ops
#define irq_platform_ops stm32_gpio_irq_ops
#define IRQ_INT_ID RDY_PIN
#define trigger_gpio_irq_ops          stm32_gpio_irq_ops
#define trigger_gpio_handle           0 // Unused macro
#define trigger_gpio_irq_extra_params stm32_trigger_gpio_irq_init_params
#else
#error "No/Invalid active platform selected"
#endif

#define ACTIVE_DEVICE_NAME	"ad7191"
#define DEVICE_NAME			"DEV_AD7191"
#define HW_MEZZANINE_NAME	"EVAL-AD7191-ASDZ" // TODO- Check hw_mezzanine_name with Apps

/* ADC Resolution */
#define ADC_RESOLUTION		24

/* ADC max count (full scale value) for unipolar inputs */
#define ADC_MAX_COUNT_UNIPOLAR	(uint32_t)((1 << ADC_RESOLUTION) - 1)

/* ADC max count (full scale value) for bipolar inputs */
#define ADC_MAX_COUNT_BIPOLAR	(uint32_t)(1 << (ADC_RESOLUTION-1))

/* Default ADC Vref voltage */
#define AD7191_DEFAULT_REF_VOLTAGE 2.5

/* Additional bit macros */
#define BYTES_PER_SAMPLE        sizeof(uint32_t)
#define STORAGE_BITS            (BYTES_PER_SAMPLE) * 8

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* Used to form a VCOM serial number */
#define	FIRMWARE_NAME	"ad7191_iio"

/****** Macros used to form a VCOM serial number ******/
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

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
//#define USE_SDRAM  	// Uncomment to use SDRAM as data buffer

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
extern struct no_os_uart_desc *uart_desc;
extern struct no_os_gpio_desc *rdy_gpio;
extern struct no_os_gpio_desc *csb_gpio;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;

int32_t init_system(void);
void data_capture_callback(void *ctx);

#endif //APP_CONFIG_H
