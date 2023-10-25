/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD777x IIO firmware application
******************************************************************************
* Copyright (c) 2022-2023 Analog Devices, Inc.
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

#include <stdio.h>
#include <stdint.h>
#include "no_os_uart.h"
#include "no_os_gpio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* List of supported platforms */
#define STM32_PLATFORM		0
#define MBED_PLATFORM		1

/* List of supported data capture modes */
#define BURST_DATA_CAPTURE		0
#define CONTINUOUS_DATA_CAPTURE		1

/* List of supported interface modes for data capturing */
#define TDM_MODE	0
#define SPI_MODE	1

/* List of ADC types for data capturing */
#define SD_ADC          0
#define SAR_ADC         1

#if !defined (ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		MBED_PLATFORM
#endif

/* Select between the SAR and SD ADC for data capture */
#if !defined(ADC_TYPE)
#define ADC_TYPE        SD_ADC
#endif

/* Select the ADC data capture mode (default is CC mode for SD ADC) */
#if !defined(DATA_CAPTURE_MODE)
/* NOTE: SAR_ADC data capture works only in burst mode */
#if (ADC_TYPE == SAR_ADC)
#define DATA_CAPTURE_MODE 	BURST_DATA_CAPTURE
#else
#define DATA_CAPTURE_MODE	CONTINUOUS_DATA_CAPTURE
#endif // ADC_TYPE
#endif // DATA_CAPTURE_MODE

// **** Note for User: ACTIVE_DEVICE selection ****//
/* Define the device type from the list of below device type defines (one at a time)
 * e.g. #define DEV_AD7770 -> This will make AD7770 as an ACTIVE_DEVICE.
 * The ACTIVE_DEVICE is default set to AD7770 if device type is not defined.
 * */
//#define DEV_AD7770

/* Comment this macro to enable Unipolar mode of operation. Default is Bipolar.
 * NOTE: There needs to be jumper configurations set respective to Bipolar/ Unipolar
 * operation. Please refer to the user guide for the same. */
#define BIPOLAR

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

#if defined(DEV_AD7770)
#define ACTIVE_DEVICE		"ad7770"
#define DEVICE_NAME			"DEV_AD7770"
#define HW_MEZZANINE_NAME   "EVAL-AD7770ARDZ"
#elif defined(DEV_AD7771)
#define ACTIVE_DEVICE		"ad7771"
#define DEVICE_NAME			"DEV_AD7771"
#define HW_MEZZANINE_NAME   "EVAL-AD7771ARDZ"
#elif defined(DEV_AD7779)
#define ACTIVE_DEVICE		"ad7779"
#define DEVICE_NAME			"DEV_AD7779"
#define HW_MEZZANINE_NAME   "EVAL-AD7779ARDZ"
#else
#warning No/Unsupported ADxxxxy symbol defined. AD7770 defined
#define DEV_AD7770
#define ACTIVE_DEVICE	"ad7770"
#define DEVICE_NAME		"DEV_AD7770"
#define HW_MEZZANINE_NAME   "EVAL-AD7770ARDZ"
#endif

/* Note: The STM32 platform supports SPI and TDM Mode of data capturing.
 * and the MBED platform supports only SPI Mode. */
#if !defined(INTERFACE_MODE)
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#define INTERFACE_MODE TDM_MODE
#else // Mbed
#define INTERFACE_MODE SPI_MODE
#endif
#endif

#if (ADC_TYPE == SD_ADC)
/* SD ADC is 32 bit */
#define	BYTES_PER_SAMPLE	sizeof(uint32_t)
#else
/* SAR ADC is 12 bit */
#define	BYTES_PER_SAMPLE	sizeof(uint16_t)
#endif

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"

#define uart_extra_init_params	stm32_uart_extra_init_params
#define spi_extra_init_params	stm32_spi_extra_init_params
#define gpio_reset_extra_init_params	stm32_gpio_reset_extra_init_params
#define gpio_mode0_extra_init_params	stm32_gpio_mode0_extra_init_params
#define gpio_mode1_extra_init_params	stm32_gpio_mode1_extra_init_params
#define gpio_mode2_extra_init_params	stm32_gpio_mode2_extra_init_params
#define gpio_mode3_extra_init_params	stm32_gpio_mode3_extra_init_params
#define gpio_dclk0_extra_init_params	stm32_gpio_dclk0_extra_init_params
#define gpio_dclk1_extra_init_params	stm32_gpio_dclk1_extra_init_params
#define gpio_dclk2_extra_init_params	stm32_gpio_dclk2_extra_init_params
#define gpio_sync_in_extra_init_params	stm32_gpio_sync_in_extra_init_params
#define gpio_convst_sar_extra_init_params	stm32_gpio_convst_sar_extra_init_params
#define gpio_drdy_extra_init_params		stm32_gpio_drdy_extra_init_params
#define gpio_error_extra_init_params	stm32_gpio_error_extra_init_params
#define trigger_gpio_irq_extra_params 	stm32_trigger_gpio_irq_init_params
#define tdm_extra_init_params			stm32_tdm_extra_init_params
#define pwm_extra_init_params   stm32_pwm_extra_init_params

#define spi_platform_ops 	stm32_spi_ops
#define gpio_platform_ops	stm32_gpio_ops
#define trigger_gpio_irq_ops stm32_gpio_irq_ops
#define uart_ops             stm32_uart_ops
#define tdm_platform_ops	stm32_tdm_platform_ops
#define i2c_ops				stm32_i2c_ops
#define pwm_ops				stm32_pwm_ops

#define trigger_gpio_handle	0	// Unused macro

#define HW_CARRIER_NAME		"NUCLEO-L552ZEQ"
#elif (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"

/* Re-define platform specific parameters */
#define uart_extra_init_params mbed_uart_extra_init_params
#define vcom_extra_init_params mbed_vcom_extra_init_params
#define spi_extra_init_params	mbed_spi_extra_init_params
#define i2c_extra_init_params	mbed_i2c_extra_init_params
#define gpio_reset_extra_init_params	mbed_gpio_reset_extra_init_params
#define gpio_mode0_extra_init_params	mbed_gpio_mode0_extra_init_params
#define gpio_mode1_extra_init_params	mbed_gpio_mode1_extra_init_params
#define gpio_mode2_extra_init_params	mbed_gpio_mode2_extra_init_params
#define gpio_mode3_extra_init_params	mbed_gpio_mode3_extra_init_params
#define gpio_dclk0_extra_init_params	mbed_gpio_dclk0_extra_init_params
#define gpio_dclk1_extra_init_params	mbed_gpio_dclk1_extra_init_params
#define gpio_dclk2_extra_init_params	mbed_gpio_dclk2_extra_init_params
#define gpio_sync_in_extra_init_params	mbed_gpio_sync_in_extra_init_params
#define gpio_convst_sar_extra_init_params	mbed_gpio_convst_sar_extra_init_params
#define gpio_drdy_extra_init_params	mbed_gpio_drdy_extra_init_params
#define gpio_error_extra_init_params    mbed_gpio_error_extra_init_params
#define trigger_gpio_irq_extra_params mbed_trigger_gpio_irq_init_params
#define pwm_extra_init_params   mbed_pwm_init_params
#define spi_platform_ops 	mbed_spi_ops
#define i2c_ops 	mbed_i2c_ops
#define gpio_platform_ops	mbed_gpio_ops
#define trigger_gpio_irq_ops	mbed_gpio_irq_ops
#define uart_ops                mbed_uart_ops
#define vcom_ops                mbed_virtual_com_ops
#define pwm_ops                 mbed_pwm_ops
#define trigger_gpio_handle	0	// Unused macro

#define HW_CARRIER_NAME		TARGET_NAME
#endif

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
// #define USE_SDRAM	// Uncomment to use SDRAM as data buffer

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

/* VCOM Serial number definition */
#define	FIRMWARE_NAME	"ad777x_iio"

#if !defined(PLATFORM_NAME)
#define PLATFORM_NAME	HW_CARRIER_NAME
#endif

/* Below USB configurations (VID and PID) are owned and assigned by ADI.
 * If intended to distribute software further, use the VID and PID owned by your
 * organization */
#define VIRTUAL_COM_PORT_VID	0x0456
#define VIRTUAL_COM_PORT_PID	0xb66c

#define VIRTUAL_COM_SERIAL_NUM	(FIRMWARE_NAME "_" DEVICE_NAME "_" STR(PLATFORM_NAME))

/* Baudrate for UART Transactions */
#define IIO_UART_BAUD_RATE	230400

/* Number of channels in the AD777x Family */
#if (ADC_TYPE == SAR_ADC)
/* Choose the SAR input for data capturing. Default is AUXAIN+/AUXAIN.
 * Please refer to the GLOBAL_MUX_CTRL bit in GLOBAL_MUX_CONFIG register
 *  the datasheet for other SAR MUX configurations */
#define SAR_MUX_CONF                    0
#define AD777x_NUM_CHANNELS				1
#else // SD_ADC
#define AD777x_NUM_CHANNELS				8
#endif

/* ADC resolution for active device */
#if (ADC_TYPE == SAR_ADC)
#define ADC_RESOLUTION		12
#else
#define ADC_RESOLUTION		24
#endif

/* ADC max count (full scale value) for unipolar inputs */
#define ADC_MAX_COUNT_UNIPOLAR	(uint32_t)((1 << ADC_RESOLUTION) - 1)

/* ADC max count (full scale value) for bipolar inputs */
#define ADC_MAX_COUNT_BIPOLAR	(uint32_t)(1 << (ADC_RESOLUTION-1))

/* Uncomment this macro to enable external master clock MCLK */
//#define ENABLE_EXT_MCLK

/* Select the external master clock frequency
 * Note: The maximum programmable PWM frequency for mbed platform
 * is only 500kHz */
#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#define AD777X_EXT_MCLK_FREQ    500000
#else
#define AD777X_EXT_MCLK_FREQ    8192000
#endif

/* AD777x Master Clock Frequency */
#if !defined (ENABLE_EXT_MCLK)
#define AD777x_MCLK_FREQ				8192000 // Fixed
#else
#define AD777x_MCLK_FREQ                AD777X_EXT_MCLK_FREQ
#endif

#define AD777x_MCLK_PERIOD	(((float)(1.0 / AD777x_MCLK_FREQ) * 1000000) * 1000)

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

int32_t init_system(void);
void data_capture_callback(void *ctx);
extern struct no_os_uart_desc *uart_iio_com_desc;
extern struct no_os_uart_desc *uart_console_stdio_desc;
extern struct no_os_gpio_desc *gpio_drdy_desc;
extern struct no_os_gpio_desc *gpio_error_desc;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;
extern struct no_os_tdm_desc *ad777x_tdm_desc;
extern struct no_os_eeprom_desc *eeprom_desc;

#endif // APP_CONFIG_H

