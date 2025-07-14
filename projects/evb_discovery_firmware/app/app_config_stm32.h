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

#include "stm32_uart.h"
#include "stm32_i2c.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* I2C timing register value for standard mode of operation
 * Check here for more understanding on I2C timing register
 * configuration: https://wiki.analog.com/resources/no-os/drivers/i2c */
#define I2C_TIMING			0x00000E14

#define I2C_DEVICE_ID       1 // I2C1

#if defined (TARGET_SDP_K1)
#define APP_UART_HANDLE     &huart5 // UART5
#define HW_CARRIER_NAME		SDP-K1
#define I2C_DEVICE_ID_EX	{3}
#else
#define APP_UART_HANDLE     &huart3 // UART3
#define HW_CARRIER_NAME		NUCLEO-H563ZI
#endif

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct stm32_i2c_init_param stm32_i2c_extra_init_params;
extern struct stm32_i2c_init_param stm32_i2c_extra_init_params_ex[];
extern struct stm32_uart_init_param stm32_uart_extra_init_params;
#if defined (TARGET_SDP_K1)
extern UART_HandleTypeDef huart5;
#else
extern UART_HandleTypeDef huart3;
#endif

void stm32_system_init(void);

#endif /* APP_CONFIG_STM32_H_ */
