/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD7134 device application
******************************************************************************
* Copyright (c) 2020-21, 2023 Analog Devices, Inc.
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

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* List of supported platforms*/
#define	MBED_PLATFORM		1
#define STM32_PLATFORM		2

/* List of data capture modes for AD717x device */
#define CONTINUOUS_DATA_CAPTURE		0
#define BURST_DATA_CAPTURE		1

/* AD7134 ASRC mode of operation for data interface */
#define CONTROLLER_MODE		0
#define TARGET_MODE			1

/* List of supported interface modes for data capturing */
#define TDM_MODE	0
#define BIT_BANGING_MODE	1

/* Select the active platform from list of supported platforms */
#define ACTIVE_PLATFORM		MBED_PLATFORM

/* Select the ADC data capture mode (default is CC mode) */
#if !defined(DATA_CAPTURE_MODE)
#define DATA_CAPTURE_MODE	CONTINUOUS_DATA_CAPTURE
#endif

/* Select the platform (default is MBED platform) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM	MBED_PLATFORM
#endif

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

// **** Note for User: ACTIVE_DEVICE selection ****//
/* Define the device type from the list of below device type defines (one at a time)
 * e.g. #define DEV_AD7134 -> This will make AD7134 as an active device.
 * The active device is default set to AD7134 if device type is not defined.
 * */
//#define DEV_AD7134

/* Name of active device */
#if defined (DEV_AD7134)
#define ACTIVE_DEVICE_NAME	"ad7134"
#define HW_MEZZANINE_NAME   "EVAL-AD7134ARDZ"
#define ACTIVE_DEVICE_ID    ID_AD7134
#elif defined (DEV_AD4134)
#define ACTIVE_DEVICE_NAME	"ad4134"
#define HW_MEZZANINE_NAME   "EVAL-AD4134ARDZ"
#define ACTIVE_DEVICE_ID    ID_AD4134
#else
#warning No / Unsupported ADxxxxy symbol defined.AD7134 defined
#define DEV_AD7134
#define ACTIVE_DEVICE_NAME	"ad7134"
#define HW_MEZZANINE_NAME   "EVAL-AD7134ARDZ"
#define ACTIVE_DEVICE_ID    ID_AD7134
#endif

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

/* Note: The STM32 platform supports TDM Mode of data capturing.
 * and the MBED platform supports Bit Banging Mode. */
#if !defined(INTERFACE_MODE)
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#define INTERFACE_MODE TDM_MODE
#else // Mbed
#define INTERFACE_MODE BIT_BANGING_MODE
#endif
#endif

/* Bytes per sample (Note: 2 bytes needed per sample) */
#define	BYTES_PER_SAMPLE	sizeof(uint16_t)

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"

/* Redefine the init params structure mapping w.r.t. platform */
#define uart_extra_init_params stm32_uart_extra_init_params
#define spi_extra_init_params stm32_spi_extra_init_params
#define tdm_extra_init_params stm32_tdm_extra_init_params
#define ext_int_extra_init_params stm32_trigger_gpio_irq_init_params
#define gpio_pdn_extra_init_params stm32_pdn_extra_init_params
#define tdm_platform_ops      stm32_tdm_platform_ops
#define spi_ops 			  stm32_spi_ops
#define uart_ops              stm32_uart_ops
#define gpio_ops			  stm32_gpio_ops
#define trigger_gpio_irq_ops	stm32_gpio_irq_ops
#define i2c_ops				stm32_i2c_ops
#else
#define ACTIVE_PLATFORM		MBED_PLATFORM
#include "app_config_mbed.h"

#define HW_CARRIER_NAME         TARGET_NAME

/* Redefine the init params structure mapping w.r.t. platform */
#define ext_int_extra_init_params mbed_ext_int_extra_init_params
#define uart_extra_init_params mbed_uart_extra_init_params
#define vcom_extra_init_params mbed_vcom_extra_init_params
#define spi_extra_init_params mbed_spi_extra_init_params
#define i2c_extra_init_params mbed_i2c_extra_init_params
#define pwm_extra_init_params mbed_pwm_extra_init_params
#define gpio_pdn_extra_init_params mbed_pdn_extra_init_params
#define trigger_gpio_irq_ops mbed_gpio_irq_ops
#define i2c_ops mbed_i2c_ops
#define gpio_ops mbed_gpio_ops
#define spi_ops mbed_spi_ops
#define uart_ops mbed_uart_ops
#define vcom_ops mbed_virtual_com_ops
#define pwm_ops mbed_pwm_ops
#define trigger_gpio_handle	0
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
#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#define PLATFORM_NAME	HW_CARRIER_NAME
#elif (ACTIVE_PLATFORM == STM32_PLATFORM)
#define PLATFORM_NAME "NUCLEO_L552ZE"
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

/* Set ASRC mode (one at a time, default is AD7134 as controller. The ASRC Mode
 * selection is applicable only for the bit banging method of data capture.
 * and not in case of SAI-TDM method. For TDM, AD7134 can operate only as
 * a controller)
 * Note: The mode configuration must be modified in the hardware to match the one set
 * in software. Refer project wiki document for required h/w changes */
#define AD7134_ASRC_MODE	CONTROLLER_MODE

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
// #define USE_SDRAM		// Uncomment to use SDRAM for data buffer

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

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
extern struct no_os_uart_desc *uart_iio_com_desc;
extern struct no_os_uart_desc *uart_console_stdio_desc;
extern struct no_os_irq_ctrl_desc *external_int_desc;
extern struct no_os_tdm_desc *ad7134_tdm_desc;
extern struct no_os_eeprom_desc *eeprom_desc;
extern struct no_os_gpio_init_param pdn_init_param;
int32_t init_system(void);
int32_t init_pwm(void);
#endif //_APP_CONFIG_H_
