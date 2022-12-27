/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD4696 device applications
******************************************************************************
* Copyright (c) 2021-22 Analog Devices, Inc.
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
#define	MBED_PLATFORM		1

/* List of data capture modes for AD4696 device */
#define CONTINUOUS_DATA_CAPTURE		0
#define BURST_DATA_CAPTURE			1

/* List of available polarity modes */
#define UNIPOLAR_MODE           0
#define PSEUDO_BIPOLAR_MODE     1

/* Select the ADC data capture mode (default is CC mode) */
#if !defined(DATA_CAPTURE_MODE)
#define DATA_CAPTURE_MODE	  CONTINUOUS_DATA_CAPTURE
#endif

/* Select the active platform (default is Mbed) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		MBED_PLATFORM
#endif

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"
#define HW_CARRIER_NAME		STR(TARGET_NAME)

/* Redefine the init params structure mapping w.r.t. platform */
#define bsy_gpio_extra_init_params mbed_gpio_bsy_extra_init_params
#define trigger_gpio_irq_extra_params mbed_trigger_gpio_irq_init_params
#define uart_extra_init_params mbed_uart_extra_init_params
#define spi_extra_init_params mbed_spi_extra_init_params
#define pwm_extra_init_params mbed_pwm_extra_init_params
#define gpio_ops                    mbed_gpio_ops
#define spi_ops                     mbed_spi_ops
#define trigger_gpio_irq_ops        mbed_gpio_irq_ops
#define trigger_gpio_handle         0 // Unused macro
#define TRIGGER_GPIO_PORT           0 // Unused macro
#define TRIGGER_INT_ID	            GPIO_IRQ_ID1
#else
#error "No/Invalid active platform selected"
#endif

// **** Note for User: ACTIVE_DEVICE selection **** //
/* Define the device type here from the list of below device type defines
 * (one at a time. Defining more than one device can result into compile error).
 * e.g. #define DEV_AD4696 -> This will make AD4696 as an ACTIVE_DEVICE.
 * The ACTIVE_DEVICE is default set to AD4696 if device type is not defined.
 * */

//#define DEV_AD4696

#if defined(DEV_AD4696)
#define ACTIVE_DEVICE		ID_AD4696
#define ACTIVE_DEVICE_NAME	"ad4696"
#elif defined(DEV_AD4695)
#define ACTIVE_DEVICE		ID_AD4695
#define ACTIVE_DEVICE_NAME	"ad4695"
#else
#warning No/Unsupported ADxxxxy symbol defined. AD4696 defined
#define DEV_AD4696
#define ACTIVE_DEVICE		ID_AD4696
#define ACTIVE_DEVICE_NAME	"ad4696"
#endif

#if defined(DEV_AD4696)
#define	NO_OF_CHANNELS		16
#define ADC_RESOLUTION		16
#elif defined(DEV_AD4695)
#define	NO_OF_CHANNELS		16
#define ADC_RESOLUTION		16
#else
/* Default config for AD4696 */
#define	NO_OF_CHANNELS		16
#define ADC_RESOLUTION		16
#endif

// **** Note for User: Polarity Mode selection **** //
/* Since the pin pairing option is same for all the channels in
 * standard sequencer mode, hence polarity mode for all the
 * channels is also kept same to avoid stale ADC output codes.
 * Make sure to change the JP6 jumper position on the Eval board to A
 * to use the PSEUDO_BIPOLAR_MODE.
 *
 * Select Pseudo bipolar mode (default is unipolar mode) for all the channels.
 * e.g. #define PSEUDO_BIPOLAR_MODE -> This will enable the PSEUDO_BIPOLAR_MODE
 * for all the channels.
 * */
#define DEFAULT_POLARITY_MODE    PSEUDO_BIPOLAR_MODE

/* Pins to be used an interrupt to trigger callback function */
#define EXT_TRIGGER_PIN 	     BUSY_PIN

/* ADC max count (full scale value) for unipolar inputs */
#define ADC_MAX_COUNT_UNIPOLAR	(uint32_t)((1 << ADC_RESOLUTION) - 1)

/* ADC max count (full scale value) for bipolar inputs */
#define ADC_MAX_COUNT_BIPOLAR	(uint32_t)(1 << (ADC_RESOLUTION-1))

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/****** Macros used to form a VCOM serial number ******/
#if !defined(DEVICE_NAME)
#define DEVICE_NAME		"DEV_AD4696"
#endif

/* Used to form a VCOM serial number */
#define	FIRMWARE_NAME	"ad4696_iio_application"

#if !defined(PLATFORM_NAME)
#define PLATFORM_NAME	HW_CARRIER_NAME
#endif
/******/

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
/* Below USB configurations (VID and PID) are owned and assigned by ADI.
 * If intended to distribute software further, use the VID and PID owned by your
 * organization */
#define VIRTUAL_COM_PORT_VID	0x0456
#define VIRTUAL_COM_PORT_PID	0xb66c
/* Serial number string is formed as: application name + device (target) name + platform (host) name */
#define VIRTUAL_COM_SERIAL_NUM	(FIRMWARE_NAME "_" DEVICE_NAME "_" PLATFORM_NAME)
#endif

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
//#define USE_SDRAM  	// Uncomment to use SDRAM as data buffer

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
extern struct no_os_uart_desc *uart_desc;
extern struct no_os_pwm_desc *pwm_desc;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;

/* Initializing system peripherals */
int32_t init_pwm(void);
int32_t init_system(void);
/* callback function in burst mode */
extern void burst_capture_callback(void *context);

#endif //APP_CONFIG_H