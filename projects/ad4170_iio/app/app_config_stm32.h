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
#include "main.h"
#include "stm32_spi.h"
#include "stm32_i2c.h"
#include "stm32_uart.h"
#include "stm32_gpio.h"
#include "app_config.h"
#if (INTERFACE_MODE != SPI_DMA_MODE)
#include "stm32_gpio_irq.h"
#else
#include "stm32_dma.h"
#endif
#include "stm32_tdm.h"
#if (INTERFACE_MODE == SPI_DMA_MODE)
#include "stm32_pwm.h"
#endif

#if defined (TARGET_SDP_K1)
#include "stm32_usb_uart.h"
#endif

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

#if defined (TARGET_SDP_K1)
/* The below configurations are specific to STM32F469NIH6 MCU on SDP-K1 Board. */
#define HW_CARRIER_NAME		SDP-K1

/* STM32 SPI Specific parameters */
#define STM32_SPI_ID		1 // SPI1
#define STM32_SPI_CS_PORT	0  // GPIO Port A
#define SPI_CSB			15 // PA_15

/* STM32 I2C Specific parameters */
#define STM32_I2C_ID		1 // I2C1

/* STM32 UART specific parameters */
#define APP_UART_HANDLE     &huart5
#define UART_IRQ_ID         UART5_IRQn

/* STM32 GPIO specific parameters */
#define DIG_AUX_1           7 // PG7
#define DIG_AUX_2	    10 // PG10
#define SYNC_INB	    9 // PG9
#define LED_GPO		    4

#define DIG_AUX_1_PORT	    6 // GPIOG
#define DIG_AUX_2_PORT	    6 // GPIOG
#define SYNC_INB_PORT	    6 // GPIOG
#define SYNC_INB_PORT_ID    GPIOG

#define GPIO_TRIGGER_INT_PORT EXTI_GPIOG // PG7

#define I2C_TIMING      0 // (Unused)

/* SPI DMA specific parameters */
#define AD469x_DMA_NUM_CHANNELS     2

#define Rx_DMA_IRQ_ID               DMA2_Stream0_IRQn
#define AD469x_TxDMA_CHANNEL_NUM    DMA_CHANNEL_7
#define AD469x_RxDMA_CHANNEL_NUM    DMA_CHANNEL_3

/* Tx Trigger timer parameters */
#define TX_TRIGGER_TIMER_ID         8 // Timer 8
/* Tx trigger period considering a MAX SPI clock of 22.5MHz and 32 bit transfer */
#define TX_TRIGGER_PERIOD           2250
#define TX_TRIGGER_DUTY_RATIO       240
#define TIMER_8_PRESCALER           0
#define TIMER_8_CLK_DIVIDER         1
#define TIMER_CHANNEL_1				1
#else
/* The below configurations are specific to STM32H563ZIT6 MCU on NUCLEO-H563ZI Board. */
#define HW_CARRIER_NAME		NUCLEO-H563ZI

/* STM32 SPI Specific parameters */
#define STM32_SPI_ID		1 // SPI1
#define STM32_SPI_CS_PORT	3  // GPIO Port D
#define SPI_CSB				14 // PD_14

/* STM32 I2C Specific parameters */
#define STM32_I2C_ID		1 // I2C1

/* STM32 UART specific parameters */
#define APP_UART_HANDLE     &huart3

/* STM32 GPIO specific parameters */
#define DIG_AUX_1    		14 // PG14
#define DIG_AUX_2			12 // PG12
#define SYNC_INB			14 // PE14
#define LED_GPO				LED1_GREEN_Pin

#define DIG_AUX_1_PORT		6 // GPIOG
#define DIG_AUX_2_PORT		6 // GPIOG
#define SYNC_INB_PORT		4 // GPIOE

#define GPIO_TRIGGER_INT_PORT EXTI_GPIOG // PG14

/* I2C timing register value for standard mode of operation
 * Check here for more understanding on I2C timing register
 * configuration: https://wiki.analog.com/resources/no-os/drivers/i2c */
#define I2C_TIMING      0x00000E14

/* TDM specific Parameters */
#define TDM_DATA_SIZE			32
#define TDM_SLOTS_PER_FRAME		1
#define TDM_FS_ACTIVE_LENGTH	8

/* Expect DMA to read 800 samples in one cycle */
#define TDM_N_SAMPLES_DMA_READ		800
/* This makes sure that the processor gets into the
 * Half complete callback function after every 400 samples */
#define TDM_DMA_READ_SIZE			TDM_N_SAMPLES_DMA_READ * TDM_SLOTS_PER_FRAME/2

#define STM32_SAI_BASE	SAI1_Block_A
#endif

#if (INTERFACE_MODE == SPI_INTERRUPT_MODE)
#define FS_CONFIG_VALUE 	FS_SINC5_AVG_24_KSPS
#define AD4170_MAX_SAMPLING_RATE    24000
#else // TDM_MODE and SPI_DMA_MODE
#if defined (DEV_AD4170)
#define FS_CONFIG_VALUE		FS_SINC5_512_KSPS
#define AD4170_MAX_SAMPLING_RATE    500000
#elif defined (DEV_AD4190)
#define FS_CONFIG_VALUE		FS_SINC3_62P5_KSPS
#define AD4170_MAX_SAMPLING_RATE    62500
#endif
#endif

#define TICKER_INTERRUPT_PERIOD_uSEC	(0) // unused

/* Max SPI Speed */
#define AD4170_MAX_SPI_SPEED     20000000

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_gpio_init_param stm32_trigger_gpio_extra_init_params;
extern struct stm32_gpio_init_param stm32_dig_aux1_gpio_extra_init_params;
extern struct stm32_gpio_init_param stm32_dig_aux2_gpio_extra_init_params;
extern struct stm32_gpio_init_param stm32_sync_inb_gpio_extra_init_params;
extern struct stm32_gpio_init_param stm32_csb_gpio_extra_init_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
extern struct stm32_tdm_init_param stm32_tdm_extra_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;
#if !defined (TARGET_SDP_K1)
extern UART_HandleTypeDef huart3;
#else
extern UART_HandleTypeDef huart5;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_tim8_ch1;
extern USBD_HandleTypeDef hUsbDeviceHS;
extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;
#endif
extern bool data_capture_operation;
extern struct iio_device_data *ad4170_iio_dev_data;
extern uint8_t num_of_active_channels;
extern volatile bool tdm_read_started;
extern volatile struct iio_device_data* iio_dev_data_g;
extern uint32_t nb_of_samples_g;
extern volatile uint32_t* buff_start_addr;
extern int32_t data_read;
extern uint32_t rxdma_ndtr;
extern volatile bool ad4170_dma_buff_full;
extern uint32_t dma_cycle_count;
extern struct stm32_spi_desc* sdesc;

#if (INTERFACE_MODE == SPI_DMA_MODE)
extern struct stm32_pwm_init_param stm32_tx_trigger_extra_init_params;
extern struct no_os_dma_init_param ad4170_dma_init_param;
extern struct stm32_dma_channel rxdma_channel;
extern struct stm32_dma_channel txdma_channel;
#endif

void tim8_config(void);
void stm32_timer_stop(void);
void stm32_system_init(void);
void stm32_abort_dma_transfer(void);
void ad4170_dma_rx_cplt(SAI_HandleTypeDef *hsai);
void ad4170_dma_rx_half_cplt(SAI_HandleTypeDef *hsai);
void ad4170_spi_dma_rx_cplt_callback(DMA_HandleTypeDef* hdma);
void ad4170_spi_dma_rx_half_cplt_callback(DMA_HandleTypeDef* hdma);
void update_buff(uint32_t* local_buf, uint32_t* buf_start_addr);
void tim8_init(struct no_os_pwm_desc *pwm_desc);
void MX_USB_DEVICE_Init(void);
extern volatile uint32_t callback_count;
#endif /* APP_CONFIG_STM32_H_ */
