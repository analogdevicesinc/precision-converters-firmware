/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD5706R IIO firmware application
******************************************************************************
* Copyright (c) 2024-2026 Analog Devices, Inc.
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
#include "no_os_uart.h"
#include "no_os_pwm.h"
#include "common_macros.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* List of data capture modes */
#define SPI_INTERRUPT	    1
#define SPI_DMA		        2

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/******************************************************************************/

/* Name of active device */
#define ACTIVE_DEVICE_NAME	"ad5706r"

/* Resolution of the DAC */
#define AD5706_DAC_RESOLUTION	16

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

/* Select the active platform (default is STM32) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

/* Select the Interface mode. Default is SPI DMA */
#if !defined(INTERFACE_MODE)
#define INTERFACE_MODE		SPI_DMA
#endif

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#else
#error "No/Invalid active platform selected"
#endif //ACTIVE_PLATFORM

/* Check if any serial port available for use as console stdio port */
#if defined(USE_PHY_COM_PORT)
/* If PHY com is selected, VCOM or alternate PHY com port can act as a console stdio port */
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif
#else
/* If VCOM is selected, PHY com port will/should act as a console stdio port */
//#define CONSOLE_STDIO_PORT_AVAILABLE
#endif

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/****** Macros used to form a VCOM serial number ******/
#define	FIRMWARE_NAME	"ad5706r_iio"

#if !defined(DEVICE_NAME)
#define DEVICE_NAME		"DEV_AD5706"
#endif

#if !defined(PLATFORM_NAME)
#define PLATFORM_NAME	HW_CARRIER_NAME
#endif
/******/

/* Enable/Disable the use of SDRAM for DAC data buffer */
//#define USE_SDRAM	// Uncomment to use SDRAM as data buffer

#define FREQ_TO_NSEC(x) (((float)(1.0 / x) * 1000000) * 1000)

/* Note: LDAC duty cycle is always 50% */
#define LDAC_DUTY_CYCLE_NSEC(x)	(x * 0.5)

/* DAC Update Duty cycle */
#define DAC_UPDATE_DUTY_CYCLE_NSEC(x)	(x * 0.05)

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

extern struct no_os_uart_desc *uart_iio_comm_desc;
extern struct no_os_uart_desc *uart_console_stdio_desc;
extern struct no_os_gpio_desc *gpio_ad0_desc;
extern struct no_os_gpio_desc *gpio_ad1_desc;
extern struct no_os_gpio_desc *gpio_ldac_tg_desc;
extern struct no_os_pwm_desc *ldac_pwm_desc;
extern struct no_os_pwm_desc *dac_update_pwm_desc;
extern struct no_os_gpio_init_param gpio_ldac_tg_params;
extern struct no_os_eeprom_desc *eeprom_desc;
extern struct no_os_gpio_desc *gpio_shutdown_desc;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;
extern struct no_os_pwm_init_param cs_init_params;
extern struct no_os_gpio_init_param csb_gpio_init_param;
extern struct no_os_pwm_desc* tx_trigger_desc;
extern struct no_os_pwm_init_param tx_trigger_init_params;
extern struct no_os_gpio_init_param ldac_pwm_gpio_params;
int32_t init_system(void);
int32_t init_pwm(void);

#endif //APP_CONFIG_H
