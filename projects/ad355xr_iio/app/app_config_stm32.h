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
#include "stm32_spi.h"
#include "stm32_gpio.h"
#include "stm32_irq.h"
#include "stm32_pwm.h"
#include "stm32_gpio_irq.h"
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

/* GPIO Pins associated with DAC */
#define RESET_PIN   11 // PG11  
#define RESET_PORT  6 // PORTG
#define LDAC_PIN    15 // PB15
#define LDAC_PORT	1 // PORTB
#define GPIO_TRIGGER_INT_PORT   1 //PORTB
#define SPI_DMA_TX_STOP_PWM_GPIO_PIN    12 //PD12
#define SPI_DMA_TX_STOP_PWM_GPIO_PORT   3 //PORTD

/* STM32 SPI Specific parameters */
#define SPI_DEVICE_ID       1 //SPI1
#define SPI_CSB             15 //PA15
#define STM32_SPI_CS_PORT   0  //PORTA

/* Interrupt Callback parameters */
#define IRQ_CTRL_ID          15 // PB15 
#define TRIGGER_INT_ID		 15 // PB15 
#define trigger_gpio_handle	 0
#define LDAC_GPIO_PRIORITY   1

/* STM32 LDAC PWM Specific parameters */
#define LDAC_PWM_ID          12 //Timer12
#define LDAC_PWM_CHANNEL     2 // Channel 2
#define LDAC_PWM_CLK_DIVIDER 2 // multiplier to get timer clock from PLCK1

/* STM32 PWM specific parameters to stop spi dma transfer */
#define SPI_DMA_TX_STOP_PWM_ID   4 //Timer4
#define SPI_DMA_TX_STOP_PWM_PRESCALER   0
#define SPI_DMA_TX_STOP_PWM_CHANNEL     1 // Channel 1
#define SPI_DMA_TX_STOP_PWM_CLK_DIVIDER 2 // multiplier to get timer clock from PLCK1

/* Redefine the init params structure mapping wrt platform */
#define spi_extra_init_params   stm32_spi_init_params
#define spi_extra_init_params_without_sw_csb    stm32_spi_init_params_without_sw_csb
#define uart_extra_init_params  stm32_uart_init_params
#define ldac_pwm_extra_init_params  stm32_ldac_pwm_init_params
#define spi_dma_tx_stop_pwm_extra_init_params   stm32_spi_dma_tx_stop_pwm_init_params
#define gpio_ldac_extra_init_params stm32_gpio_ldac_init_params
#define spi_dma_tx_stop_pwm_gpio_extra_init_params  stm32_spi_dma_tx_stop_pwm_gpio_init_params
#define gpio_reset_extra_init_params stm32_gpio_reset_init_params
#define ext_int_extra_init_params   stm32_trigger_gpio_irq_init_params

/* Platform Ops */
#define irq_platform_ops    stm32_gpio_irq_ops
#define gpio_ops			stm32_gpio_ops
#define spi_ops				stm32_spi_ops
#define pwm_ops				stm32_pwm_ops
#define uart_ops            stm32_uart_ops

/* Define the max possible sampling rate for a given platform.
 * Note: This is derived by testing the firmware on SDP-K1 controller
 * board with STM32F469NI MCU.
 * The max possible sampling rate can vary from board to board */
#if (INTERFACE_MODE == SPI_DMA)
#define MAX_SAMPLING_RATE		274423
#define LDAC_PWM_DUTY_CYCLE		99
#define MAX_SPI_SCLK        11250000
#define LDAC_PWM_PRESCALER   0
#define SPI_DMA_TX_STOP_PWM_DUTY_CYCLE   50
#else
#define MAX_SAMPLING_RATE		21593
#define LDAC_PWM_DUTY_CYCLE		50
#define MAX_SPI_SCLK        22500000
#define LDAC_PWM_PRESCALER   3
#endif

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
extern struct stm32_uart_init_param stm32_uart_init_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
extern struct stm32_spi_init_param stm32_spi_init_params;
extern struct stm32_spi_init_param stm32_spi_init_params_without_sw_csb;
extern struct stm32_gpio_init_param stm32_gpio_ldac_init_params;
extern struct stm32_gpio_init_param
	stm32_spi_dma_tx_stop_pwm_gpio_init_params;
extern struct stm32_gpio_init_param stm32_gpio_reset_init_params;
extern struct stm32_pwm_init_param stm32_ldac_pwm_init_params;
extern struct stm32_pwm_init_param stm32_spi_dma_tx_stop_pwm_init_params;
extern UART_HandleTypeDef huart5;
extern SPI_HandleTypeDef hspi1;
extern TIM_HandleTypeDef htim4;
uint32_t spi_dma_tx_stop_pwm_frquency[NUMBER_OF_CHANNELS];

int32_t stm32_spi_dma_enable(struct stm32_spi_desc* spidesc,
			     struct iio_device_data* iio_dev_data, uint16_t num_of_bytes_transfer,
			     uint8_t start_addr);
int32_t stm32_spi_dma_disable(struct stm32_spi_desc* spidesc);
#endif // APP_CONFIG_STM32_H_
