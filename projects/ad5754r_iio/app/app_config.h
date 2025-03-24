/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD5754R device application
******************************************************************************
* Copyright (c) 2024,2025 Analog Devices, Inc.
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
#include "ad5754r.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* List of supported platforms */
#define	MBED_PLATFORM		1
#define STM32_PLATFORM      2

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/******************************************************************************/

/* Select the active platform (default is Mbed) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM	 STM32_PLATFORM
#endif

// **** Note for User on selection of Active Device ****//
/* Define the device type here from the list of below device type defines
 * (one at a time. Defining more than one device can result into compile error).
 * e.g. #define DEV_CN0586 -> This will make CN0586 as an active device.
 * The active device is default set to CN0586 if device type is not defined.
 * */

#define DEV_CN0586

#ifndef DEV_CN0586
#define DEV_AD5754R
#endif

#if defined(DEV_CN0586)
#define ACTIVE_DEVICE_NAME	"ad5754r"
#define DEVICE_NAME			"DEV_CN0586"
#define HW_MEZZANINE_NAME	"EVAL-CN0586-ARDZ"
#else
#define ACTIVE_DEVICE_NAME	"ad5754r"
#define DEVICE_NAME			"DEV_AD5754R"
#define HW_MEZZANINE_NAME	"EVAL-AD5754REBZ"
#endif

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

/* DAC Reference Voltage */
#define AD5754R_VREF  2.5

/* DAC maximum count in offset binary code */
#define DAC_MAX_COUNT_BIN_OFFSET	(uint32_t)((1 << AD5754R_MAX_RESOLUTION) - 1)

/* DAC maximum count in 2s complement code */
#define DAC_MAX_COUNT_2S_COMPL   (uint32_t)(1 << (AD5754R_MAX_RESOLUTION-1))

/* Define the Binary/Two's complement coding (default Binary) */
//#define USE_TWOS_COMPLEMENT_CODING

#if !defined(USE_TWOS_COMPLEMENT_CODING)
#define USE_BINARY_CODING
#endif

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"

#define HW_CARRIER_NAME		TARGET_NAME

/* Redefine the init params structure mapping w.r.t. platform */
#define pwm_extra_init_params mbed_pwm_extra_init_params
#define uart_extra_init_params mbed_uart_extra_init_params
#define vcom_extra_init_params mbed_vcom_extra_init_params
#define spi_extra_init_params mbed_spi_extra_init_params
#define i2c_extra_init_params mbed_i2c_extra_init_params
#define trigger_gpio_irq_extra_params mbed_trigger_gpio_irq_init_params
#define ldac_gpio_extra_init_params mbed_ldac_gpio_init_params
#define clear_gpio_extra_init_params mbed_clear_gpio_init_params
#define dac_gpio_ops mbed_gpio_ops
#define TRIGGER_INT_ID GPIO_IRQ_ID1
#define trigger_gpio_handle         0 // Unused macro

#elif (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#define HW_CARRIER_NAME TARGET_NAME
#define pwm_extra_init_params stm32_pwm_extra_init_params
#define uart_extra_init_params stm32_uart_extra_init_params
#define vcom_extra_init_params stm32_vcom_extra_init_params
#define spi_extra_init_params stm32_spi_extra_init_params
#define trigger_gpio_irq_extra_params stm32_trigger_gpio_irq_init_params
#define ldac_gpio_extra_init_params stm32_ldac_gpio_init_params
#define clear_gpio_extra_init_params stm32_clear_gpio_init_params
#define dac_gpio_ops stm32_gpio_ops
#define trigger_gpio_handle         0 // Unused macro
#else
#error "No/Invalid active platform selected"
#endif

/****** Macros used to form a VCOM serial number ******/

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* Used to form a VCOM serial number */
#define	FIRMWARE_NAME	"ad5754r_iio"

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
#if (ACTIVE_PLATFORM == MBED_PLATFORM || ACTIVE_PLATFORM == STM32_PLATFORM)
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif
#else
/* If VCOM is selected, PHY com port will/should act as a console stdio port */
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
//#define USE_SDRAM

/* PWM period and duty cycle */
#define CONV_TRIGGER_PERIOD_NSEC(x)		(((float)(1.0 / x) * 1000000) * 1000)
#define CONV_TRIGGER_DUTY_CYCLE_NSEC(x)	((CONV_TRIGGER_PERIOD_NSEC(x) * 9) / 10)

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

extern struct no_os_uart_desc *uart_iio_com_desc;
extern struct no_os_uart_desc *uart_console_stdio_desc;
extern struct no_os_gpio_init_param ldac_gpio_params;
extern struct no_os_gpio_init_param clear_gpio_params;
extern struct no_os_eeprom_desc *eeprom_desc;
extern struct no_os_gpio_desc *trigger_gpio_desc;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;
extern struct no_os_pwm_desc *pwm_desc;
extern struct no_os_pwm_init_param pwm_init_params;

int32_t init_pwm(void);
int32_t init_system(void);

#endif /* APP_CONFIG_H_ */