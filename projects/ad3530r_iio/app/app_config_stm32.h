/***************************************************************************//**
 *   @file app_config_stm32.h
 *   @brief Header file for STM32 platform configurations
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
#include "stm32_gpio.h"
#include "stm32_irq.h"
#include "stm32_pwm.h"
#include "stm32_i2c.h"
#include "stm32_gpio_irq.h"
#include "stm32_dma.h"
#include "stm32_usb_uart.h"
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
#define APP_UART_USB_HANDLE &hUsbDeviceHS

/* GPIO Pins associated with DAC */
#define RESET_PIN       11      // PG11
#define RESET_PORT      6       // PORTG
#define LDAC_PIN        10      // PA10
#define LDAC_PORT	    0       // PORT A
#define GPIO_TRIGGER_INT_PORT   0

/* STM32 SPI Specific parameters */
#define SPI_DEVICE_ID       1 // SPI1
#define SPI_CSB             15 // PA15
#define STM32_SPI_CS_PORT   0  // PORTA

/* STM32 I2C Specific parameters */
#define I2C_DEVICE_ID       1 // I2C1
#define I2C_TIMING          0 // (Unused)

/* Interrupt Callback parameters */
#define IRQ_CTRL_ID          10
#define TRIGGER_INT_ID		 10
#define trigger_gpio_handle	 0
#define LDAC_GPIO_PRIORITY   1

/* STM32 LDAC PWM Specific parameters */
#define LDAC_PWM_ID          1  // Timer 1
#define LDAC_PWM_CHANNEL     3  // Channel 3
#define LDAC_PWM_CLK_DIVIDER 2 // multiplier to get timer clock from PLCK1
#define PWM_GPIO_PORT       LDAC_PORT
#define PWM_GPIO_PIN        LDAC_PIN
#define LDAC_PWM_HANDLE     htim1

/* Peripheral IDs (Unused) */
#define UART_ID      0

/* Tx trigger Timer specific parameters */
#define TIMER8_ID                          8
#define TIMER_8_PRESCALER                  0
#define TIMER_8_CLK_DIVIDER                2
#define TIMER_CHANNEL_1                    1
#define TIMER8_HANDLE                  htim8

#define Rx_DMA_IRQ_ID        DMA2_Stream0_IRQn
#define TxDMA_CHANNEL_NUM    DMA_CHANNEL_7
#define RxDMA_CHANNEL_NUM    DMA_CHANNEL_3

#define DMA_NUM_CHANNELS             2

/* Redefine the init params structure mapping wrt platform */
#define spi_extra_init_params   stm32_spi_init_params
#define uart_extra_init_params  stm32_uart_init_params
#define i2c_extra_init_params   stm32_i2c_init_params
#define pwm_extra_init_params  stm32_ldac_pwm_init_params
#define gpio_ldac_extra_init_params stm32_gpio_ldac_init_params
#define gpio_reset_extra_init_params stm32_gpio_reset_init_params
#define csb_gpio_extra_init_params stm32_csb_gpio_init_params
#define trigger_gpio_irq_extra_params   stm32_trigger_gpio_irq_init_params
#define gpio_pwm_extra_init_params  stm32_pwm_ldac_gpio_init_params
#define tx_trigger_extra_init_params  stm32_tx_trigger_extra_init_params
#define vcom_extra_init_params      stm32_vcom_extra_init_params

/* Platform Ops */
#define trigger_gpio_irq_ops    stm32_gpio_irq_ops
#define gpio_ops			    stm32_gpio_ops
#define spi_ops				    stm32_spi_ops
#define pwm_ops				    stm32_pwm_ops
#define uart_ops                stm32_uart_ops
#define i2c_ops				    stm32_i2c_ops
#define dma_ops                 stm32_dma_ops
#define vcom_ops				stm32_usb_uart_ops

/* Maximum SPI clock frequency in Hz */
#define MAX_SPI_SCLK            22500000

/* Define the max possible sampling (or update) rate per channel for a given platform.
 * Note: This is derived by testing the firmware on SDP-K1 controller
 * board with STM32F469NI MCU.
 * The max possible sampling rate can vary from board to board */
#if (INTERFACE_MODE == SPI_INTERRUPT)
#define MAX_SAMPLING_RATE		        40000
#define LDAC_PWM_DUTY_CYCLE_PERCENT		50
#define LDAC_PWM_PRESCALER              3
#else //SPI_DMA
#define MAX_SAMPLING_RATE		         560000
#define MAX_SAMPLING_RATE_STREAMING_MODE 1400000
#define LDAC_PULSE_WIDTH_NS              250
#define LDAC_PWM_PRESCALER               3
#endif

/* PWM configuration for 22.5MHz SPI clock */
#define TX_TRIGGER_PERIOD          400
#define TX_TRIGGER_DUTY_CYCLE_NS   50

/* Number of pulse repetitions for tx timer in one-pulse
 * mode (bytes per sample(2) + address bytes (2) - 1)
 */
#define NUM_PULSE_REPETITIONS	   3

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
extern struct stm32_uart_init_param stm32_uart_init_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
extern struct stm32_spi_init_param stm32_spi_init_params;
extern struct stm32_i2c_init_param stm32_i2c_init_params;
extern struct stm32_gpio_init_param stm32_gpio_ldac_init_params;
extern struct stm32_gpio_init_param stm32_gpio_reset_init_params;
extern struct stm32_pwm_init_param stm32_ldac_pwm_init_params;
extern struct stm32_gpio_init_param stm32_pwm_ldac_gpio_init_params;
extern UART_HandleTypeDef huart5;
extern USBD_HandleTypeDef hUsbDeviceHS;
extern TIM_HandleTypeDef LDAC_PWM_HANDLE;

#if (INTERFACE_MODE == SPI_DMA)
extern DMA_HandleTypeDef hdma_tim8_ch1;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern TIM_HandleTypeDef TIMER8_HANDLE;

extern struct stm32_gpio_init_param stm32_csb_gpio_init_params;
extern struct stm32_pwm_init_param stm32_tx_trigger_extra_init_params;
extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;
extern struct stm32_dma_channel rxdma_channel;
extern struct stm32_dma_channel txdma_channel_single_instr_mode;
extern struct stm32_dma_channel txdma_channel_stream_mode;

void receivecomplete_callback(DMA_HandleTypeDef* hdma);
void tim8_config(void);
int stm32_timer_stop(void);
int stm32_timer_enable(void);
int stm32_abort_dma_transfer(void);
#endif

void stm32_system_init(void);
int reconfig_stm32_params(void);

#endif // APP_CONFIG_STM32_H_
