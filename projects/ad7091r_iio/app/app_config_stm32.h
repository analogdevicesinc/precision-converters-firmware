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
#define TARGET_NAME		SDP_K1

/* STM32 UART specific parameters */
#define APP_UART_HANDLE     &huart5
#define UART_IRQ_ID     UART5_IRQn
#define APP_UART_USB_HANDLE &hUsbDeviceHS

/* GPIO Pins associated with ADC */
#define RESET_PIN       9  // PG9
#define RESET_PORT      6  // PORT G
#define CNV_PIN        10  // PA10
#define CNV_PORT	    0  // PORT A
#define BSY_PIN         7  // PG7
#define BSY_PORT	    6  // PORT G
#define GPIO_TRIGGER_INT_PORT   BSY_PORT

/* STM32 SPI Specific parameters */
#define SPI_CSB             15 // PA15
#define STM32_SPI_CS_PORT   0  // PORTA

#define SPI_CS_PORT_NUM             0  // PORTA
#define SPI_CS_PIN_NUM              15 // PA_15

/* Peripheral IDs */
#define UART_ID             5 // UART5
#define I2C_DEVICE_ID       1 // I2C1
#define SPI_DEVICE_ID       1 // SPI1

#define I2C_TIMING          0 // (Unused)

/* Interrupt Callback parameters */
#define TRIGGER_INT_ID		 BSY_PIN
#define trigger_gpio_handle	 0
#define BSY_GPIO_PRIORITY    1

/* STM32 CNV PWM Specific parameters */
#define CNV_PWM_ID          1  // Timer 1
#define CNV_PWM_CHANNEL     3  // Channel 3
#define CNV_PWM_CLK_DIVIDER 2  // multiplier to get timer clock from PLCK1
#define PWM_GPIO_PORT       CNV_PORT
#define PWM_GPIO_PIN        CNV_PIN

/* STM32 CS PWM Specific parameters */
#define CS_TIMER_ID             2
#define CS_TIMER_PRESCALER      0
#define CS_TIMER_CHANNEL        1
#define TIMER_2_CLK_DIVIDER     2

/* Tx trigger Timer specifc parameters */
#define TIMER8_ID                          8
#define TIMER_8_PRESCALER                  0
#define TIMER_8_CLK_DIVIDER                2
#define TIMER_CHANNEL_1                    1

#define Rx_DMA_IRQ_ID        DMA2_Stream0_IRQn
#define TxDMA_CHANNEL_NUM    DMA_CHANNEL_7
#define RxDMA_CHANNEL_NUM    DMA_CHANNEL_3

#define AD7091R_DMA_NUM_CHANNELS             2

/* Redefine the init params structure mapping wrt platform */
#define spi_extra_init_params   stm32_spi_init_params
#define uart_extra_init_params  stm32_uart_init_params
#define i2c_extra_init_params   stm32_i2c_init_params
#define pwm_extra_init_params  stm32_cnv_pwm_init_params
#define cnv_gpio_extra_init_param stm32_gpio_cnv_init_params
#define reset_gpio_extra_init_param stm32_gpio_reset_init_params
#define trigger_gpio_irq_extra_params   stm32_trigger_gpio_irq_init_params
#define pwm_gpio_extra_init_params  stm32_pwm_cnv_gpio_init_params
#define alt_bsy_gpio_extra_init_params   stm32_gpio_gp0_extra_init_params
#define tx_trigger_extra_init_params  stm32_tx_trigger_extra_init_params
#define cs_extra_init_params          stm32_cs_extra_init_params
#define cs_pwm_gpio_extra_init_params stm32_cs_pwm_gpio_extra_init_params
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

/* Maximum SPI clock rate in Hz */
#define MAX_SPI_SCLK            40000000

/* Define the max possible sampling (or update) rate per channel for a given platform.
 * Note: This is derived by testing the firmware on SDP-K1 controller
 * board with STM32F469NI MCU.
 * The max possible sampling rate can vary from board to board */
#if (INTERFACE_MODE == SPI_INTERRUPT)
#if (DATA_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE)
#define MAX_SAMPLING_RATE		        40000
#else //Burst mode
#define MAX_SAMPLING_RATE		        50000
#endif
#define PWM_DUTY_CYCLE_PERCENT		    90
#define CNV_PWM_PRESCALER               3
#define PWM_DUTY_CYCLE_NSEC				360
#else //SPI_DMA
#define MAX_SAMPLING_RATE		       830000
#define CHIP_SELECT_DUTY_CYCLE_NS      530
#define CNV_PWM_PRESCALER              1
#define PWM_DUTY_CYCLE_NSEC			   250
#endif

/* PWM configuration for 45MHz SPI clock */
#define TX_TRIGGER_PERIOD          400
#define TX_TRIGGER_DUTY_CYCLE_NS   30

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
extern struct stm32_uart_init_param stm32_uart_init_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
extern struct stm32_spi_init_param stm32_spi_init_params;
extern struct stm32_i2c_init_param stm32_i2c_init_params;
extern struct stm32_gpio_init_param stm32_gpio_cnv_init_params;
extern struct stm32_gpio_init_param stm32_gpio_reset_init_params;
extern struct stm32_pwm_init_param stm32_cnv_pwm_init_params;
extern struct stm32_gpio_init_param stm32_pwm_cnv_gpio_init_params;
extern struct stm32_gpio_init_param stm32_gpio_gp0_extra_init_params;
extern UART_HandleTypeDef huart5;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern USBD_HandleTypeDef hUsbDeviceHS;

#if (INTERFACE_MODE == SPI_DMA)
extern DMA_HandleTypeDef hdma_tim8_ch1;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern uint32_t rxdma_ndtr;
extern uint32_t dma_cycle_count;

extern struct stm32_pwm_init_param stm32_cs_extra_init_params;
extern struct stm32_pwm_init_param stm32_tx_trigger_extra_init_params;
extern struct stm32_dma_channel rxdma_channel;
extern struct stm32_dma_channel txdma_channel;
extern struct stm32_gpio_init_param stm32_cs_pwm_gpio_extra_init_params;
extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;

void receivecomplete_callback(DMA_HandleTypeDef* hdma);
void halfcmplt_callback(DMA_HandleTypeDef* hdma);
void update_buff(uint32_t* local_buf, uint32_t* buf_start_addr);
void stm32_cs_output_gpio_config(bool is_gpio);
void tim8_config(void);
void tim2_config(void);
void stm32_timer_stop(void);
void stm32_timer_enable(void);
void stm32_abort_dma_transfer(void);
#endif

void stm32_system_init(void);
void ad7091r8_pulse_convst_stm(void);
int ad7091r8_read_one_stm(uint8_t channel,
			  uint16_t* read_val);
void configure_intr_priority(void);

#endif // APP_CONFIG_STM32_H_
