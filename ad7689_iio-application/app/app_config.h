/***************************************************************************//*
 * @file    app_config.h
 * @brief   Header file for application configurations (platform-agnostic)
******************************************************************************
 * Copyright (c) 2021-22 Analog Devices, Inc.
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

/* List of data capture modes */
#define CONTINUOUS_DATA_CAPTURE		0
#define BURST_DATA_CAPTURE			1

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/******************************************************************************/

/**** ACTIVE_DEVICE selection *****
* Define the device type here from the available list of devices (one at a time)
* e.g. #define DEV_AD7689 -> This will make AD7689 as an ACTIVE_DEVICE.
**/
//#define	DEV_AD7689

/* Name of the active device */
#if defined(DEV_AD7689)
#define ACTIVE_DEVICE		ID_AD7689
#define ACTIVE_DEVICE_NAME	"ad7689"
#elif defined(DEV_AD7682)
#define ACTIVE_DEVICE		ID_AD7682
#define ACTIVE_DEVICE_NAME	"ad7682"
#elif defined(DEV_AD7949)
#define ACTIVE_DEVICE		ID_AD7949
#define ACTIVE_DEVICE_NAME	"ad7949"
#elif defined(DEV_AD7699)
#define ACTIVE_DEVICE		ID_AD7699
#define ACTIVE_DEVICE_NAME	"ad7699"
#else
#warning No/Unsupported device selected. AD7689 used as default.
#define ACTIVE_DEVICE		ID_AD7689
#define ACTIVE_DEVICE_NAME	"ad7689"
#endif

/* Select the active platform (default is Mbed) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		MBED_PLATFORM
#endif

/* Select the ADC data capture mode (default is CC mode) */
#if !defined(DATA_CAPTURE_MODE)
#define DATA_CAPTURE_MODE	CONTINUOUS_DATA_CAPTURE
#endif

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"

#define HW_CARRIER_NAME		STR(TARGET_NAME)

/* Redefine the init params structure mapping w.r.t. platform */
#define pwm_extra_init_params mbed_pwm_extra_init_params
#define uart_extra_init_params mbed_uart_extra_init_params
#define spi_extra_init_params mbed_spi_extra_init_params
#define i2c_extra_init_params mbed_i2c_extra_init_params
#define trigger_gpio_irq_extra_params mbed_trigger_gpio_irq_init_params
#define trigger_gpio_extra_init_params mbed_trigger_gpio_extra_init_params
#define trigger_gpio_ops mbed_gpio_ops
#define irq_ops mbed_gpio_irq_ops
#define gpio_ops mbed_gpio_ops
#define spi_ops mbed_spi_ops
#define i2c_ops mbed_i2c_ops
#define trigger_gpio_irq_ops mbed_gpio_irq_ops
#define trigger_gpio_handle 0	// Unused macro
#define IRQ_INT_ID GPIO_IRQ_ID1
#define TRIGGER_GPIO_PORT 0  // Unused macro
#define TRIGGER_GPIO_PIN  PWM_TRIGGER
#define TRIGGER_INT_ID	GPIO_IRQ_ID1
#else
#error "No/Invalid active platform selected"
#endif

/* Expected HW ID */
#define HW_MEZZANINE_NAME	"EVAL-AD7689-ARDZ"

/* ADC resolution for active device */
#if defined(DEV_AD7949)
#define ADC_RESOLUTION		14
#else
#define ADC_RESOLUTION		16
#endif

/* ADC max count (full scale value) for unipolar inputs */
#define ADC_MAX_COUNT_UNIPOLAR	(uint32_t)((1 << ADC_RESOLUTION) - 1)

/* ADC max count (full scale value) for bipolar inputs */
#define ADC_MAX_COUNT_BIPOLAR	(uint32_t)(1 << (ADC_RESOLUTION-1))

/* Max number of ADC channels */
#if defined(DEV_AD7682)
#define TEMPERATURE_CHN		4
#define ADC_CHN_COUNT		5	// Chn0-3 + 1 temperature channel
#else
#define TEMPERATURE_CHN		8
#define ADC_CHN_COUNT		9	// Chn0-7 + 1 temperature channel
#endif

/****** Macros used to form a VCOM serial number ******/
#if !defined(DEVICE_NAME)
#define DEVICE_NAME		"DEV_AD7689"
#endif

/* Used to form a VCOM serial number */
#if !defined(FIRMWARE_NAME)
#define	FIRMWARE_NAME	"ad7689_iio_application"
#endif

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

/* Default baud rate for IIO UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
//#define USE_SDRAM		// Uncomment to use SDRAM for data buffer

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct no_os_uart_desc *uart_desc;
extern struct no_os_gpio_desc *led_gpio_desc;
extern struct no_os_eeprom_desc *eeprom_desc;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;

int32_t init_system(void);
int32_t init_pwm_trigger(void);
uint8_t get_eeprom_detected_dev_addr(void);
bool is_eeprom_valid_dev_addr_detected(void);

#endif /* _APP_CONFIG_H_ */
