/***************************************************************************//**
 *   @file    app_config_stm32.h
 *   @brief   Header file for STM32 platform configurations
********************************************************************************
 * Copyright (c) 2023-2025 Analog Devices, Inc.
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
#include "stm32_gpio.h"
#include "stm32_uart.h"
#include "stm32_dma.h"
#include "stm32_pwm.h"

#ifdef STM32F469xx
#include "stm32_spi.h"
#include "stm32_usb_uart.h"
#endif
#ifdef STM32H563xx
#include "stm32_i3c.h"
#include "stm32_usb_uart.h"
#endif

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Timer IDs */
#define TIMER_ID(x)                        x
/* Timer Channels */
#define TIMER_CHANNEL(x)                   x

/* Total number of DMA channels necessary */
#define AD405x_DMA_NUM_CHANNELS            2

#ifdef STM32F469xx

/* Note: The SDP-K1 board with the STM32F469NI MCU has been used
* for developing the firmware. The below parameters will change depending
* on the controller used. */
#define TARGET_NAME                 SDP_K1

/* Supports available in the platform */
#define SPI_SUPPORT_AVAILABLE
#define SDRAM_SUPPORT_AVAILABLE
#define CONSOLE_STDIO_PORT_AVAILABLE

#define I2C_EXTRA_PARAM_PTR			NULL
#define I2C_MAX_SPEED_HZ			100000

/* Pin mapping for AD405X w.r.t Arduino Headers */
#define I2C_DEV_ID					1    // I2C1
#define UART_MODULE					5    // UART5
#define UART_HANDLE					huart5
#define UART_IRQ					UART5_IRQn
#define SPI_DEVICE_ID               1    // SPI1
#define SPI_CS_PIN_NUM				15   // PA_15
#define SPI_CS_PORT_NUM       		0    // PORTA = 0
#define CNV_PIN_NUM					10   // PA_10
#define CNV_PORT_NUM				0    // PORTA = 0
#define GP0_PIN_NUM					15   // PB_15
#define GP0_PORT_NUM				1    // PORTB = 1
#define GP1_PIN_NUM					11   // PG_11
#define GP1_PORT_NUM				6    // PORTG = 6
#define STM32_GP1_IRQ               EXTI15_10_IRQn
#define STM32_DMA_CONT_TRIGGER      DMA2_Stream2_IRQn
#define STM32_DMA_SPI_RX_TRIGGER    DMA2_Stream0_IRQn

/* Timer specific macros used for calculating pwm
 * period and duty cycle */
#define CNV_TIMER_BURST_AVG_PRESCALER      7
#define CNV_TIMER_PRESCALER                1
#define CS_TIMER_PRESCALER                 0
#define TX_TRIGGER_TIMER_PRESCALER         0
#define CNV_TIMER_CLK_DIVIDER              2
#define CS_TIMER_CLK_DIVIDER               2
#define TX_TRIGGER_TIMER_CLK_DIVIDER       2

/* Timer Channels */
#define CNV_TIMER_CHANNEL                  TIMER_CHANNEL(3)
#define CS_TIMER_CHANNEL                   TIMER_CHANNEL(1)
#define TX_TRIGGER_TIMER_CHANNEL           TIMER_CHANNEL(1)

#define CNV_TIMER_ID                       TIMER_ID(1)
#define CS_TIMER_ID	                       TIMER_ID(2)
#define TX_TRIGGER_TIMER_ID		           TIMER_ID(8)

#define CNV_TIMER_HANDLE                   &htim1
#define CS_TIMER_HANDLE                    &htim2
#define TX_TRIGGER_TIMER_HANDLE            &htim8

#define CNV_TIMER_TYPE					   STM32_PWM_TIMER_TIM
#define CS_TIMER_TYPE					   STM32_PWM_TIMER_TIM
#define TX_TRIGGER_TIMER_TYPE			   STM32_PWM_TIMER_TIM

#define CNV_PWM_TIMER_IRQ_ID		 	   0

#define STM32_DMA_OPS						stm32_dma_ops

#define AD405x_TxDMA_HANDLE    		hdma_tim8_ch1
#define AD405x_RxDMA_HANDLE		    hdma_spi1_rx
#define AD405x_TxDMA_CHANNEL_NUM    DMA_CHANNEL_7
#define AD405x_RxDMA_CHANNEL_NUM    DMA_CHANNEL_3
#define Rx_DMA_IRQ_ID           DMA2_Stream0_IRQn

#endif

#ifdef STM32H563xx

/* Note: The NUCLEO-H563ZI board with the STM32H563ZI MCU has been used
* for developing the firmware. The below parameters will change depending
* on the controller used. */
#define TARGET_NAME                 NUCLEO-H563ZI

/* Supports available in the platform */
#define I3C_SUPPORT_AVAILABLE
#define CONSOLE_STDIO_PORT_AVAILABLE

/**** I2C Parameters ****/
#define I2C_DEV_ID					1    // I2C1
#define I2C_MAX_SPEED_HZ			100000
#define I2C_EXTRA_PARAM_PTR			&i2c_extra_init_params
/* I2C timing register value for standard mode of operation
 * Check here for more understanding on I2C timing register
 * configuration: https://wiki.analog.com/resources/no-os/drivers/i2c */
#define I2C_TIMING				0x00000E14
/**** End I2C Parameters ****/

/**** I3C Parameters ****/
#define I3C_DEV_ID					1    // I3C1
#define I3C_HANDLE					hi3c1
#define I3C_CR_REG					I3C_HANDLE.Instance->CR
/**** End I3C Parameters ****/

/**** UART Parameters ****/
#define UART_MODULE					3    // UART3
#define UART_HANDLE					huart3
#define UART_IRQ					USART3_IRQn
/**** End UART Parameters ****/

/* Pin mapping for AD405X w.r.t Arduino Headers */
#define CNV_PORT_NUM				0	// Unused
#define CNV_PIN_NUM					0	// Unused
#define GP0_PIN_NUM					15   // PD_15
#define GP0_PORT_NUM				3    // PORTD = 3
#define GP1_PIN_NUM					3    // PF_3
#define GP1_PORT_NUM				5    // PORTF = 5
#define STM32_GP1_IRQ               EXTI3_IRQn

/* Timer specific macros used for calculating pwm
 * period and duty cycle */
#define CNV_TIMER_BURST_AVG_PRESCALER       8
#define CNV_TIMER_PRESCALER                 1
#define CNV_TIMER_CLK_DIVIDER				1

/**** DMA Parameters ****/
#define STM32_DMA_OPS						stm32_gpdma_ops
#define I3C_TX_DMA_CHANNEL_NUM    			(uint32_t)GPDMA1_Channel0
#define I3C_RX_DMA_CHANNEL_NUM    			(uint32_t)GPDMA1_Channel1
#define I3C_TX_DMA_HANDLE					handle_GPDMA1_Channel0
#define I3C_RX_DMA_HANDLE					handle_GPDMA1_Channel1
#define I3C_TX_DMA_IRQ_ID           		GPDMA1_Channel0_IRQn
#define I3C_RX_DMA_IRQ_ID           		GPDMA1_Channel1_IRQn

#define AD405x_TxDMA_HANDLE    		I3C_TX_DMA_HANDLE
#define AD405x_RxDMA_HANDLE		    I3C_RX_DMA_HANDLE
#define AD405x_TxDMA_CHANNEL_NUM    I3C_TX_DMA_CHANNEL_NUM
#define AD405x_RxDMA_CHANNEL_NUM    I3C_RX_DMA_CHANNEL_NUM
#define Tx_DMA_IRQ_ID           	I3C_TX_DMA_IRQ_ID
#define Rx_DMA_IRQ_ID           	I3C_RX_DMA_IRQ_ID
/**** End DMA Parameters ****/

/**** Conversion Timer Parameters ****/
#define CNV_TIMER_ID                 	TIMER_ID(1)
#define CNV_TIMER_CHANNEL            	TIMER_CHANNEL(1)
#define CNV_TIMER_HANDLE				&hlptim1
#define CNV_TIMER_TYPE					STM32_PWM_TIMER_LPTIM
#define CNV_PWM_TIMER_IRQ_ID		 	LPTIM1_IRQn
/**** End Conversion Timer Parameters ****/

#endif

/* Redefine the init params structure mapping w.r.t. platform */
#define uart_extra_init_params      	stm32_uart_extra_init_params
#define vcom_extra_init_params      	stm32_vcom_extra_init_params
#define i2c_extra_init_params       	stm32_i2c_extra_init_params
#define cnv_extra_init_params       	stm32_gpio_cnv_extra_init_params
#define pwm_extra_init_params       	stm32_pwm_cnv_extra_init_params
#define pwm_gpio_extra_init_params  	stm32_pwm_gpio_extra_init_params
#define gp0_extra_init_params       	stm32_gpio_gp0_extra_init_params
#define gp1_extra_init_params       	stm32_gpio_gp1_extra_init_params
#define trigger_gpio_irq_extra_params 	stm32_gpio_irq_extra_init_params
#define dma_extra_init_params       	stm32_dma_extra_init_params
#define cs_extra_init_params        	stm32_cs_extra_init_params
#define tx_trigger_extra_init_params	stm32_tx_trigger_extra_init_params
#ifdef SPI_SUPPORT_AVAILABLE
#define spi_extra_init_params       	stm32_spi_extra_init_params
#endif
#ifdef I3C_SUPPORT_AVAILABLE
#define i3c_extra_init_params       	stm32_i3c_extra_init_params
#endif

/* Platform ops */
#define gpio_ops                    stm32_gpio_ops
#define i2c_ops                     stm32_i2c_ops
#define uart_ops                    stm32_uart_ops
#define vcom_ops                    stm32_usb_uart_ops
#define pwm_ops                     stm32_pwm_ops
#define trigger_gpio_irq_ops        stm32_gpio_irq_ops
#define dma_ops                     STM32_DMA_OPS
#ifdef SPI_SUPPORT_AVAILABLE
#define spi_ops		                stm32_spi_ops
#endif
#ifdef I3C_SUPPORT_AVAILABLE
#define i3c_ops		                stm32_i3c_ops
#endif

#define MAX_SPI_SCLK                22500000
#define MAX_SPI_SCLK_45MHz			45000000

/* Define the max possible sampling (or output data) rate for a given platform.
 * This is also used to find the time period to trigger a periodic conversion event.
 * Note: Max possible ODR is 1000KSPS per channel for burst/continuous data capture on
 * IIO client. This is derived by testing the firmware on STM32F469NI MCU @22Mhz SPI clock.
 * The max possible ODR can vary from board to board and data continuity is not guaranteed
 * above this ODR on IIO oscilloscope
 */
#if (APP_CAPTURE_MODE == WINDOWED_DATA_CAPTURE)
#define SAMPLING_RATE_SPI_INTR					   62500
#else
#define SAMPLING_RATE_SPI_INTR					   30000 //Note: Can be set as high as 62500 with -O3 optimization on ARM GCC
#endif
#define CONV_TRIGGER_DUTY_CYCLE_NSEC(x)	   (x / 10)
#define SAMPLING_RATE_SPI_DMA			   (1000000)
#define SAMPLING_RATE_I3C_INTR			   (30000)
#define SAMPLING_RATE_I3C_DMA			   (140000)

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern I2C_HandleTypeDef hi2c1;
extern DMA_HandleTypeDef AD405x_TxDMA_HANDLE;
extern DMA_HandleTypeDef AD405x_RxDMA_HANDLE;
extern UART_HandleTypeDef UART_HANDLE;

#ifdef STM32F469xx
/* Conversion Pulse Timer */
extern TIM_HandleTypeDef htim1;
/* CS Pulse Timer */
extern TIM_HandleTypeDef htim2;
/* TX Trigger Pulse Timer */
extern TIM_HandleTypeDef htim8;
extern SPI_HandleTypeDef hspi1;
extern USBD_HandleTypeDef hUsbDeviceHS;
#endif

#ifdef STM32H563xx
extern LPTIM_HandleTypeDef hlptim1;
extern I3C_HandleTypeDef I3C_HANDLE;
extern PCD_HandleTypeDef hpcd_USB_DRD_FS;
#endif

extern volatile bool data_ready;

extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;
#ifdef SPI_SUPPORT_AVAILABLE
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
#endif
#ifdef I3C_SUPPORT_AVAILABLE
extern struct stm32_i3c_init_param stm32_i3c_extra_init_params;
#endif
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;
extern struct stm32_gpio_init_param stm32_pwm_gpio_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_cnv_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_gp0_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_gp1_extra_init_params;
extern struct stm32_gpio_init_param stm32_cs_pwm_gpio_extra_init_params;
extern struct stm32_gpio_irq_init_param stm32_gpio_irq_extra_init_params;

extern struct stm32_pwm_init_param stm32_pwm_cnv_extra_init_params;
extern struct stm32_pwm_init_param stm32_dma_extra_init_params;
extern struct stm32_pwm_init_param stm32_cs_extra_init_params;
extern struct stm32_pwm_init_param stm32_tx_trigger_extra_init_params;
extern struct stm32_dma_init_param stm32_spi_dma_extra_init_params;

#ifdef SPI_SUPPORT_AVAILABLE
extern struct stm32_dma_channel spi_dma_rxdma_channel;
extern struct stm32_dma_channel spi_dma_txdma_channel;
#endif
#ifdef I3C_SUPPORT_AVAILABLE
extern struct stm32_dma_channel i3c_dma_rxdma_channel;
extern struct stm32_dma_channel i3c_dma_txdma_channel;
#endif
extern uint32_t rxdma_ndtr;
extern volatile int dma_cycle_count;

void stm32_system_init(void);
void stm32_system_init_post_verification(void);
void stm32_timer_enable(void);
void stm32_timer_stop(void);
void stm32_cs_output_gpio_config(bool is_gpio);
void stm32_config_spi_data_frame_format(bool is_16_bit);
void stm32_config_cnv_prescalar(void);
int stm32_abort_dma_transfer(void);
void update_buff(uint8_t *local_buf, uint8_t *buf_start_addr);
void halfcmplt_callback(DMA_HandleTypeDef * hdma);
void receivecomplete_callback(DMA_HandleTypeDef * hdma);
__weak unsigned int ux_device_stack_tasks_run(void);
#endif /* APP_CONFIG_STM32_H_ */
