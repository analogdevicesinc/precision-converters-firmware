/***************************************************************************//**
 *   @file    app_config_stm32.h
 *   @brief   Header file for STM32 platform configurations
********************************************************************************
 * Copyright (c) 2024-2026 Analog Devices, Inc.
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
#include "stm32_spi.h"
#include "stm32_gpio.h"
#include "stm32_uart.h"
#include "stm32_dma.h"
#include "stm32_gpio_irq.h"
#include "app_config.h"
#include "ad5706r.h"
#include "stm32_usb_uart.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* Note: The SDP-K1 board with the STM32F469NI MCU has been used
* for developing the firmware. The below parameters will change depending
* on the controller used. */
#define HW_CARRIER_NAME		SDP-K1

/* Pin mapping for AD5706R w.r.t Arduino Headers */
#define SPI_DEVICE_ID				1 // SPI1
#define SPI_CSB						15   // PA_15
#define SPI_CS_PORT_BASE            GPIOA
#define SPI_CS_PORT_NUM             0    // PORTA = 0

#define I2C_DEV_ID                  1    // I2C1
#define UART_MODULE                 5    // UART5
#define UART_IRQ                    UART5_IRQn

#define GPIO_AD0			11 // PA11
#define GPIO_AD1			9 // PG9
#define GPIO_LDAC_TG		15 // PB15
#define GPIO_RESET			11 // PG11
#define GPIO_SHUTDOWN		10 // PG10
#define GPIO_DAC_UPDATE		12 // PD12
#define AD0_PORT			0 // PORTA
#define AD1_PORT			6 // PORTG
#define LDAC_TG_PORT		1 // PORTB
#define RESET_PORT			6 // PORTG
#define SHUTDOWN_PORT		6 // PORTG
#define DAC_UPDATE_PORT		3 // PORTD

/* LDAC Timer configurations */
#define PWM_TIMER_ID		1 // Timer 1
#define TRIGGER_INT_ID		GPIO_DAC_UPDATE
#define TIMER_1_PRESCALER	1
#define TIMER_1_CLK_DIVIDER	2
#define TIMER_CHANNEL_3		3
#define PWM_TIMER_HANDLE    htim1

/* DAC Update timer configurations */
#define DAC_UPDATE_TIMER_ID	4 // Timer 4
#define TIMER_CHANNEL_1		1 // Channel 1
#define TIMER_4_PRESCALER	0
#define TIMER_4_CLK_DIVIDER	2
#define DAC_UPDATE_TIMER_HANDLE  htim4

/* Tx Trigger configurations for 20MHz SPI clock */
#define TIMER8_ID               8
#define TX_TRIGGER_PERIOD		410 // TODO: Update this value based on the Max SPI Clock
#define TX_TRIGGER_DUTY_RATIO	10
#define TIMER_8_PRESCALER	0
#define TIMER_8_CLK_DIVIDER	2
#define TIMER8_HANDLE           htim8

/* CS PWM configurations */
#define CS_TIMER_ID             2
#define CS_TIMER_PRESCALER     	0
#define CS_TIMER_CHANNEL        1
#define TIMER_2_CLK_DIVIDER     2
#define CS_TIMER_HANDLE         htim2

#define I2C_TIMING		0 // Unused

#define DAC_UPDATE_GPIO_PRIORITY   1
#define trigger_gpio_handle	    0

#define AD5706_DMA_NUM_CHANNELS             2
#define AD5706_TxDMA_CHANNEL_NUM    DMA_CHANNEL_7
#define AD5706_RxDMA_CHANNEL_NUM    DMA_CHANNEL_3
#define Rx_DMA_IRQ_ID        DMA2_Stream0_IRQn

/* Redefine the init params structure mapping w.r.t. platform */
#ifdef USE_VIRTUAL_COM_PORT
#define vcom_extra_init_params      stm32_vcom_extra_init_params
#define vcom_ops                    stm32_usb_uart_ops
#else
#define uart_extra_init_params 		stm32_uart_extra_init_params
#define uart_ops                    stm32_uart_ops
#endif
#define spi_extra_init_params 		stm32_spi_extra_init_params
#define i2c_extra_init_params		stm32_i2c_extra_init_params
#define gpio_ad0_extra_params		stm32_gpio_ad0_init_params
#define gpio_ad1_extra_params		stm32_gpio_ad1_init_params
#define gpio_ldac_tg_extra_params	stm32_gpio_ldac_tg_init_params
#define gpio_shutdown_extra_params	stm32_gpio_shutdown_init_params
#define gpio_reset_extra_params		stm32_gpio_reset_init_params
#define ldac_pwm_extra_init_params	stm32_ldac_pwm_init_params
#define dac_update_pwm_extra_init_params stm32_dac_update_pwm_init_params
#define ldac_pwm_gpio_extra_init_params    stm32_ldac_pwm_gpio_extra_init_params
#define dac_update_pwm_gpio_extra_init_params    stm32_dac_update_pwm_gpio_extra_init_params
#if (INTERFACE_MODE == SPI_DMA)
#define tx_trigger_extra_init_params  stm32_tx_trigger_extra_init_params
#define csb_gpio_extra_init_params stm32_cs_gpio_extra_init_params
#define cs_extra_init_params          stm32_cs_extra_init_params
#define dma_ops			    stm32_dma_ops
#else
#define trigger_gpio_irq_extra_params   stm32_trigger_gpio_irq_init_params
#define trigger_gpio_irq_ops	    stm32_gpio_irq_ops
#endif
#define gpio_ops                    stm32_gpio_ops
#define gpio_ops                    stm32_gpio_ops
#define spi_ops		                stm32_spi_ops
#define i2c_ops						stm32_i2c_ops
#define pwm_ops						stm32_pwm_ops

// TODO: Update this value after SPI clock issues have been resolved
#define MAX_SPI_SCLK_FREQ		20000000
#if (INTERFACE_MODE == SPI_DMA)
#define AD5706_MAX_UPDATE_RATE	500000 // TODO: Update the Max rate based on the max SPI clock
#else
#define AD5706_MAX_UPDATE_RATE	50000
#endif

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

void stm32_system_init(void);
void SystemClock_Config(void);

extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_ad0_init_params;
extern struct stm32_gpio_init_param stm32_gpio_ad1_init_params;
extern struct stm32_gpio_init_param stm32_gpio_ldac_tg_init_params;
extern struct stm32_gpio_init_param stm32_gpio_shutdown_init_params;
extern struct stm32_gpio_init_param stm32_gpio_reset_init_params;
extern struct stm32_pwm_init_param  stm32_ldac_pwm_init_params;
extern struct stm32_pwm_init_param  stm32_dac_update_pwm_init_params;
extern struct stm32_gpio_init_param stm32_ldac_pwm_gpio_extra_init_params;
extern struct stm32_gpio_init_param stm32_dac_update_pwm_gpio_extra_init_params;

extern TIM_HandleTypeDef PWM_TIMER_HANDLE;
extern TIM_HandleTypeDef DAC_UPDATE_TIMER_HANDLE;

#if (INTERFACE_MODE == SPI_DMA)
void ad5706r_rx_cplt_callback(DMA_HandleTypeDef* hdma);
int ad5706r_abort_dma_transfers(struct ad5706r_dev *device);
int ad5706r_timers_enable(struct ad5706r_dev *device);
int ad5706r_init_tx_trigger(void);

extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_tim8_ch1;
extern TIM_HandleTypeDef TIMER8_HANDLE;
extern TIM_HandleTypeDef CS_TIMER_HANDLE;
extern struct stm32_dma_channel txdma_channel;
extern struct stm32_dma_channel rxdma_channel;
extern struct stm32_pwm_init_param stm32_tx_trigger_extra_init_params;
extern struct stm32_gpio_init_param stm32_cs_gpio_extra_init_params;
extern struct stm32_pwm_init_param stm32_cs_extra_init_params;
extern struct no_os_dma_init_param ad5706r_dma_init_param;
extern struct stm32_spi_init_param* spi_init_param;
extern uint8_t n_bytes;
#else
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
#endif // INTERFACE_MODE

#ifdef USE_VIRTUAL_COM_PORT
void MX_USB_DEVICE_Init(void);
extern USBD_HandleTypeDef hUsbDeviceHS;
extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;
#else
extern UART_HandleTypeDef huart5;
extern struct stm32_uart_init_param stm32_uart_extra_init_params;
#endif // USE_VIRTUAL_COM_PORT
extern bool hw_mode_enabled;
extern bool sw_mode_enabled;

#endif /* APP_CONFIG_STM32_H_ */
