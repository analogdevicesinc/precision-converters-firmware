/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Header file for application configurations (platform-agnostic)
******************************************************************************
* Copyright (c) 2020-2023, 2025 Analog Devices, Inc.
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
#include <common_macros.h>

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* List of data capture modes */
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

// **** Note for User: ACTIVE_DEVICE selection ****//
/* Define the device type here from the list of below device type defines
 * (one at a time. Defining more than one device can result into compile error).
 * e.g. #define DEV_AD7606B -> This will make AD7606B as an ACTIVE_DEVICE.
 * The ACTIVE_DEVICE is default set to AD7606B if device type is not defined.
 * */

//#define DEV_AD7606C_16

#if defined(DEV_AD7605_4)
#define ACTIVE_DEVICE		ID_AD7605_4
#define ACTIVE_DEVICE_NAME	"ad7605-4"
#define DEVICE_NAME		    "DEV_AD7605_4"
#elif defined(DEV_AD7606_4)
#define ACTIVE_DEVICE		ID_AD7606_4
#define ACTIVE_DEVICE_NAME	"ad7606-4"
#define DEVICE_NAME		    "DEV_AD7606_4"
#elif defined(DEV_AD7606_6)
#define ACTIVE_DEVICE		ID_AD7606_6
#define ACTIVE_DEVICE_NAME	"ad7606-6"
#define DEVICE_NAME		    "DEV_AD7606_6"
#elif defined(DEV_AD7606_8)
#define ACTIVE_DEVICE		ID_AD7606_8
#define ACTIVE_DEVICE_NAME	"ad7606-8"
#define DEVICE_NAME		    "DEV_AD7606_8"
#elif defined(DEV_AD7606B)
#define ACTIVE_DEVICE		ID_AD7606B
#define ACTIVE_DEVICE_NAME	"ad7606b"
#define DEVICE_NAME		    "DEV_AD7606B"
#elif defined(DEV_AD7606C_16)
#define ACTIVE_DEVICE		ID_AD7606C_16
#define ACTIVE_DEVICE_NAME	"ad7606c-16"
#define DEVICE_NAME		    "DEV_AD7606C_16"
#elif defined(DEV_AD7606C_18)
#define ACTIVE_DEVICE		ID_AD7606C_18
#define ACTIVE_DEVICE_NAME	"ad7606c-18"
#define DEVICE_NAME		    "DEV_AD7606C_18"
#elif defined(DEV_AD7608)
#define ACTIVE_DEVICE		ID_AD7608
#define ACTIVE_DEVICE_NAME	"ad7608"
#define DEVICE_NAME		    "DEV_AD7608"
#elif defined(DEV_AD7609)
#define ACTIVE_DEVICE		ID_AD7609
#define ACTIVE_DEVICE_NAME	"ad7609"
#define DEVICE_NAME		    "DEV_AD7609"
#else
#warning No/Unsupported ADxxxxy symbol defined. AD7606B defined
#define DEV_AD7606B
#define ACTIVE_DEVICE		ID_AD7606B
#define ACTIVE_DEVICE_NAME	"ad7606b"
#define DEVICE_NAME		    "DEV_AD7606B"
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

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#define pwm_extra_init_params stm32_pwm_extra_init_params
#define vcom_extra_init_params stm32_vcom_extra_init_params
#define vcom_ops  stm32_usb_uart_ops
#define uart_extra_init_params stm32_uart_extra_init_params
#define uart_ops stm32_uart_ops
#define spi_extra_init_params stm32_spi_extra_init_params
#define i2c_extra_init_params stm32_i2c_extra_init_params
#define trigger_gpio_irq_extra_params stm32_trigger_gpio_irq_init_params
#define trigger_gpio_extra_init_params stm32_trigger_gpio_extra_init_params
#define reset_gpio_extra_init_params stm32_reset_gpio_extra_init_params
#define convst_gpio_extra_init_params stm32_convst_gpio_extra_init_params
#define busy_gpio_extra_init_params stm32_busy_gpio_extra_init_params
#define osr0_gpio_extra_init_params stm32_osr0_gpio_extra_init_params
#define osr1_gpio_extra_init_params stm32_osr1_gpio_extra_init_params
#define osr2_gpio_extra_init_params stm32_osr2_gpio_extra_init_params
#define range_gpio_extra_init_params stm32_range_gpio_extra_init_params
#define stdby_gpio_extra_init_params stm32_stdby_gpio_extra_init_params
#define pwm_gpio_extra_init_params  stm32_pwm_gpio_extra_init_params
#define trigger_gpio_ops stm32_gpio_ops
#define irq_ops stm32_gpio_irq_ops
#define gpio_ops stm32_gpio_ops
#define spi_ops stm32_spi_ops
#define i2c_ops stm32_i2c_ops
#define pwm_ops stm32_pwm_ops
#define trigger_gpio_irq_ops stm32_gpio_irq_ops
#define trigger_gpio_handle 0	// Unused macro
#define TRIGGER_GPIO_PIN  IRQ_INT_ID
#define TRIGGER_INT_ID	IRQ_INT_ID
#else
#error "No/Invalid active platform selected"
#endif

/* PWM period and duty cycle */
#define CONV_TRIGGER_PERIOD_NSEC		(((float)(1.0 / SAMPLING_RATE) * 1000000) * 1000)
#define CONV_TRIGGER_DUTY_CYCLE_NSEC	(CONV_TRIGGER_PERIOD_NSEC / 2)

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
#define	FIRMWARE_NAME	"ad7606_iio"

#if !defined(PLATFORM_NAME)
#define PLATFORM_NAME	HW_CARRIER_NAME
#endif

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

/* Below USB configurations (VID and PID) are owned and assigned by ADI.
 * If intended to distribute software further, use the VID and PID owned by your
 * organization */
#define VIRTUAL_COM_PORT_VID	0x0456
#define VIRTUAL_COM_PORT_PID	0xb66c
/* Serial number string is formed as: application name + device (target) name + platform (host) name */
#define VIRTUAL_COM_SERIAL_NUM	(FIRMWARE_NAME "_" DEVICE_NAME "_" STR(PLATFORM_NAME))

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
