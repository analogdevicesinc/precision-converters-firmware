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
#define UART_IRQ_ID         UART5_IRQn
#define UART_ID             5
#define APP_UART_USB_HANDLE hUsbDeviceHS

/* STM32 I2C Specific parameters */
#define I2C_DEVICE_ID  1
#define I2C_TIMING     0 // (Unused)

/* Pin mapping w.r.t. target */
#define OSR0_PIN   0 // PA0
#define OSR0_PORT  0 // PORTA
#define OSR1_PIN   7// PORTG
#define OSR1_PORT  6 // PG7
#define OSR2_PIN   9 // PORTD
#define OSR2_PORT  6 // PG7
#define RESET_PIN  11// PA_11
#define RESET_PORT  0 // PORTA
#define CONVST_PIN  10 // PA_10
#define CONVST_PORT 0  // PORTA
#define BUSY_PIN   10 // PG_10
#define BUSY_PORT  6  // PORTG
#define RANGE_PIN 11 // PG_11
#define RANGE_PORT 6 // PORTG
#define STDBY_PIN 15 // PB_15
#define STDBY_PORT 1 // PORTB
#define LED_GPO 5   // PK_5
#define LED_PORT 10 // PORTK

#define TRIGGER_GPIO_PORT   3 //PORTD
#define PWM_TRIGGER       12

/* STM32 SPI Specific parameters */
#define SPI_DEVICE_ID       1 // SPI1
#define SPI_CSB             15 //PA15
#define SPI_CS_PORT   0  //PORTA

/* Interrupt Callback parameters */
#define INT_EVENT         12 // PD12
#define IRQ_INT_ID		 12 // PD12
#define trigger_gpio_handle	 0

/* Priority of the Interrupt */
#define RDY_GPIO_PRIORITY   1

/* Define the max possible sampling (or output data) rate for a given platform.
 * This is also used to find the time period to trigger a periodic conversion event.
 * Note: Max possible ODR is 16KSPS per channel for continuous data capture on
 * IIO client. This is derived by testing the firmware on SDP-K1 controller board
 * @22Mhz SPI clock. The max possible ODR can vary from board to board and
 * data continuity is not guaranteed above this ODR on IIO oscilloscope */
#define SAMPLING_RATE					(20000)

/* STM32  PWM Specific parameters */
#define PWM_ID          4 //Timer4
#define PWM_CHANNEL     1 // Channel 1
#define PWM_CLK_DIVIDER 2 // multiplier to get timer clock from PLCK1
#define PWM_PRESCALER   3
#define PWM_HANDLE      htim4

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

extern UART_HandleTypeDef huart5;
extern USBD_HandleTypeDef APP_UART_USB_HANDLE;
extern TIM_HandleTypeDef PWM_HANDLE;

extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
extern struct stm32_gpio_init_param stm32_trigger_gpio_extra_init_params;
extern struct stm32_gpio_init_param stm32_reset_gpio_extra_init_params;
extern struct stm32_gpio_init_param stm32_convst_gpio_extra_init_params;
extern struct stm32_gpio_init_param stm32_busy_gpio_extra_init_params;
extern struct stm32_gpio_init_param stm32_osr0_gpio_extra_init_params;
extern struct stm32_gpio_init_param stm32_osr1_gpio_extra_init_params;
extern struct stm32_gpio_init_param stm32_osr2_gpio_extra_init_params;
extern struct stm32_gpio_init_param stm32_range_gpio_extra_init_params;
extern struct stm32_gpio_init_param stm32_stdby_gpio_extra_init_params;
extern struct stm32_gpio_init_param stm32_pwm_gpio_extra_init_params;
extern struct stm32_pwm_init_param stm32_pwm_extra_init_params;
extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;

extern void stm32_system_init(void);

#endif

