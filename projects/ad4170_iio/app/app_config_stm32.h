/***************************************************************************//**
 *   @file    app_config_stm32.h
 *   @brief   Header file for STM32 platform configurations
********************************************************************************
 * Copyright (c) 2023-24 Analog Devices, Inc.
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
#include "stm32_gpio_irq.h"
#include "stm32_tdm.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

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

/* Note: The below macro and the type of digital filter chosen together
 * decides the output data rate to be configured for the device.
 * Filter configuration can be modified by changing the macro "AD4170_FILTER_CONFIG"
 * in the respective user configuration header file.
 * Please refer to the datasheet for more details on the other filter configurations.
 * It has to be noted that this is not the maximum ODR permissible by the device, but
 * a value specific to the NUCLEO-H563ZI platform tested with a 10MHz SPI clock. The maximum
 * ODR might vary across platforms and data continuity is not guaranteed above this ODR
 * on the IIO Client*/
#if (INTERFACE_MODE == SPI_MODE)
#define FS_CONFIG_VALUE 	20 // Value corresponding to 24KSPS ODR (per channel) with Sinc5 average filter
#else // TDM_MODE
#define FS_CONFIG_VALUE		1 // Value correspoinding to 512ksps ODR (per channel) with Sinc5 filter
#endif

#define TICKER_INTERRUPT_PERIOD_uSEC	(0) // unused

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
extern UART_HandleTypeDef huart3;
extern bool data_capture_operation;
extern struct iio_device_data *ad4170_iio_dev_data;
extern uint8_t num_of_active_channels;
extern volatile bool tdm_read_started;
void stm32_system_init(void);
void ad4170_dma_rx_cplt(SAI_HandleTypeDef *hsai);
void ad4170_dma_rx_half_cplt(SAI_HandleTypeDef *hsai);
#endif /* APP_CONFIG_STM32_H_ */
