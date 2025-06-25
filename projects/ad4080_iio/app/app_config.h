/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD4080 IIO firmware application
******************************************************************************
* Copyright (c) 2023-25 Analog Devices, Inc.
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
/* List of supported platforms */
#define STM32_PLATFORM		1

/* List of supported data capture modes */
#define BUSRT_DATA_CAPTURE			0
#define CONTINUOUS_DATA_CAPTURE		1

/* List of oscillator frequencies available  */
#define OSC_40M				0
#define OSC_20M				1
#define OSC_10M				2

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/* Name of active device */
#define ACTIVE_DEVICE_NAME	"ad4080"

#define	FIRMWARE_NAME		"ad4080_iio"

#define DEVICE_NAME			"DEV_AD4080"

#define HW_MEZZANINE_NAME	"EVAL-AD4080ARDZ"

/* Number of channels */
#define NUMBER_OF_CHANNELS 	1

/****************************** Configurations ********************************/
/* Select the active platform (default is STM32) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

#if (ACTIVE_PLATFORM != STM32_PLATFORM)
#error "No/Invalid active platform selected"
#endif /* ACTIVE_PLATFORM */

/* Select active oscillator */
#if !defined(ACTIVE_OSC)
#define ACTIVE_OSC			OSC_40M
#endif

/* Quad SPI support */
// Uncomment the following to use QSPI
//#define USE_QUAD_SPI

/* Determine state based on active osc selected */
#if (ACTIVE_OSC == OSC_40M)
#define OSC_40M_DEFAULT_STATE	NO_OS_GPIO_HIGH
#define OSC_20M_DEFAULT_STATE	NO_OS_GPIO_LOW
#define OSC_10M_DEFAULT_STATE	NO_OS_GPIO_LOW
#elif (ACTIVE_OSC == OSC_20M)
#define OSC_40M_DEFAULT_STATE	NO_OS_GPIO_LOW
#define OSC_20M_DEFAULT_STATE	NO_OS_GPIO_HIGH
#define OSC_10M_DEFAULT_STATE	NO_OS_GPIO_LOW
#else
#define OSC_10M_DEFAULT_STATE	NO_OS_GPIO_HIGH
#define OSC_40M_DEFAULT_STATE	NO_OS_GPIO_LOW
#define OSC_20M_DEFAULT_STATE	NO_OS_GPIO_LOW
#endif

/* Redefine the init params structure mapping w.r.t. platform */
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#define config_spi_extra_init_params 		stm32_config_spi_extra_init_params
#define data_spi_extra_init_params 			stm32_data_spi_extra_init_params
#define data_qspi_extra_init_params 		stm32_data_qspi_extra_init_params
#define uart_extra_init_params 				stm32_uart_extra_init_params
#define vcom_extra_init_params				stm32_vcom_extra_init_params
#define gpio_xtal_osc_en_extra_init_params 	stm32_gpio_xtal_osc_en_init_params
#define gpio_gp1_extra_init_params 			stm32_gpio_gp1_init_params
#define gpio_gp2_extra_init_params 			stm32_gpio_gp2_init_params
#define gpio_gp3_extra_init_params 			stm32_gpio_gp3_init_params
#define gpio_40m_osc_extra_init_params 		stm32_gpio_40m_osc_init_params
#define gpio_20m_osc_extra_init_params 		stm32_gpio_20m_osc_init_params
#define gpio_10m_osc_extra_init_params 		stm32_gpio_10m_osc_init_params
#define gpio_afe_ctrl_extra_init_params 	stm32_gpio_afe_ctrl_init_params
#define i2c_extra_init_params 				stm32_i2c_extra_init_params
#define gpio_ops							stm32_gpio_ops
#define spi_ops								stm32_spi_ops
#define xspi_ops							stm32_xspi_ops
#define uart_ops							stm32_uart_ops
#define vcom_ops 							stm32_usb_uart_ops
#define i2c_ops								stm32_i2c_ops
#else
#error "No / Invalid active platform selected"
#endif /* ACTIVE_PLATFORM */

/* Select the ADC data capture mode (Only Burst Capture mode is supported) */
#if !defined(DATA_CAPTURE_MODE)
#define DATA_CAPTURE_MODE			BURST_DATA_CAPTURE
#endif

/* Enable the UART/VirtualCOM port connection (default VCOM) */
/* (Uncomment to select UART) */
//#define USE_PHY_COM_PORT

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

/* Check if any serial port available for use as console stdio port */
#if defined(USE_VIRTUAL_COM_PORT)
/* If VCOM is selected, PHY com port will/should act as a console stdio port */
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif

/* EVAL-AD4080-ARDZ CNV Clock Frequency */
#define AD4080_CNV_CLK_FREQ_HZ			(40000000/(NO_OS_BIT(ACTIVE_OSC)))

/* ADC resolution for active device (in bits) */
#define AD4080_ADC_RESOLUTION_BITS		20

/* ADC resolution for active device with sign extension (in bits) */
#define AD4080_SIGN_EXTENDED_RESOLUTION_BITS	\
						(AD4080_ADC_RESOLUTION_BITS + 4)

/* ADC resolution for active device with sign extension (in bytes) */
#define AD4080_SIGN_EXTENDED_RESOLUTION_BYTES	\
						(AD4080_SIGN_EXTENDED_RESOLUTION_BITS / 8)

/* ADC reference voltage in volts */
#define ADC_REF_VOLTAGE					3

/* Number of ADC Sign Extension Bits */
#define AD4080_SIGN_EXT_BITS			4

/* ADC max count (full scale value) for bipolar input */
#define ADC_MAX_COUNT 	(uint32_t)(1 << (AD4080_ADC_RESOLUTION_BITS - 1))

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/****** Macros used to form a VCOM serial number ******/
#if !defined(PLATFORM_NAME)
#define PLATFORM_NAME	HW_CARRIER_NAME
#endif
/******/

/* Below USB configurations (VID and PID) are owned and assigned by ADI.
 * If intended to distribute software further, use the VID and PID owned by your
 * organization */
#define VIRTUAL_COM_PORT_VID	0x0456
#define VIRTUAL_COM_PORT_PID	0xb66c

/* Serial number string is formed as:
 * application name + device (target) name + platform (host) name */
#define VIRTUAL_COM_SERIAL_NUM	\
				(FIRMWARE_NAME "_" DEVICE_NAME "_" STR(PLATFORM_NAME))

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
/* (Use following macros to use SDRAM for data buffer) */
//#define USE_SDRAM

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
extern struct no_os_gpio_desc *gpio_afe_ctrl_desc;
extern struct no_os_gpio_desc *gpio_gp1_desc;
extern struct no_os_gpio_desc *gpio_gp2_desc;
extern struct no_os_gpio_desc *gpio_gp3_desc;
extern struct no_os_gpio_desc *gpio_xtal_osc_en_desc;
extern struct no_os_gpio_desc *gpio_osc_en_40m_desc;
extern struct no_os_gpio_desc *gpio_osc_en_20m_desc;
extern struct no_os_gpio_desc *gpio_osc_en_10m_desc;
extern struct no_os_spi_init_param config_spi_init_params;
extern struct no_os_spi_init_param data_spi_init_params;
#ifdef USE_QUAD_SPI
extern struct no_os_spi_desc *quad_spi_desc;
extern struct no_os_spi_init_param qspi_init_params;
#endif
extern struct no_os_uart_desc *uart_iio_comm_desc;
extern struct no_os_eeprom_desc *eeprom_desc;

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
int32_t init_system(void);

#endif /* APP_CONFIG_H */
