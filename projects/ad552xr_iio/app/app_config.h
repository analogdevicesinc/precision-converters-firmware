/***************************************************************************//**
 *   @file    app_config.h
 *   @brief   Configuration file for AD552XR IIO firmware application
********************************************************************************
 * Copyright (c) 2026 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include "common_macros.h"
#include "no_os_pwm.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
/* List of data capture modes */
#define SPI_INTERRUPT	    1
#define SPI_DMA		    	2

/* Macros for stringification */
#define XSTR(s)		#s
#define STR(s)		XSTR(s)

/******************************************************************************/
/* Resolution of the dac */
#define AD552XR_DAC_RESOLUTION	16

/* Enable the UART/VirtualCOM port connection (default VCOM) */
//#define USE_PHY_COM_PORT		// Uncomment to select UART

#if !defined(USE_PHY_COM_PORT)
#define USE_VIRTUAL_COM_PORT
#endif
/* Select the active platform */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

#if (ACTIVE_PLATFORM != STM32_PLATFORM)
#error "No/Invalid active platform selected"
#endif /* ACTIVE_PLATFORM */

/* Select the Interface mode. Default is SPI DMA */
#if !defined(INTERFACE_MODE)
#define INTERFACE_MODE		SPI_DMA
#endif

/* AD552XR Max Sample Rate is 1MSPS */
#define AD552XR_MAX_SAMPLE_RATE		(1E6)

#if (INTERFACE_MODE == SPI_INTERRUPT)
/* In Release mode, the observed sampling frequency is 50KSPS,
 * whereas in Debug mode, it is limited to 10KSPS */
#if defined (DEBUG)
#define AD552XR_IIO_SAMPLE_RATE		(10E3)
#else
#define AD552XR_IIO_SAMPLE_RATE		(50E3)
#endif
#elif (INTERFACE_MODE == SPI_DMA)
// Not available, calculated based on SPI frequency
#endif

/* Redefine the init params structure mapping w.r.t. platform */
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#define spi_extra_init_params stm32_spi_extra_init_params
#define spi_ops stm32_spi_ops
#define i2c_extra_init_params stm32_i2c_extra_init_params
#define i2c_ops stm32_i2c_ops
#define vcom_ops stm32_usb_uart_ops
#define vcom_extra_init_params stm32_vcom_extra_init_params
#define uart_extra_init_params stm32_uart_extra_init_params
#define uart_ops stm32_uart_ops
#define gpio_output_extra_init_params stm32_gpio_output_extra_init_params
#define gpio_input_extra_init_params stm32_gpio_input_extra_init_params
#define gpio_ldac_tgp_pwm_extra_init_params stm32_gpio_ldac_tgp_pwm_extra_init_params
#define gpio_cs_pwm_extra_init_params stm32_gpio_cs_pwm_extra_init_params
#define gpio_ops stm32_gpio_ops
#define pwm_ops stm32_pwm_ops
#define irq_ops stm32_irq_ops
#define dma_ops stm32_dma_ops
#define pwm_tgp_extra_init_params stm32_pwm_tgp_extra_init_params
#define pwm_tgp_trigger_mode_extra_init_params stm32_pwm_tgp_trigger_mode_extra_init_params
#define pwm_dac_update_extra_init_params stm32_pwm_dac_update_extra_init_params
#define pwm_dma_trigger_extra_init_params stm32_pwm_dma_trigger_extra_init_params
#endif

/* Check if any serial port available for use as console stdio port */
#if defined(USE_VIRTUAL_COM_PORT)
/* If PHY com is selected, VCOM or alternate PHY com port can act as a console stdio port */
/* If VCOM is selected, PHY com port will/should act as a console stdio port */
#define CONSOLE_STDIO_PORT_AVAILABLE
#endif

/* Baud rate for IIO application UART interface */
#define IIO_UART_BAUD_RATE	(230400)

/* Number of IIO devices for the application */
#define AD552XR_IIO_NUM_DEVICES		4

/* Enable/Disable the use of SDRAM for ADC data capture buffer */
/* (Use following macros to use SDRAM for data buffer) */
//#define USE_SDRAM

/* Convert Hz into ns and vice-versa */
#define HZ_NS_CONVERT(x) (uint32_t)((1.0 / x) * 1E9)

/* Note: LDAC duty cycle is always 50% */
#define LDAC_DUTY_CYCLE_NSEC(x)	(x * 0.5)

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
extern struct no_os_spi_init_param spi_init_params;
extern struct no_os_i2c_init_param i2c_init_params;
extern struct no_os_gpio_init_param gpio_clear_n_init_params;
extern struct no_os_gpio_init_param gpio_reset_n_init_params;
extern struct no_os_gpio_init_param gpio_alarm_n_init_params;
extern struct no_os_gpio_init_param gpio_md_addr0_init_params;
extern struct no_os_gpio_init_param gpio_md_addr1_init_params;
extern struct no_os_gpio_init_param gpio_ldac_tgpx_init_params[NUM_TGPx];
extern struct no_os_uart_init_param uart_iio_comm_init_params;
extern struct no_os_uart_init_param uart_console_stdio_init_params;
extern struct no_os_uart_desc *uart_iio_comm_desc;
extern struct no_os_uart_desc *uart_console_stdio_desc;
extern struct no_os_eeprom_desc *eeprom_desc;
#if (INTERFACE_MODE == SPI_INTERRUPT)
extern struct no_os_irq_init_param irq_iio_trigger_init_params;
#endif

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
int32_t set_timer_prescaler(struct no_os_pwm_desc *desc, uint32_t prescaler);

#endif /* APP_CONFIG_H_ */
