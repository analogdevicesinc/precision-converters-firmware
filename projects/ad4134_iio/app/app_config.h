/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD4134 device application
******************************************************************************
* Copyright (c) 2020-21, 2023-25 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/
#ifndef _APP_CONFIG_H_
#define _APP_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include "ad713x.h"
#include "common_macros.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* List of data capture modes for AD717x device */
#define CONTINUOUS_DATA_CAPTURE		0
#define BURST_DATA_CAPTURE			1

/* AD7134 ASRC mode of operation for data interface */
#define CONTROLLER_MODE		0
#define TARGET_MODE			1

/* List of supported interface modes for data capturing */
#define TDM_MODE			0
#define BIT_BANGING_MODE	1

/* Select the ADC data capture mode (default is CC mode) */
#if !defined(DATA_CAPTURE_MODE)
#define DATA_CAPTURE_MODE	CONTINUOUS_DATA_CAPTURE
#endif

/* Select the platform (default is STM32 platform) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

// **** Note for User: ACTIVE_DEVICE selection ****//
/* Define the device type from the list of below device type defines (one at a time)
 * e.g. #define DEV_AD7134 -> This will make AD7134 as an active device.
 * The active device is default set to AD4134 if device type is not defined.
 * */
//#define DEV_AD4134

/* Name of active device */
#if defined (DEV_AD7134)
#define ACTIVE_DEVICE_NAME	"ad7134"
#define HW_MEZZANINE_NAME   "EVAL-AD7134ARDZ"
#define ACTIVE_DEVICE_ID    ID_AD7134
#elif defined (DEV_AD4134)
#define ACTIVE_DEVICE_NAME	"ad4134"
#define HW_MEZZANINE_NAME   "EVAL-CN0561-ARDZ"
#define ACTIVE_DEVICE_ID    ID_AD4134
#else
#define INFO_MSG(x) _Pragma(#x)
INFO_MSG(message("No / Unsupported ADxxxxy symbol defined.AD4134 defined"))
#define DEV_AD4134
#define ACTIVE_DEVICE_NAME	"ad4134"
#define HW_MEZZANINE_NAME   "EVAL-CN0561-ARDZ"
#define ACTIVE_DEVICE_ID    ID_AD4134
#endif

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

/* Note:
 * 1. In the STM32 platform, SDPK1 supports Bit Banging Mode.
 * 2. In the STM32 platform, Nucleo-H563 supports TDM Mode.
 */
#if !defined(INTERFACE_MODE)
#define INTERFACE_MODE 		BIT_BANGING_MODE
#endif

/* Bytes per sample (Note: 2 bytes needed per sample) */
#define	BYTES_PER_SAMPLE	sizeof(uint16_t)

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"

#ifdef STM32F469xx
#define HW_CARRIER_NAME         SDP_K1
#else
#define HW_CARRIER_NAME         NUCLEO_H563ZI
#endif

/* Redefine the init params structure mapping w.r.t. platform */
#define vcom_extra_init_params stm32_vcom_extra_init_params
#define uart_extra_init_params stm32_uart_extra_init_params
#define spi_extra_init_params stm32_spi_extra_init_params
#define tdm_extra_init_params stm32_tdm_extra_init_params
#define ext_int_extra_init_params stm32_trigger_gpio_irq_init_params
#define gpio_pdn_extra_init_params stm32_pdn_extra_init_params
#define gpio_input_extra_init_params stm32_input_extra_init_params
#define gpio_output_extra_init_params stm32_output_extra_init_params
#define i2c_extra_init_params stm32_i2c_extra_init_params
#define tdm_platform_ops      stm32_tdm_platform_ops
#define spi_ops 			  stm32_spi_ops
#define uart_ops              stm32_uart_ops
#define gpio_ops			  stm32_gpio_ops
#define pwm_ops               stm32_pwm_ops
#define pwm_extra_init_params stm32_pwm_extra_init_params
#define trigger_gpio_irq_ops	stm32_gpio_irq_ops
#ifdef STM32F469xx
#define vcom_ops    stm32_usb_uart_ops
#endif
#define i2c_ops				stm32_i2c_ops
#endif // ACTIVE_PLATFORM

/* ADC resolution for active device
 * Note: Data capture interface is designed to for 16-bit data format only */
#define ADC_RESOLUTION		16

/* ADC max count (full scale value) for unipolar inputs */
#define ADC_MAX_COUNT_UNIPOLAR	(uint32_t)((1 << ADC_RESOLUTION) - 1)

/* ADC max count (full scale value) for bipolar inputs */
#define ADC_MAX_COUNT_BIPOLAR	(uint32_t)(1 << (ADC_RESOLUTION-1))

/* Max ADC channels */
#define AD7134_NUM_CHANNELS	4

/****** Macros used to form a VCOM serial number ******/
#define DEVICE_NAME		"DEV_AD7134"

#if !defined(PLATFORM_NAME)
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#define PLATFORM_NAME HW_CARRIER_NAME
#endif // ACTIVE_PLATFORM
#endif // PLATFORM_NAME

/* VCOM Serial number definition */
#define	FIRMWARE_NAME	"ad7134_iio"

/* Below USB configurations (VID and PID) are owned and assigned by ADI.
* If intended to distribute software further, use the VID and PID owned by your
* organization */
#define VIRTUAL_COM_PORT_VID	0x0456
#define VIRTUAL_COM_PORT_PID	0xb66c
#define VIRTUAL_COM_SERIAL_NUM	(FIRMWARE_NAME "_" DEVICE_NAME "_" STR(PLATFORM_NAME))

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* Set ASRC mode (one at a time, default is AD7134 as controller.)
 * The ASRC Mode selection is applicable only for the bit banging method.
 * For TDM, AD7134 can operate only as a controller.
 * Note: The mode configuration must be modified in the hardware to match the one set
 * in software. Refer project documentation for required h/w changes */
#define AD7134_ASRC_MODE	CONTROLLER_MODE

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
// #define USE_SDRAM		// Uncomment to use SDRAM for data buffer

/* Check if any serial port available for use as console stdio port */
#if defined(USE_PHY_COM_PORT)
/* If PHY com is selected, VCOM or alternate PHY com port can act as a console stdio port */
#if (ACTIVE_PLATFORM == STM32_PLATFORM && INTERFACE_MODE == BIT_BANGING_MODE)
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif
#else
/* If VCOM is selected, PHY com port will/should act as a console stdio port */
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
extern struct no_os_uart_desc *uart_iio_com_desc;
extern struct no_os_uart_desc *uart_console_stdio_desc;
extern struct no_os_irq_ctrl_desc *external_int_desc;
extern struct no_os_tdm_desc *ad7134_tdm_desc;
extern struct no_os_eeprom_desc *eeprom_desc;
extern struct no_os_gpio_init_param pdn_init_param;
extern struct no_os_pwm_desc *pwm_desc;

int32_t init_system(void);
int32_t init_pwm(void);

#endif //_APP_CONFIG_H_
