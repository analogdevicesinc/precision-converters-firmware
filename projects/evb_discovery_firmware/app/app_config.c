/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module (platform-agnostic)
 *   @details This module performs the system configurations
********************************************************************************
 * Copyright (c) 2022-23 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdbool.h>

#include "app_config.h"
#include "eeprom_config.h"
#include "no_os_uart.h"
#include "no_os_eeprom.h"
#include "no_os_delay.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/* This value is calculated for SDP-K1 eval board (STM32F469NI MCU)
 * at 180Mhz core clock frequency */
#define EEPROM_OPS_START_DELAY		0xfffff

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* UART init parameters */
static struct no_os_uart_init_param uart_init_params = {
	.device_id = NULL,
	.baud_rate = IIO_UART_BAUD_RATE,
	.size = NO_OS_UART_CS_8,
	.parity = NO_OS_UART_PAR_NO,
	.stop = NO_OS_UART_STOP_1_BIT,
	.platform_ops = &uart_ops,
	.extra = &uart_extra_init_params
};

/* EEPROM init parameters */
struct no_os_eeprom_init_param eeprom_init_params = {
	.device_id = 0,
	.platform_ops = &eeprom_ops,
	.extra = &eeprom_extra_init_params
};

/* UART descriptor */
struct no_os_uart_desc *uart_desc;

/* EEPROM descriptor */
struct no_os_eeprom_desc *eeprom_desc;

/* Valid EEPROM device address detected by firmaware */
static uint8_t eeprom_detected_dev_addr;
static bool valid_eeprom_addr_detected;

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/

/**
 * @brief 	Initialize the UART peripheral
 * @return	0 in case of success, negative error code otherwise
 */
static int32_t init_uart(void)
{
	return no_os_uart_init(&uart_desc, &uart_init_params);
}

/**
 * @brief 	Initialize the system peripherals
 * @return	0 in case of success, negative error code otherwise
 */
int32_t init_system(void)
{
	int32_t ret;
	static volatile uint8_t eeprom_addr;
	static volatile uint8_t dummy_data;
	static volatile uint32_t cnt;

	ret = init_uart();
	if (ret) {
		return ret;
	}

#if defined (TARGET_SDP_K1)
	/* ~100msec Delay before starting EEPROM operations for SDP-K1.
	 * This delay makes sure that MCU is stable after power on
	 * cycle before doing any EEPROM operations */
	for (cnt = 0; cnt < EEPROM_OPS_START_DELAY; cnt++);
#endif

	ret = no_os_eeprom_init(&eeprom_desc, &eeprom_init_params);
	if (ret) {
		return ret;
	}

	/* Detect valid EEPROM */
	valid_eeprom_addr_detected = false;
	for (eeprom_addr = EEPROM_DEV_ADDR_START;
	     eeprom_addr <= EEPROM_DEV_ADDR_END; eeprom_addr++) {
		ret = load_eeprom_dev_address(eeprom_desc, eeprom_addr);
		if (ret) {
			return ret;
		}

		ret = no_os_eeprom_read(eeprom_desc, 0, (uint8_t *)&dummy_data, 1);
		if (!ret) {
			/* Valid EEPROM address detected */
			eeprom_detected_dev_addr = eeprom_addr;
			valid_eeprom_addr_detected = true;
			break;
		}
	}

	if (!valid_eeprom_addr_detected) {
		printf("No valid EEPROM address detected\r\n");
	} else {
		printf("Valid EEPROM address detected: %d\r\n", eeprom_addr);
	}

	return 0;
}

/**
 * @brief 	Get the EEPROM device address detected by firmware
 * @return	EEPROM device address
 */
uint8_t get_eeprom_detected_dev_addr(void)
{
	return eeprom_detected_dev_addr;
}

/**
 * @brief 	Return the flag indicating if valid EEPROM address is detected
 * @return	EEPROM valid address detect flag (true/false)
 */
bool is_eeprom_valid_dev_addr_detected(void)
{
	return valid_eeprom_addr_detected;
}
