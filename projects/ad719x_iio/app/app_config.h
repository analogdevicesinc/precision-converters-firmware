/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for ad719x IIO firmware application
******************************************************************************
* Copyright (c) 2021-23 Analog Devices, Inc.
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

/* List of data capture modes for AD719X device */
#define CONTINUOUS_DATA_CAPTURE		0
#define BURST_DATA_CAPTURE			1

/* List of selectable modes for AD719X device */
#define NORMAL_MODE           0
#define NOISE_TEST            1
#define FAST_50HZ_TEST        2

/* List of polarity modes */
#define BIPOLAR_MODE           0
#define UNIPOLAR_MODE          1

/* List of Input Configuration */
#define DIFFERENTIAL_INPUT                 0
#define PSEUDO_DIFFERENTIAL_INPUT          1

/* Select the active platform (default is Mbed) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		MBED_PLATFORM
#endif

/* Select the ADC data capture mode (default is CC mode) */
#if !defined(DATA_CAPTURE_MODE)
#define DATA_CAPTURE_MODE	 CONTINUOUS_DATA_CAPTURE
#endif

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"
#define HW_CARRIER_NAME		TARGET_NAME

/* Redefine the init params structure mapping w.r.t. platform */
#define spi_extra_init_params         mbed_spi_extra_init_params
#if defined(USE_VIRTUAL_COM_PORT)
#define uart_extra_init_params mbed_vcom_extra_init_params
#define uart_ops mbed_virtual_com_ops
#else
#define uart_extra_init_params mbed_uart_extra_init_params
#define uart_ops mbed_uart_ops
#endif
#define i2c_extra_init_params         mbed_i2c_extra_init_params
#define gpio_sync_init_params         mbed_gpio_sync_extra_init_params
#define trigger_gpio_irq_extra_params mbed_trigger_gpio_irq_init_params
#define gpio_ops                      mbed_gpio_ops
#define irq_ops                       mbed_irq_ops
#define spi_ops                       mbed_spi_ops
#define i2c_ops                       mbed_i2c_ops
#define trigger_gpio_irq_ops          mbed_gpio_irq_ops
#define trigger_gpio_handle           0 // Unused macro
#define TRIGGER_INT_ID	              GPIO_IRQ_ID1
#else
#error "No/Invalid active platform selected"
#endif

// **** Note for User on selection of Active Device ****//
/* Define the device type here from the list of below device type defines
 * (one at a time. Defining more than one device can result into compile error).
 * e.g. #define DEV_AD7193 -> This will make AD7193 as an active device.
 * The active device is default set to AD7193 if device type is not defined.
 * */
//#define DEV_AD7193

#if defined(DEV_AD7190)
#define ACTIVE_DEVICE_NAME	"ad7190"
#define DEVICE_NAME			"DEV_AD7190"
#define ACTIVE_DEVICE_ID	 ID_AD7190
#define HW_MEZZANINE_NAME	"EVAL-AD7190-ASDZ"
#elif defined(DEV_AD7192)
#define ACTIVE_DEVICE_NAME	"ad7192"
#define DEVICE_NAME			"DEV_AD7192"
#define ACTIVE_DEVICE_ID	 ID_AD7192
#define HW_MEZZANINE_NAME	"EVAL-AD7192-ASDZ"
#elif defined(DEV_AD7193)
#define ACTIVE_DEVICE_NAME	"ad7193"
#define DEVICE_NAME			"DEV_AD7193"
#define ACTIVE_DEVICE_ID	 ID_AD7193
#define HW_MEZZANINE_NAME	"EVAL-AD7193-ASDZ"
#elif defined(DEV_AD7195)
#define ACTIVE_DEVICE_NAME	"ad7195"
#define DEVICE_NAME			"DEV_AD7195"
#define ACTIVE_DEVICE_ID	 ID_AD7195
#define HW_MEZZANINE_NAME	"EVAL-AD7195-ASDZ"
#else
#warning No/Unsupported ADxxxxy symbol defined. AD7193 defined
#define DEV_AD7193
#define ACTIVE_DEVICE_NAME	"ad7193"
#define DEVICE_NAME			"DEV_AD7193"
#define ACTIVE_DEVICE_ID	 ID_AD7193
#define HW_MEZZANINE_NAME	"EVAL-AD7193-ASDZ"
#endif // Device Select (Active Device name definition)

#if defined(DEV_AD7190) || defined(DEV_AD7192) || defined(DEV_AD7195)
#define	NO_OF_CHANNELS		4
#else
#define NO_OF_CHANNELS		8
#endif
#define ADC_RESOLUTION		24

/* Active Mode */
#if !defined(ACTIVE_MODE)
#define ACTIVE_MODE         NORMAL_MODE
#endif

/* **** Note for User: Unipolar/Bipolar Configuration ****
 * The analog input to the AD7190/2/3/5 can accept either unipolar or
 * bipolar input voltage ranges. A bipolar input range does not
 * imply that the part can tolerate negative voltages with respect
 * to system AGND. If the ADC is configured for unipolar operation,
 * the output code is natural (straight) binary. When the ADC
 * is configured for bipolar operation, the output code is offset binary.
 *
 * The default is unipolar mode when in normal mode.
 * Edit the POLARITY_CONFIG macro to change to polarity settings.
 * e.g. #if (ACTIVE_MODE == NORMAL_MODE)
 *      #define POLARITY_CONFIG BIPOLAR_MODE
 * This will configure all the channels as bipolar mode.
 * */
#if (ACTIVE_MODE == NORMAL_MODE)
#define POLARITY_CONFIG UNIPOLAR_MODE
#else
#define POLARITY_CONFIG BIPOLAR_MODE
#endif

/* **** Note for User: Pseudo Differential Input Configuration ****
 * The analog inputs of AD7193 can be configured as differential
 * inputs or pseudo differential analog inputs.
 *
 * In pseudo differential mode, AD7190/2/5 has four pseudo differential
 * analog inputs. In case of fully differential input, the AD7190/2/5 is
 * configured to have two differential analog inputs.
 *
 * In case of pseudo differential input, the AD7193 is configured to have eight
 * pseudo differential analog inputs. In case of differential input,
 * the AD7193 is configured to have four differential analog inputs.
 *
 * The default is pseudo differential input when in normal mode.
 * Edit the INPUT_CONFIG macro to change to configuration.
 * e.g. #if (ACTIVE_MODE == NORMAL_MODE)
 *      #define INPUT_CONFIG DIFFERENTIAL_INPUT
 * This will enable the differential mode for all the channels.
 * */
#if (ACTIVE_MODE == NORMAL_MODE)
#define INPUT_CONFIG PSEUDO_DIFFERENTIAL_INPUT
#else
#define INPUT_CONFIG DIFFERENTIAL_INPUT
#endif

/* ADC max count (full scale value) for unipolar inputs */
#define ADC_MAX_COUNT_UNIPOLAR	(uint32_t)((1 << ADC_RESOLUTION) - 1)

/* ADC max count (full scale value) for bipolar inputs */
#define ADC_MAX_COUNT_BIPOLAR	(uint32_t)(1 << (ADC_RESOLUTION-1))

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* Used to form a VCOM serial number */
#define	FIRMWARE_NAME	"ad719x_iio"

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
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;
extern struct no_os_eeprom_desc *eeprom_desc;

int32_t init_system(void);
/* callback function in burst mode */
extern void burst_capture_callback(void *context);

#endif //APP_CONFIG_H
