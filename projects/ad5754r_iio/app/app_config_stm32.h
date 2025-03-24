/***************************************************************************//**
 *   @file    app_config_stm32.h
 *   @brief   Header file for STM32 platform configurations
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

#include "stm32_i2c.h"
#include "stm32_irq.h"
#include "stm32_gpio_irq.h"
#include "stm32_spi.h"
#include "stm32_gpio.h"
#include "stm32_uart.h"
#include "stm32_pwm.h"
#include "stm32_usb_uart.h"
#include "stm32_uart_stdio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* Note: The SDP-K1 board with the STM32F469NI MCU has been used
* for developing the firmware. The below parameters will change depending
* on the controller used. */
#define TARGET_NAME                 SDP_K1

/* STM32 UART specific parameters */
#define APP_UART_USB_HANDLE	hUsbDeviceHS
#define APP_UART_HANDLE     &huart5
#define UART_IRQ_ID     UART5_IRQn
#define UART_ID             5
#define I2C_DEV_ID          1    // I2C1

/* STM32 SPI Specific parameters */
#define SPI_DEVICE_ID   1    // SPI1
#define SPI_CSB    	15   // PA_15
#define SPI_CS_PORT     0    // SPI Port A

/* GPIO Pins associated with DAC */
#define LDAC_GPIO_PORT   0 // PORT A
#define LDAC_GPIO	10 // PA_10 
#define CLEAR_GPIO_PORT	 6 // PORT G
#define CLEAR_GPIO       9 // PG_9

#define TRIGGER_INT_ID    LDAC_GPIO
#define IRQ_CTRL_ID       10

/* STM32 LDAC PWM Specific parameters */
#define LDAC_PWM_ID          1 // Timer1
#define LDAC_PWM_CHANNEL     3 // Channel3
#define LDAC_PWM_CLK_DIVIDER 2 // multiplier to get timer clock from PLCK1
#define LDAC_PWM_PRESCALER   3

/* Priority of the LDAC Interrupt */
#define LDAC_GPIO_PRIORITY 1

/* Max spi clk speed */
#define MAX_SPI_CLK 11250000

/* platform ops */
#define gpio_ops                    stm32_gpio_ops
#define spi_ops		                stm32_spi_ops
#define i2c_ops                     stm32_i2c_ops
#define uart_ops                    stm32_uart_ops
#define pwm_ops                     stm32_pwm_ops
#define trigger_gpio_irq_ops        stm32_gpio_irq_ops
#define vcom_ops                    stm32_usb_uart_ops

/* Define the max possible sampling (or update) rate for a given platform.
 * Note: Max possible update rate is 45.82 KSPS per channel on IIO client.
 * This is derived by testing the firmware on SDP-K1 controller board with STM32F469NI MCU
 * using GCC and ARM compilers. The max possible update rate can vary from board to board and
 * data continuity is not guaranteed above this update rate */
#define MAX_SAMPLING_RATE	(uint32_t)(45823)

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi1;
extern UART_HandleTypeDef huart5;
extern TIM_HandleTypeDef htim1;
extern USBD_HandleTypeDef	APP_UART_USB_HANDLE;

extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;
extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_gpio_init_param stm32_ldac_gpio_init_params;
extern struct stm32_gpio_init_param stm32_clear_gpio_init_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
extern struct stm32_pwm_init_param stm32_pwm_extra_init_params;
extern struct stm32_gpio_init_param stm32_pwm_gpio_init_params;

void stm32_system_init(void);
#endif /* APP_CONFIG_STM32_H_ */
