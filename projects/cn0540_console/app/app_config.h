/*******************************************************************************
 *   @file   app_config.h
 *   @brief  Configuration module for CN0540 console application
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

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>

#include "no_os_uart.h"
#include "no_os_i2c.h"
#include "no_os_spi.h"
#include "no_os_gpio.h"
#include "no_os_irq.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* List of supported platforms*/
#define	STM32_PLATFORM		1

/* Select the active platform (default is STM32) */
#if !defined(ACTIVE_PLATFORM)
#define ACTIVE_PLATFORM		STM32_PLATFORM
#endif

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
#include "app_config_stm32.h"
#else
#error "No/Invalid active platform selected"
#endif

/* Baud rate for IIO application UART interface */
#define UART_BAUD_RATE	(230400)

/* ADC reference voltage in mV */
#define ADC_REF_VOLTAGE		4096

/*  MCLK in kHz */
#define MCLK_KHZ			16384

/* Default Sample Rate in Hz */
#define DEFAULT_SAMPLE_RATE	32000

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
extern struct no_os_uart_init_param uart_init_param;
extern const struct no_os_gpio_init_param gpio_red_led_param;
extern const struct no_os_gpio_init_param gpio_blue_led_param;
extern const struct no_os_gpio_init_param gpio_rst_param;
extern const struct no_os_gpio_init_param gpio_buf_en_param;
extern struct no_os_irq_init_param trigger_gpio_irq_param;
extern struct no_os_irq_ctrl_desc *trigger_irq_desc;
extern const float programmable_FIR[56];
extern const uint8_t count_of_active_coeffs;

int32_t gpio_trigger_init(void);
int32_t sdpk1_gpio_setup(void);
int32_t adc_hard_reset(void);

#endif //APP_CONFIG_H