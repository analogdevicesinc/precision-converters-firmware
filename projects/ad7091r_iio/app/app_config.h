/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD7091R device application
******************************************************************************
* Copyright (c) 2024 Analog Devices, Inc.
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
#include "ad7091r8.h"
#include "no_os_pwm.h"
#include "no_os_util.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* List of supported platforms */
#define STM32_PLATFORM      1

/* List of data capture modes for AD7091R device */
#define CONTINUOUS_DATA_CAPTURE 0
#define BURST_DATA_CAPTURE      1

/* List of data capture methods supported by hardware platform */
#define SPI_DMA                         0
#define SPI_INTERRUPT                   1

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/* Select the ADC data capture mode (default is CC mode) */
#if !defined(DATA_CAPTURE_MODE)
#define DATA_CAPTURE_MODE BURST_DATA_CAPTURE
#endif

/******************************************************************************/

/* Select the active platform (default is STM32) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

/* Enable the UART port connection */
/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

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

/* Note: The STM32 platform supports SPI interrupt and SPI DMA Mode
 * for data capturing. (Default is SPI DMA mode)
 * */
//#define INTERFACE_MODE SPI_INTERRUPT  // Uncomment to select spi interface mode

#if !defined(INTERFACE_MODE)
#define INTERFACE_MODE   SPI_DMA
#endif

#include "app_config_stm32.h"

/* ADC Internal VREF in millivolts */
#define ADC_INTERNAL_VREF_MV    2500

/* ADC VREF in millivolts (Default is internal reference)
 * Note: When using the internal reference, the on-chip reference value is fixed at 2.5V.
 * External reference value can be applied in range of 1.0V to Vdd */
//#define ADC_VREF_MV     2500

#if !defined(ADC_VREF_MV)
#define ADC_VREF_MV     ADC_INTERNAL_VREF_MV
#endif

/* ADC VDD Input in Volts */
#define ADC_VDD_V   3.3

/* ADC Vref minimum value in Volts */
#define ADC_MIN_VREF    1.0

/* Active device and resolution */
#define ACTIVE_DEVICE_NAME  "ad7091r-8"
#define DEVICE_NAME		    "DEV_AD7091R_8"
#define ACTIVE_DEVICE_ID    AD7091R8
#define ADC_RESOLUTION      12

/* HW ID of the target EVB */
#define HW_MEZZANINE_NAME "EVAL-AD7091R-8ARDZ"

/* ADC max count (full scale value) */
#define ADC_MAX_COUNT   (uint32_t)((1 << ADC_RESOLUTION) - 1)

#define HW_CARRIER_NAME		TARGET_NAME

#define	BYTES_PER_SAMPLE	sizeof(uint16_t)

/****** Macros used to form a VCOM serial number ******/

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* Used to form a VCOM serial number */
#define	FIRMWARE_NAME	"ad7091r_iio"

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

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
//#define USE_SDRAM		// Uncomment to use SDRAM for data buffer

/* PWM period and duty cycle */
#define CONV_TRIGGER_PERIOD_NSEC(x) (((float)(1.0 / x) * 1000000) * 1000)
#define CONV_TRIGGER_DUTY_CYCLE_NSEC(x) (((float)PWM_DUTY_CYCLE_PERCENT / 100) * CONV_TRIGGER_PERIOD_NSEC(x))

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

extern struct no_os_pwm_desc *pwm_desc;
extern struct no_os_uart_desc *uart_iio_com_desc;
extern struct no_os_uart_desc *uart_console_stdio_desc;
extern struct no_os_eeprom_desc *eeprom_desc;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;
extern struct no_os_pwm_init_param pwm_init_params;

#if (INTERFACE_MODE == SPI_DMA)
extern struct no_os_pwm_desc* tx_trigger_desc;
extern struct no_os_pwm_init_param cs_init_params;
extern struct no_os_dma_init_param ad7091r_dma_init_param;
extern struct no_os_gpio_init_param pwm_gpio_params;
extern struct no_os_gpio_init_param cs_pwm_gpio_params;
extern volatile struct iio_device_data* global_iio_dev_data;
extern uint32_t global_nb_of_samples;
extern volatile uint32_t* buff_start_addr;
extern int32_t data_read;
#endif

extern void burst_capture_callback(void *context);
int init_system(void);
int init_pwm_trigger(void);

#endif /* APP_CONFIG_H_ */
