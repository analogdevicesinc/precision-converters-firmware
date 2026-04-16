/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD4692 device applications
******************************************************************************
* Copyright (c) 2024, 2026 Analog Devices, Inc.
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
#include "common_macros.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

/* Select the active platform */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#else
#error "No/Invalid active platform selected"
#endif

// **** Note for User: ACTIVE_DEVICE selection **** //
/* Define the device type here from the list of below device type defines
 * (one at a time. Defining more than one device can result into compile error).
 * e.g. #define DEV_AD4692 -> This will make AD4692 as an ACTIVE_DEVICE.
 * The ACTIVE_DEVICE is default set to AD4692 if device type is not defined.
 * */

#define DEV_AD4692

#if defined(DEV_AD4692)
#define ACTIVE_DEVICE_NAME      "ad4692"
#define	NO_OF_CHANNELS          16
#define HW_MEZZANINE_NAME       "EVAL-AD4692-ARDZ"
#else
#warning No/Unsupported ADxxxxy symbol defined. AD4692 defined
#define DEV_AD4692
#define ACTIVE_DEVICE_NAME      "ad4692"
#define	NO_OF_CHANNELS          16
#define HW_MEZZANINE_NAME       "EVAL-AD4692-ARDZ"
#endif

#define AD4692_AS_SLOT_REG(n)   (0x100 + (n))
#define ADC_RESOLUTION          16

/****** Macros used to form a VCOM serial number ******/
#if !defined(DEVICE_NAME)
#define DEVICE_NAME		"DEV_AD4692"
#endif

/* Used to form a VCOM serial number */
#define	FIRMWARE_NAME	"ad4692_iio"

#if !defined(PLATFORM_NAME)
#define PLATFORM_NAME	HW_CARRIER_NAME
#endif

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* PHY com port will/should act as a console stdio port */
#if defined(USE_VIRTUAL_COM_PORT)
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
#define USE_SDRAM  	// Uncomment to use SDRAM as data buffer

/* Bytes per sample
 * Note: This is applicable to 16 bit data read in manual mode */
#define	BYTES_PER_SAMPLE	sizeof(uint16_t)

/* Number of data storage bits (needed for IIO client to plot ADC data) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

#define CONV_TRIGGER_PERIOD_NSEC(x)		(((float)(1.0 / x) * 1000000) * 1000)
#define CONV_TRIGGER_DUTY_CYCLE_NSEC(x)	(CONV_TRIGGER_PERIOD_NSEC(x) / 10)

/* Reference voltage in uV */
#define AD4692_VREF             4096000

/* Number of shifts to be performed to generate the 5 bit command */
#define AD4692_SHIFT_N	3

/* CNV on time in nanoseconds */
#define CNV_ON_TIME     50

/* Number of cycles of offset in case of Manual Mode */
#define N_CYCLE_OFFSET	2

/* Number of IIO devices */
#define NUM_OF_IIO_DEVICES	         2

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

extern struct no_os_uart_desc *uart_iio_com_desc;
extern struct no_os_uart_desc *uart_console_stdio_desc;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;
extern struct no_os_gpio_desc *cnv_gpio_desc;
extern struct no_os_eeprom_desc* eeprom_desc;
extern struct no_os_pwm_desc *tx_trigger_desc;
extern struct no_os_pwm_desc *spi_burst_pwm_desc;
extern struct no_os_pwm_init_param pwm_spi_burst_init;
extern struct no_os_irq_init_param trigger_gpio_irq_params;

int32_t init_system(void);
int32_t set_timer_prescaler(struct no_os_pwm_desc *desc, uint32_t prescaler);
int init_gpio(void);
void remove_gpio(void);
int init_pwm(void);
void remove_pwm(void);
void ad4692_data_capture_callback(void *ctx);
int32_t init_interrupt(void);
void remove_interrupt(void);

#endif // APP_CONFIG_H
