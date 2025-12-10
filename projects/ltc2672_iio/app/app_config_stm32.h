/***************************************************************************//**
 *   @file    app_config_stm32.h
 *   @brief   Header file for STM32 platform configurations
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

#include <stdint.h>
#include "stm32_uart.h"
#include "stm32_spi.h"
#include "stm32_i2c.h"
#include "stm32_gpio.h"
#include "stm32_usb_uart.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Note: The SDP-K1 board with the STM32F469NI MCU has been used
 * for developing the firmware. The below parameters will change depending
 * on the controller used. */
#define TARGET_NAME SDP_K1

/* STM32 UART specific parameters */
#define APP_UART_HANDLE &huart5
#define UART_IRQ_ID UART5_IRQn

/* STM32 I2C specific parameters */
#define I2C_DEV_ID          1    // I2C1

/* STM32 SPI Specific parameters */
#define SPI_DEVICE_ID       1  //SPI1
#define SPI_CSB             15 //PA15
#define STM32_SPI_CS_PORT   0  //PORTA
#if defined(DC2903A)
#define MAX_SPI_SCLK        4500000
#else
#define MAX_SPI_SCLK        18000000
#endif

/* STM32 GPIOs */
#define GPIO_CLR_PIN        10 //PG10
#define GPIO_CLR_PORT       6  //PORTG
#define GPIO_LDAC_PIN       10 //PA10
#define GPIO_LDAC_PORT      0  //PORTA
#define GPIO_TGP_PIN        11 //PA11
#define GPIO_TGP_PORT       0  //PORTA
#define GPIO_FAULT_PIN      12 //PA12
#define GPIO_FAULT_PORT     3  //PORTD

/* Toggle Timer configurations */
#define TOGGLE_PWM_ID				1
#define TOGGLE_PWM_PRESCALER		1
#define TOGGLE_PWM_CHANNEL			4
#define TOGGLE_PWM_CLK_MULTIPLIER	2
#define TOGGLE_PWM_HANDLE			htim1

/* Peripheral IDs (Unused) */
#define UART_ID 0

/* Redefine the init params structure mapping wrt platform */
#define spi_extra_init_params stm32_spi_init_params
#define uart_extra_init_params stm32_uart_init_params
#define vcom_extra_init_params stm32_vcom_extra_init_params
#define gpio_ldac_extra_params stm32_gpio_ldac_params
#define gpio_clear_extra_params stm32_gpio_clear_params
#define gpio_toggle_extra_params stm32_gpio_toggle_params
#define gpio_fault_extra_params stm32_gpio_fault_params
#define toggle_pwm_extra_init_params stm32_toggle_pwm_init_params
#define toggle_pwm_gpio_extra_params stm32_toggle_pwm_gpio_params
#define trigger_gpio_irq_extra_params stm32_trigger_gpio_irq_params

/* Redefine platform ops mapping wrt STM32 platform */
#define gpio_ops stm32_gpio_ops
#define i2c_ops stm32_i2c_ops
#define spi_ops stm32_spi_ops
#define uart_ops stm32_uart_ops
#define vcom_ops stm32_usb_uart_ops
#define pwm_ops	stm32_pwm_ops
#define trigger_gpio_irq_ops stm32_gpio_irq_ops

#define LTC2672_MAX_TOGGLE_RATE 500000

#define FREQ_TO_NSEC(x)	(((float)(1.0 / x) * 1000000) * 1000)
#define DUTY_CYCLE_NSEC(x)	((x) / 2)

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct stm32_uart_init_param stm32_uart_init_params;
extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;
extern struct stm32_spi_init_param stm32_spi_init_params;
extern struct stm32_gpio_init_param stm32_gpio_ldac_params;
extern struct stm32_gpio_init_param stm32_gpio_clear_params;
extern struct stm32_gpio_init_param stm32_gpio_toggle_params;
extern struct stm32_gpio_init_param stm32_gpio_fault_params;
extern struct stm32_pwm_init_param stm32_toggle_pwm_init_params;
extern struct stm32_gpio_init_param stm32_toggle_pwm_gpio_params;
extern struct stm32_pwm_init_param stm32_ldac_pwm_init_params;
extern struct stm32_gpio_init_param stm32_ldac_pwm_gpio_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_params;

extern UART_HandleTypeDef huart5;
extern USBD_HandleTypeDef hUsbDeviceHS;
extern TIM_HandleTypeDef TOGGLE_PWM_HANDLE;

void stm32_system_init(void);

#endif /* APP_CONFIG_STM32_H_ */
