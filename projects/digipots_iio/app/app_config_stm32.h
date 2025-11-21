/***************************************************************************//**
 *   @file app_config_stm32.h
 *   @brief Header file for STM32 platform configurations
********************************************************************************
 * Copyright (c) 2023-2024 Analog Devices, Inc.
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
#include "stm32_usb_uart.h"
#include "stm32_spi.h"
#include "stm32_gpio.h"
#include "stm32_i2c.h"
#include "usb_device.h"
#include "app_config.h"

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
#define UART_ID 0
#define I2C_ID                  1    // I2C1

/* STM32 VCOM specific parameters */
#define APP_UART_USB_HANDLE		hUsbDeviceHS
#define APP_UART_USB_IRQ		OTG_HS_IRQn

/* STM32 SPI Specific parameters */
#define SPI_DEVICE_ID       1 //SPI1
#define SPI_CSB             15 //PA15
#define STM32_SPI_CS_PORT   0  //PORTA

/* STM32 GPIO Specific parameters */
#define RESET_GPIO_PORT		6
#define RESET_GPIO_PIN		11

#define LRDAC_GPIO_PORT     6
#define LRDAC_GPIO_PIN      9

#define WP_GPIO_PORT       6
#define WP_GPIO_PIN        10

#define DIS_GPIO_PORT       6
#define DIS_GPIO_PIN        7

#define INDEP_GPIO_PORT     0
#define INDEP_GPIO_PIN      11

/* Redefine the init params structure mapping wrt platform */
#define spi_extra_init_params   stm32_spi_init_params
#define uart_extra_init_params  stm32_uart_init_params
#define vcom_extra_init_params  stm32_vcom_extra_init_params
#define reset_gpio_extra_init_params	stm32_reset_gpio_init_params
#define wp_gpio_extra_init_params	    stm32_wp_gpio_init_params
#define lrdac_gpio_extra_init_params	 stm32_lrdac_gpio_init_params
#define dis_gpio_extra_init_params	    stm32_dis_gpio_init_params
#define indep_gpio_extra_init_params	stm32_indep_gpio_init_params

/* Platform Ops */
#define irq_platform_ops    stm32_gpio_irq_ops
#define gpio_ops			stm32_gpio_ops
#define spi_ops				stm32_spi_ops
#define i2c_ops stm32_i2c_ops
#define uart_ops            stm32_uart_ops
#define vcom_ops            stm32_usb_uart_ops

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
extern struct stm32_uart_init_param stm32_uart_init_params;
extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
extern struct stm32_spi_init_param stm32_spi_init_params;
extern struct stm32_gpio_init_param stm32_reset_gpio_init_params;
extern struct stm32_gpio_init_param stm32_wp_gpio_init_params;
extern struct stm32_gpio_init_param stm32_lrdac_gpio_init_params;
extern struct stm32_gpio_init_param stm32_dis_gpio_init_params;
extern struct stm32_gpio_init_param stm32_indep_gpio_init_params;

extern UART_HandleTypeDef huart5;
extern SPI_HandleTypeDef hspi1;
extern USBD_HandleTypeDef hUsbDeviceHS;
void stm32_system_init(void);
extern void SystemClock_Config(void);
#endif // APP_CONFIG_STM32_H_
