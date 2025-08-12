/*************************************************************************//**
 *   @file   ad405x_user_config.c
 *   @brief  User configuration file for AD405X
******************************************************************************
* Copyright (c) 2022-2024 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include "ad405x_user_config.h"
#include "app_config.h"
#include "no_os_spi.h"
#include "no_os_gpio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
#define NUM_I3C_DEVS		2
/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
#ifdef SPI_SUPPORT_AVAILABLE
static struct no_os_spi_init_param spi_init_params = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = MAX_SPI_SCLK,
	.mode = NO_OS_SPI_MODE_0,
	.chip_select = SPI_CS_PIN_NUM,
	.bit_order = NO_OS_SPI_BIT_ORDER_MSB_FIRST,
	.platform_ops = &spi_ops,
	.extra = &spi_extra_init_params
};

/* ad405x cnv init parameters */
static struct no_os_gpio_init_param gpio_cnv_param = {
	.port = CNV_PORT_NUM,
	.number = CNV_PIN_NUM,
	.platform_ops = &gpio_ops,
	.extra = &cnv_extra_init_params
};
#endif

#ifdef I3C_SUPPORT_AVAILABLE
volatile uint8_t ad405x_i3c_dyn_addr = AD405X_I3C_GEN_DYN_ADDR_DEFAULT;

extern const struct no_os_i3c_init_param *i3c_devs_param[NUM_I3C_DEVS];

/* AD406x I3C bus init param */
struct no_os_i3c_bus_init_param i3c_bus_init_params = {
	.device_id = I3C_DEV_ID,
	.platform_ops = &i3c_ops,
	.devs = i3c_devs_param,
	.num_devs = NUM_I3C_DEVS,
	.extra = &i3c_extra_init_params
};

/* AD4062 I3C init param */
struct no_os_i3c_init_param ad4062_i3c_init_params = {
	.bus = &i3c_bus_init_params,
	.pid = AD405X_I3C_GEN_PID(0xC, AD405X_INSTANCE_ID),
	.is_i3c = true,
	.addr = AD405X_I3C_GEN_DYN_ADDR_DEFAULT,
	.is_static = false,
};

/* AD4060 I3C init param */
struct no_os_i3c_init_param ad4060_i3c_init_params = {
	.bus = &i3c_bus_init_params,
	.pid = AD405X_I3C_GEN_PID(0xA, AD405X_INSTANCE_ID),
	.is_i3c = true,
	.addr = AD405X_I3C_GEN_DYN_ADDR_DEFAULT,
	.is_static = false,
};

/* array of devices on the I3C bus */
const struct no_os_i3c_init_param *i3c_devs_param[NUM_I3C_DEVS] = {
	&ad4062_i3c_init_params, &ad4060_i3c_init_params
};
#endif

/* ad405x gpio0 init parameters */
struct no_os_gpio_init_param gpio_gpio0_param = {
	.port = GP0_PORT_NUM,
	.number = GP0_PIN_NUM,
	.platform_ops = &gpio_ops,
	.extra = &gp0_extra_init_params
};

/* ad405x gpio1 init parameters */
struct no_os_gpio_init_param gpio_gpio1_param = {
	.port = GP1_PORT_NUM,
	.number = GP1_PIN_NUM,
	.platform_ops = &gpio_ops,
	.extra = &gp1_extra_init_params
};

/* Initialize the AD405X device structure */
struct ad405x_init_param ad405x_init_params = {
#ifdef SPI_SUPPORT_AVAILABLE
	.comm_type = AD405X_SPI_COMM,
	.comm_init.spi_init = &spi_init_params,
	.gpio_cnv = &gpio_cnv_param,
#elif defined(I3C_SUPPORT_AVAILABLE)
	.comm_type = AD405X_I3C_COMM,
	.comm_init.i3c_init = &ad4062_i3c_init_params,
	.gpio_cnv = NULL,
#endif

	.gpio_gpio0 = &gpio_gpio0_param,
	.gpio_gpio1 = &gpio_gpio1_param,
};
