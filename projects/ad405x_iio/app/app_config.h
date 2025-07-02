/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Application configurations module for AD405X
******************************************************************************
* Copyright (c) 2022-2025 Analog Devices, Inc.
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
#include "common.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* List of supported platforms */
#define	MBED_PLATFORM		1
#define STM32_PLATFORM		2

/* List of data capture modes supported by application*/
#define CONTINUOUS_DATA_CAPTURE		    0
#define WINDOWED_DATA_CAPTURE			1

/* List of supported data format by device */
#define STRAIGHT_BINARY		            0
#define TWOS_COMPLEMENT	                1

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/******************************************************************************/

/* Select the active platform (default is STM32) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

/* Enable the UART/VirtualCOM port connection (default VCOM) */
// #define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

/* Select the application data capture mode (default is Windowed mode) */
#if !defined(APP_CAPTURE_MODE)
#define APP_CAPTURE_MODE	   WINDOWED_DATA_CAPTURE
#endif

/* Select the ADC output data format (default is twos complement mode) */
#if !defined(ADC_DATA_FORMAT)
#define ADC_DATA_FORMAT	       TWOS_COMPLEMENT
#endif

#define ACTIVE_DEVICE_NAME	"ad405x"
#define DEVICE_NAME			"DEV_AD405x"

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"
#define HW_CARRIER_NAME		    	TARGET_NAME
#define CONSOLE_STDIO_PORT_AVAILABLE
#elif (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#define HW_CARRIER_NAME		    	TARGET_NAME
#define IIO_TRIGGER_HANDLE_SPI      0    // Unused macro
#define TRIGGER_INT_ID_SPI_INTR     GP1_PIN_NUM
#define IIO_TRIGGER_HANDLE_I3C      CNV_TIMER_HANDLE
#define TRIGGER_INT_ID_I3C_INTR	    CNV_PWM_TIMER_IRQ_ID
#else
#error "No/Invalid active platform selected"
#endif

#define DEFAULT_BURST_SAMPLE_RATE           2000000

/* ADC reference voltage (Range: 2.5 to 3.3v) */
#define ADC_REF_VOLTAGE		2.5

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

/*
 * Console STDIO on VCOM is supported only if the client is
 * actively listening to the data on VCOM.
 *
 * Comment the below section to enable STDIO on VCOM.
 */
#ifdef USE_PHY_COM_PORT
#ifdef CONSOLE_STDIO_PORT_AVAILABLE
#undef CONSOLE_STDIO_PORT_AVAILABLE
#endif
#endif

/* ADC data buffer size */
#if defined(SDRAM_SUPPORT_AVAILABLE)
#define DATA_BUFFER_SIZE			SDRAM_SIZE_BYTES
#else
#ifdef SPI_SUPPORT_AVAILABLE
#define DATA_BUFFER_SIZE			(131072)		// 128kbytes
#define DUMMY_DATA_COUNT			0
#else
#define DATA_BUFFER_SIZE			(524288)		// 512kbytes
#define DUMMY_DATA_COUNT			1
#endif
#endif
#define DATA_BUFFER_SIZE_CONT		64000

/******************************************************************************/

#define STORAGE_BITS_SAMPLE 16
#define AD4050_SAMPLE_RES   12
#define AD4052_SAMPLE_RES   16
#define AD4060_SAMPLE_RES   12
#define AD4062_SAMPLE_RES   16

#define STORAGE_BITS_AVG    32
#define AD4050_AVG_RES      14
#define AD4052_AVG_RES      20
#define AD4060_AVG_RES      14
#define AD4062_AVG_RES      20

/* Number of storage bytes for each sample */
#define BYTES_PER_SAMPLE(x)   (x/8)

/* Converts pwm period in nanoseconds to sampling frequency in samples per second */
#define PWM_PERIOD_TO_FREQUENCY(x)       (1000000000.0 / x)
/* Converts sampling frequency in samples per second to pwm period in nanoseconds */
#define PWM_FREQUENCY_TO_PERIOD(x)       PWM_PERIOD_TO_FREQUENCY(x)

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
extern struct no_os_eeprom_init_param eeprom_init_params;
extern struct no_os_pwm_desc *pwm_desc;
extern struct no_os_irq_ctrl_desc *pwm_irq_desc;
extern struct no_os_uart_desc *uart_iio_com_desc;
extern struct no_os_uart_desc *uart_console_stdio_desc;
extern struct no_os_gpio_desc *trigger_gpio_desc;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;
extern struct no_os_gpio_init_param cs_pwm_gpio_params;
extern struct no_os_gpio_init_param pwm_gpio_params;
extern struct no_os_pwm_init_param spi_dma_pwm_init_params;
extern struct no_os_pwm_init_param spi_intr_pwm_init_params;
extern struct no_os_dma_xfer_desc dma_tx_desc;
extern struct no_os_dma_ch dma_chan;
extern struct no_os_pwm_init_param cs_init_params;
extern struct no_os_dma_init_param ad405x_dma_init_param;
extern volatile uint8_t *buff_start_addr;
extern volatile struct iio_device_data* iio_dev_data_g;
extern uint32_t nb_of_bytes_g;
extern uint32_t nb_of_bytes_remaining_g;
extern uint32_t data_read;
extern struct no_os_pwm_desc* tx_trigger_desc;
extern struct no_os_dma_desc *ad405x_dma_desc;
extern struct no_os_pwm_desc* cs_pwm_desc;
extern struct no_os_dma_xfer_desc i3c_cr_dma_xfer;
extern struct no_os_pwm_init_param i3c_dma_pwm_init_params;
extern struct no_os_pwm_init_param i3c_intr_pwm_init_params;
extern volatile uint8_t ad405x_i3c_dyn_addr;
extern uint8_t bytes_per_sample;

int32_t init_pwm(void);
int32_t deinit_pwm(void);
int32_t ad405x_gpio_reset(void);
int32_t init_system(void);
int32_t init_system_post_verification(void);

#endif //APP_CONFIG_H
