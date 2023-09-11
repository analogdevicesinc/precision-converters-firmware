/***************************************************************************//*
 * @file    app_config.h
 * @brief   Header file for application configurations (platform-agnostic)
******************************************************************************
 * Copyright (c) 2023 Analog Devices, Inc.
 * Copyright (c) 2023 BayLibre, SAS
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/

#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <limits.h>
/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* List of supported platforms */
#define	MBED_PLATFORM		1

/* List of data capture modes */
#define CONTINUOUS_DATA_CAPTURE		0
#define BURST_DATA_CAPTURE			1

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/******************************************************************************/

/**** ACTIVE_DEVICE selection *****
* Define the device type here from the available list of devices (one at a time)
* e.g. #define DEV_AD2S1210 -> This will make AD2S1210 as an ACTIVE_DEVICE.
**/
/* #define	DEV_AD2S1210 */

/* Name of the active device */
#if !defined(DEV_AD2S1210)
#define DEV_AD2S1210
#define ACTIVE_DEVICE		ID_AD2S1210
#define ACTIVE_DEVICE_NAME	"ad2s1210"
#define DEVICE_NAME		    "DEV_AD2S1210"
#endif

/* Select the active platform (default is Mbed) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		MBED_PLATFORM
#endif

/* Select the RESOLVER data capture mode (default is CC mode) */
#if !defined(DATA_CAPTURE_MODE)
#define DATA_CAPTURE_MODE	CONTINUOUS_DATA_CAPTURE
#endif

/* Enable the UART/VirtualCOM port connection (default VCOM) */
/* #define USE_PHY_COM_PORT		// Uncomment to select UART */

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"

#define HW_CARRIER_NAME         TARGET_NAME

/* Redefine the init params structure mapping w.r.t. platform */
#define pwm_extra_init_params mbed_pwm_extra_init_params
#define uart_extra_init_params mbed_uart_extra_init_params
#define vcom_extra_init_params mbed_vcom_extra_init_params
#define spi_extra_init_params mbed_spi_extra_init_params
#define trigger_gpio_irq_extra_params mbed_trigger_gpio_irq_init_params
#define trigger_gpio_extra_init_params mbed_trigger_gpio_extra_init_params
#define trigger_gpio_ops mbed_gpio_ops
#define irq_ops mbed_gpio_irq_ops
#define gpio_ops mbed_gpio_ops
#define spi_ops mbed_spi_ops
#define uart_ops mbed_uart_ops
#define vcom_ops mbed_virtual_com_ops
#define pwm_ops mbed_pwm_ops
#define trigger_gpio_irq_ops mbed_gpio_irq_ops
#define trigger_gpio_handle 0	/* Unused macro */
#define IRQ_INT_ID GPIO_IRQ_ID1
#define TRIGGER_GPIO_PORT 0  /* Unused macro */
#define TRIGGER_GPIO_PIN  PWM_TRIGGER
#define TRIGGER_INT_ID	GPIO_IRQ_ID1
#else
#error "No/Invalid active platform selected"
#endif

/* Expected HW ID */
#define HW_MEZZANINE_NAME	"EVAL-AD2S1210SDZ"
#define HW_NAME			"ad2s1210"
#define HW_VENDOR		"Analog Devices"
#define NUM_CTX_ATTR		4

#define RESOLVER_CHANNELS		 3
#define RESOLVER_MAX_ATTR		10

/* Max count is always 16 bit. LSBs are ignored in lower resolutions */
#define RESOLVER_MAX_COUNT_UNIPOLAR	(uint32_t)(USHRT_MAX)
#define RESOLVER_MAX_COUNT_BIPOLAR	(uint32_t)(SHRT_MAX)

/* Not all resolutions are supported use driver defined resolutions 10, 12, 14, 16 */
#define AD2S1210_RESOLUTION		AD2S1210_RES_16BIT

#define AD2S1210_FCLKIN		8192000

#define MATH_PI                   3.1415926f
/* velocity scale depends on resolution */
#define AD2S1210_POS_IIO_SCALE		(2 * MATH_PI / RESOLVER_MAX_COUNT_UNIPOLAR)

#if (AD2S1210_FCLKIN == 8192000)
#define AD2S1210_TRACKING_RATE_10BIT    2500
#define AD2S1210_TRACKING_RATE_12BIT    1000
#define AD2S1210_TRACKING_RATE_14BIT    500
#define AD2S1210_TRACKING_RATE_16BIT    125
#elif (AD2S1210_FCLKIN == 10240000)
#define AD2S1210_TRACKING_RATE_10BIT    3125
#define AD2S1210_TRACKING_RATE_12BIT    1250
#define AD2S1210_TRACKING_RATE_14BIT    625
#define AD2S1210_TRACKING_RATE_16BIT    156
#if (AD2S1210_FCLKIN != 10240000)
#warn "unknown tracking rate"
#endif
#endif

/****** Macros used to form a VCOM serial number ******/
/* Used to form a VCOM serial number */
#define	FIRMWARE_NAME	"ad2s1210_iio"

#if !defined(PLATFORM_NAME)
#define PLATFORM_NAME	HW_CARRIER_NAME
#endif

/* Below USB configurations (VID and PID) are owned and assigned by ADI.
 * If intended to distribute software further, use the VID and PID owned by your
 * organization
 */
#define VIRTUAL_COM_PORT_VID	0x0456
#define VIRTUAL_COM_PORT_PID	0xb66c
#define VIRTUAL_COM_SERIAL_NUM	(FIRMWARE_NAME "_" DEVICE_NAME "_" STR(PLATFORM_NAME))

#if defined(USE_PHY_COM_PORT)
/* If PHY com is selected, VCOM or alternate PHY com port can act as a console stdio port */
#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif
#else
/* If VCOM is selected, PHY com port will/should act as a console stdio port */
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif

/* Default baud rate for IIO UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* Enable/Disable the use of SDRAM for RESOLVER data capture buffer */
/* #define USE_SDRAM		// Uncomment to use SDRAM for data buffer */

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct no_os_uart_desc *uart_desc;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;

int32_t init_system(void);
int32_t init_pwm_trigger(void);

#endif /* _APP_CONFIG_H_ */
