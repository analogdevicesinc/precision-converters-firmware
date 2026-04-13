/***************************************************************************//**
 *   @file    app_config_stm32.h
 *   @brief   Header file for STM32 platform configurations
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

#include "stm32_hal.h"
#include "stm32_i2c.h"
#include "stm32_irq.h"
#include "stm32_gpio_irq.h"
#include "stm32_spi.h"
#include "stm32_gpio.h"
#include "stm32_uart.h"
#include "stm32_pwm.h"
#include "stm32_dma.h"
#include "stm32_usb_uart.h"
#include "ad4692_iio.h"
#include "app_config.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* Note: The SDP-K1 board with the STM32F469NI MCU has been used
* for developing the firmware. The below parameters will change depending
* on the controller used. */
#define HW_CARRIER_NAME		SDP_K1

/* Pin mapping w.r.t Arduino Headers */
#define I2C_DEV_ID                  1    // I2C1
#define UART_MODULE                 5    // UART5
#if defined (USE_PHY_COM_PORT)
#define UART_IRQ					UART5_IRQn
#else
#define UART_IRQ					OTG_HS_IRQn
#define CONSOLE_IRQ					UART5_IRQn
#endif
#define SPI_DEVICE_ID               1    // SPI1
#define SPI_CSB						15   // PA_15
#define SPI_CS_PORT_BASE            GPIOA
#define SPI_CS_PORT_NUM             0    // PORTA = 0
#define CNV_PIN_NUM                 10   // PA_10
#define CNV_PORT_NUM                0    // PORTA = 0
#define CNV_PORT_BASE               GPIOA
#define BSY_PIN_NUM                 0 // PA_0
#define BSY_PORT_NUM                0  // PORTA = 0
#define RESET_PIN_NUM               9    // PG_9
#define RESET_PORT_NUM              6    // PORTG = 6
#define TRIGGER_INT_ID	            BSY_PIN_NUM
#define TRIGGER_GPIO_PORT           0    // Unused macro
#define TRIGGER_GPIO_PIN            GP0_PIN_NUM
#define MAX_SPI_BAUDRATE            22500000

/* Dummy PWM for triggering SPI Burst data capture */
#define SPI_BURST_PWM_ID		12 // PD_12
#define SPI_BURST_PWM_PORT		3 // PORTD

#define gpio_ops                    stm32_gpio_ops
#define spi_ops		                stm32_spi_ops
#define i2c_ops                     stm32_i2c_ops
#define vcom_ops                    stm32_usb_uart_ops
#define uart_ops                    stm32_uart_ops
#define pwm_ops                     stm32_pwm_ops
#define dma_ops                     stm32_dma_ops
#define trigger_gpio_irq_ops        stm32_gpio_irq_ops
#define trigger_gpio_handle         0    // Unused macro
#define vcom_extra_init_params      stm32_vcom_extra_init_params
#define uart_extra_init_params      stm32_uart_extra_init_params
#define spi_extra_init_params       stm32_spi_extra_init_params
#define gpio_input_extra_init_params stm32_gpio_input_extra_init_params
#define gpio_output_extra_init_params     stm32_gpio_output_extra_init_params
#define pwm_cnv_extra_init_params   stm32_pwm_cnv_extra_init_params
#define pwm_spi_burst_extra_init_params   stm32_pwm_spi_burst_extra_init_params
#define trigger_gpio_irq_extra_params stm32_gpio_irq_extra_init_params
#define cnv_pwm_gpio_extra_init_params    stm32_cnv_pwm_gpio_extra_init_params
#define tx_trigger_extra_init_params  stm32_tx_trigger_extra_init_params
#define i2c_extra_init_params         stm32_i2c_extra_init_params
#define spi_burst_extra_init_params stm32_spi_burst_pwm_gpio_extra_init_params

/* Timer specific macros used for calculating pwm
 * period and duty cycle */
#define CNV_TIMER_ID		1 // Timer 1
#define TIMER_1_PRESCALER   1
#define TIMER_1_CLK_DIVIDER 2
#define CNV_TIMER_HANDLE    htim1

/* SPI Burst Timer specific parameters */
#define SPI_BURST_TIMER_ID		4 //TIM4
#define TIMER_4_PRESCALER   	1
#define TIMER_4_CLK_DIVIDER 	1
#define SPI_BURST_TIMER_HANDLE	htim4

/* Timer Channels */
#define TIMER_CHANNEL_3     3

/* Tx Trigger timer parameters */
#define TX_TRIGGER_TIMER_ID         8 // Timer 8
#define TX_TRIGGER_PERIOD           400
#define TX_TRIGGER_DUTY_RATIO       30
#define TX_TRIGGER_PRESCALER        0
#define TX_TRIGGER_CLK_DIVIDER      2
#define TIMER_CHANNEL_1				1
#define TX_TRIGGER_TIMER_HANDLE     htim8

#define AD4692_DMA_NUM_CHANNELS		2
#define Rx_DMA_IRQ_ID               DMA2_Stream0_IRQn
#define AD4692_TxDMA_CHANNEL_NUM    DMA_CHANNEL_7
#define AD4692_RxDMA_CHANNEL_NUM    DMA_CHANNEL_3

#define I2C_TIMING                  0

/* Priority for the GPIO interrupt */
#define GPIO_IRQ_PRIORITY       1

/* Maximum sampling rate for each ADC Mode. These values are arrived at, based on
 * empirical measurements done with the logic analyzer using the worst-case settings.
 * This is derived by testing the firmware on STM32F469NI MCU @22Mhz SPI clock.
 * The max possible ODR can vary from board to board and data continuity is not guaranteed
 * above this ODR on IIO oscilloscope */
#define S_RATE_MANUAL_DMA	(800000)
#define S_RATE_MANUAL_INTR	(50000)

/* Sampling rate for Standard Sequencer in Averaged Data Readback Mode */
#define S_RATE_CNV_CLOCK_INTR_STD_AVG   (6250)
#define S_RATE_CNV_BURST_STD_AVG	    (5750)
#define S_RATE_SPI_BURST_STD_AVG	    (5750)

/* Sampling rate for Standard Sequencer in Accumulator Data Readback Mode */
#define S_RATE_CNV_CLOCK_INTR_STD_ACC   (5500)
#define S_RATE_CNV_BURST_STD_ACC	    (5000)
#define S_RATE_SPI_BURST_STD_ACC	    (5000)

/* Sampling rate for Advanced Sequencer in Average Data Readback Mode */
#define S_RATE_CNV_CLOCK_INTR_ADV_AVG	(6250)
#define S_RATE_CNV_BURST_ADV_AVG	(4500)
#define S_RATE_SPI_BURST_ADV_AVG	(4500)

/* Sampling rate for Advanced Sequencer in Accumulator Data Readback Mode */
#define S_RATE_CNV_CLOCK_INTR_ADV_ACC	(5500)
#define S_RATE_CNV_BURST_ADV_ACC	(4000)
#define S_RATE_SPI_BURST_ADV_ACC	(4000)

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_tim8_ch1;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim4;

extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_output_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_input_extra_init_params;
extern struct stm32_gpio_irq_init_param stm32_gpio_irq_extra_init_params;
extern struct stm32_pwm_init_param stm32_pwm_cnv_extra_init_params;
extern struct stm32_gpio_init_param stm32_cnv_pwm_gpio_extra_init_params;
extern struct stm32_pwm_init_param stm32_tx_trigger_extra_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;
extern struct no_os_dma_init_param ad4692_dma_init_param;
extern struct stm32_dma_channel rxdma_channel;
extern struct stm32_dma_channel txdma_channel;
extern struct stm32_pwm_init_param stm32_pwm_spi_burst_extra_init_params;
extern struct stm32_gpio_init_param stm32_spi_burst_pwm_gpio_extra_init_params;
extern struct iio_device_data *ad4692_iio_dev_data;
extern uint8_t num_of_active_channels;
extern volatile struct iio_device_data* iio_dev_data_g;
extern uint32_t nb_of_samples_g;
extern volatile uint32_t* buff_start_addr;
extern uint32_t data_read;
extern uint32_t rxdma_ndtr;
extern volatile bool ad4692_dma_buff_full;
extern uint32_t dma_cycle_count;
extern struct no_os_gpio_desc* csb_gpio_desc;
extern uint32_t callback_count;
extern USBD_HandleTypeDef hUsbDeviceHS;
extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;
void MX_USB_DEVICE_Init(void);
extern UART_HandleTypeDef huart5;

void stm32_system_init(void);
int ad4692_config_and_start_pwm(struct ad4692_desc *desc);
void ad4692_stop_timer(void);
void ad4692_spi_dma_rx_cplt_callback(DMA_HandleTypeDef* hdma);
void ad4692_spi_dma_rx_half_cplt_callback(DMA_HandleTypeDef* hdma);
void stm32_abort_dma_transfer(void);
void update_buff(uint8_t *local_buf, uint8_t *buf_start_addr);
void SystemClock_Config(void);
int32_t tx_trigger_init(void);
void stm32_tim4_init(void);

#endif /* APP_CONFIG_STM32_H_ */
