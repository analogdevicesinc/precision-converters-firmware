/***************************************************************************//**
 *   @file app_config_stm32.h
 *   @brief Header file for STM32 platform configurations
********************************************************************************
 * Copyright (c) 2025 Analog Devices, Inc.
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

#include "stm32_uart.h"
#include "stm32_spi.h"
#include "stm32_i2c.h"
#include "stm32_gpio.h"
#include "stm32_irq.h"
#include "stm32_gpio_irq.h"
#include "stm32_pwm.h"
#include "main.h"
#include "stm32_uart_stdio.h"
#include "stm32_usb_uart.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Note: The SDP-K1 board with the STM32F469NI MCU has been used
* for developing the firmware. The below parameters will change depending
* on the controller used. */
#define HW_CARRIER_NAME		SDP-K1

/* STM32 SPI Specific parameters */
#define SPI_DEVICE_ID 1 // SPI1
#define SPI_CS_PORT	  0 // GPIO Port A
#define SPI_CSB	   15 // PA_15

/* STM32 UART specific parameters */
#define APP_UART_HANDLE     &huart5
#define APP_UART_USB_HANDLE	 hUsbDeviceHS

/* UART Device ID */
#define UART_IRQ_ID		UART5_IRQn

/* LED Port K and Pin 10 */
#define LED_GPO 5  // PK_5
#define LED_PORT 10 // Port K

/* I2C Device ID */
#define I2C_DEVICE_ID  1 // I2C1
#define I2C_TIMING    0 // (Unused)

/* STM32 TRIGGER Specific parameters */
#define TRIGGER_INT_ID 12
#define TRIGGER_GPIO_PORT 3
#define TRIGGER_GPIO_PIN  12

/* STM32  PWM Specific parameters */
#define PWM_ID          4 //Timer4
#define PWM_CHANNEL     1 // Channel 1
#define PWM_CLK_DIVIDER 2 // multiplier to get timer clock from PLCK1
#define PWM_PRESCALER   3

/* Priority of RDY Interrupt */
#define RDY_GPIO_PRIORITY 1

/* Define the max possible sampling (or output data) rate for a given platform.
 * This is also used to find the time period to trigger a periodic conversion event.
 * Note: Max possible ODR is 62.5KSPS per channel for continuous data capture on
 * IIO client. This is derived by testing the firmware on SDP-K1 controller board
 * @22Mhz SPI clock. The max possible ODR can vary from board to board and
 * data continuity is not guaranteed above this ODR on IIO oscilloscope */
#define SAMPLING_RATE					(62000)
#define CONV_TRIGGER_PERIOD_NSEC		(((float)(1.0 / SAMPLING_RATE) * 1000000) * 1000)
#define CONV_TRIGGER_DUTY_CYCLE_NSEC	(CONV_TRIGGER_PERIOD_NSEC / 2)

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

extern UART_HandleTypeDef huart5;
extern USBD_HandleTypeDef	APP_UART_USB_HANDLE;

extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
extern struct stm32_gpio_init_param stm32_trigger_gpio_extra_init_params;
extern struct stm32_pwm_init_param stm32_pwm_extra_init_params;
extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;
extern struct stm32_gpio_init_param stm32_pwm_gpio_extra_init_params;

extern void stm32_system_init(void);

#endif // APP_CONFIG_STM32_H_
