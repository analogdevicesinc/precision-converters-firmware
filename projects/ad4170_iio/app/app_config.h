/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for AD4170 device application
******************************************************************************
* Copyright (c) 2021-24 Analog Devices, Inc.
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
#include "ad4170.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/* List of supported platforms */
#define	MBED_PLATFORM		1
#define STM32_PLATFORM		2

/* List of demo mode configurations */
#define		USER_DEFAULT_CONFIG		0
#define		RTD_2WIRE_CONFIG		1
#define		RTD_3WIRE_CONFIG		2
#define		RTD_4WIRE_CONFIG		3
#define		THERMISTOR_CONFIG		4
#define		THERMOCOUPLE_CONFIG		5
#define		ACCELEROMETER_CONFIG	6
#define		LOADCELL_CONFIG			7

/* List of data capture modes for AD4170 device */
#define CONTINUOUS_DATA_CAPTURE		0
#define BURST_DATA_CAPTURE			1

/* List of supported interface modes for data capturing */
#define TDM_MODE	0
#define SPI_INTERRUPT_MODE	1
#define SPI_DMA_MODE    2

/* List of supported IIO clients
 * Note: Local client is supported only for Mbed platform
 * for now
 * */
#define IIO_CLIENT_REMOTE	0	// Remote (PC) IIO client
#define IIO_CLIENT_LOCAL	1	// Local (display) IIO client

// **** Note for User: ACTIVE_DEVICE selection ****//
/* Define the device type from the list of below device type defines (one at a time)
 * e.g. #define DEV_AD4170 -> This will make AD4170 as an ACTIVE_DEVICE.
 * The ACTIVE_DEVICE is default set to AD4170 if device type is not defined.
 * */
//#define DEV_AD4170

#if defined(DEV_AD4170)
#define ACTIVE_DEVICE_NAME	"ad4170"
#define DEVICE_NAME		    "DEV_AD4170"
#define ACTIVE_DEVICE_ID    ID_AD4170
#define HW_MEZZANINE_NAME	"EVAL-AD4170-ARDZ"
#else
#warning No/Unsupported ADxxxxy symbol defined. AD4170 defined
#define DEV_AD4170
#define ACTIVE_DEVICE_NAME	"ad4170"
#define DEVICE_NAME		    "DEV_AD4170"
#define ACTIVE_DEVICE_ID    ID_AD4170
#define HW_MEZZANINE_NAME	"EVAL-AD4170-ARDZ"
#endif

/* Select the active platform (default is Mbed) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		MBED_PLATFORM
#endif

/* Select active IIO client */
#if !defined(ACTIVE_IIO_CLIENT)
#define ACTIVE_IIO_CLIENT	IIO_CLIENT_REMOTE
#endif

/* Note: The STM32 platform supports SPI interrupt and TDM DMA data capturing
 * using the Nucleo-H563ZI and SPI DMA Mode using the SDP-K1
 * while the MBED platform supports only SPI interrupt Mode */
#if !defined(INTERFACE_MODE)
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
/* Note: SDP-K1 supports only SPI DMA Mode in stm32 platform*/
#if defined (TARGET_SDP_K1)
#define INTERFACE_MODE SPI_DMA_MODE
#else // Nucleo H563
#define INTERFACE_MODE TDM_MODE
#endif
#else // Mbed
#define INTERFACE_MODE SPI_INTERRUPT_MODE
#endif
#endif

/* Select the demo mode configuration (default is user config) */
#if !defined(ACTIVE_DEMO_MODE_CONFIG)
#define ACTIVE_DEMO_MODE_CONFIG		USER_DEFAULT_CONFIG
#endif

/* Select the ADC data capture mode (default is CC mode) */
#if !defined(DATA_CAPTURE_MODE)
#define DATA_CAPTURE_MODE	CONTINUOUS_DATA_CAPTURE
#endif

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"
#define HW_CARRIER_NAME         TARGET_NAME
/* Redefine the init params structure mapping w.r.t. platform */
#define ticker_int_extra_init_params mbed_ticker_int_extra_init_params
#if defined(USE_VIRTUAL_COM_PORT)
#define uart_extra_init_params mbed_vcom_extra_init_params
#define uart_ops mbed_virtual_com_ops
#else
#define uart_extra_init_params mbed_uart_extra_init_params
#define uart_ops mbed_uart_ops
#endif
#define spi_extra_init_params mbed_spi_extra_init_params
#define i2c_extra_init_params mbed_i2c_extra_init_params
#define trigger_gpio_irq_extra_params mbed_trigger_gpio_irq_init_params
#define gpio_dig_aux1_extra_init_params mbed_dig_aux1_gpio_extra_init_params
#define gpio_dig_aux2_extra_init_params mbed_dig_aux2_gpio_extra_init_params
#define gpio_sync_inb_extra_init_params mbed_sync_inb_gpio_extra_init_params
#define trigger_gpio_extra_init_params mbed_trigger_gpio_extra_init_params
#define trigger_gpio_ops mbed_gpio_ops
#define irq_ops		mbed_gpio_irq_ops
#define ticker_irq_ops  mbed_irq_ops
#define gpio_ops	mbed_gpio_ops
#define spi_ops		mbed_spi_ops
#define i2c_ops		mbed_i2c_ops
#define trigger_gpio_irq_ops mbed_gpio_irq_ops
#define trigger_gpio_handle 0	// Unused macro
#define TRIGGER_GPIO_PORT 0  // Unused macro
#define TRIGGER_GPIO_PIN  DIG_AUX_1
#define TRIGGER_INT_ID	GPIO_IRQ_ID1
#define TICKER_ID TICKER_INT_ID
#define SPI_DEVICE_ID 	0 // unused
#define I2C_DEVICE_ID	0 // Unused
#define TRIGGER_GPIO_IRQ_CTRL_ID 0 // Unused
#elif (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"

#define spi_extra_init_params 	stm32_spi_extra_init_params
#define uart_extra_init_params 	stm32_uart_extra_init_params
#define trigger_gpio_extra_init_params	stm32_trigger_gpio_extra_init_params
#define trigger_gpio_irq_extra_params	stm32_trigger_gpio_irq_init_params
#define gpio_dig_aux1_extra_init_params stm32_dig_aux1_gpio_extra_init_params
#define gpio_dig_aux2_extra_init_params stm32_dig_aux2_gpio_extra_init_params
#define gpio_sync_inb_extra_init_params stm32_sync_inb_gpio_extra_init_params
#define csb_gpio_extra_init_params		stm32_csb_gpio_extra_init_params
#define ticker_int_extra_init_params	stm32_ticket_int_init_params
#define tdm_extra_init_params stm32_tdm_extra_init_params
#define i2c_extra_init_params stm32_i2c_extra_init_params
#if (INTERFACE_MODE == SPI_DMA_MODE)
#define tx_trigger_extra_init_params  stm32_tx_trigger_extra_init_params
#endif

#define spi_ops		stm32_spi_ops
#define uart_ops	stm32_uart_ops
#define gpio_ops	stm32_gpio_ops
#define i2c_ops 	stm32_i2c_ops
#define irq_ops		stm32_gpio_irq_ops
#define tdm_ops      stm32_tdm_platform_ops
#define trigger_gpio_irq_ops stm32_gpio_irq_ops
#if (INTERFACE_MODE == SPI_DMA_MODE)
#define dma_ops stm32_dma_ops
#define pwm_ops      stm32_pwm_ops
#endif

#define TRIGGER_GPIO_PORT 			DIG_AUX_1_PORT
#define TRIGGER_GPIO_PIN  			DIG_AUX_1
#define TRIGGER_GPIO_IRQ_CTRL_ID 	TRIGGER_GPIO_PIN
#define CSB_GPIO_PORT				STM32_SPI_CS_PORT

#define SPI_DEVICE_ID 		STM32_SPI_ID
#define I2C_DEVICE_ID		STM32_I2C_ID
#define TRIGGER_INT_ID 		0 // unused
#define trigger_gpio_handle	0 // unused
#define DMA_IRQ_ID			 GPDMA1_Channel7_IRQn
#else
#error "No/Invalid active platform selected"
#endif

/* List the differential/single-ended channels based on active device.
 * Note : There can be max 16 channels in the device sequencer but since
 * input pairs can be only 3/6/4/8/16, only those many channels are exposed
 * out, based on the user selected channel configuration.
 **/
#if defined(DEV_AD4170)
#define	DIFFERENTIAL_CHNS	4
#define	SINGLE_ENDED_CHNS	8
#endif

/* Include user config files and params according to active/selected demo mode config */
#if (ACTIVE_DEMO_MODE_CONFIG == USER_DEFAULT_CONFIG)
#include "ad4170_user_config.h"
#define ad4170_init_params	ad4170_user_config_params
#elif ((ACTIVE_DEMO_MODE_CONFIG == RTD_2WIRE_CONFIG) || \
(ACTIVE_DEMO_MODE_CONFIG == RTD_3WIRE_CONFIG) || (ACTIVE_DEMO_MODE_CONFIG == RTD_4WIRE_CONFIG))
#include "ad4170_rtd_config.h"
#define ad4170_init_params	ad4170_rtd_config_params
#elif (ACTIVE_DEMO_MODE_CONFIG == THERMISTOR_CONFIG)
#include "ad4170_thermistor_config.h"
#define ad4170_init_params	ad4170_thermistor_config_params
#elif (ACTIVE_DEMO_MODE_CONFIG == THERMOCOUPLE_CONFIG)
#include "ad4170_thermocouple_config.h"
#define ad4170_init_params	ad4170_thermocouple_config_params
#elif (ACTIVE_DEMO_MODE_CONFIG == ACCELEROMETER_CONFIG)
#include "ad4170_accelerometer_config.h"
#define ad4170_init_params	ad4170_accelerometer_config_params
#elif (ACTIVE_DEMO_MODE_CONFIG == LOADCELL_CONFIG)
#include "ad4170_loadcell_config.h"
#define ad4170_init_params	ad4170_loadcell_config_params
#else
#include "ad4170_user_config.h"
#define ad4170_init_params	ad4170_user_config_params
#warning "No/Invalid active demo config selected, user config used as default"
#endif

/* ADC resolution for active device */
#define ADC_RESOLUTION		24

/* Bytes per sample */
#define	BYTES_PER_SAMPLE	sizeof(uint32_t)	// For ADC resolution of 24-bits

/* ADC max count (full scale value) for unipolar inputs */
#define ADC_MAX_COUNT_UNIPOLAR	(uint32_t)((1 << ADC_RESOLUTION) - 1)

/* ADC max count (full scale value) for bipolar inputs */
#define ADC_MAX_COUNT_BIPOLAR	(uint32_t)(1 << (ADC_RESOLUTION-1))

/* Default ADC reference voltages for each reference source */
#define AD4170_REFIN_REFIN1_VOLTAGE		2.5
#define AD4170_REFIN_REFIN2_VOLTAGE		2.5
#define AD4170_REFIN_AVDD_VOLTAGE		5.0
#define AD4170_REFIN_REFOUT_VOLTAGE		2.5

/****** Macros used to form a VCOM serial number ******/
#define	FIRMWARE_NAME	"ad4170_iio"

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

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
//#define USE_SDRAM		// Uncomment to use SDRAM for data buffer

/* Calculations for sampling frequency (used to define timeout in IIO client):
 * Note: Below calculations are based on default user configurations set in the
 *		  ad4170_xyz_config.h files. These configurations are used for data capturing.
 * Clock: Internal 16Mhz oscillotor
 * Filter Type: Selected in user config files
 * Filter FS: Selected in user config files
 * Filter ODR Average (as defined in datasheet): Selected in user config files
 **/
/* AD4170 default internal clock frequency (Fclock = 16Mhz)*/
#define AD4170_INTERNAL_CLOCK			(16000000U)

/* Default sampling frequency for AD4170 (in SPS) */
#define AD4170_DEFLT_SAMPLING_FREQUENCY	(AD4170_INTERNAL_CLOCK / FS_TO_ODR_CONV_SCALER)

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

extern struct no_os_gpio_init_param gpio_init_ldac_n;
extern struct no_os_gpio_init_param gpio_init_rdy;
extern struct no_os_gpio_init_param gpio_init_sync_inb;
extern struct no_os_gpio_desc *led_gpio_desc;
extern struct no_os_uart_desc *uart_desc;
extern struct no_os_gpio_desc *trigger_gpio_desc;
extern struct no_os_gpio_desc *csb_gpio_desc;
extern struct no_os_spi_init_param spi_init_params;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;
extern struct no_os_tdm_desc *ad4170_tdm_desc;
extern struct no_os_eeprom_desc *eeprom_desc;
extern struct no_os_pwm_desc *tx_trigger_desc;
int32_t init_system(void);
void ticker_callback(void *ctx);

#endif //_APP_CONFIG_H_
