/*!
 *****************************************************************************
  @file:  ad5770_user_config.c
  @brief: AD5770R user configuration
  @details:
 -----------------------------------------------------------------------------
 *
Copyright (c) 2020-2022 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.
By using this software you agree to the terms of the associated
Analog Devices Software License Agreement.
 ******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#include "app_config.h"
#include "no_os_spi.h"
#include "mbed_spi.h"
#include "ad5770r.h"

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

struct mbed_spi_init_param spi_init_extra_params = {
	.spi_clk_pin = SPI_SCK,
	.spi_miso_pin = SPI_HOST_SDI,
	.spi_mosi_pin = SPI_HOST_SDO,
	.use_sw_csb = false
};

struct ad5770r_init_param ad5770r_user_param = {
	/* SPI */
	.spi_init = {
		.max_speed_hz = 2500000,
		.chip_select = SPI_CSB,
		.mode = NO_OS_SPI_MODE_0,
		.extra = &spi_init_extra_params,
		.platform_ops = &mbed_spi_ops
	},

	/* Device SPI Settings */
	.dev_spi_settings = {
		.addr_ascension = false,
		.single_instruction = false,
		.stream_mode_length = 0
	},

	/* Device Settings */
	.channel_config = {
		.en0 = true,
		.en1 = true,
		.en2 = true,
		.en3 = true,
		.en4 = true,
		.en5 = true,
		.sink0 = false
	},

	.output_mode = {
		{
			.output_scale = 0x00,
			.output_range_mode = 0x00
		},
		{
			.output_scale = 0x00,
			.output_range_mode = 0x00
		},
		{
			.output_scale = 0x00,
			.output_range_mode = 0x00
		},
		{
			.output_scale = 0x00,
			.output_range_mode = 0x00
		},
		{
			.output_scale = 0x00,
			.output_range_mode = 0x00
		},
		{
			.output_scale = 0x00,
			.output_range_mode = 0x00
		},
	},

	.external_reference = true,
	.reference_selector = AD5770R_EXT_REF_1_25_V,
	.alarm_config = {
		.open_drain_en = false,
		.thermal_shutdown_en = false,
		.background_crc_en = false,
		.temp_warning_msk = false,
		.over_temp_msk = false,
		.neg_ch0_msk = false,
		.iref_fault_msk = false,
		.background_crc_msk = false
	},
	.output_filter = {
		AD5770R_OUTPUT_FILTER_RESISTOR_60_OHM,
		AD5770R_OUTPUT_FILTER_RESISTOR_60_OHM,
		AD5770R_OUTPUT_FILTER_RESISTOR_60_OHM,
		AD5770R_OUTPUT_FILTER_RESISTOR_60_OHM,
		AD5770R_OUTPUT_FILTER_RESISTOR_60_OHM,
		AD5770R_OUTPUT_FILTER_RESISTOR_60_OHM
	},
	.mon_setup = {
		.monitor_function = AD5770R_CURRENT_MONITORING,
		.mux_buffer = false,
		.ib_ext_en = true,
		.monitor_channel = AD5770R_CH3
	},
	.mask_hw_ldac = {
		.en0 = true,
		.en1 = true,
		.en2 = true,
		.en3 = true,
		.en4 = true,
		.en5 = true,
	},
	.dac_value = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0},
	.page_mask = {
		.dac_data_page_mask = 0x0000,
		.input_page_mask = 0x0000
	},
	.mask_channel_sel = {
		.en0 = true,
		.en1 = true,
		.en2 = true,
		.en3 = true,
		.en4 = true,
		.en5 = true,
	},
	.sw_ldac = {
		.en0 = true,
		.en1 = true,
		.en2 = true,
		.en3 = true,
		.en4 = true,
		.en5 = true,
	},
	.input_value = {0x1, 0x2, 0x3, 0x4, 0x5, 0x6}
};
