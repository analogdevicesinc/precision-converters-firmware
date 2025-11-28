/***************************************************************************//**
 *   @file    app_config_stm32.h
 *   @brief   Header file for STM32 platform configurations
********************************************************************************
 * Copyright (c) 2021,2023-25 Analog Devices, Inc.
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
#include "app_config.h"
#include "stm32_uart.h"
#include "stm32_spi.h"
#include "stm32_pwm.h"
#include "stm32_gpio.h"
#include "stm32_i2c.h"
#include "stm32_gpio_irq.h"
#ifdef STM32F469xx
#include "stm32_usb_uart.h"
#include "stm32_uart_stdio.h"
#include "usb_device.h"
#elif defined STM32H563xx
#include "stm32_tdm.h"
#endif

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
#if defined(STM32H563xx)
/* The below pin mapping is specific to STM32H563ZIT6 MCU on NUCLEO-H563ZI Board */
#define STM32_SPI_ID			1  // SPI1
#define SPI_CSB					14 // PD_14
#define STM32_SPI_CS_PORT		3  // GPIO Port D

#define DCLK_PIN		14 // PG14
#define ODR_PIN			13 // PE13
#define DOUT0_PIN		14 // PE14
#define DOUT1_PIN		11 // PE11
#define PDN_PIN         6  // PB6

#define GPIO_TRIGGER_INT_PORT 4 // PORTE
#define PDN_PORT              1 // PORTB

/* STM32 specific UART parameters */
#define STM32_UART_BASE	3

/* STM32 specific SAI Parameters */
#define STM32_SAI_BASE	SAI1_Block_A

#define APP_UART_HANDLE     huart3

/* TDM specific Parameters */
#define TDM_DATA_SIZE			16
#define TDM_SLOTS_PER_FRAME		4
#define TDM_FS_ACTIVE_LENGTH	8

/* This makes sure that the processor gets into the
 * Half complete callback function after every 400 samples */
#define TDM_N_SAMPLES_DMA_READ		800
#define TDM_DMA_READ_SIZE 	TDM_N_SAMPLES_DMA_READ * TDM_SLOTS_PER_FRAME/2

/* GPIO Pin Mask Values (Unused) */
#define		DCLK_PIN_MASK	0
#define		ODR_PIN_MASK	0
#define		DOUT0_PIN_MASK	0
#define		DOUT1_PIN_MASK	0

#define		DOUT1_IDR		0
#define		PORTD_IDR		0
#define		DCLK_IDR		0
#define     DOUT0_IDR       0

#define UART_IRQ_ID			  USART3_IRQn
#define UART_DEVICE_ID  	  0
#define SPI_DEVICE_ID		  STM32_SPI_ID
#define INTR_GPIO_TRIGGER_HANDLE	  0	// Unused macro
#define IRQ_INT_ID			  ODR_PIN
#define DMA_IRQ_ID			 GPDMA1_Channel7_IRQn
#define I2C_DEVICE_ID           1 // I2C1

/* I2C timing register value for standard mode of operation
 * Check here for more understanding on I2C timing register
 * configuration: https://wiki.analog.com/resources/no-os/drivers/i2c */
#define I2C_TIMING				0x00000E14

/* Define the max possible sampling (or output data) rate for a given platform.
 * Note: Max possible ODR is 500KSPS per channel for continuous data capture on
 * IIO client. This is derived by testing the firmware on NUCLEO-H563ZI controller
 * board. The max possible ODR can vary from board to board and data
 * continuity is not guaranteed above this ODR on IIO oscilloscope */
#define SAMPLING_RATE		(500000)
#else
/* STM32 SPI specific parameters */
#define STM32_SPI_ID	  1  // SPI1
#define SPI_CSB			  15 // PA_15
#define STM32_SPI_CS_PORT 0  // GPIO Port 0

#define DCLK_PIN     7 //PG_7
#define ODR_PIN 	 12 // PD_12
#define DOUT0_PIN    9 //PG_9
#define DOUT1_PIN    11 //PA_11
#define PDN_PIN      0 //PA_0

#define DCLK_PORT	 6 //PG_7
#define ODR_PORT	 3 // PD_12
#define DOUT0_PORT   6 //PG_9
#define DOUT1_PORT   0 //PA_11
#define PDN_PORT     0 //PA_0

/* STM32 specific USB UART parameters */
#define APP_UART_USB_HANDLE		hUsbDeviceHS
#define APP_UART_USB_IRQ	 	OTG_HS_IRQn

/* STM32 specific UART parameters */
#define APP_UART_HANDLE 		huart5
#define UART_IRQ_ID			  	UART5_IRQn
#define UART_DEVICE_ID  	  	5

#define SPI_DEVICE_ID		  STM32_SPI_ID
#define IRQ_INT_ID			  ODR_PIN
#define I2C_DEVICE_ID         1 // I2C1

/* STM32 PWM Specific parameters */
#define PWM_ID          4 // Timer4
#define PWM_CHANNEL     1 // Channel 2
#define PWM_CLK_DIVIDER 2 // multiplier to get timer clock from PCLK2
#define PWM_PRESCALER   3
#define PWM_HANDLE      htim4

#define GPIO_TRIGGER_INT_PORT		ODR_PORT
#define INTR_GPIO_TRIGGER_HANDLE	0	// Unused macro

#define SAMPLING_RATE		(12000)
/* PWM period and duty cycle for AD7134 ASRC target mode. The low period of ODR as per specs
 * must be minimum 3 * Tdclk in target mode. The min possible Fdclk for SDP-K1 (STM32F469NI)
 * platform is ~3Mhz (based on time to sample data over DOUT), which gives Tdclk as ~333nsec.
 * So ODR min low time must be 333ns * 3 = ~1usec. This is achieved by dividing total ODR
 * period by 40 as below for 16KSPS ODR */
#define CONV_TRIGGER_PERIOD_NSEC		(((float)(1.0 / SAMPLING_RATE) * 1000000) * 1000)
#define CONV_TRIGGER_DUTY_CYCLE_NSEC	(CONV_TRIGGER_PERIOD_NSEC / 40)

/* Memory map for GPIOs on SDP-K1/STM32F4xxx MCU to read the values.
 * Mbed specific GPIO read/write library functions are very time stringent.
 * Since data capture on AD7134 is done using bit banging method, memory mapped
 * IOs are used for faster access of IO pins.
 * IF USING ANY OTHER MBED BOARD MAKE SURE MEMORY MAP IS UPDATED ACCORDINGLY */

/* Memory address of PORTx IDR (input data) register (Base + 0x10 offset) */
#define		DOUT1_IDR		(*((volatile uint32_t *)0x40020010)) // PORTA IDR
#define		ODR_IDR			(*((volatile uint32_t *)0x40020C10)) // PORTD_IDR
#define		DCLK_IDR		(*((volatile uint32_t *)0x40021810)) // PORTG IDR
#define     DOUT0_IDR       (*((volatile uint32_t *)0x40021810)) // PORTG IDR
#define		DCLK_ODR		(*((volatile uint32_t *)0x40021814)) // PORTG ODR

/* Pin mask values for GPIOs */
#define		DCLK_PIN_MASK	(uint32_t)(1 << DCLK_PIN)
#define		ODR_PIN_MASK	(uint32_t)(1 << ODR_PIN)
#define		DOUT0_PIN_MASK	(uint32_t)(1 << DOUT0_PIN)
#define		DOUT1_PIN_MASK	(uint32_t)(1 << DOUT1_PIN)

/* I2C timing register value for standard mode of operation
 * Check here for more understanding on I2C timing register
 * configuration: https://wiki.analog.com/resources/no-os/drivers/i2c */
#define I2C_TIMING				0x00000E14
#endif

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/
extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_spi_init_param stm32_spi_extra_init_params;
extern struct stm32_tdm_init_param stm32_tdm_extra_init_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_init_params;
extern struct stm32_gpio_init_param stm32_pdn_extra_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;
extern struct stm32_pwm_init_param stm32_pwm_extra_init_params;
extern struct stm32_gpio_init_param stm32_pwm_gpio_init_params;
extern struct stm32_usb_uart_init_param stm32_vcom_extra_init_params;
extern struct stm32_gpio_init_param stm32_input_extra_init_params;
extern struct stm32_gpio_init_param stm32_output_extra_init_params;
extern UART_HandleTypeDef APP_UART_HANDLE;
#ifdef STM32F469xx
extern TIM_HandleTypeDef htim4;
extern USBD_HandleTypeDef APP_UART_USB_HANDLE;
#endif
#if (INTERFACE_MODE == TDM_MODE)
extern bool data_capture_operation;
extern struct iio_device_data *ad7134_iio_dev_data;
void ad7134_dma_rx_cplt(void *hsai);
void ad7134_dma_rx_half_cplt(void *hsai);
#endif

void stm32_system_init(void);

#endif /* APP_CONFIG_STM32_H_ */

