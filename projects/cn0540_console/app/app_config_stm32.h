/*******************************************************************************
 *   @file   app_config_stm32.h
 *   @brief  Header file for STM32 platform configurations
********************************************************************************
Copyright 2025(c) Analog Devices, Inc.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of Analog Devices, Inc. nor the names of its
   contributors may be used to endorse or promote products derived from this
   software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY ANALOG DEVICES, INC. “AS IS” AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL ANALOG DEVICES, INC. BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************/

#ifndef APP_CONFIG_STM32_H_
#define APP_CONFIG_STM32_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "stm32_uart.h"
#include "stm32_i2c.h"
#include "stm32_spi.h"
#include "stm32_gpio.h"
#include "stm32_gpio_irq.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

//DAC address
#define LTC2606_I2C_ADDRESS 0x10

/* STM32 UART specific parameters */
#define UART_DEVICE_ID		5
#define APP_UART_HANDLE     &huart5
#define UART_IRQ_ID         UART5_IRQn

/* STM32 GPIO specific parameters */
#define RED_LED_PIN		1 // PA1
#define RED_LED_PORT	0 // PORT A
#define BLUE_LED_PIN	0 // PA0
#define BLUE_LED_PORT	0 // PORT A
#define RESET_PIN		10 // PG10
#define RESET_PORT		6 // PORT G
#define BUF_EN_PIN		15 // PB15
#define BUF_EN_PORT		1 // PORT B

/* Interrupt Callback parameters */
#define IRQ_CTRL_ID		7 // PG7

/* STM32 SPI specific parameters */
#define SPI_DEVICE_ID		1 // SPI1
#define MAX_SPI_SCLK		20000000
#define SPI_CS_PIN			15 // PA15

/* STM32 I2C specific parameters */
#define I2C_DEVICE_ID		1 // I2C1

/* Redefine the init params structure mapping w.r.t. platform */
#define uart_extra_init_params stm32_uart_extra_init_params
#define gpio_red_led_params stm32_gpio_red_led_params
#define gpio_blue_led_params stm32_gpio_blue_led_params
#define gpio_rst_params stm32_gpio_rst_params
#define gpio_buf_en_params stm32_gpio_buf_en_params
#define trigger_gpio_irq_params stm32_trigger_gpio_irq_params
#define spi_init_extra_params stm32_spi_init_extra_params

/* Platform ops */
#define gpio_ops                    stm32_gpio_ops
#define spi_ops		                stm32_spi_ops
#define i2c_ops                     stm32_i2c_ops
#define uart_ops                    stm32_uart_ops
#define gpio_irq_ops				stm32_gpio_irq_ops

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern UART_HandleTypeDef huart5;
extern struct stm32_uart_init_param stm32_uart_extra_init_params;
extern struct stm32_gpio_init_param stm32_gpio_red_led_params;
extern struct stm32_gpio_init_param stm32_gpio_blue_led_params;
extern struct stm32_gpio_init_param stm32_gpio_rst_params;
extern struct stm32_gpio_init_param stm32_gpio_buf_en_params;
extern struct stm32_gpio_irq_init_param stm32_trigger_gpio_irq_params;
extern struct stm32_spi_init_param stm32_spi_init_extra_params;

extern void stm32_system_init(void);

#endif /* APP_CONFIG_STM32_H_ */