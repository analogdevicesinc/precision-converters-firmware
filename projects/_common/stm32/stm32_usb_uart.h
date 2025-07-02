/***************************************************************************//**
 * @file    stm32_usb_uart.h
 * @brief   VCOM driver for stm32 as a no_os_uart implementation.
********************************************************************************
* Copyright (c) 2025 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef _STM32_USB_UART_H_
#define _STM32_USB_UART_H_

#include <stdint.h>
#include <stdbool.h>
#include "no_os_uart.h"
#include "stm32_hal.h"
#include "ux_api.h"
#include "ux_device_class_cdc_acm.h"

/**
 * @struct stm32_usb_uart_init_param
 * @brief Specific initialization parameters for stm32 UART over USB.
 */
struct stm32_usb_uart_init_param {
	/** PCD instance */
	PCD_HandleTypeDef *hpcd;
};

/**
 * @struct stm32_usb_uart_desc
 * @brief stm32 platform specific UART over USB descriptor.
 */
struct stm32_usb_uart_desc {
	/** PCD instance */
	PCD_HandleTypeDef *hpcd;
	/** USB UART instance */
	UX_SLAVE_CLASS_CDC_ACM *husbdevice;
	/** FIFO */
	struct lf256fifo *fifo;
};

/**
 * @brief stm32 specific UART over USB platform ops structure.
 */
extern const struct no_os_uart_platform_ops stm32_usb_uart_ops;

#endif
