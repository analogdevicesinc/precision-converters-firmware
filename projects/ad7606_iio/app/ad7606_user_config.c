/*************************************************************************//**
 *   @file   ad7606_user_config.c
 *   @brief  User configuration file for AD7606 device
******************************************************************************
* Copyright (c) 2020-2022 Analog Devices, Inc.
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

#include "app_config.h"
#include "ad7606_user_config.h"
#include "ad7606_support.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

#define		USER_CONFIG_RANGE	(DEFAULT_CHN_RANGE * 1000)

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

struct no_os_gpio_init_param gpio_init_reset = {
	.number = RESET_PIN,
	.platform_ops = &gpio_ops,
	.extra = &reset_gpio_extra_init_params
};

struct no_os_gpio_init_param gpio_init_convst = {
	.number = CONVST_PIN,
	.platform_ops = &gpio_ops,
	.extra = &convst_gpio_extra_init_params
};

struct no_os_gpio_init_param gpio_init_busy = {
	.number = BUSY_PIN,
	.platform_ops = &gpio_ops,
	.extra = &busy_gpio_extra_init_params
};

struct no_os_gpio_init_param gpio_init_osr0 = {
	.number = OSR0_PIN,
	.platform_ops = &gpio_ops,
	.extra = &osr0_gpio_extra_init_params
};

struct no_os_gpio_init_param gpio_init_osr1 = {
	.number = OSR1_PIN,
	.platform_ops = &gpio_ops,
	.extra = &osr1_gpio_extra_init_params
};

struct no_os_gpio_init_param gpio_init_osr2 = {
	.number = OSR2_PIN,
	.platform_ops = &gpio_ops,
	.extra = &osr2_gpio_extra_init_params
};

struct no_os_gpio_init_param gpio_init_range= {
	.number = RANGE_PIN,
	.platform_ops = &gpio_ops,
	.extra = &range_gpio_extra_init_params
};

struct no_os_gpio_init_param gpio_init_stdby = {
	.number = STDBY_PIN,
	.platform_ops = &gpio_ops,
	.extra = &stdby_gpio_extra_init_params
};

/* Initialize the AD7606 device structure */
struct ad7606_init_param ad7606_init_str = {
	// Define SPI init parameters structure
	.spi_init = {
		.max_speed_hz = 22500000,   	// Max SPI Speed
		.chip_select = SPI_CSB,   		// Chip Select
		.mode = NO_OS_SPI_MODE_2,   	// CPOL = 1, CPHA = 0
		.extra = &spi_extra_init_params,	// SPI extra configurations
		.platform_ops = &spi_ops
	},

	.gpio_reset = &gpio_init_reset,
	.gpio_convst = &gpio_init_convst,
	.gpio_busy = &gpio_init_busy,
	.gpio_stby_n = &gpio_init_stdby,
	.gpio_range = &gpio_init_range,
	.gpio_os0 = &gpio_init_osr0,
	.gpio_os1 = &gpio_init_osr1,
	.gpio_os2 = &gpio_init_osr2,
	.gpio_par_ser = NULL,

	.device_id = ACTIVE_DEVICE,
	.oversampling = { 0, AD7606_OSR_1 },
	.sw_mode = true,

	// Below settings (except range) applies only to AD7606B and AD7606C devices

	/* Device Configs */
	{
		.op_mode = AD7606_NORMAL,
		.dout_format = AD7606_1_DOUT,
		.ext_os_clock = false,
		.status_header = false
	},

	/* Diagnostic flags setting */
	{
		.rom_crc_err_en = false,
		.mm_crc_err_en = false,
		.int_crc_err_en = false,
		.spi_write_err_en = false,
		.spi_read_err_en = false,
		.busy_stuck_high_err_en = false,
		.clk_fs_os_counter_en = false,
		.interface_check_en = false
	},

	/* Default offset for all channels */
	.offset_ch = { 0, 0, 0, 0, 0, 0, 0, 0 },

	/* Default phase (0x00) for all channels */
	.phase_ch = { 0, 0, 0, 0, 0, 0, 0, 0 },

	/* Default gain (0x00) for all channels */
	.gain_ch = { 0, 0, 0, 0, 0, 0, 0, 0 },

	/* Default range for all channels */
	.range_ch = {
#if defined(DEV_AD7609)
		{-USER_CONFIG_RANGE, USER_CONFIG_RANGE, true},
		{-USER_CONFIG_RANGE, USER_CONFIG_RANGE, true},
		{-USER_CONFIG_RANGE, USER_CONFIG_RANGE, true},
		{-USER_CONFIG_RANGE, USER_CONFIG_RANGE, true},
		{-USER_CONFIG_RANGE, USER_CONFIG_RANGE, true},
		{-USER_CONFIG_RANGE, USER_CONFIG_RANGE, true},
		{-USER_CONFIG_RANGE, USER_CONFIG_RANGE, true},
		{-USER_CONFIG_RANGE, USER_CONFIG_RANGE, true},
#else
		{-USER_CONFIG_RANGE, USER_CONFIG_RANGE, false},
		{-USER_CONFIG_RANGE, USER_CONFIG_RANGE, false},
		{-USER_CONFIG_RANGE, USER_CONFIG_RANGE, false},
		{-USER_CONFIG_RANGE, USER_CONFIG_RANGE, false},
		{-USER_CONFIG_RANGE, USER_CONFIG_RANGE, false},
		{-USER_CONFIG_RANGE, USER_CONFIG_RANGE, false},
		{-USER_CONFIG_RANGE, USER_CONFIG_RANGE, false},
		{-USER_CONFIG_RANGE, USER_CONFIG_RANGE, false}
#endif
	}
};
