/*************************************************************************//**
 *   @file   app_config.h
 *   @brief   Application configurations module for AD3530R
******************************************************************************
* Copyright (c) 2022-25 Analog Devices, Inc.
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
#include "common.h"
#include "no_os_gpio.h"
#include "no_os_uart.h"
#include "no_os_irq.h"
#include "no_os_pwm.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* List of supported platforms */
#define STM32_PLATFORM      1

/* List of data transmit methods supported by hardware platform */
#define SPI_DMA				0
#define SPI_INTERRUPT		1

/* List of supported DAC data stream modes */
#define CYCLIC_STREAM			0
#define ARBITRARY_STREAM		1

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/******************************************************************************/

/* Name of active device */
#define ACTIVE_DEVICE_NAME	"ad3530r"

/* Select the active platform */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

/* Select the DAC data stream mode (default is Cyclic stream mode) */
#if !defined(DATA_STREAM_MODE)
#define DATA_STREAM_MODE	CYCLIC_STREAM
#endif

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

/* Interface mode either spi interrupt or spi dma (default spi dma) */
//#define INTERFACE_MODE SPI_INTERRUPT // Uncomment to select spi interrupt

#if !defined(INTERFACE_MODE)
#define INTERFACE_MODE SPI_DMA
#endif

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#else
#error "No/Invalid active platform selected"
#endif

/* HW ID of the target EVB */
#define HW_MEZZANINE_NAME	"EVAL-AD3530RARDZ"

/* DAC resolution for active device */
#define DAC_RESOLUTION		16

/* Number of DAC channels */
#define DAC_MAX_CHANNELS		16

/* DAC reference voltage (Range: 2.5 to 3.3v) */
#define DAC_REF_VOLTAGE		2.5

/* DAC max count (full scale value) */
#define DAC_MAX_COUNT       (uint32_t)((1 << DAC_RESOLUTION) - 1)

/****** Macros used to form a VCOM serial number ******/
#define	FIRMWARE_NAME	"ad353xr_iio"

#define DEVICE_NAME		"DEV_AD3530R"

#if !defined(PLATFORM_NAME)
#define PLATFORM_NAME	HW_CARRIER_NAME
#endif
/******/

/* Below USB configurations (VID and PID) are owned and assigned by ADI.
 * If intended to distribute software further, use the VID and PID owned by your
 * organization */
#define VIRTUAL_COM_PORT_VID	0x0456
#define VIRTUAL_COM_PORT_PID	0xb66c

/* Serial number string is formed as: application name + device (target) name + platform (host) name */
#define VIRTUAL_COM_SERIAL_NUM	(FIRMWARE_NAME "_" DEVICE_NAME "_" STR(PLATFORM_NAME))

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* Check if any serial port available for use as console stdio port */
#if defined(USE_PHY_COM_PORT)
/* If PHY com is selected, VCOM or alternate PHY com port can act as a console stdio port */
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif
#else
/* If VCOM is selected, PHY com port will/should act as a console stdio port */
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif

/* Enable/Disable the use of SDRAM for DAC data streaming buffer */
//#define USE_SDRAM		// Uncomment to use SDRAM for data buffer

/* PWM period and duty cycle */
#define CONV_TRIGGER_PERIOD_NSEC(x)		(((float)(1.0 / x) * 1000000) * 1000)
#define CONV_TRIGGER_DUTY_CYCLE_NSEC(x,y)	(((float)y / 100) * CONV_TRIGGER_PERIOD_NSEC(x))

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
extern struct no_os_pwm_desc *pwm_desc;
extern struct no_os_uart_desc *uart_iio_com_desc;
extern struct no_os_gpio_desc *trigger_gpio_desc;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;
extern struct no_os_spi_init_param spi_init_params;
extern struct no_os_eeprom_desc *eeprom_desc;
extern struct no_os_pwm_init_param pwm_init_params;
extern struct no_os_gpio_desc* csb_gpio_desc;
extern struct no_os_pwm_init_param pwm_init_params;

#if (INTERFACE_MODE == SPI_DMA)
extern struct no_os_pwm_desc* tx_trigger_desc;
extern struct no_os_dma_init_param ad3530r_dma_init_param;
extern struct no_os_gpio_init_param pwm_gpio_params;
#endif

int32_t init_pwm(void);
int32_t init_system(void);

#endif //APP_CONFIG_H
