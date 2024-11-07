/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Application configurations module for AD405X
******************************************************************************
* Copyright (c) 2022-2024 Analog Devices, Inc.
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
#include "no_os_gpio.h"
#include "no_os_uart.h"
#include "no_os_irq.h"
#include "no_os_pwm.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* List of supported platforms */
#define	MBED_PLATFORM		1
#define STM32_PLATFORM		2

/* List of data capture modes supported by application*/
#define CONTINUOUS_DATA_CAPTURE		    0
#define WINDOWED_DATA_CAPTURE			1

/* List of supported data capture modes by device */
#define SAMPLE_MODE			            0
#define BURST_AVERAGING_MODE		    1
#define AVERAGING_MODE					2

/* List of supported data format by device */
#define STRAIGHT_BINARY		            0
#define TWOS_COMPLEMENT	                1

/* List of data capture methods supported by hardware platform */
#define SPI_DMA                         0
#define SPI_INTERRUPT                   1

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/******************************************************************************/

/* Select the active platform (default is Mbed) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

/* Select the application data capture mode (default is CC mode) */
#if !defined(APP_CAPTURE_MODE)
#define APP_CAPTURE_MODE	   WINDOWED_DATA_CAPTURE
#endif

/* Select the ADC data capture mode (default is sample mode) */
#if !defined(ADC_CAPTURE_MODE)
#define ADC_CAPTURE_MODE	   SAMPLE_MODE
#endif

/* Select the ADC output data format (default is twos complement mode) */
#if !defined(ADC_DATA_FORMAT)
#define ADC_DATA_FORMAT	       TWOS_COMPLEMENT
#endif

/* Note: The STM32 platform supports SPI interrupt and SPI DMA Mode
 * for data capturing. The MBED platform supports only SPI interrupt mode
 * */
#if !defined(INTERFACE_MODE)
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#define INTERFACE_MODE   SPI_DMA
#else // Mbed
#define INTERFACE_MODE   SPI_INTERRUPT
#endif
#endif

// **** Note for User on selection of Active Device ****//
/* Define the device type here from the list of below device type defines
 * (one at a time. Defining more than one device can result into compile error).
 * e.g. #define DEV_AD4050 -> This will make AD4050 as an active device.
 * The active device is default set to AD4052 if device type is not defined.
 * */
// #define DEV_AD4050

#if defined(DEV_AD4052)
#define ACTIVE_DEVICE_NAME	"ad4052"
#define DEVICE_NAME			"DEV_AD4052"
#define ACTIVE_DEVICE_ID	 ID_AD4052
#define HW_MEZZANINE_NAME	"EVAL-AD4052-ARDZ"
#define ADC_SAMPLE_MODE_RESOLUTION		    16
#define ADC_AVERAGING_MODE_RESOLUTION		20
#define ADC_BURST_AVG_MODE_RESOLUTION		20
#elif defined(DEV_AD4050)
#define ACTIVE_DEVICE_NAME	"ad4050"
#define DEVICE_NAME			"DEV_AD4050"
#define ACTIVE_DEVICE_ID	 ID_AD4050
#define HW_MEZZANINE_NAME	"EVAL-AD4050-ARDZ"
#define ADC_SAMPLE_MODE_RESOLUTION		    12
#define ADC_AVERAGING_MODE_RESOLUTION		14
#define ADC_BURST_AVG_MODE_RESOLUTION		14
#else
#define ACTIVE_DEVICE_NAME	"ad4052"
#define DEVICE_NAME			"DEV_AD4052"
#define ACTIVE_DEVICE_ID	 ID_AD4052
#define HW_MEZZANINE_NAME	"EVAL-AD4052-ARDZ"
#define ADC_SAMPLE_MODE_RESOLUTION		    16
#define ADC_AVERAGING_MODE_RESOLUTION		20
#define ADC_BURST_AVG_MODE_RESOLUTION		20
#endif

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"
#define HW_CARRIER_NAME		    	TARGET_NAME
#elif (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#define HW_CARRIER_NAME		    	TARGET_NAME
#if (INTERFACE_MODE != SPI_DMA)
#define trigger_gpio_handle         0    // Unused macro
#else
#define trigger_gpio_handle         STM32_DMA_CONT_HANDLE
#endif
#if (INTERFACE_MODE == SPI_DMA)
#define TRIGGER_INT_ID	            STM32_DMA_CONT_TRIGGER
#else
#define TRIGGER_INT_ID	            GP1_PIN_NUM
#endif
#else
#error "No/Invalid active platform selected"
#endif

#define DEFAULT_BURST_SAMPLE_RATE           2000000

/* ADC reference voltage (Range: 2.5 to 3.3v) */
#define ADC_REF_VOLTAGE		2.5

#if (ADC_CAPTURE_MODE == SAMPLE_MODE)
#if (ADC_DATA_FORMAT == STRAIGHT_BINARY)
#define ADC_MAX_COUNT 	(uint32_t)(1 << (ADC_SAMPLE_MODE_RESOLUTION))
#else
#define ADC_MAX_COUNT 	(uint32_t)(1 << (ADC_SAMPLE_MODE_RESOLUTION - 1))
#endif
#else
#if (ADC_DATA_FORMAT == STRAIGHT_BINARY)
#define ADC_MAX_COUNT 	(uint32_t)(1 << (ADC_BURST_AVG_MODE_RESOLUTION))
#else
#define ADC_MAX_COUNT 	(uint32_t)(1 << (ADC_BURST_AVG_MODE_RESOLUTION - 1))
#endif
#endif

/* Time taken for the application to process the interrupt and
 * push data into iio buffer. */
#define MIN_DATA_CAPTURE_TIME_NS       8000

/* Time taken by the MCU to Jump into the ISR after the occurrence of
 * data ready event */
#if (APP_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
#define MIN_INTERRUPT_OVER_HEAD        4500
#else
#define MIN_INTERRUPT_OVER_HEAD			3000
#endif

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/****** Macros used to form a VCOM serial number ******/
/* Used to form a VCOM serial number */
#define	FIRMWARE_NAME	"ad405x_iio"

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

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
//#define USE_SDRAM	// Uncomment to use SDRAM as data buffer

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
extern struct no_os_pwm_desc *pwm_desc;
extern struct no_os_uart_desc *uart_iio_com_desc;
extern struct no_os_uart_desc *uart_console_stdio_desc;
extern struct no_os_gpio_desc *trigger_gpio_desc;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;
extern struct no_os_eeprom_desc *eeprom_desc;
extern struct no_os_gpio_init_param cs_pwm_gpio_params;
extern struct no_os_gpio_init_param pwm_gpio_params;

#if (INTERFACE_MODE == SPI_DMA)
extern struct no_os_dma_xfer_desc dma_tx_desc;
extern struct no_os_dma_ch dma_chan;
extern struct no_os_pwm_init_param cs_init_params;
extern struct no_os_pwm_init_param pwm_init_params;
extern struct no_os_dma_init_param ad405x_dma_init_param;
extern struct no_os_gpio_init_param cs_pwm_gpio_params;
extern struct no_os_gpio_init_param pwm_gpio_params;
extern volatile uint32_t* buff_start_addr;
extern volatile struct iio_device_data* iio_dev_data_g;
extern uint32_t nb_of_samples_g;
extern int32_t data_read;
extern struct no_os_pwm_desc* tx_trigger_desc;
#endif

int32_t init_pwm(void);
int32_t ad405x_gpio_reset(void);
int32_t init_system(void);

#endif //APP_CONFIG_H
