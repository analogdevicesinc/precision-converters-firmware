/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD469x device applications
******************************************************************************
* Copyright (c) 2021-24 Analog Devices, Inc.
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
#include <common_macros.h>

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* List of data capture modes for AD469X device */
#define CONTINUOUS_DATA_CAPTURE		0
#define BURST_DATA_CAPTURE			1

/* List of available polarity modes */
#define UNIPOLAR_MODE           0
#define PSEUDO_BIPOLAR_MODE     1

/* List of data capture methods supported by hardware platform */
#define SPI_DMA                         0
#define SPI_INTERRUPT                   1

/* Select the ADC data capture mode (default is CC mode) */
#if !defined(DATA_CAPTURE_MODE)
#define DATA_CAPTURE_MODE	  CONTINUOUS_DATA_CAPTURE
#endif

/* Select the active platform (default is stm32) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

/* Note: The STM32 platform supports SPI interrupt and SPI DMA Mode
 * for data capturing. The MBED platform supports only SPI interrupt mode
 * */
#if !defined(INTERFACE_MODE)
#define INTERFACE_MODE   SPI_DMA
#endif

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#define HW_CARRIER_NAME		    	TARGET_NAME
/* Redefine the init params structure mapping w.r.t. platform */
#define uart_extra_init_params        stm32_uart_extra_init_params
#define spi_extra_init_params         stm32_spi_extra_init_params
#define cnv_extra_init_params         stm32_gpio_cnv_extra_init_params
#define pwm_extra_init_params         stm32_pwm_cnv_extra_init_params
#define pwm_gpio_extra_init_params    stm32_pwm_gpio_extra_init_params
#define bsy_extra_init_params         stm32_gpio_gp0_extra_init_params
#define gp1_extra_init_params         stm32_gpio_gp1_extra_init_params
#define trigger_gpio_irq_extra_params stm32_gpio_irq_extra_init_params
#define reset_extra_init_params       stm32_gpio_reset_extra_init_params
#define cs_extra_init_params          stm32_cs_extra_init_params
#define tx_trigger_extra_init_params  stm32_tx_trigger_extra_init_params
#define cs_pwm_gpio_extra_init_params stm32_cs_pwm_gpio_extra_init_params
#define vcom_extra_init_params      stm32_vcom_extra_init_params
#else
#error "No/Invalid active platform selected"
#endif

// **** Note for User: ACTIVE_DEVICE selection **** //
/* Define the device type here from the list of below device type defines
 * (one at a time. Defining more than one device can result into compile error).
 * e.g. #define DEV_AD4696 -> This will make AD4696 as an ACTIVE_DEVICE.
 * The ACTIVE_DEVICE is default set to AD4696 if device type is not defined.
 * */

//#define DEV_AD4696

#if defined(DEV_AD4696)
#define ACTIVE_DEVICE		ID_AD4696
#define ACTIVE_DEVICE_NAME	"ad4696"
#define HW_MEZZANINE_NAME	"EVAL-AD4696-ARDZ"
#define	NO_OF_CHANNELS		16
#elif defined(DEV_AD4697)
#define ACTIVE_DEVICE		ID_AD4697
#define ACTIVE_DEVICE_NAME	"ad4697"
#define HW_MEZZANINE_NAME	"EVAL-AD4697-ARDZ"
#define	NO_OF_CHANNELS		8
#else
#warning No/Unsupported ADxxxxy symbol defined. AD4696 defined
#define DEV_AD4696
#define ACTIVE_DEVICE		ID_AD4696
#define ACTIVE_DEVICE_NAME	"ad4696"
#define HW_MEZZANINE_NAME	"EVAL-AD4696-ARDZ"
#define	NO_OF_CHANNELS		16
#endif

#define ADC_RESOLUTION		16

// **** Note for User: Polarity Mode selection **** //
/* Select the polarity mode. Default is unipolar.
 * e.g. #define PSEUDO_BIPOLAR_MODE -> This will enable the PSEUDO_BIPOLAR_MODE
 * for all the channels.
 * */
#define DEFAULT_POLARITY_MODE    UNIPOLAR_MODE

/* ADC max count (full scale value) for unipolar inputs */
#define ADC_MAX_COUNT_UNIPOLAR	(uint32_t)((1 << ADC_RESOLUTION) - 1)

/* ADC max count (full scale value) for bipolar inputs */
#define ADC_MAX_COUNT_BIPOLAR	(uint32_t)(1 << (ADC_RESOLUTION-1))

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/****** Macros used to form a VCOM serial number ******/
#if !defined(DEVICE_NAME)
#define DEVICE_NAME		"DEV_AD4697"
#endif

/* Used to form a VCOM serial number */
#define	FIRMWARE_NAME	"ad469x_iio"

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

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* Check if any serial port available for use as console stdio port */
#if defined(USE_VIRTUAL_COM_PORT)
/* If VCOM is selected, PHY com port will/should act as a console stdio port */
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
//#define USE_SDRAM  	// Uncomment to use SDRAM as data buffer

/* Bytes per sample. This count should divide the total 256 bytes into 'n' equivalent
 * ADC samples as IIO library requests only 256bytes of data at a time in a given
 * data read query.
 * For 1 to 8-bit ADC, bytes per sample = 1 (2^0)
 * For 9 to 16-bit ADC, bytes per sample = 2 (2^1)
 * For 17 to 32-bit ADC, bytes per sample = 4 (2^2)
 **/
#define	BYTES_PER_SAMPLE	sizeof(uint16_t)	// For ADC resolution of 16-bits

/* Number of data storage bits (needed for IIO client to plot ADC data) */
#define CHN_STORAGE_BITS	(BYTES_PER_SAMPLE * 8)

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
extern struct no_os_uart_desc *uart_iio_com_desc;
extern struct no_os_pwm_desc *pwm_desc;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;
extern struct no_os_eeprom_desc *eeprom_desc;
#if (INTERFACE_MODE == SPI_DMA)
extern struct no_os_pwm_init_param cs_init_params;
extern struct no_os_dma_init_param ad469x_dma_init_param;
extern struct no_os_gpio_init_param pwm_gpio_params;
extern struct no_os_gpio_init_param cs_pwm_gpio_params;
extern volatile struct iio_device_data* global_iio_dev_data;
extern struct no_os_pwm_init_param pwm_init_params;
extern uint32_t global_nb_of_samples;
extern volatile uint32_t* buff_start_addr;
extern int32_t data_read;
extern struct no_os_pwm_desc* tx_trigger_desc;
#endif

/* Initializing system peripherals */
int32_t init_pwm(void);
int32_t init_system(void);
/* callback function in burst mode */
extern void burst_capture_callback(void *context);

#endif //APP_CONFIG_H
