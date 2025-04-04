/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD717x and AD411x IIO firmware application
******************************************************************************
* Copyright (c) 2021-23,2025 Analog Devices, Inc.
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
#include "no_os_gpio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* List of supported platforms */
#define	MBED_PLATFORM		1
#define STM32_PLATFORM      2

/* List of data capture modes for AD717x device */
#define CONTINUOUS_DATA_CAPTURE		0
#define BURST_DATA_CAPTURE			1

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/* Select the active platform */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif // ACTIVE_PLATFORM

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

// **** Note for User on selection of Active Device ****//
/* Define the device type here from the list of below device type defines
 * (one at a time. Defining more than one device can result into compile error).
 * e.g. #define DEV_AD4111 -> This will make AD4111 as an active device.
 * The active device is default set to AD4111 if device type is not defined.
 * */
// #define DEV_AD4111

#if defined(DEV_AD4111)
#define ACTIVE_DEVICE_NAME	"ad4111"
#define DEVICE_NAME			"DEV_AD4111"
#define ACTIVE_DEVICE_ID	 ID_AD4111
#define HW_MEZZANINE_NAME	"Eval-AD4111SDZ"
#elif defined(DEV_AD4112)
#define ACTIVE_DEVICE_NAME	"ad4112"
#define DEVICE_NAME			"DEV_AD4112"
#define ACTIVE_DEVICE_ID	 ID_AD4112
#define HW_MEZZANINE_NAME	"EVAL-AD4112SDZ"
#elif defined(DEV_AD4114)
#define ACTIVE_DEVICE_NAME	"ad4114"
#define DEVICE_NAME			"DEV_AD4114"
#define ACTIVE_DEVICE_ID	 ID_AD4114
#define HW_MEZZANINE_NAME	"EVAL-AD4114SDZ"
#elif defined(DEV_AD4115)
#define ACTIVE_DEVICE_NAME	"ad4115"
#define DEVICE_NAME			"DEV_AD4115"
#define ACTIVE_DEVICE_ID	 ID_AD4115
#define HW_MEZZANINE_NAME	"EVAL-AD4115SDZ"
#elif defined(DEV_AD4116)
#define ACTIVE_DEVICE_NAME	"ad4116"
#define DEVICE_NAME			"DEV_AD4116"
#define ACTIVE_DEVICE_ID	 ID_AD4116
#define HW_MEZZANINE_NAME	"EVAL-AD4116SDZ"
#elif defined(DEV_AD7172_2)
#define AD7172_2_INIT
#define ACTIVE_DEVICE_NAME	"ad7172-2"
#define DEVICE_NAME			"DEV_AD7172_2"
#define ACTIVE_DEVICE_ID	 ID_AD7172_2
#define HW_MEZZANINE_NAME	"EVAL-AD7172-2SDZ"
#elif defined(DEV_AD7172_4)
#define AD7172_4_INIT
#define ACTIVE_DEVICE_NAME	"ad7172-4"
#define DEVICE_NAME			"DEV_AD7172_4"
#define ACTIVE_DEVICE_ID	 ID_AD7172_4
#define HW_MEZZANINE_NAME	"EVAL-AD7172-4SDZ"
#elif defined(DEV_AD7173_8)
#define AD7173_8_INIT
#define ACTIVE_DEVICE_NAME	"ad7173-8"
#define DEVICE_NAME			"DEV_AD7173_8"
#define ACTIVE_DEVICE_ID	 ID_AD7173_8
#define HW_MEZZANINE_NAME	"EVAL-AD7173-8SDZ"
#elif defined(DEV_AD7175_2)
#define AD7175_2_INIT
#define ACTIVE_DEVICE_NAME	"ad7175-2"
#define DEVICE_NAME			"DEV_AD7175_2"
#define ACTIVE_DEVICE_ID	 ID_AD7175_2
#define HW_MEZZANINE_NAME	"EVAL-AD7175-2SDZ"
#elif defined(DEV_AD7175_8)
#define AD7175_8_INIT
#define ACTIVE_DEVICE_NAME	"ad7175-8"
#define DEVICE_NAME			"DEV_AD7175_8"
#define ACTIVE_DEVICE_ID	 ID_AD7175_8
#define HW_MEZZANINE_NAME	"EVAL-AD7175-8SDZ"
#elif defined(DEV_AD7176_2)
#define AD7176_2_INIT
#define ACTIVE_DEVICE_NAME	"ad7176-2"
#define DEVICE_NAME			"DEV_AD7176_2"
#define ACTIVE_DEVICE_ID	 ID_AD7176_2
#define HW_MEZZANINE_NAME	"EVAL-AD7176-2SDZ"
#elif defined(DEV_AD7177_2)
#define AD7177_2_INIT
#define ACTIVE_DEVICE_NAME	"ad7177-2"
#define DEVICE_NAME			"DEV_AD7177_2"
#define ACTIVE_DEVICE_ID	 ID_AD7177_2
#define HW_MEZZANINE_NAME	"EVAL-AD7177-2SDZ"
#else
#warning No/Unsupported ADxxxxy symbol defined. AD4111 defined
#define DEV_AD4111
#define ACTIVE_DEVICE_NAME	"ad4111"
#define DEVICE_NAME			"DEV_AD4111"
#define ACTIVE_DEVICE_ID	 ID_AD4111
#define HW_MEZZANINE_NAME	"Eval-AD4111SDZ"
#endif // Device Select (Active Device name definition)

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"
#define HW_CARRIER_NAME         TARGET_NAME
/* Redefine the init params structure mapping w.r.t. platform */
#define vcom_extra_init_params mbed_vcom_extra_init_params
#define uart_extra_init_params mbed_uart_extra_init_params
#define spi_extra_init_params	mbed_spi_extra_init_params
#define ext_int_extra_init_params mbed_trigger_gpio_irq_init_params
#define i2c_extra_init_params mbed_i2c_extra_init_params
#define uart_ops mbed_uart_ops
#define vcom_ops mbed_virtual_com_ops
#define csb_platform_ops mbed_gpio_ops
#define rdy_platform_ops mbed_gpio_ops
#define irq_platform_ops mbed_gpio_irq_ops
#define spi_platform_ops mbed_spi_ops
#define i2c_ops mbed_i2c_ops
#define trigger_gpio_handle	0
#define IRQ_INT_ID	GPIO_IRQ_ID1
#elif (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#define HW_CARRIER_NAME		    	TARGET_NAME
/* Redefine the init params structure mapping w.r.t. platform */
#define uart_extra_init_params        stm32_uart_extra_init_params
#define spi_extra_init_params         stm32_spi_extra_init_params
#define vcom_extra_init_params  stm32_vcom_extra_init_params
#define uart_extra_init_params        stm32_uart_extra_init_params
#define ext_int_extra_init_params stm32_trigger_gpio_irq_init_params
#define uart_ops stm32_uart_ops
#define vcom_ops  stm32_usb_uart_ops
#define irq_platform_ops stm32_gpio_irq_ops
#define csb_platform_ops stm32_gpio_ops
#define rdy_platform_ops stm32_gpio_ops
#define spi_platform_ops stm32_spi_ops
#define irq_ops          stm32_irq_ops
#define i2c_ops		stm32_i2c_ops
#define trigger_gpio_irq_ops stm32_gpio_irq_ops
#define trigger_gpio_handle 0	// Unused macro
#define IRQ_INT_ID  RDY_PIN
#else
#error "No/Invalid active platform selected"
#endif

/* VCOM Serial number definition */
#define	FIRMWARE_NAME	"ad717x_iio"

#if !defined(PLATFORM_NAME)
#define PLATFORM_NAME	HW_CARRIER_NAME
#endif

#if !defined(EVB_INTERFACE)
#define EVB_INTERFACE	"SDP_120"
#endif

/* Below USB configurations (VID and PID) are owned and assigned by ADI.
 * If intended to distribute software further, use the VID and PID owned by your
 * organization */
#define VIRTUAL_COM_PORT_VID	0x0456
#define VIRTUAL_COM_PORT_PID	0xb66c
#define VIRTUAL_COM_SERIAL_NUM	(FIRMWARE_NAME "_" DEVICE_NAME "_" STR(PLATFORM_NAME))

/* Definition for number of channels for the selected device */
#if defined(DEV_AD4111) || defined(DEV_AD4112) || \
	defined(DEV_AD4114) || defined(DEV_AD4115) || defined (DEV_AD4116) ||\
	defined(DEV_AD7173_8) || defined(DEV_AD7175_8)
#define NUMBER_OF_CHANNELS	16U
#define NUMBER_OF_SETUPS	8U
#elif defined(DEV_AD7172_4)
#define NUMBER_OF_CHANNELS	8U
#define NUMBER_OF_SETUPS	8U
#else //AD7172_2, AD71725_2, AD7176-2, AD7177-2
#define NUMBER_OF_CHANNELS	4U
#define NUMBER_OF_SETUPS	4U
#endif // Device Select

/* Select the ADC data capture mode (default is CC mode) */
#if !defined(DATA_CAPTURE_MODE)
#define DATA_CAPTURE_MODE	CONTINUOUS_DATA_CAPTURE
#endif

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
//#define USE_SDRAM	// Uncomment to use SDRAM for data buffer

/* ADC Reference Voltage in volts */
#define AD717X_INTERNAL_REFERENCE	2.5
#define AD717x_EXTERNAL_REFERENCE	2.5
#define AD717X_AVDD_AVSS_REFERENCE	2.5

/* Baud Rate for IIO Application */
#define IIO_UART_BAUD_RATE		(230400)

/* AD717x Sampling Rate of the device in SPS, excluding the fractional part.
 * The following are the possible values of sampling frequencies (in SPS):
 *
 * AD4111, AD41112, AD4114, AD4115:
 * 31250, 15625, 10417, 5208, 2957, 1007, 503, 381, 200, 100, 59, 49, 20, 16, 10, 5, 2, 1.
 *
 * AD4116:
 * 625000, 31250, 15625, 10416, 5194, 2496, 1007, 499, 390, 200, 100, 59, 49, 20, 16, 10, 5, 2, 1.
 *
 *AD7175_2, AD7175_8, AD7176_2:
 * 31250, 25000, 10000, 5000, 2500, 1000, 500, 397, 200, 100, 59, 49, 20, 16, 10, 5
 *
 *AD7177_2:
 *10000, 5000, 2500, 1000, 500, 397, 200, 100, 59, 49, 20, 16, 10, 5
 *
 * Note: The below defined sampling frequency is applicable for all setups */
#define AD717x_SAMPLING_RATE		(31250)

/* ODR[4:0] bits*/
#if defined (DEV_AD4111) || defined (DEV_AD4112) || defined (DEV_AD4114) ||\
 defined (DEV_AD4115) || defined (DEV_AD7172_2) ||\
	defined (DEV_AD7172_4) || defined (DEV_AD7173_8)
#if (AD717x_SAMPLING_RATE == 31250)
#define AD717x_ODR_SEL	    0
#elif (AD717x_SAMPLING_RATE == 15625)
#define AD717x_ODR_SEL	    6
#elif (AD717x_SAMPLING_RATE == 10417)
#define AD717x_ODR_SEL	    7
#elif (AD717x_SAMPLING_RATE == 5208)
#define AD717x_ODR_SEL	    8
#elif (AD717x_SAMPLING_RATE == 2957)
#define AD717x_ODR_SEL	    9
#elif (AD717x_SAMPLING_RATE == 1007)
#define AD717x_ODR_SEL	    10
#elif (AD717x_SAMPLING_RATE == 503)
#define AD717x_ODR_SEL	    11
#elif (AD717x_SAMPLING_RATE == 381)
#define AD717x_ODR_SEL	    12
#elif (AD717x_SAMPLING_RATE == 200)
#define AD717x_ODR_SEL	    13
#elif (AD717x_SAMPLING_RATE == 100)
#define AD717x_ODR_SEL	    14
#elif (AD717x_SAMPLING_RATE == 59)
#define AD717x_ODR_SEL	    15
#elif (AD717x_SAMPLING_RATE == 49)
#define AD717x_ODR_SEL	    16
#elif (AD717x_SAMPLING_RATE == 20)
#define AD717x_ODR_SEL	    17
#elif (AD717x_SAMPLING_RATE == 16)
#define AD717x_ODR_SEL	    18
#elif (AD717x_SAMPLING_RATE == 10)
#define AD717x_ODR_SEL	    19
#elif (AD717x_SAMPLING_RATE == 5)
#define AD717x_ODR_SEL	    20
#elif (AD717x_SAMPLING_RATE == 2)
#define AD717x_ODR_SEL	    21
#elif (AD717x_SAMPLING_RATE == 1)
#define AD717x_ODR_SEL	    22
#else
#warining "Invalid sampling frequency selection, using 31250 as default"
#define AD717x_SAMPLING_RATE	31250
#define AD717x_ODR_SEL			0
#endif

#elif defined (DEV_AD4116)
#if (AD717x_SAMPLING_RATE == 625000)
#define AD717x_ODR_SEL      0
#elif (AD717x_SAMPLING_RATE == 31250)
#define AD717x_ODR_SEL      4
#elif (AD717x_SAMPLING_RATE == 15625)
#define AD717x_ODR_SEL      6
#elif (AD717x_SAMPLING_RATE == 10416)
#define AD717x_ODR_SEL      7
#elif (AD717x_SAMPLING_RATE == 5194)
#define AD717x_ODR_SEL      8
#elif (AD717x_SAMPLING_RATE == 2496)
#define AD717x_ODR_SEL      9
#elif (AD717x_SAMPLING_RATE == 1007)
#define AD717x_ODR_SEL      10
#elif (AD717x_SAMPLING_RATE == 499)
#define AD717x_ODR_SEL      11
#elif (AD717x_SAMPLING_RATE == 390)
#define AD717x_ODR_SEL      12
#elif (AD717x_SAMPLING_RATE == 200)
#define AD717x_ODR_SEL      13
#elif (AD717x_SAMPLING_RATE == 100)
#define AD717x_ODR_SEL      14
#elif (AD717x_SAMPLING_RATE == 59)
#define AD717x_ODR_SEL      15
#elif (AD717x_SAMPLING_RATE == 49)
#define AD717x_ODR_SEL      16
#elif (AD717x_SAMPLING_RATE == 20)
#define AD717x_ODR_SEL      17
#elif (AD717x_SAMPLING_RATE == 16)
#define AD717x_ODR_SEL      18
#elif (AD717x_SAMPLING_RATE == 10)
#define AD717x_ODR_SEL      19
#elif (AD717x_SAMPLING_RATE == 5)
#define AD717x_ODR_SEL      20
#elif (AD717x_SAMPLING_RATE == 2)
#define AD717x_ODR_SEL      21
#elif (AD717x_SAMPLING_RATE == 1)
#define AD717x_ODR_SEL      22
#endif
#elif defined (DEV_AD7175_2) || defined (DEV_AD7175_8) || defined (DEV_AD7176_2) || defined (DEV_AD7177_2)
#if !defined (DEV_AD7177_2)
#if (AD717x_SAMPLING_RATE == 31250)
#define AD717x_ODR_SEL	    4
#elif (AD717x_SAMPLING_RATE == 25000)
#define AD717x_ODR_SEL	    5
#elif (AD717x_SAMPLING_RATE == 15625)
#define AD717x_ODR_SEL	    6
#endif // DEV_AD7177_2
#elif (AD717x_SAMPLING_RATE == 10000)
#define AD717x_ODR_SEL	    7
#elif (AD717x_SAMPLING_RATE == 5000)
#define AD717x_ODR_SEL	    8
#elif (AD717x_SAMPLING_RATE == 2500)
#define AD717x_ODR_SEL	    9
#elif (AD717x_SAMPLING_RATE == 1000)
#define AD717x_ODR_SEL	    10
#elif (AD717x_SAMPLING_RATE == 500)
#define AD717x_ODR_SEL	    11
#elif (AD717x_SAMPLING_RATE == 397)
#define AD717x_ODR_SEL	    12
#elif (AD717x_SAMPLING_RATE == 200)
#define AD717x_ODR_SEL	    13
#elif (AD717x_SAMPLING_RATE == 100)
#define AD717x_ODR_SEL	    14
#elif (AD717x_SAMPLING_RATE == 59)
#define AD717x_ODR_SEL	    15
#elif (AD717x_SAMPLING_RATE == 49)
#define AD717x_ODR_SEL	    16
#elif (AD717x_SAMPLING_RATE == 20)
#define AD717x_ODR_SEL	    17
#elif (AD717x_SAMPLING_RATE == 16)
#define AD717x_ODR_SEL	    18
#elif (AD717x_SAMPLING_RATE == 10)
#define AD717x_ODR_SEL	    19
#elif (AD717x_SAMPLING_RATE == 5)
#define AD717x_ODR_SEL	    20
#else
#warning "Invalid sampling frequency selection, using 31250 as default"
#if defined(DEV_AD7177_2)
#define AD717x_SAMPLING_RATE	10000
#define AD717x_ODR_SEL		7
#else
#define AD717x_SAMPLING_RATE	31250
#define AD717x_ODR_SEL			4
#endif // DEV_AD7177_2 warning
#endif
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

/* Denominator of the scale factor to be applied while converting raw values to actual voltage */
#if  defined(DEV_AD4111) || defined(DEV_AD4112) || \
	defined(DEV_AD4114) || defined(DEV_AD4115) || defined (DEV_AD4116)
#define SCALE_FACTOR_DR			0.1
#else
#define SCALE_FACTOR_DR			1
#endif

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

extern struct no_os_uart_desc *uart_desc;

extern struct no_os_gpio_desc *csb_gpio;

extern struct no_os_gpio_desc *rdy_gpio;

extern struct no_os_irq_ctrl_desc *trigger_irq_desc;

extern struct no_os_eeprom_desc *eeprom_desc;

int32_t init_system(void);

#endif // APP_CONFIG_H

