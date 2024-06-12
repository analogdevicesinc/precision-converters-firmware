/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD355xr device application
******************************************************************************
* Copyright (c) 2023-2024 Analog Devices, Inc.
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
#include "common.h"
#include "no_os_uart.h"
#include "no_os_pwm.h"
#include "no_os_irq.h"
#include "no_os_gpio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* List of supported platforms */
#define	MBED_PLATFORM		0
#define STM32_PLATFORM      1

/* List of data transmit methods supported by hardware platform */
#define SPI_DMA				0
#define SPI_INTERRUPT		1

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/******************************************************************************/

//*** Note for user to select active device ***//
/* Define the device type here
 * (use only one define at time. Difining multiple devices gives compilation error)
 * e.g. #define DEV_AD3541R will select AD3541R as the active device
 * */
// #define DEV_AD3552R

#if defined(DEV_AD3541R)
#define ACTIVE_DEVICE_NAME	 "ad3541r"
#define DEVICE_NAME		     "DEV_AD3541R"
#define ACTIVE_DEVICE_ID	 AD3541R_ID
#define NUMBER_OF_CHANNELS	 1U
#elif defined(DEV_AD3542R_12)
#define ACTIVE_DEVICE_NAME	 "ad3542r-12"
#define DEVICE_NAME		     "DEV_AD3542R_12"
#define ACTIVE_DEVICE_ID	 AD3542R_ID
#define NUMBER_OF_CHANNELS	 2U
#elif defined(DEV_AD3542R_16)
#define ACTIVE_DEVICE_NAME	 "ad3542r-16"
#define DEVICE_NAME		     "DEV_AD3542R_16"
#define ACTIVE_DEVICE_ID	 AD3542R_ID
#define NUMBER_OF_CHANNELS	 2U
#elif defined(DEV_AD3551R)
#define ACTIVE_DEVICE_NAME	 "ad3551r"
#define DEVICE_NAME		     "DEV_AD3551R"
#define ACTIVE_DEVICE_ID	 AD3551R_ID
#define NUMBER_OF_CHANNELS	 1U
#elif defined(DEV_AD3552R)
#define ACTIVE_DEVICE_NAME	 "ad3552r"
#define DEVICE_NAME		     "DEV_AD3552R"
#define ACTIVE_DEVICE_ID	 AD3552R_ID
#define NUMBER_OF_CHANNELS	 2U
#else
#warning No/Unsupported ADxxxxy symbol defined. AD3552R defined
#define DEV_AD3552R
#define ACTIVE_DEVICE_NAME	 "ad3552r"
#define DEVICE_NAME		     "DEV_AD3552R"
#define ACTIVE_DEVICE_ID	 AD3552R_ID
#define NUMBER_OF_CHANNELS	 2U
#endif

/* Select the active platform (default is Mbed) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		MBED_PLATFORM
#endif

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

/* Interface mode either spi interrupt or spi dma
 * spi dma works only with cyclic waveform generation and fast mode on stm platform */
#if !defined(INTERFACE_MODE)
#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#define INTERFACE_MODE SPI_INTERRUPT
#else // STM32
#define INTERFACE_MODE SPI_DMA
#endif
#endif

/* DAC resolution for active device
 * If active device is DEV_AD3542R_12 then resolution is 12 bits.
 * But user need to give dac code which are 12-bit code multiplied by 16.
 * */
#define DAC_RESOLUTION 16

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"
#define HW_CARRIER_NAME		TARGET_NAME
#elif (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#else
#error "No/Invalid active platform selected"
#endif

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/****** Macros used to form a VCOM serial number ******/
#if !defined(DEVICE_NAME)
#define DEVICE_NAME		"DEV_AD3552R"
#endif

/* Used to form a VCOM serial number */
#define	FIRMWARE_NAME	"ad355xr_iio"

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

/* Check if any serial port available for use as console stdio port */
#if defined(USE_PHY_COM_PORT)
/* If PHY com is selected, VCOM or alternate PHY com port can act as a console stdio port */
#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif
#else
/* If VCOM is selected, PHY com port will/should act as a console stdio port */
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif

/* Enable/Disable the use of SDRAM for DAC data buffer */
//#define USE_SDRAM		// Uncomment to use SDRAM for data buffer

/* PWM period and duty cycle */
#define CONV_PERIOD_NSEC(x)	    (((float)(1.0 / x) * 1000000) * 1000)
#define CONV_FREQUENCY_HZ(x)	    (((float)(1.0 / x) * 1000000) * 1000)
#define CONV_DUTY_CYCLE_NSEC(x,y) (((float)y / 100) * CONV_PERIOD_NSEC(x))

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

extern struct no_os_uart_desc *uart_iio_com_desc;
extern struct no_os_pwm_desc *ldac_pwm_desc;
extern struct no_os_pwm_desc *spi_dma_tx_stop_pwm_desc;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;

int32_t init_system(void);
int32_t init_ldac_pwm_trigger(void);
void ldac_pos_edge_detect_callback(void* ctx);
#endif /* APP_CONFIG_H_ */
