/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD738x device application
******************************************************************************
* Copyright (c) 2022-23 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* List of supported platforms */
#define	MBED_PLATFORM		1

/* List of supported data capture modes */
#define BURST_DATA_CAPTURE			0
#define CONTINUOUS_DATA_CAPTURE		1

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/******************************************************************************/

/* Name of active device */
#define ACTIVE_DEVICE_NAME	"ad7380-2"

/* Select the active platform (default is Mbed) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		MBED_PLATFORM
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

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"

#define HW_CARRIER_NAME		STR(TARGET_NAME)

/* Redefine the init params structure mapping w.r.t. platform */
#define pwm_extra_init_params mbed_pwm_extra_init_params
#define uart_extra_init_params mbed_uart_extra_init_params
#define vcom_extra_init_params mbed_vcom_extra_init_params
#define spi_extra_init_params mbed_spi_extra_init_params
#define trigger_gpio_extra_init_params mbed_trigger_gpio_extra_init_params
#define trigger_gpio_irq_extra_params mbed_trigger_gpio_irq_init_params
#define trigger_gpio_ops mbed_gpio_ops
#define spi_ops mbed_spi_ops
#define uart_ops mbed_uart_ops
#define vcom_ops mbed_virtual_com_ops
#define trigger_gpio_irq_ops mbed_gpio_irq_ops
#define trigger_gpio_handle	0	// Unused macro
#define UART_MODULE	0	// Unused macro
#define SPI_MODULE	0	// Unused macro
#define TRIGGER_GPIO_PORT 0  // Unused macro
#define TRIGGER_GPIO_PIN  PWM_TRIGGER
#define TRIGGER_INT_ID	GPIO_IRQ_ID1
#else
#error "No/Invalid active platform selected"
#endif

/* ADC resolution for active device */
#define ADC_RESOLUTION		16

/* Number of ADC channels */
#define ADC_CHANNELS		2

/* ADC reference voltage (Range: 2.5 to 3.3v) */
#define ADC_REF_VOLTAGE		3.3

/* ADC max count (full scale value) for unipolar inputs */
#define ADC_MAX_COUNT_UNIPOLAR	(uint32_t)((1 << ADC_RESOLUTION) - 1)

/* ADC max count (full scale value) for bipolar inputs */
#define ADC_MAX_COUNT_BIPOLAR	(uint32_t)(1 << (ADC_RESOLUTION-1))

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/****** Macros used to form a VCOM serial number ******/
#if !defined(DEVICE_NAME)
#define DEVICE_NAME		"DEV_AD7380_2"
#endif

/* Used to form a VCOM serial number */
#if !defined(FIRMWARE_NAME)
#define	FIRMWARE_NAME	"ad738x_iio"
#endif

#if !defined(PLATFORM_NAME)
#define PLATFORM_NAME	HW_CARRIER_NAME
#endif

/* Below USB configurations (VID and PID) are owned and assigned by ADI.
 * If intended to distribute software further, use the VID and PID owned by your
 * organization */
#define VIRTUAL_COM_PORT_VID	0x0456
#define VIRTUAL_COM_PORT_PID	0xb66c
/* Serial number string is formed as: application name + device (target) name + platform (host) name */
#define VIRTUAL_COM_SERIAL_NUM	(FIRMWARE_NAME "_" DEVICE_NAME "_" PLATFORM_NAME)

/* Check if any serial port available for use as console stdio port */
#if defined(USE_PHY_COM_PORT)
/* If PHY com is selected, VCOM or alternate PHY com port can act as a console stdio port */
#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif
#else
/* If VCOM is selected, PHY com port will/should act as a console stdio port */
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
//#define USE_SDRAM		// Uncomment to use SDRAM for data buffer

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

extern struct no_os_uart_desc *uart_iio_com_desc;
extern struct no_os_uart_desc *uart_console_stdio_desc;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;
extern struct no_os_spi_init_param spi_init_params;

int32_t init_system(void);
int32_t init_pwm_trigger(void);

#endif // APP_CONFIG_H_
