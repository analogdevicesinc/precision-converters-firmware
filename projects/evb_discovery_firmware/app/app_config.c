/***************************************************************************//**
 *   @file    app_config.c
 *   @brief   Application configurations module (platform-agnostic)
 *   @details This module performs the system configurations
********************************************************************************
 * Copyright (c) 2022-24 Analog Devices, Inc.
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
#include "common.h"
#include "no_os_uart.h"
#include "no_os_eeprom.h"
#include "no_os_delay.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

#ifdef ENABLE_EXTENDED_EEPROM_SEARCH
#define EEPROM_MAX_DEVICES (sizeof((uint32_t [])I2C_DEVICE_ID_EX)/sizeof(uint32_t))
#endif

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

/* I2C init parameters */
static struct no_os_i2c_init_param no_os_i2c_init_params = {
	.device_id = I2C_DEVICE_ID,
	.platform_ops = &i2c_ops,
	.max_speed_hz = 100000,
	.extra = &i2c_extra_init_params
};

/* EEPROM init parameters */
static struct eeprom_24xx32a_init_param eeprom_extra_init_params = {
	.i2c_init = &no_os_i2c_init_params
};

/* EEPROM init parameters */
static struct no_os_eeprom_init_param eeprom_init_params = {
	.device_id = 0,
	.platform_ops = &eeprom_24xx32a_ops,
	.extra = &eeprom_extra_init_params
};

#ifdef ENABLE_EXTENDED_EEPROM_SEARCH

const uint32_t i2c_device_id_ex[] = I2C_DEVICE_ID_EX;

struct no_os_i2c_init_param no_os_i2c_init_params_ex[] = {
	{
		.device_id = i2c_device_id_ex[0],
		.platform_ops = &i2c_ops,
		.max_speed_hz = 100000,
		.extra = &i2c_extra_init_params_ex
	}
};

/* EEPROM init parameters */
static struct eeprom_24xx32a_init_param eeprom_extra_init_params_ex = {
	.i2c_init = no_os_i2c_init_params_ex
};

#endif

/* UART descriptor */
struct no_os_uart_desc *uart_desc;

/* EEPROM descriptor */
struct no_os_eeprom_desc *eeprom_desc;

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

#if (ACTIVE_PLATFORM == STM32_PLATFORM)
	stm32_system_init();
#endif

	ret = init_uart();
	if (ret) {
		return ret;
	}

	ret = eeprom_init(&eeprom_desc, &eeprom_init_params);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Generator function for the EEPROM descriptor
 *
 * @param desc[in,out] - Pointer to the EEPROM descriptor
 * @param idx[in,out] - Pointer to the index of the EEPROM descriptor
 * @return 0 in case of success, negative error code otherwise
 */
int32_t get_next_eeprom_desc(struct no_os_eeprom_desc **desc, uint32_t *idx)
{
#ifdef ENABLE_EXTENDED_EEPROM_SEARCH
	int32_t ret;
	struct no_os_eeprom_desc *eeprom_desc = NULL;
	if (!desc || !idx) {
		return -EINVAL;
	}
	if (*desc) {
		ret = no_os_eeprom_remove(*desc);
		if (ret) {
			return ret;
		}
	}

	if (*idx == EEPROM_MAX_DEVICES) {
		*idx = 0;
		eeprom_desc = NULL;
	} else {
		eeprom_extra_init_params_ex.i2c_init = &no_os_i2c_init_params_ex[*idx];
		eeprom_init_params.extra = &eeprom_extra_init_params_ex;
		ret = eeprom_init(&eeprom_desc, &eeprom_init_params);
		if (ret) {
			return ret;
		}
	}

	(*idx)++;
	*desc = eeprom_desc;

	return 0;
#else
	return -ENOTSUP;
#endif
}
