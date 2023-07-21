/***************************************************************************//**
 *   @file    app_config_mbed.h
 *   @brief   Header file for Mbed platform configurations
********************************************************************************
 * Copyright (c) 2021, 2023 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef APP_CONFIG_MBED_H_
#define APP_CONFIG_MBED_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <PinNames.h>

#include "mbed_uart.h"
#include "mbed_gpio_irq.h"
#include "mbed_spi.h"
#include "mbed_i2c.h"
#include "mbed_gpio.h"
#include "mbed_pwm.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Pin mapping of SDP-K1 w.r.t Arduino connector */
#define SPI_CSB			ARDUINO_UNO_D10
#define SPI_HOST_SDO	ARDUINO_UNO_D11
#define SPI_HOST_SDI	ARDUINO_UNO_D12
#define SPI_SCK			ARDUINO_UNO_D13

#define I2C_SCL			ARDUINO_UNO_D15
#define I2C_SDA			ARDUINO_UNO_D14

#define UART_TX			CONSOLE_TX
#define	UART_RX			CONSOLE_RX

#define DCLK_PIN		ARDUINO_UNO_D2
#define ODR_PIN			ARDUINO_UNO_D3
#define DOUT0_PIN		ARDUINO_UNO_D4
#define DOUT1_PIN		ARDUINO_UNO_D5
#define PDN_PIN         ARDUINO_UNO_D1

/* Port numbers */
#define PDN_PORT                0  // (Unused)

/* Memory map for GPIOs on SDP-K1/STM32F4xxx MCU to read the values.
 * Mbed specific GPIO read/write library functions are very time stringent.
 * Since data capture on AD7134 is done using bit banging method, memory mapped
 * IOs are used for faster access of IO pins.
 * IF USING ANY OTHER MBED BOARD MAKE SURE MEMORY MAP IS UPDATED ACCORDINGLY */

/* Memory address of PORTx IDR (input data) register (Base + 0x10 offset) */
#define		DOUT1_IDR		(*((volatile uint32_t *)0x40020010)) // PORTA IDR
#define		ODR_IDR		(*((volatile uint32_t *)0x40020C10))     // PORTD_IDR
#define		DCLK_IDR		(*((volatile uint32_t *)0x40021810)) // PORTG IDR
#define     DOUT0_IDR       (*((volatile uint32_t *)0x40021810)) // PORTG IDR

#define		DCLK_ODR		(*((volatile uint32_t *)0x40021814))  // PORTG ODR

/* Pin numbers corresponding to GPIOs */
#define		DCLK_PIN_NUM	7		// PG7 (Arduino D2)
#define		ODR_PIN_NUM		12		// PD12 (Arduino D3)
#define		DOUT0_PIN_NUM	9		// PG9 (Arduino D4)
#define		DOUT1_PIN_NUM	11		// PA11 (Arduino D5)

/* Pin mask values for GPIOs */
#define		DCLK_PIN_MASK	(uint32_t)(1 << DCLK_PIN_NUM)
#define		ODR_PIN_MASK	(uint32_t)(1 << ODR_PIN_NUM)
#define		DOUT0_PIN_MASK	(uint32_t)(1 << DOUT0_PIN_NUM)
#define		DOUT1_PIN_MASK	(uint32_t)(1 << DOUT1_PIN_NUM)

#define UART_IRQ_ID	0 // Unused
#define IRQ_INT_ID	GPIO_IRQ_ID1
#define UART_DEVICE_ID  0
#define SPI_DEVICE_ID   0
#define I2C_DEVICE_ID   0

/* Define the max possible sampling (or output data) rate for a given platform.
 * Note: Max possible ODR is 12 KSPS per channel for continuous data capture on IIO client.
 * This is derived by testing the firmware on SDP-K1 controller board with STM32F469NI MCU
 * using GCC and ARM compilers. The max possible ODR can vary from board to board and
 * data continuity is not guaranteed above this ODR on IIO oscilloscope */
#define SAMPLING_RATE		(12000)

/* PWM period and duty cycle for AD7134 ASRC target mode. The low period of ODR as per specs
 * must be minimum 3 * Tdclk in target mode. The min possible Fdclk for SDP-K1 (STM32F469NI)
 * platform is ~3Mhz (based on time to sample data over DOUT), which gives Tdclk as ~333nsec.
 * So ODR min low time must be 333ns * 3 = ~1usec. This is achieved by dividing total ODR
 * period by 40 as below for 16KSPS ODR */
#define CONV_TRIGGER_PERIOD_NSEC		(((float)(1.0 / SAMPLING_RATE) * 1000000) * 1000)
#define CONV_TRIGGER_DUTY_CYCLE_NSEC	(CONV_TRIGGER_PERIOD_NSEC / 40)

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct mbed_gpio_irq_init_param mbed_ext_int_extra_init_params;
extern struct mbed_uart_init_param mbed_uart_extra_init_params;
extern struct mbed_uart_init_param mbed_vcom_extra_init_params;
extern struct mbed_spi_init_param mbed_spi_extra_init_params;
extern struct mbed_i2c_init_param mbed_i2c_extra_init_params;
extern struct mbed_pwm_init_param mbed_pwm_extra_init_params;
extern struct mbed_gpio_init_param mbed_pdn_extra_init_params;
void ad7134_configure_intr_priority(void);

#endif /* APP_CONFIG_MBED_H_ */
