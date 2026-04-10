/***************************************************************************//**
 * @file    ad717x_user_config.c
 * @brief   User Configuration source for AD717x-AD411x IIO Application
********************************************************************************
* Copyright (c) 2021-22,2025-26 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include "ad717x_user_config.h"
#include "ad717x.h"
#include "ad717x_iio.h"
#include "no_os_util.h"
#include "ad411x_regs.h"
#include "ad4113_regs.h"
#include "ad7172_2_regs.h"
#include "ad7172_4_regs.h"
#include "ad7173_8_regs.h"
#include "ad7175_2_regs.h"
#include "ad7175_8_regs.h"
#include "ad7176_2_regs.h"
#include "ad7177_2_regs.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

struct ad717x_device_map_info device_map_table[] = {
	[ID_AD4111] = {
		.name = "ad4111",
		.regs = ad4111_regs,
		.num_regs = NO_OS_ARRAY_SIZE(ad4111_regs),
		.num_channels = 16,
		.num_setups = 8,
		.use_input_pairs = true,
		.scale_factor = 0.1,
		.resolution = 24,
		.supports_open_wire = true
	},
	[ID_AD4112] = {
		.name = "ad4112",
		.regs = ad4111_regs,
		.num_regs = NO_OS_ARRAY_SIZE(ad4111_regs),
		.num_channels = 16,
		.num_setups = 8,
		.use_input_pairs = true,
		.scale_factor = 0.1,
		.resolution = 24,
		.supports_open_wire = false
	},
	[ID_AD4113] = {
		.name = "ad4113",
		.regs = ad4113_regs,
		.num_regs = NO_OS_ARRAY_SIZE(ad4113_regs),
		.num_channels = 16,
		.num_setups = 8,
		.use_input_pairs = true,
		.scale_factor = 0.1,
		.resolution = 16,
		.supports_open_wire = true
	},
	[ID_AD4114] = {
		.name = "ad4114",
		.regs = ad4111_regs,
		.num_regs = NO_OS_ARRAY_SIZE(ad4111_regs),
		.num_channels = 16,
		.num_setups = 8,
		.use_input_pairs = true,
		.scale_factor = 0.1,
		.resolution = 24,
		.supports_open_wire = false
	},
	[ID_AD4115] = {
		.name = "ad4115",
		.regs = ad4111_regs,
		.num_regs = NO_OS_ARRAY_SIZE(ad4111_regs),
		.num_channels = 16,
		.num_setups = 8,
		.use_input_pairs = true,
		.scale_factor = 0.1,
		.resolution = 24,
		.supports_open_wire = false
	},
	[ID_AD4116] = {
		.name = "ad4116",
		.regs = ad4111_regs,
		.num_regs = NO_OS_ARRAY_SIZE(ad4111_regs),
		.num_channels = 16,
		.num_setups = 8,
		.use_input_pairs = true,
		.scale_factor = 0.1,
		.resolution = 24,
		.supports_open_wire = false
	},
	[ID_AD7172_2] = {
		.name = "ad7172-2",
		.regs = ad7172_2_regs,
		.num_regs = NO_OS_ARRAY_SIZE(ad7172_2_regs),
		.num_channels = 4,
		.num_setups = 4,
		.use_input_pairs = false,
		.scale_factor = 1.0,
		.resolution = 24,
		.supports_open_wire = false
	},
	[ID_AD7172_4] = {
		.name = "ad7172-4",
		.regs = ad7172_4_regs,
		.num_regs = NO_OS_ARRAY_SIZE(ad7172_4_regs),
		.num_channels = 8,
		.num_setups = 8,
		.use_input_pairs = false,
		.scale_factor = 1.0,
		.resolution = 24,
		.supports_open_wire = false
	},
	[ID_AD7173_8] = {
		.name = "ad7173-8",
		.regs = ad7173_8_regs,
		.num_regs = NO_OS_ARRAY_SIZE(ad7173_8_regs),
		.num_channels = 16,
		.num_setups = 8,
		.use_input_pairs = false,
		.scale_factor = 1.0,
		.resolution = 24,
		.supports_open_wire = false
	},
	[ID_AD7175_2] = {
		.name = "ad7175-2",
		.regs = ad7175_2_regs,
		.num_regs = NO_OS_ARRAY_SIZE(ad7175_2_regs),
		.num_channels = 4,
		.num_setups = 4,
		.use_input_pairs = false,
		.scale_factor = 1.0,
		.resolution = 24,
		.supports_open_wire = false
	},
	[ID_AD7175_8] = {
		.name = "ad7175-8",
		.regs = ad7175_8_regs,
		.num_regs = NO_OS_ARRAY_SIZE(ad7175_8_regs),
		.num_channels = 16,
		.num_setups = 8,
		.use_input_pairs = false,
		.scale_factor = 1.0,
		.resolution = 24,
		.supports_open_wire = false
	},
	[ID_AD7176_2] = {
		.name = "ad7176-2",
		.regs = ad7176_2_regs,
		.num_regs = NO_OS_ARRAY_SIZE(ad7176_2_regs),
		.num_channels = 4,
		.num_setups = 4,
		.use_input_pairs = false,
		.scale_factor = 1.0,
		.resolution = 24,
		.supports_open_wire = false
	},
	[ID_AD7177_2] = {
		.name = "ad7177-2",
		.regs = ad7177_2_regs,
		.num_regs = NO_OS_ARRAY_SIZE(ad7177_2_regs),
		.num_channels = 4,
		.num_setups = 4,
		.use_input_pairs = false,
		.scale_factor = 1.0,
		.resolution = 24,
		.supports_open_wire = false
	}
};

/* Default setup configuration */
struct ad717x_channel_setup default_setups[AD717x_MAX_SETUPS] = {
	[0 ... AD717x_MAX_SETUPS - 1] = {
		.bi_unipolar = true,
		.ref_buff = false,
		.input_buff = true,
		.ref_source = INTERNAL_REF,
	}
};

/* Default AD717x family filter configuration */
struct ad717x_filtcon default_ad717x_filtcons[AD717x_MAX_SETUPS] = {
	[0 ... AD717x_MAX_SETUPS - 1] = {
		.sinc3_map = false,
		.enhfilten = false,
		.enhfilt = sps27_db47_ms36p7,
		.oder = sinc5_sinc1,
		.odr = sps_31250_a
	}
};

/* Default AD411x family channel map entry */
struct ad717x_channel_map default_ad411x_chan_maps[AD717x_MAX_CHANNELS] = {
	[0 ... AD717x_MAX_CHANNELS - 1] = {
		.channel_enable = false,
		.setup_sel = 0,
		.analog_inputs.analog_input_pairs = VIN0_VIN1,
	}
};

/* Default AD717x family channel map entry */
struct ad717x_channel_map default_ad717x_chan_maps[AD717x_MAX_CHANNELS] = {
	[0 ... AD717x_MAX_CHANNELS - 1] = {
		.channel_enable = false,
		.setup_sel = 0,
		.analog_inputs.ainp.pos_analog_input = AIN0,
		.analog_inputs.ainp.neg_analog_input = AIN1,
	}
};

/* AD717x Init Parameters */
ad717x_init_param ad717x_init_params = {
	.spi_init = {
		.device_id = SPI_DEVICE_ID,
		.max_speed_hz = MAX_SPI_SCLK,
		.chip_select = SPI_CSB,
		.mode = NO_OS_SPI_MODE_3,
		.platform_ops = &spi_platform_ops,
		.extra = &spi_extra_init_params
	},
	.ref_en = true,
	.mode =  CONTINUOUS,
};

/******************************************************************************/
/************************** Functions Declaration *****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definition ******************************/
/******************************************************************************/
