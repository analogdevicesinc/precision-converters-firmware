/*!
 *****************************************************************************
  @file:  ad5592r_user_config.c
  @brief: Device parameters, structure and initial condition settings
  @details: Settings for parameters and descriptors for interface protocols. Start up configuration

 -----------------------------------------------------------------------------
 Copyright (c) 2020-2022 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
*****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "app_config.h"

#include "ad5592r-base.h"
#include "mbed_platform_support.h"
#include "no_os_i2c.h"
#include "no_os_spi.h"
#include "mbed_spi.h"
#include "mbed_i2c.h"

/******************************************************************************/
/***************************** Type Definitions **********************************/
/******************************************************************************/
struct mbed_spi_init_param spi_init_extra_params = {
	.spi_clk_pin = SPI_SCK,
	.spi_miso_pin = SPI_HOST_SDI,
	.spi_mosi_pin = SPI_HOST_SDO,
};

struct mbed_i2c_init_param i2c_init_extra_params = {
	.i2c_sda_pin = I2C_SDA,
	.i2c_scl_pin = I2C_SCL
};

struct no_os_spi_init_param spi_user_params = {
	.max_speed_hz = 10000000,
	.chip_select = SPI_CSB,
	.mode = NO_OS_SPI_MODE_2,
	.platform_ops = &mbed_spi_ops,
	.extra = &spi_init_extra_params,
};

struct no_os_i2c_init_param i2c_user_params = {
	.max_speed_hz = 100000,
	.slave_address = AD5593R_I2C,
	.platform_ops = &mbed_i2c_ops,
	.extra = &i2c_init_extra_params
};

/******************************************************************************/
/***************************** Constants  **********************************/
/******************************************************************************/
const struct ad5592r_init_param ad5592r_user_param = {
	.int_ref = false
};

const struct ad5592r_dev ad5592r_dev_user = {
	.ops = NULL,
	.spi = NULL,
	.i2c = NULL,
	.spi_msg = 0,
	.num_channels = NUM_CHANNELS,
	.cached_dac = { 0, 0, 0, 0, 0, 0, 0 },
	.cached_gp_ctrl = 0,
	.channel_modes = {
		CH_MODE_UNUSED,
		CH_MODE_UNUSED,
		CH_MODE_UNUSED,
		CH_MODE_UNUSED,
		CH_MODE_UNUSED,
		CH_MODE_UNUSED,
		CH_MODE_UNUSED,
		CH_MODE_UNUSED
	},
	.channel_offstate = {
		CH_OFFSTATE_PULLDOWN,
		CH_OFFSTATE_PULLDOWN,
		CH_OFFSTATE_PULLDOWN,
		CH_OFFSTATE_PULLDOWN,
		CH_OFFSTATE_PULLDOWN,
		CH_OFFSTATE_PULLDOWN,
		CH_OFFSTATE_PULLDOWN,
		CH_OFFSTATE_PULLDOWN
	},
	.gpio_out = 0,
	.gpio_in = 0,
	.gpio_val = 0,
	.ldac_mode = 0,
};
