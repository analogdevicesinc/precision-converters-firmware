/***************************************************************************//**
 *   @file    app_config_stm32.h
 *   @brief   Header file for STM32 platform configurations
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

#include "stm32_hal.h"
#include "stm32_i2c.h"
#include "stm32_irq.h"
#include "stm32_gpio_irq.h"
#include "stm32_spi.h"
#include "stm32_gpio.h"
#include "stm32_uart.h"
#include "stm32_dma.h"
#include "stm32_pwm.h"
#include "stm32_usb_uart.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* Note: The SDP-K1 board with the STM32F469NI MCU has been used
* for developing the firmware. The below parameters will change depending
* on the controller used. */
#define TARGET_NAME                 SDP_K1

/* Pin mapping for AD405X w.r.t Arduino Headers */
#define I2C_DEV_ID					1    // I2C1
#define UART_MODULE					5    // UART5
#define UART_IRQ					UART5_IRQn
#define SPI_DEVICE_ID               1    // SPI1
#define SPI_CS_PIN_NUM				15   // PA_15
#define STM32_SPI_CS_PORT_BASE      GPIOA
#define STM32_SPI_CS_PORT_NUM       0    // PORTA = 0
#define CNV_PIN_NUM					10   // PA_10
#define CNV_PORT_NUM				0    // PORTA = 0
#define GP0_PIN_NUM					15   // PB_15
#define GP0_PORT_NUM				1    // PORTB = 1
#define GP1_PIN_NUM					11   // PG_11
#define GP1_PORT_NUM				6    // PORTG = 6
#define STM32_GP1_IRQ               EXTI15_10_IRQn
#define STM32_DMA_CONT_HANDLE       hdma_tim1_ch2
#define STM32_DMA_CONT_TRIGGER      DMA2_Stream2_IRQn
#define STM32_DMA_SPI_RX_TRIGGER    DMA2_Stream0_IRQn

/* Timer specific macros used for calculating pwm
 * period and duty cycle */
#if (ADC_CAPTURE_MODE == BURST_AVERAGING_MODE)
#define TIMER_1_PRESCALER                  7
#define TIMER_2_PRESCALER                  3
#else
#define TIMER_1_PRESCALER                  1
#define TIMER_2_PRESCALER                  0
#endif
#define TIMER_1_CLK_DIVIDER                2
#define TIMER_2_CLK_DIVIDER                2

/* Timer Channels */
#define TIMER_CHANNEL_1                    1
#define TIMER_CHANNEL_2                    2
#define TIMER_CHANNEL_3                    3

#define TIMER_8_PRESCALER                  0
#define TIMER_8_CLK_DIVIDER                2

#define TRIGGER_GPIO_PORT           0    // Unused macro
#define TRIGGER_GPIO_PIN            PWM_TRIGGER

#define TIMER1_ID                          1
#define TIMER2_ID                          2
#define TIMER8_ID                          8

#define AD405x_DMA_NUM_CHANNELS             2

#define AD405x_TxDMA_CHANNEL_NUM    DMA_CHANNEL_7
#define AD405x_RxDMA_CHANNEL_NUM    DMA_CHANNEL_3
#define Rx_DMA_IRQ_ID           DMA2_Stream0_IRQn

/* Redefine the init params structure mapping w.r.t. platform */
#define uart_extra_init_params      stm32_uart_extra_init_params
#define spi_extra_init_params       stm32_spi_extra_init_params
#define cnv_extra_init_params       stm32_gpio_cnv_extra_init_params
#define pwm_extra_init_params       stm32_pwm_cnv_extra_init_params
#define pwm_gpio_extra_init_params  stm32_pwm_gpio_extra_init_params
#define gp0_extra_init_params       stm32_gpio_gp0_extra_init_params
#define gp1_extra_init_params       stm32_gpio_gp1_extra_init_params
#define trigger_gpio_irq_extra_params stm32_gpio_irq_extra_init_params
#define dma_extra_init_params       stm32_dma_extra_init_params
#define cs_extra_init_params        stm32_cs_extra_init_params
#define tx_trigger_extra_init_params          stm32_tx_trigger_extra_init_params
#define vcom_extra_init_params      stm32_vcom_extra_init_params

/* Platform ops */
#define gpio_ops                    stm32_gpio_ops
#define spi_ops		                stm32_spi_ops
#define i2c_ops                     stm32_i2c_ops
#define uart_ops                    stm32_uart_ops
#define pwm_ops                     stm32_pwm_ops
#define trigger_gpio_irq_ops        stm32_gpio_irq_ops
#define dma_ops                     stm32_dma_ops
#define vcom_ops                    stm32_usb_uart_ops

#define MAX_SPI_SCLK                22500000
#define MAX_SPI_SCLK_45MHz			45000000

/* Define the max possible sampling (or output data) rate for a given platform.
 * This is also used to find the time period to trigger a periodic conversion event.
 * Note: Max possible ODR is 1000KSPS per channel for burst/continuous data capture on
 * IIO client. This is derived by testing the firmware on STM32F469NI MCU @22Mhz SPI clock.
 * The max possible ODR can vary from board to board and data continuity is not guaranteed
 * above this ODR on IIO oscilloscope
 */
#if (INTERFACE_MODE == SPI_INTERRUPT)
#if (APP_CAPTURE_MODE == WINDOWED_DATA_CAPTURE)
#define SAMPLING_RATE					   62500
#else
#define SAMPLING_RATE					   30000 //Note: Can be set as high as 62500 with -O3 optimization on ARM GCC
#endif
#define CONV_TRIGGER_DUTY_CYCLE_NSEC(x)	(x / 10)
#else
#define SAMPLING_RATE					   (1000000)
#define DMA_MSB_DUTY_CYCLE_NS              200
#define DMA_LSB_DUTY_CYCLE_NS              600
#define OUTPUT_COMPARE_DUTY_CYCLE_NS       200
#define CHIP_SELECT_DUTY_CYCLE_NS          800
#endif
#define CONV_TRIGGER_PERIOD_NSEC(x)		   (((float)(1.0 / x) * 1000000) * 1000)

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/
extern I2C_HandleTypeDef hi2c1;
extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern DMA_HandleTypeDef hdma_tim1_ch3;
extern DMA_HandleTypeDef hdma_tim1_ch2;
extern DMA_HandleTypeDef hdma_tim8_ch1;
extern UART_HandleTypeDef huart5;
extern USBD_HandleTypeDef hUsbDeviceHS;
extern volatile bool data_ready;

extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_gpio_init_param stm32_pwm_gpio_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_cnv_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_gp0_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_gp1_extra_init_params;
extern struct stm32_gpio_init_param stm32_cs_pwm_gpio_extra_init_params;
extern struct stm32_gpio_irq_init_param stm32_gpio_irq_extra_init_params;

extern struct stm32_pwm_init_param stm32_pwm_cnv_extra_init_params;
#if (INTERFACE_MODE == SPI_DMA)
extern struct stm32_pwm_init_param stm32_dma_extra_init_params;
extern struct stm32_pwm_init_param stm32_oc_extra_init_params;
extern struct stm32_pwm_init_param stm32_cs_extra_init_params;
extern struct stm32_pwm_init_param stm32_tx_trigger_extra_init_params;
extern volatile bool ad405x_conversion_flag;
extern struct stm32_dma_init_param stm32_spi_dma_extra_init_params;
extern struct stm32_dma_channel rxdma_channel;
extern struct stm32_dma_channel txdma_channel;
extern uint32_t rxdma_ndtr;
extern int dma_cycle_count;
extern uint32_t callback_count;
#endif

void halfcmplt_callback(DMA_HandleTypeDef * hdma);
void update_buff(uint32_t* local_buf, uint32_t* buf_start_addr);
void stm32_system_init(void);
void stm32_timer_enable(void);
void stm32_timer_stop(void);
void stm32_cs_output_gpio_config(bool is_gpio);
void stm32_cnv_output_gpio_config(bool is_gpio);
void stm32_configure_spi_dma(struct no_os_spi_init_param* spi_init_par,
			     struct no_os_spi_desc* spi_desc, bool is_dma_mode);
int stm32_abort_dma_transfer(void);
void receivecomplete_callback(DMA_HandleTypeDef * hdma);

#endif /* APP_CONFIG_STM32_H_ */
