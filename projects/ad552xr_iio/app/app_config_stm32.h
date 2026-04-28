/***************************************************************************//**
 *   @file    app_config_stm32.h
 *   @brief   Header file for STM32 platform configurations
********************************************************************************
 * Copyright (c) 2026 Analog Devices, Inc.
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
#include "main.h"
#include "stm32_spi.h"
#include "stm32_i2c.h"
#include "stm32_gpio.h"
#include "stm32_uart.h"
#include "stm32_pwm.h"
#include "stm32_irq.h"
#include "stm32_dma.h"
#include "stm32_usb_uart.h"
#include "usb_device.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
#if defined (TARGET_SDP_K1)
/* The below configurations are specific to STM32F469NIH6 MCU on SDP-K1 Board. */
#define HW_CARRIER_NAME		SDP_K1
#endif

/**** SPI Parameters ****/
#define SPI_DEVICE_ID	1	// SPI1
#define SPI_CSB_PORT	0	// GPIO Port A
#define SPI_CSB			15	// PA15 (ARDUINO UNO D10)
#define SPI_SPEED		(22500000)	// Speed in Hz
/**** End SPI Parameters ****/

/**** I2C Parameters ****/
#define I2C_DEVICE_ID	1	// I2C1

/* I2C timing register value for standard mode of operation
 * Check here for more understanding on I2C timing register
 * configuration: https://wiki.analog.com/resources/no-os/drivers/i2c */
#define I2C_TIMING			0 // Unused
/**** End I2C Parameters ****/

/* CLEAR_N GPIO (input to DAC) */
#define GPIO_CLEAR_N_PORT		6	// GPIO Port G
#define GPIO_CLEAR_N			10	// PG10 (ARDUINO UNO D7)

/* RESET_N GPIO (input to DAC) */
#define GPIO_RESET_N_PORT		6	// GPIO Port G
#define GPIO_RESET_N			11	// PG11 (ARDUINO UNO D8)

/* ALARM_N GPIO (output from DAC) */
#define GPIO_ALARM_N_PORT		3	// GPIO Port D
#define GPIO_ALARM_N			4	// PA6 (ARDUINO UNO D4)

/* MD_ADDR0 GPIO (input to DAC) */
#define GPIO_MD_ADDR0_PORT		0	// GPIO Port A
#define GPIO_MD_ADDR0			2	// PA2 (ARDUINO UNO A0)

/* MD_ADDR1 GPIO (input to DAC) */
#define GPIO_MD_ADDR1_PORT		0	// GPIO Port A
#define GPIO_MD_ADDR1			4	// PA4 (ARDUINO UNO A1)

/* Number of TGPx pins */
#define NUM_TGPx				4

/* LDAC_TOGGLE0 GPIO (input to DAC) */
#define GPIO_LDAC_TOGGLE0_PORT	1	// GPIO Port B
#define GPIO_LDAC_TOGGLE0		15	// PB15 (ARDUINO UNO D9)

/* LDAC_TOGGLE1 GPIO (input to DAC) */
#define GPIO_LDAC_TOGGLE1_PORT	0	// GPIO Port A
#define GPIO_LDAC_TOGGLE1		10	// PA10 (ARDUINO UNO D6)

/* LDAC_TOGGLE2 GPIO (input to DAC) */
#define GPIO_LDAC_TOGGLE2_PORT	0	// GPIO Port A
#define GPIO_LDAC_TOGGLE2		11	// PA11 (ARDUINO UNO D5)

/* LDAC_TOGGLE3 GPIO (input to DAC) */
#define GPIO_LDAC_TOGGLE3_PORT	3	// GPIO Port D
#define GPIO_LDAC_TOGGLE3		12	// PD12 (ARDUINO UNO D3)

/**** Virtual COM port Parameters ****/
#define APP_UART_USB_HANDLE		hUsbDeviceHS
/**** End Virtual COM port Parameters ****/

/**** UART Parameters ****/
#define UART_DEVICE_ID	5
#define UART_HANDLE		huart5
#define UART_IRQ_ID		UART5_IRQn
/**** End UART Parameters ****/

/**** Timer Parameters ****/
/* TGP Timer - Global TGP timer for all TGPx pins */
#define TIM_TGP_INSTANCE_ID 		1
#define TIM_TGP_CH_ID				3
#define TIM_TGP_PRESCALER			0
#define TIM_TGP_CLK_DIVIDER			2
#define TIM_TGP_HANDLE				htim1

/* DAC Update Timer parameters */
#define TIM_DAC_UPDATE_INSTANCE_ID	2
#define TIM_DAC_UPDATE_CH_ID		1
#define TIM_DAC_UPDATE_PRESCALER	0
#define TIM_DAC_UPDATE_CLK_DIVIDER	2
#define TIM_DAC_UPDATE_HANDLE		htim2
#define TIM_DAC_UPDATE_IRQ_ID		TIM2_IRQn

/* DMA Trigger timer */
#define TIM_DMA_TRIGGER_INSTANCE_ID		8
#define TIM_DMA_TRIGGER_CH_ID			3
#define TIM_DMA_TRIGGER_PRESCALER		0
#define TIM_DMA_TRIGGER_CLK_DIVIDER		2
#define TIM_DMA_TRIGGER_HANDLE			htim8
/**** End Timer Parameters ****/

/**** DMA Parameters ****/
#define DMA_NUM_CHANNELS 		2
#define TX_DMA_CH_ID			DMA_CHANNEL_7
#define RX_DMA_CH_ID			DMA_CHANNEL_3
#define TX_DMA_CH_HANDLE		hdma_tim8_ch3
#define RX_DMA_CH_HANDLE		hdma_spi1_rx
#define Rx_DMA_IRQ_ID			DMA2_Stream0_IRQn
/**** End DMA Parameters ****/

#define IRQ_IIO_TRIGGER_ID		TIM_DAC_UPDATE_IRQ_ID

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
extern USBD_HandleTypeDef APP_UART_USB_HANDLE;
extern UART_HandleTypeDef UART_HANDLE;
extern TIM_HandleTypeDef TIM_TGP_HANDLE;
extern TIM_HandleTypeDef TIM_DAC_UPDATE_HANDLE;
extern TIM_HandleTypeDef TIM_DMA_TRIGGER_HANDLE;
extern DMA_HandleTypeDef TX_DMA_CH_HANDLE;
extern DMA_HandleTypeDef RX_DMA_CH_HANDLE;
/* STM32 Extra Init Params */
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_output_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_input_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_ldac_tgp_pwm_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_cs_pwm_extra_init_params;
extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;
extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_pwm_init_param stm32_pwm_tgp_extra_init_params;
extern struct stm32_pwm_init_param stm32_pwm_tgp_trigger_mode_extra_init_params;
extern struct stm32_pwm_init_param stm32_pwm_dac_update_extra_init_params;
extern struct stm32_pwm_init_param stm32_pwm_dma_trigger_extra_init_params;

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
int32_t stm32_init_system(void);

#endif // APP_CONFIG_STM32_H_
