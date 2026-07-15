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

#include <stdint.h>
#include "stm32_uart.h"
#include "stm32_i2c.h"
#include "stm32_spi.h"
#include "stm32_gpio.h"
#include "stm32_irq.h"
#include "stm32_pwm.h"
#include "stm32_gpio_irq.h"
#include "app_config.h"
#include "stm32_usb_uart.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Note: The SDP-K1 board with the STM32F469NI MCU has been used
* for developing the firmware. The below parameters will change depending
* on the controller used. */
#define HW_CARRIER_NAME		SDP_K1

/* STM32 UART specific parameters */
#define APP_UART_HANDLE     &huart5
#define UART_IRQ_ID     UART5_IRQn
#define UART_ID  5
#define APP_UART_USB_HANDLE		hUsbDeviceHS

/* STM32 I2C Specific parameters */
#define I2C_DEVICE_ID  1
#define I2C_TIMING          0 // (Unused)

/* GPIO Pins */
#define GPIO_A0 1
#define GPIO_A0_PORT 0
#define GPIO_A1 0
#define GPIO_A1_PORT 0
#define GPIO_SAMPLE	9
#define GPIO_SAMPLE_PORT	6
#define GPIO_RES0	11
#define GPIO_RES0_PORT	0
#define GPIO_RES1	10
#define GPIO_RES1_PORT	0

/* STM32 SPI Specific parameters */
#define SPI_DEVICE_ID       1 // SPI1
#define SPI_CSB             15 // PA15
#define STM32_SPI_CS_PORT   0  // PORTA


/* STM32 PWM Specific parameters */
#define PWM_ID          4 // Timer 4
#define PWM_CHANNEL     1 // Channel 1
#define PWM_CLK_DIVIDER 2 // multiplier to get timer clock from PLCK1
#define PWM_PRESCALER   3
#define PWM_HANDLE      &htim4

/* Interrupt Callback parameters */
#define TRIGGER_GPIO_IRQ_CTRL_ID          12 // PD12
#define TRIGGER_INT_ID		 TRIGGER_GPIO_IRQ_CTRL_ID
#define GPIO_TRIGGER_INT_PORT   3 // PORTD
#define trigger_gpio_handle	 PWM_HANDLE
#define GPIO_PRIORITY 1

/* Define a sampling rate for a given setup.
 * This is used to find the time period to trigger a periodic conversion event.
 * Currently the value was experimentally found by testing the firmware on SDP-K1
 * controller board @20Mhz SPI clock, with fly wires to breakout board.
 * This can vary from board to board, the exact maximum value was not determined
 * as 16k seems reasonable for this setup.
 * */
#define SAMPLING_RATE                                   (16000)
#define CONV_TRIGGER_PERIOD_NSEC                (((float)(1.0 / SAMPLING_RATE) * 1000000) * 1000)
#define CONV_TRIGGER_DUTY_CYCLE_NSEC    (CONV_TRIGGER_PERIOD_NSEC / 2)

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_init_params;
extern struct stm32_gpio_init_param stm32_pwm_gpio_init_params;
extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_pwm_init_param stm32_pwm_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_reset_init_params;
extern struct stm32_gpio_init_param stm32_clear_gpio_init_params;

extern UART_HandleTypeDef huart5;
extern TIM_HandleTypeDef htim4;
extern USBD_HandleTypeDef	APP_UART_USB_HANDLE;

extern void stm32_system_init(void);

#endif

