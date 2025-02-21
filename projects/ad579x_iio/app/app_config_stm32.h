/***************************************************************************//**
 *   @file app_config_stm32.h
 *   @brief Header file for STM32 platform configurations
********************************************************************************
 * Copyright (c) 2024 Analog Devices, Inc.
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

/* GPIO Pins associated with DAC */
#define RESET_PIN   10 // PG10
#define RESET_PORT  6 // PORTG
#define LDAC_PIN    12 // PD12
#define LDAC_PORT	3 // PORTD
#define CLR_PIN     7 // PG7
#define CLR_PORT    6 // PORTG

#define GPIO_TRIGGER_INT_PORT   3 //PORTD

/* STM32 SPI Specific parameters */
#define SPI_DEVICE_ID       1 // SPI1
#define SPI_CSB             15 //PA15
#define STM32_SPI_CS_PORT   0  //PORTA

/* Interrupt Callback parameters */
#define TRIGGER_GPIO_IRQ_CTRL_ID          12 // PD12
#define TRIGGER_INT_ID		 12 // PD12
#define trigger_gpio_handle	 0
#define LDAC_GPIO_PRIORITY   1

/* Define the max possible sampling (or update) rate for a given platform.
 * Note: Max possible update rate is 71.428 KSPS per channel on IIO client.
 * This is derived by testing the firmware on SDP-K1 controller board with STM32F469NI MCU
 * using GCC and ARM compilers. The max possible update rate can vary from board to board and
 * data continuity is not guaranteed above this update rate */
#define MAX_SAMPLING_RATE					(71428)

/* STM32 LDAC PWM Specific parameters */
#define LDAC_PWM_ID          4 //Timer12
#define LDAC_PWM_CHANNEL     1 // Channel 2
#define LDAC_PWM_CLK_DIVIDER 2 // multiplier to get timer clock from PLCK1
#define LDAC_PWM_PRESCALER   3

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_ldac_init_params;
extern struct stm32_gpio_init_param stm32_pwm_ldac_gpio_init_params;
extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_pwm_init_param stm32_pwm_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_reset_init_params;
extern struct stm32_gpio_init_param stm32_clear_gpio_init_params;

extern UART_HandleTypeDef huart5;
extern USBD_HandleTypeDef	APP_UART_USB_HANDLE;

extern void stm32_system_init(void);

#endif

