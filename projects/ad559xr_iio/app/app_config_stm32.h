/***************************************************************************//**
 *   @file app_config_stm32.h
 *   @brief Header file for STM32 platform configurations
********************************************************************************
 * Copyright (c) 2026 Analog Devices, Inc.
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
#include "stm32_irq.h"
#include "main.h"
#include "stm32_pwm.h"
#include "stm32_usb_uart.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Note: The SDP-K1 board with the STM32F469NI MCU has been used
* for developing the firmware. The below parameters will change depending
* on the controller used. */
#define HW_CARRIER_NAME		SDP_K1

/* STM32 SPI Specific parameters */
#define MAX_SPI_BAUDRATE    20000000
#define SPI_DEVICE_ID		1 // SPI1
#define SPI_CS_PORT	        0 // GPIO Port A
#define SPI_CSB				15 // PA_15

/* STM32 UART specific parameters */
#define APP_UART_HANDLE     &huart5
#define APP_UART_USB_HANDLE hUsbDeviceHS

/* UART Device ID */
#define UART_IRQ_ID			UART5_IRQn

/* STM32 I2C Specific parameters */
#define I2C_DEVICE_ID           1 // I2C1
#define I2C_BAUDRATE 400000

/* STM32 PWM Specific parameters */
#define PWM_TIM_IRQ_ID		TIM1_CC_IRQn
#define APP_TIM_HANDLE		&htim1
#define TIMER_PWM_ID			1 // Timer 1
#define TIMER1_ID           1
#define TIMER_1_PRESCALER                  0
#define TIMER_1_CLK_DIVIDER 2
#define TIMER_CHANNEL_3    3 // Channel 3

/* Minimum Duty cycle needed */
#define DUTY_CYCLE_IN_NS   50

/* Minimum Period needed at 20 Mhz Clock frequency
 * This is calculated as 16* 50ns + duty cycle + offset time needed when one channel is enabled
 * This offset is a result of time needed for activation of Interrupt in STM32 platform and additional
 * processing after each iteration are captured */
#define PERIOD_IN_NS       22500

/* Period needed for one read at 20Mhz
 * Period for one read (9us)  */
#define PERIOD_FOR_ONE_READ 9000

#define INTR_CALLBACK_EVENT      NO_OS_EVT_TIM_PWM_PULSE_FINISHED
#define INTR_CALLBACK_PERIPHERAL NO_OS_TIM_IRQ
#define trigger_handle		APP_TIM_HANDLE
#define TRIGGER_INT_ID		PWM_TIM_IRQ_ID

/* Priority for the PWM Timer */
#define PWM_PRIORITY 1

/* Define the init params structure mapping wrt STM32 platform */
#define spi_extra_init_params   	stm32_spi_extra_init_params
#define uart_extra_init_params 		stm32_uart_extra_init_params
#define gpio_ops			stm32_gpio_ops
#define spi_ops            stm32_spi_ops
#define i2c_ops                     stm32_i2c_ops
#define uart_ops stm32_uart_ops
#define irq_platform_ops stm32_irq_ops
#define pwm_ops                     stm32_pwm_ops
#define vcom_ops 							stm32_usb_uart_ops
#define vcom_extra_init_params				stm32_vcom_extra_init_params

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
extern UART_HandleTypeDef huart5;
extern USBD_HandleTypeDef		APP_UART_USB_HANDLE;
extern TIM_HandleTypeDef htim1;

extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;
extern struct stm32_pwm_init_param stm32_pwm_extra_init_params;
extern struct stm32_pwm_desc *pwm_extra_params;
extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;

extern void stm32_system_init();
void MX_USB_DEVICE_Init(void);

#endif // APP_CONFIG_STM32_H_
