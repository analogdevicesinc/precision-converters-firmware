/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD4130 device applications
******************************************************************************
* Copyright (c) 2020-2022 Analog Devices, Inc.
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

/* List of supported platforms */
#define	MBED_PLATFORM		1

/* List of data capture modes for AD4130 device */
#define BURST_DATA_CAPTURE			0
#define CONTINUOUS_DATA_CAPTURE		1
#define FIFO_DATA_CAPTURE			2

/* List of demo mode configurations */
#define		USER_DEFAULT_CONFIG		0
#define		RTD_2WIRE_CONFIG		1
#define		RTD_3WIRE_CONFIG		2
#define		RTD_4WIRE_CONFIG		3
#define		THERMISTOR_CONFIG		4
#define		THERMOCOUPLE_CONFIG		5
#define		LOADCELL_CONFIG			6
#define		ECG_CONFIG				7
#define		NOISE_TEST_CONFIG		8
#define		POWER_TEST_CONFIG		9

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/******************************************************************************/

/* Name of active device */
#define ACTIVE_DEVICE_NAME	"ad4130-8"

/* Select the AD4130 package type (required for interrupt source) */
#define AD4130_WLCSP_PACKAGE_TYPE
//#define AD4130_LFCSP_PACKAGE_TYPE

/* Select the active platform (default is Mbed) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		MBED_PLATFORM
#endif

/* Select the demo mode configuration (default is user config) */
#if !defined(ACTIVE_DEMO_MODE_CONFIG)
#define ACTIVE_DEMO_MODE_CONFIG		USER_DEFAULT_CONFIG
#endif

/* Select the ADC data capture mode (default is CC mode) */
#if !defined(DATA_CAPTURE_MODE)
#define DATA_CAPTURE_MODE	CONTINUOUS_DATA_CAPTURE
#endif

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"

#define HW_CARRIER_NAME		STR(TARGET_NAME)

/* Redefine the init params structure mapping w.r.t. platform */
#define ext_int_extra_init_params mbed_ext_int_extra_init_params
#define uart_extra_init_params mbed_uart_extra_init_params
#define spi_extra_init_params mbed_spi_extra_init_params
#define i2c_extra_init_params mbed_i2c_extra_init_params
#define trigger_gpio_irq_extra_params mbed_trigger_gpio_irq_init_params
#define trigger_gpio_extra_init_params mbed_trigger_gpio_extra_init_params
#define trigger_gpio_ops mbed_gpio_ops
#define irq_ops		mbed_gpio_irq_ops
#define gpio_ops	mbed_gpio_ops
#define spi_ops		mbed_spi_ops
#define i2c_ops		mbed_i2c_ops
#define trigger_gpio_irq_ops mbed_gpio_irq_ops
#define trigger_gpio_handle 0	// Unused macro
#define TRIGGER_GPIO_PORT 0  // Unused macro
#define TRIGGER_GPIO_PIN  CONV_MON
#define TRIGGER_INT_ID	GPIO_IRQ_ID1
#else
#error "No/Invalid active platform selected"
#endif

/* Expected HW ID */
#define HW_MEZZANINE_NAME	"EV-AD4130-8ASDZ-U1"

/* Include user config files and params according to active/selected
 * demo mode config */
#if (ACTIVE_DEMO_MODE_CONFIG == USER_DEFAULT_CONFIG)
#include "ad4130_user_config.h"
#define ad4130_init_params	ad4130_user_config_params
#elif ((ACTIVE_DEMO_MODE_CONFIG == RTD_2WIRE_CONFIG) || \
(ACTIVE_DEMO_MODE_CONFIG == RTD_3WIRE_CONFIG) || \
(ACTIVE_DEMO_MODE_CONFIG == RTD_4WIRE_CONFIG))
#include "ad4130_rtd_config.h"
#define ad4130_init_params	ad4130_rtd_config_params
#elif (ACTIVE_DEMO_MODE_CONFIG == THERMISTOR_CONFIG)
#include "ad4130_thermistor_config.h"
#define ad4130_init_params	ad4130_thermistor_config_params
#elif (ACTIVE_DEMO_MODE_CONFIG == THERMOCOUPLE_CONFIG)
#include "ad4130_thermocouple_config.h"
#define ad4130_init_params	ad4130_thermocouple_config_params
#elif (ACTIVE_DEMO_MODE_CONFIG == LOADCELL_CONFIG)
#include "ad4130_loadcell_config.h"
#define ad4130_init_params	ad4130_loadcell_config_params
#elif (ACTIVE_DEMO_MODE_CONFIG == ECG_CONFIG)
#include "ad4130_ecg_config.h"
#define ad4130_init_params	ad4130_ecg_config_params
#elif (ACTIVE_DEMO_MODE_CONFIG == NOISE_TEST_CONFIG)
#include "ad4130_noise_test_config.h"
#define ad4130_init_params	ad4130_noise_test_config_params
#elif (ACTIVE_DEMO_MODE_CONFIG == POWER_TEST_CONFIG)
#include "ad4130_power_test_config.h"
#define ad4130_init_params	ad4130_power_test_config_params
#else
#include "ad4130_user_config.h"
#define ad4130_init_params	ad4130_user_config_params
#warning "No/Invalid active demo config selected, user config used as default"
#endif

/* ADC resolution for active device */
#define ADC_RESOLUTION		24

/* Number of ADC presets/setups */
#define ADC_PRESETS			7

/* Number of actually used ADC channels.
 * Note : There can be max 16 channels in the device sequencer but since
 * input pairs can be only 8 or 16, either 8 or 16 channels are exposed
 * out, based on the user selected channel configuration.
 * The auxilary inputs(such as temperature, ref, etc) are not used.
 * */
#define	ADC_DIFFERENTIAL_CHNS	8
#define	ADC_PSEUDO_DIFF_CHNS	16

/* Default ADC reference voltages for each reference source */
#define AD4130_REFIN1_VOLTAGE			2.5
#define AD4130_REFIN2_VOLTAGE			2.5
#define AD4130_AVDD_VOLTAGE				3.3		// 3.3 or 1.8
#define AD4170_2_5V_INT_REF_VOLTAGE		2.5
#define AD4170_1_25V_INT_REF_VOLTAGE	1.25

/* ADC max count (full scale value) for unipolar inputs */
#define ADC_MAX_COUNT_UNIPOLAR	(uint32_t)((1 << ADC_RESOLUTION) - 1)

/* ADC max count (full scale value) for bipolar inputs */
#define ADC_MAX_COUNT_BIPOLAR	(uint32_t)(1 << (ADC_RESOLUTION-1))

/****** Macros used to form a VCOM serial number ******/
#if !defined(FIRMWARE_NAME)
#define	FIRMWARE_NAME	"ad4130_iio_application"
#endif

#if !defined(DEVICE_NAME)
#define DEVICE_NAME		"DEV_AD4130"
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

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
//#define USE_SDRAM		// Uncomment to use SDRAM for data buffer

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

extern struct no_os_uart_desc *uart_desc;
extern struct no_os_gpio_desc *trigger_gpio_desc;
extern struct no_os_spi_init_param spi_init_params;
extern struct no_os_eeprom_desc *eeprom_desc;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;

int32_t init_system(void);
uint8_t get_eeprom_detected_dev_addr(void);
bool is_eeprom_valid_dev_addr_detected(void);
void ad4130_fifo_event_handler(void *ctx);

#endif //_APP_CONFIG_H_
