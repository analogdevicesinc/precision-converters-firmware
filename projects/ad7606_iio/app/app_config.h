/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Header file for application configurations (platform-agnostic)
******************************************************************************
* Copyright (c) 2020-2023 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

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

/* List of data capture modes */
#define BURST_DATA_CAPTURE			0
#define CONTINUOUS_DATA_CAPTURE		1

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

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

// **** Note for User: ACTIVE_DEVICE selection ****//
/* Define the device type here from the list of below device type defines
 * (one at a time. Defining more than one device can result into compile error).
 * e.g. #define DEV_AD7606B -> This will make AD7606B as an ACTIVE_DEVICE.
 * The ACTIVE_DEVICE is default set to AD7606B if device type is not defined.
 * */

//#define DEV_AD7606B

#if defined(DEV_AD7605_4)
#define ACTIVE_DEVICE		ID_AD7605_4
#define ACTIVE_DEVICE_NAME	"ad7605-4"
#elif defined(DEV_AD7606_4)
#define ACTIVE_DEVICE		ID_AD7606_4
#define ACTIVE_DEVICE_NAME	"ad7606-4"
#elif defined(DEV_AD7606_6)
#define ACTIVE_DEVICE		ID_AD7606_6
#define ACTIVE_DEVICE_NAME	"ad7606-6"
#elif defined(DEV_AD7606_8)
#define ACTIVE_DEVICE		ID_AD7606_8
#define ACTIVE_DEVICE_NAME	"ad7606-8"
#elif defined(DEV_AD7606B)
#define ACTIVE_DEVICE		ID_AD7606B
#define ACTIVE_DEVICE_NAME	"ad7606b"
#elif defined(DEV_AD7606C_16)
#define ACTIVE_DEVICE		ID_AD7606C_16
#define ACTIVE_DEVICE_NAME	"ad7606c-16"
#elif defined(DEV_AD7606C_18)
#define ACTIVE_DEVICE		ID_AD7606C_18
#define ACTIVE_DEVICE_NAME	"ad7606c-18"
#elif defined(DEV_AD7608)
#define ACTIVE_DEVICE		ID_AD7608
#define ACTIVE_DEVICE_NAME	"ad7608"
#elif defined(DEV_AD7609)
#define ACTIVE_DEVICE		ID_AD7609
#define ACTIVE_DEVICE_NAME	"ad7609"
#else
#warning No/Unsupported ADxxxxy symbol defined. AD7606B defined
#define DEV_AD7606B
#define ACTIVE_DEVICE		ID_AD7606B
#define ACTIVE_DEVICE_NAME	"ad7606b"
#endif

#if defined(DEV_AD7605_4)
#define	AD7606X_ADC_CHANNELS	4
#define AD7606X_ADC_RESOLUTION	16
#elif defined(DEV_AD7606_4)
#define	AD7606X_ADC_CHANNELS	4
#define AD7606X_ADC_RESOLUTION	16
#elif defined(DEV_AD7606_6)
#define	AD7606X_ADC_CHANNELS	6
#define AD7606X_ADC_RESOLUTION	16
#elif defined(DEV_AD7606_8)
#define	AD7606X_ADC_CHANNELS	8
#define AD7606X_ADC_RESOLUTION	16
#elif defined(DEV_AD7606B)
#define	AD7606X_ADC_CHANNELS	8
#define AD7606X_ADC_RESOLUTION	16
#elif defined(DEV_AD7606C_16)
#define	AD7606X_ADC_CHANNELS	8
#define AD7606X_ADC_RESOLUTION	16
#elif defined(DEV_AD7606C_18)
#define	AD7606X_ADC_CHANNELS	8
#define AD7606X_ADC_RESOLUTION	18
#elif defined(DEV_AD7608)
#define	AD7606X_ADC_CHANNELS	8
#define AD7606X_ADC_RESOLUTION	18
#elif defined(DEV_AD7609)
#define	AD7606X_ADC_CHANNELS	8
#define AD7606X_ADC_RESOLUTION	18
#else
/* Default config for AD7606B */
#define	AD7606X_ADC_CHANNELS	8
#define AD7606X_ADC_RESOLUTION	16
#endif

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"

#define HW_CARRIER_NAME		STR(TARGET_NAME)

/* Redefine the init params structure mapping w.r.t. platform */
#define pwm_extra_init_params mbed_pwm_extra_init_params
#if defined(USE_VIRTUAL_COM_PORT)
#define uart_extra_init_params mbed_vcom_extra_init_params
#define uart_ops mbed_virtual_com_ops
#else
#define uart_extra_init_params mbed_uart_extra_init_params
#define uart_ops mbed_uart_ops
#endif
#define spi_extra_init_params mbed_spi_extra_init_params
#define trigger_gpio_irq_extra_params mbed_trigger_gpio_irq_init_params
#define trigger_gpio_extra_init_params mbed_trigger_gpio_extra_init_params
#define reset_gpio_extra_init_params mbed_reset_gpio_extra_init_params
#define convst_gpio_extra_init_params mbed_convst_gpio_extra_init_params
#define busy_gpio_extra_init_params mbed_busy_gpio_extra_init_params
#define osr0_gpio_extra_init_params mbed_osr0_gpio_extra_init_params
#define osr1_gpio_extra_init_params mbed_osr1_gpio_extra_init_params
#define osr2_gpio_extra_init_params mbed_osr2_gpio_extra_init_params
#define range_gpio_extra_init_params mbed_range_gpio_extra_init_params
#define stdby_gpio_extra_init_params mbed_stdby_gpio_extra_init_params
#define trigger_gpio_ops mbed_gpio_ops
#define irq_ops mbed_gpio_irq_ops
#define gpio_ops mbed_gpio_ops
#define spi_ops mbed_spi_ops
#define trigger_gpio_irq_ops mbed_gpio_irq_ops
#define trigger_gpio_handle 0	// Unused macro
#define IRQ_INT_ID GPIO_IRQ_ID1
#define TRIGGER_GPIO_PORT 0  // Unused macro
#define TRIGGER_GPIO_PIN  PWM_TRIGGER
#define TRIGGER_INT_ID	GPIO_IRQ_ID1
#else
#error "No/Invalid active platform selected"
#endif

/* ADC max count (full scale value) for unipolar inputs */
#define ADC_MAX_COUNT_UNIPOLAR	(uint32_t)((1 << AD7606X_ADC_RESOLUTION) - 1)

/* ADC max count (full scale value) for bipolar inputs */
#define ADC_MAX_COUNT_BIPOLAR	(uint32_t)(1 << (AD7606X_ADC_RESOLUTION-1))

/* Bytes per sample. This count should divide the total 256 bytes into 'n' equivalent
 * ADC samples as IIO library requests only 256bytes of data at a time in a given
 * data read query.
 * For 1 to 8-bit ADC, bytes per sample = 1 (2^0)
 * For 9 to 16-bit ADC, bytes per sample = 2 (2^1)
 * For 17 to 32-bit ADC, bytes per sample = 4 (2^2)
 **/
#if (AD7606X_ADC_RESOLUTION == 18)
#define	BYTES_PER_SAMPLE	sizeof(uint32_t)	// For ADC resolution of 18-bits
#else
#define	BYTES_PER_SAMPLE	sizeof(uint16_t)	// For ADC resolution of 16-bits
#endif

/****** Macros used to form a VCOM serial number ******/
#if !defined(FIRMWARE_NAME)
#define	FIRMWARE_NAME	"ad7606_iio"
#endif

#if !defined(DEVICE_NAME)
#define DEVICE_NAME		"DEV_AD7606B"
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

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
//#define USE_SDRAM		// Uncomment to use SDRAM for data buffer

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct no_os_uart_desc *uart_desc;
extern struct no_os_gpio_desc *led_gpio_desc;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;

int32_t init_system(void);
int32_t init_pwm_trigger(void);

#endif /* _APP_CONFIG_H_ */
