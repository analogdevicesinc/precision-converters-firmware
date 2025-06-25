/***************************************************************************//**
 *   @file app_config_stm32.h
 *   @brief Header file for STM32 platform configurations
********************************************************************************
 * Copyright (c) 2024-25 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef APP_CONFIG_STM32_H_
#define APP_CONFIG_STM32_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "main.h"
#include "stm32_uart.h"
#include "stm32_spi.h"
#include "stm32_gpio.h"
#include "stm32_i2c.h"
#include "stm32_dma.h"

#if defined (TARGET_SDP_K1)
#include "stm32_usb_uart.h"
#endif

#ifdef USE_QUAD_SPI
#include "stm32_xspi.h"
#endif

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

#if defined (TARGET_SDP_K1)
/* The below configurations are specific to STM32F469NIH6 MCU on SDP-K1 Board. */
#define HW_CARRIER_NAME		SDP_K1

/**** SPI Parameters ****/
/* Pin mapping for AD4080 DCS w.r.t. Arduino Headers */
#define SPI_DEVICE_ID			1 	// SPI1
#define SPI_CS_PORT				6  	// GPIO Port G
#define SPI_CSB					11 	// PG11 (ARDUINO_UNO_D8)
#define SPI_DCS_CSB_PORT		0  	// GPIO Port A
#define SPI_DCS_CSB				15 	// PA15 (ARDUINO_UNO_D10)
#define SPI_CFG_SPEED			11250000
#define SPI_DATA_SPEED			11250000
/**** End SPI Parameters ****/

/**** I2C Parameters ****/
#define I2C_DEVICE_ID       	1	// I2C1
#define I2C_TIMING				0	// (Unused)
/**** End I2C Parameters ****/

/**** UART Parameters ****/
#define UART_DEVICE_ID			5
#define APP_UART_HANDLE			huart5
#define UART_IRQ_ID				UART5_IRQn
#define APP_UART_USB_HANDLE		hUsbDeviceHS
/**** End UART Parameters ****/

/**** GPIO Parameters ****/
/* Pins used for monitoring ADC status information */
/* AFE CTRL GPIO (ARDUINO UNO D9) */
#define GPIO_AFE_CTRL_PORT		1
#define GPIO_AFE_CTRL			15	// PB_15

/* AD4080 GP1 GPIO (ARDUINO UNO D7) (input to ADC) */
#define GPIO_GP1_PORT			6
#define GPIO_GP1				10	// PG10

/* AD4080 GP2 GPIO (ARDUINO UNO D5) (output from ADC) */
#define GPIO_GP2_PORT			0
#define GPIO_GP2				11	// PA11

/* Pin used to trigger external conversion clock (ARDUINO UNO D4) */
#define GPIO_XTAL_OSC_EN_PORT	6
#define GPIO_XTAL_OSC_EN   		9	// PG9

/* AD4080 GP3 GPIO (ARDUINO UNO D3) (output from ADC) */
#define GPIO_GP3_PORT			3
#define GPIO_GP3				12 	// PD12

/* Oscillator enable GPIO (40M - D2, 20M - D1, 10M - D0) */
#define GPIO_OSC_EN_40M_PORT	6
#define GPIO_OSC_EN_40M			7 	// PG7
#define GPIO_OSC_EN_20M_PORT	0
#define GPIO_OSC_EN_20M			0	// PA0
#define GPIO_OSC_EN_10M_PORT	0
#define GPIO_OSC_EN_10M			1	// PA1
/**** End GPIO Parameters ****/
#else
/* The below configurations are specific to STM32H563ZIT6 MCU on NUCLEO-H563ZI Board. */
#define HW_CARRIER_NAME		NUCLEO_H563ZI

/**** SPI Parameters ****/
/* Pin mapping for AD4080 DCS w.r.t. Arduino Headers */
#define SPI_DEVICE_ID			1 	// SPI1
#define SPI_CS_PORT				5  	// GPIO Port F
#define SPI_CSB					3 	// PF3 (ARDUINO UNO D8)
#define SPI_DCS_CSB_PORT		3  	// GPIO Port D
#define SPI_DCS_CSB				14 	// PD14 (ARDUINO UNO D10)
#define SPI_CFG_SPEED			15625000
#define SPI_DATA_SPEED			15625000
/**** End SPI Parameters ****/

/**** QSPI Parameters ****/
#define QSPI_DEVICE_ID		1
#define QSPI_SPEED			15625000
#define QSPI_DMA_HANDLE		handle_GPDMA1_Channel0
#define QSPI_DMA_CH			GPDMA1_Channel0
#define QSPI_DMA_IRQ		GPDMA1_Channel0_IRQn
#define QSPI_DMA_NUM_CH		1
/**** End OCTOSPI Parameters ****/

/**** I2C Parameters ****/
#define I2C_DEVICE_ID       	1	// I2C1

/* I2C timing register value for standard mode of operation
 * Check here for more understanding on I2C timing register
 * configuration: https://wiki.analog.com/resources/no-os/drivers/i2c */
#define I2C_TIMING				0x00000E14
/**** End I2C Parameters ****/

/**** UART Parameters ****/
#define UART_DEVICE_ID			3
#define APP_UART_HANDLE			huart3
#define UART_IRQ_ID				USART3_IRQn
/**** End UART Parameters ****/

/**** GPIO Parameters ****/
/* Pins used for monitoring ADC status information */

/* AFE CTRL GPIO (ARDUINO UNO D9) */
#define GPIO_AFE_CTRL_PORT		3
#define GPIO_AFE_CTRL			15	// PD_15

/* AD4080 GP1 GPIO (ARDUINO UNO D7) (input to ADC) */
#define GPIO_GP1_PORT			6
#define GPIO_GP1				12	// PG12

/* AD4080 GP2 GPIO (ARDUINO UNO D5) (output from ADC) */
#define GPIO_GP2_PORT			6
#define GPIO_GP2				11	// PE11

/* Pin used to trigger external conversion clock (ARDUINO UNO D4) */
#define GPIO_XTAL_OSC_EN_PORT	4
#define GPIO_XTAL_OSC_EN   		14	// PE14

/* AD4080 GP3 GPIO (ARDUINO UNO D3) (output from ADC) */
#define GPIO_GP3_PORT			4
#define GPIO_GP3				13 	// PE13

/* Oscillator enable GPIO (40M - D2, 20M - D1, 10M - D0) */
#define GPIO_OSC_EN_40M_PORT	6
#define GPIO_OSC_EN_40M			14 	// PG14
#define GPIO_OSC_EN_20M_PORT	1
#define GPIO_OSC_EN_20M			6	// PB6
#define GPIO_OSC_EN_10M_PORT	1
#define GPIO_OSC_EN_10M			7	// PB7
/**** End GPIO Parameters ****/
#endif

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
extern UART_HandleTypeDef 		APP_UART_HANDLE;
extern DMA_HandleTypeDef 		QSPI_DMA_HANDLE;
#if defined (TARGET_SDP_K1)
extern USBD_HandleTypeDef		APP_UART_USB_HANDLE;
#endif

/* STM32 Extra Init Parameters */
extern struct stm32_spi_init_param stm32_config_spi_extra_init_params;
extern struct stm32_spi_init_param stm32_data_spi_extra_init_params;
extern struct stm32_xspi_init_param stm32_data_qspi_extra_init_params;
extern struct stm32_uart_init_param stm32_uart_extra_init_params;
#if defined (TARGET_SDP_K1)
extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;
#endif
extern struct stm32_gpio_init_param stm32_gpio_xtal_osc_en_init_params;
extern struct stm32_gpio_init_param stm32_gpio_gp1_init_params;
extern struct stm32_gpio_init_param stm32_gpio_gp2_init_params;
extern struct stm32_gpio_init_param stm32_gpio_gp3_init_params;
extern struct stm32_gpio_init_param stm32_gpio_40m_osc_init_params;
extern struct stm32_gpio_init_param stm32_gpio_20m_osc_init_params;
extern struct stm32_gpio_init_param stm32_gpio_10m_osc_init_params;
extern struct stm32_gpio_init_param stm32_gpio_afe_ctrl_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
void stm32_system_init(void);

#if defined (TARGET_SDP_K1)
void MX_USB_DEVICE_Init(void);
#endif

#endif // APP_CONFIG_STM32_H_
