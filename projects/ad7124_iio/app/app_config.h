/***************************************************************************//*
 * @file    app_config.h
 * @brief   Header File for the application configuration for AD7124 IIO app.
******************************************************************************
 * Copyright (c) 2023-25 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/

#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

/*****************************************************************************/
/***************************** Include Files *********************************/
/*****************************************************************************/

#include <stdint.h>
#include "common_macros.h"

/*****************************************************************************/
/********************** Macros and Constants Definition **********************/
/*****************************************************************************/

/* Select the active platform */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

/* List of data capture modes */
#define CONTINUOUS_DATA_CAPTURE 0
#define BURST_DATA_CAPTURE 1

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/* List of supported IIO clients
 * - Local client is supported only on DISCO-F769NI (mbed & stm32 platform).
 * - DISCO-F769NI only supports local client.
 * - NUCLEO-H563ZI (stm32 platform) supports only remote client.
 * - SDP-K1 (mbed platform) supports only remote client.
 */
#define IIO_CLIENT_REMOTE    1	// Remote (PC) IIO client
#define IIO_CLIENT_LOCAL     2	// Local (display) IIO client

/* Select active IIO client */
#if !defined(ACTIVE_IIO_CLIENT)
#define ACTIVE_IIO_CLIENT	IIO_CLIENT_REMOTE
#endif

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
//#define USE_SDRAM	// Uncomment to use SDRAM for data buffer

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

// **** Note for User on selection of Active Device ****//
/* Define the device type here from the list of below device type defines
 * (one at a time. Defining more than one device can result into compile error).
 * e.g. #define DEV_AD7142_4= -> This will make AD7124-4 as an active device.
 * The active device is default set to AD7124-4 if device type is not defined.
 * */
//#define DEV_AD7124_4

#if defined(DEV_AD7124_4)
#define DEVICE_NAME			"DEV_AD7124_4"
#define ACTIVE_DEVICE_NAME	"ad7124-4"
#define NUM_OF_CHANNELS 8
#define HW_MEZZANINE_NAME	"EVAL-AD7124-4ASDZ"

#elif defined(DEV_AD7124_8)
#define DEVICE_NAME			"DEV_AD7124_8"
#define ACTIVE_DEVICE_NAME	"ad7124-8"
#define NUM_OF_CHANNELS 16
#define HW_MEZZANINE_NAME	"EVAL-AD7124-8ASDZ"

#else
#warning  No/Unsupported ADxxxx symbol defined. AD7124_4 defined
#define DEV_AD7124_4
#define DEVICE_NAME			"DEV_AD7124_4"
#define ACTIVE_DEVICE_NAME	"ad7124-4"
#define NUM_OF_CHANNELS 8
#define HW_MEZZANINE_NAME	"EVAL-AD7124-4ASDZ"
#endif

/* List of Input modes */
#define PSUEDO_DIFFERENTIAL_MODE  0
#define DIFFERENTIAL_MODE 1

/* Select Input Mode -
 * AD7124-4 can be operated with 4 differential and 7 pseudo differential channels
 * AD7124-8 can be operated with 8 differential and 15 psuedo differential channels
 * */
#if !defined(INPUT_MODE)
#define INPUT_MODE PSEUDO_DIFFERENTIAL_MODE
#endif

/* ADC resolution for active device */
#define ADC_RESOLUTION			24

/* ADC max count (full scale value) for unipolar inputs */
#define ADC_MAX_COUNT_UNIPOLAR	(int32_t)((1 << ADC_RESOLUTION) - 1)

/* ADC max count (full scale value) for bipolar inputs */
#define ADC_MAX_COUNT_BIPOLAR	(int32_t)(1 << (ADC_RESOLUTION-1))

/* Redefine the init params structure mapping w.r.t. platform */
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
/* Redefine the init params structure mapping wrt platform */
#define spi_extra_init_params   	stm32_spi_extra_init_params
#define uart_extra_init_params 		stm32_uart_extra_init_params
#define gpio_platform_ops			stm32_gpio_ops
#define spi_platform_ops            stm32_spi_ops
#define uart_ops stm32_uart_ops
#define i2c_ops stm32_i2c_ops
#define i2c_extra_init_params stm32_i2c_extra_init_params
#define irq_platform_ops stm32_gpio_irq_ops
#define ext_int_extra_init_params   stm32_trigger_gpio_irq_init_params
#define IRQ_INT_ID RDY_PIN
#define ticker_int_extra_init_params 0 //(Unused)
#define trigger_gpio_handle	0
#else
#error "No/Invalid active platform selected"
#endif
//#endif

#if !defined(PLATFORM_NAME)
#define PLATFORM_NAME	HW_CARRIER_NAME
#endif

/* Baud Rate for IIO Application */
#define IIO_UART_BAUD_RATE		(230400)

/* Check if any serial port available for use as console stdio port */
#if defined(USE_VIRTUAL_COM_PORT)
/* If VCOM is selected, PHY com port will/should act as a console stdio port */
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif

/* Select the ADC data capture mode (default is CC mode) */
#if !defined(DATA_CAPTURE_MODE)
#define DATA_CAPTURE_MODE CONTINUOUS_DATA_CAPTURE
#endif

#define LVGL_TICK_TIME_US	5000
#define LVGL_TICK_TIME_MS	(LVGL_TICK_TIME_US / 1000)

/******************************************************************************/
/********************** Public Declarations ***********************************/
/******************************************************************************/

int init_system(void);
extern struct no_os_uart_desc *uart_desc;
extern struct no_os_eeprom_desc *eeprom_desc;
extern struct no_os_gpio_desc *csb_gpio;
extern struct no_os_gpio_desc *rdy_gpio;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;
void data_capture_callback(void *ctx);
void ticker_callback(void* ctx);
void lvgl_tick_callback(void* ctx);

#endif // APP_CONFIG_H
