/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD579x device application
******************************************************************************
* Copyright (c) 2023-24 Analog Devices, Inc.
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

/* List of supported platforms */
#define	MBED_PLATFORM		1

/* List of supported DAC data stream modes */
#define CYCLIC_STREAM			0
#define ARBITRARY_STREAM		1

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/******************************************************************************/

/* Select the active platform (default is Mbed) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		MBED_PLATFORM
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

// **** Note for User: ACTIVE_DEVICE selection **** //
/* Define the device type here from the list of below device type defines
 * (one at a time. Defining more than one device can result into compile error).
 * e.g. #define DEV_AD5780 -> This will make AD5780 as an ACTIVE_DEVICE.
 * The ACTIVE_DEVICE is default set to AD5780 if device type is not defined.
 * */

//#define DEV_AD5780

#if defined(DEV_AD5780)
#define ACTIVE_DEVICE_NAME "ad5780"
#define DEVICE_NAME        "DEV_AD5780"
#define ACTIVE_DEVICE	    ID_AD5780
#define HW_MEZZANINE_NAME	"EVAL-AD5780ARDZ"
#define DAC_RESOLUTION      18
#elif defined(DEV_AD5781)
#define ACTIVE_DEVICE_NAME "ad5781"
#define DEVICE_NAME        "DEV_AD5781"
#define ACTIVE_DEVICE		ID_AD5781
#define HW_MEZZANINE_NAME	"EVAL-AD5781ARDZ"
#define DAC_RESOLUTION      18
#elif defined(DEV_AD5790)
#define ACTIVE_DEVICE_NAME "ad5790"
#define DEVICE_NAME        "DEV_AD5790"
#define ACTIVE_DEVICE		ID_AD5790
#define HW_MEZZANINE_NAME	"EVAL-AD5790ARDZ"
#define DAC_RESOLUTION      20
#elif defined(DEV_AD5791)
#define ACTIVE_DEVICE_NAME "ad5791"
#define DEVICE_NAME        "DEV_AD5791"
#define ACTIVE_DEVICE		ID_AD5791
#define HW_MEZZANINE_NAME	"EVAL-AD5791ARDZ"
#define DAC_RESOLUTION      20
#elif defined(DEV_AD5760)
#define ACTIVE_DEVICE_NAME "ad5760"
#define DEVICE_NAME        "DEV_AD5760"
#define ACTIVE_DEVICE		ID_AD5760
#define HW_MEZZANINE_NAME	"EVAL-AD5760ARDZ"
#define DAC_RESOLUTION      16
#else
#warning No/Unsupported ADxxxx symbol defined. AD5780 defined
#define DEV_AD5780
#define ACTIVE_DEVICE_NAME "ad5780"
#define DEVICE_NAME        "DEV_AD5780"
#define ACTIVE_DEVICE		ID_AD5780
#define HW_MEZZANINE_NAME	"EVAL-AD5780ARDZ"
#define DAC_RESOLUTION      18
#endif

/* Define DAC voltage reference here. When internal reference is used,
 * devices AD5781, AD5791 support only one voltage span (-10 to 10V) by
 * default and the devices AD5760, AD5780, AD5790 support two voltage
 * spans based on the jumper setting (-10 to 10V, 0 to 10V).
 * The default voltage reference is internal reference (-10 to 10V). */

//#define INT_REF_0V_TO_10V

#if defined(INT_REF_M10V_TO_10V)  //internal reference bipolar
#define DAC_CH_SPAN 20
#define DAC_VREFN   -10.0
#define DAC_VREFN_GAIN_OF_TWO   -30.0
#elif defined(INT_REF_0V_TO_10V)  //internal reference unipolar
#define DAC_CH_SPAN 10
#define DAC_VREFN   0.0
#define DAC_VREFN_GAIN_OF_TWO   -10.0
#elif defined(EXT_REF)  //external reference
#define DAC_VREFN   -10.0
#define DAC_VREFP   10.0
#define DAC_CH_SPAN (DAC_VREFP-DAC_VREFN)
#else
#warning No/Unsupported Reference selection defined. Internal reference -10V to 10V defined
#define INT_REF_M10V_TO_10V
#define DAC_CH_SPAN 20
#define DAC_VREFN   -10.0
#define DAC_VREFN_GAIN_OF_TWO   -30.0
#endif

/* Number of DAC Channels */
#define AD579X_NUM_CHANNELS 1

/* DAC maximum count */
#define DAC_MAX_COUNT (uint32_t)((1 << DAC_RESOLUTION) - 1)

/* DAC maximum count in offset binary code */
#define DAC_MAX_COUNT_BIN_OFFSET	(uint32_t)((1 << DAC_RESOLUTION) - 1)

/* DAC maximum count in 2s complement code */
#define DAC_MAX_COUNT_2S_COMPL   (uint32_t)(1 << (DAC_RESOLUTION-1))

/* Number of voltage spans possible for linearity compensation error handling */
#define NUM_OF_V_SPANS  5

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
#define gpio_ops               mbed_gpio_ops
#define spi_ops mbed_spi_ops
#define i2c_ops mbed_i2c_ops
#define uart_ops mbed_uart_ops
#define vcom_ops mbed_virtual_com_ops
#define pwm_ops mbed_pwm_ops
#define trigger_gpio_irq_ops mbed_gpio_irq_ops
#else
#error "No/Invalid active platform selected"
#endif

/****** Macros used to form a VCOM serial number ******/

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* Used to form a VCOM serial number */
#define	FIRMWARE_NAME	"ad579x_iio"

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

/* Enable/Disable the use of SDRAM for DAC data streaming buffer */
//#define USE_SDRAM		// Uncomment to use SDRAM for data buffer

/* PWM period and duty cycle */
#define CONV_PERIOD_NSEC(x)		(((float)(1.0 / x) * 1000000) * 1000)
#define CONV_DUTY_CYCLE_NSEC(x)	(CONV_PERIOD_NSEC(x) / 2)

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

extern struct no_os_pwm_desc *pwm_desc;
extern struct no_os_uart_desc *uart_iio_com_desc;
extern struct no_os_uart_desc *uart_console_stdio_desc;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;
extern struct no_os_eeprom_desc *eeprom_desc;

int32_t init_pwm_trigger(void);
int32_t init_system(void);

#endif /* APP_CONFIG_H_ */