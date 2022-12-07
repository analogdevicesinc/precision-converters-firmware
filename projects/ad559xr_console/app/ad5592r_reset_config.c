/*!
 *****************************************************************************
  @file:  ad5592r_reset_config.c
  @brief: Device parameters, structure and reset condition settings
  @details: Settings for device upon reset

 -----------------------------------------------------------------------------
 Copyright (c) 2020 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
*****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "app_config.h"
#include "ad5592r-base.h"

/******************************************************************************/
/***************************** Constants  **********************************/
/******************************************************************************/
const struct ad5592r_dev ad5592r_dev_reset = {
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
