/***************************************************************************//**
 *   @file    app_config_mbed.h
 *   @brief   Header file for STM32 platform configurations
********************************************************************************
 * Copyright (c) 2022-2024 Analog Devices, Inc.
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
#include "stm32_tdm.h"
#include "stm32_gpio.h"
#include "stm32_i2c.h"
#include "stm32_gpio_irq.h"
#include "stm32_pwm.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Note: The NUCLEO-H563ZI board with the STM32H563ZIT6 MCU has been used
 * for developing the firmware. The below parameters will change depending
 * on the controller used. */

/* STM32 SPI Macro definitions */
#define STM32_SPI_ID			1		// SPI1
#define STM32_SPI_CS_PORT		3	// GPIO Port D
#define SPI_CSB					14		// GPIO Pin 14

/* GPIO Pins for the Pin control mode in AD777x */
#define GPIO_RESET_PIN			14 	// PG14
#define GPIO_MODE0_PIN			0   // Unused
#define GPIO_MODE1_PIN			0   // Unused
#define GPIO_MODE2_PIN			0   // Unused
#define GPIO_MODE3_PIN			0   // Unused

#define GPIO_DCLK0_PIN			0   // Unused
#define GPIO_DCLK1_PIN			0   // Unused
#define GPIO_DCLK2_PIN			0   // Unused
#define GPIO_SYNC_IN_PIN		7 	// PB7
#define GPIO_CONVST_SAR_PIN		6 	// PB6
#define GPIO_DRDY_PIN				3  // PF3
#define GPIO_ERROR_LED			LED3_RED_Pin

/* Port names */
#define GPIO_RESET_PORT 	6 // GPIOG
#define GPIO_MODE0_PORT		0 // Unused
#define GPIO_MODE1_PORT		0 // Unused
#define GPIO_MODE2_PORT		0 // Unused
#define GPIO_MODE3_PORT		0 // Unused
#define GPIO_DCLK0_PORT		0 // Unused
#define GPIO_DCLK1_PORT		0 // Unused
#define GPIO_DCLK2_PORT		0 // Unused
#define GPIO_SYNC_PORT		1 // GPIOB
#define GPIO_CONVST_PORT	3 // GPIOD
#define GPIO_DRDY_PORT		5 // GPIOF
#define GPIO_ERROR_LED_PORT		0 // GPIOA

#define GPIO_TRIGGER_INT_PORT	EXTI_GPIOF

#define APP_UART_HANDLE     &huart3
#define SAI_BASE            SAI1_Block_A

#define IRQ_INT_ID			GPIO_DRDY_PIN
#define UART_IRQ_ID			USART3_IRQn
#define DMA_IRQ_ID 		    GPDMA1_Channel7_IRQn
#define DRDY_IRQ_CTRL_ID	GPIO_DRDY_PIN
#define UART_DEVICE_ID      0
#define SPI_DEVICE_ID		STM32_SPI_ID
#define I2C_DEVICE_ID       1 // I2C1
#define MCLK_PWM_ID			1 // Timer 1
#define TIMER1_ID           1
#define MCLK_PWM_PRESCALER  1
#define MCLK_PWM_CHANNEL    3 // Channel 3
#define MCLK_PWM_CLK_DIVIDER 2
#define MCLK_PWM_HANDLE     htim1

/* I2C timing register value for standard mode of operation
 * Check here for more understanding on I2C timing register
 * configuration: https://wiki.analog.com/resources/no-os/drivers/i2c */
#define I2C_TIMING			0x00000E14

/* pre-empt and sub priority for the peripherals except UART */
#define PERIPH_INTR_PRE_EMPT_PRIORITY		7
#define PERIPH_INTR_SUB_PRI_PRIORITY		3

/* pre-empt and sub priority for UART peripheral */
#define UART_PRE_EMPT_PRIORITY		2
#define UART_SUB_PRI_PRIORITY		0

/* SAI-TDM configurations for 32 bit channel data and 8 channels */
#define TDM_DATA_SIZE			32
#define TDM_SLOTS_PER_FRAME		8
#define TDM_FS_ACTIVE_LENGTH	1

/* This makes sure that the processor gets into the
 * Half complete callback function after every 400 samples */
#define TDM_N_SAMPLES_DMA_READ		800
#define TDM_DMA_READ_SIZE			TDM_N_SAMPLES_DMA_READ * TDM_SLOTS_PER_FRAME/2

/* Define the Sampling Frequency. It needs to be noted that the
 * maximum sampling frequency attainable in SPI Mode is
 * only 8ksps (which is not the maximum ODR permissible by the device),
 * Beyond with continuous data capturing cannot be guaranteed.
 * This restriction is due to the time taken
 * by the SPI drivers to read the data of all the 8 channels
 * available on the SPI line. Occurrence of successive interrupts
 * during the SPI read time posts a restriction on the maximum
 * achievable ODR on the software
 * The maximum permissible device ODR can be achieved via TDM Mode. */
#if (INTERFACE_MODE == TDM_MODE)
#if defined (DEV_AD7770)
#define AD777x_SAMPLING_FREQUENCY		32000
#elif defined (DEV_AD7771)
#define AD777x_SAMPLING_FREQUENCY		128000
#else
#define AD777x_SAMPLING_FREQUENCY		16000
#endif
#else
#define AD777x_SAMPLING_FREQUENCY		8000
#endif

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_reset_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_mode0_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_mode1_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_mode2_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_mode3_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_dclk0_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_dclk1_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_dclk2_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_sync_in_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_convst_sar_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_drdy_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_error_extra_init_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
extern struct stm32_tdm_init_param 	stm32_tdm_extra_init_params;
extern struct stm32_pwm_init_param stm32_pwm_extra_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;
extern UART_HandleTypeDef huart3;
extern TIM_HandleTypeDef htim1;
extern bool data_capture_operation;
extern struct iio_device_data *ad777x_iio_dev_data;
void SystemClock_Config(void);
HAL_StatusTypeDef HAL_Init(void);
void MX_LPUART1_UART_Init(void);
void MX_SPI1_Init(void);
void MX_ICACHE_Init(void);
void MX_GPIO_Init(void);
void ad777x_configure_intr_priority(void);
void ad777x_dma_rx_cplt(SAI_HandleTypeDef *hsai);
void ad777x_dma_rx_half_cplt(SAI_HandleTypeDef *hsai);

#endif /* APP_CONFIG_STM32_H_ */
