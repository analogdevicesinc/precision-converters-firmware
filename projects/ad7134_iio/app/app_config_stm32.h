/***************************************************************************//**
 *   @file    app_config_stm32.h
 *   @brief   Header file for STM32 platform configurations
********************************************************************************
 * Copyright (c) 2021 Analog Devices, Inc.
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

#include "app_config.h"
#include "stm32_uart.h"
#include "stm32_spi.h"
#include "stm32_hal.h"
#include "stm32_tdm.h"
#include "stm32_gpio.h"
#include "stm32_i2c.h"
#include "stm32_gpio_irq.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* The below pin mapping is specific to STM32L552ZE MCU on NUCLEO-L552ZE-Q Board */
#define STM32_SPI_ID			1  // SPI1
#define SPI_CSB					14 // PD_14
#define STM32_SPI_CS_PORT		3  // GPIO Port D

#define DCLK_PIN		15 // PF15
#define ODR_PIN			13 // PE13
#define DOUT0_PIN		14 // PF14
#define DOUT1_PIN		11 // PE11
#define PDN_PIN         8  // PD8

#define GPIO_TRIGGER_INT_PORT 4 // PORTE
#define PDN_PORT              3 // PORTD

/* STM32 specific UART parameters */
#define STM32_UART_BASE	3

/* STM32 specific SAI Parameters */
#define STM32_SAI_BASE	SAI1_Block_A

#define APP_UART_HANDLE     &hlpuart1

/* TDM specific Parameters */
#define TDM_DATA_SIZE			16
#define TDM_SLOTS_PER_FRAME		4
#define TDM_FS_ACTIVE_LENGTH	8

/* This makes sure that the processor gets into the
 * Half complete callback function after every 400 samples */
#define TDM_N_SAMPLES_DMA_READ		800
#define TDM_DMA_READ_SIZE 	TDM_N_SAMPLES_DMA_READ * TDM_SLOTS_PER_FRAME/2

/* GPIO Pin Mask Values (Unused) */
#define		DCLK_PIN_MASK	0
#define		ODR_PIN_MASK	0
#define		DOUT0_PIN_MASK	0
#define		DOUT1_PIN_MASK	0

#define		DOUT1_IDR		0
#define		PORTD_IDR		0
#define		DCLK_IDR		0
#define     DOUT0_IDR       0

#define UART_IRQ_ID			  LPUART1_IRQn
#define UART_DEVICE_ID  	  0
#define SPI_DEVICE_ID		  STM32_SPI_ID
#define trigger_gpio_handle	  0	// Unused macro
#define IRQ_INT_ID			  ODR_PIN
#define DMA_IRQ_ID			 DMA1_Channel1_IRQn
#define I2C_DEVICE_ID           0

/* Define the max possible sampling (or output data) rate for a given platform.
 * Note: Max possible ODR is 500KSPS per channel for continuous data capture on
 * IIO client. This is derived by testing the firmware on NUCLEO-F767 controller
 * board. The max possible ODR can vary from board to board and data
 * continuity is not guaranteed above this ODR on IIO oscilloscope */
#define SAMPLING_RATE		(500000)

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_tdm_init_param stm32_tdm_extra_init_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
extern struct stm32_gpio_init_param stm32_pdn_extra_init_params;
extern UART_HandleTypeDef hlpuart1;
extern bool data_capture_operation;
extern struct iio_device_data *ad7134_iio_dev_data;
extern void SystemClock_Config(void);
extern HAL_StatusTypeDef HAL_Init(void);
void MX_LPUART1_UART_Init(void);
void MX_SPI1_Init(void);
void MX_ICACHE_Init(void);
void MX_GPIO_Init(void);
void ad7134_dma_rx_cplt(SAI_HandleTypeDef *hsai);
void ad7134_dma_rx_half_cplt(SAI_HandleTypeDef *hsai);
#endif /* APP_CONFIG_STM32_H_ */

