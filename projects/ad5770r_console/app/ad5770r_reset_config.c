/*!
 *****************************************************************************
  @file:  ad5770r_reset_config.c
  @brief: AD5770R initialization configuration
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

#include "no_os_spi.h"
#include "ad5770r.h"

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

const struct ad5770r_dev ad5770r_dev_reset = {
	/* SPI */
	.spi_desc = NULL,

	/* Device SPI Settings */
	.dev_spi_settings = {
		.addr_ascension = false,
		.single_instruction = false,
		.stream_mode_length = 0
	},

	/* Device Settings */
	.channel_config = {
		.en0 = false,
		.en1 = false,
		.en2 = false,
		.en3 = false,
		.en4 = false,
		.en5 = false,
		.sink0 = true
	},

	.output_mode = {
		{
			.output_scale = 0x00,
			.output_range_mode = 0x00
		},
		{
			.output_scale = 0x00,
			.output_range_mode = 0x02
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

	.external_reference = false,
	.reference_selector = AD5770R_EXT_REF_2_5_V,
	.alarm_config = {
		.open_drain_en = false,
		.thermal_shutdown_en = false,
		.background_crc_en = true,
		.temp_warning_msk = false,
		.over_temp_msk = false,
		.neg_ch0_msk = false,
		.iref_fault_msk = false,
		.background_crc_msk = true
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
		.monitor_function = AD5770R_DISABLE,
		.mux_buffer = false,
		.ib_ext_en = false,
		.monitor_channel = AD5770R_CH0
	},
	.mask_hw_ldac = {
		.en0 = false,
		.en1 = false,
		.en2 = false,
		.en3 = false,
		.en4 = false,
		.en5 = false,
	},
	.dac_value = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
	.page_mask = {
		.dac_data_page_mask = 0x0000,
		.input_page_mask = 0x0000
	},
	.mask_channel_sel = {
		.en0 = false,
		.en1 = false,
		.en2 = false,
		.en3 = false,
		.en4 = false,
		.en5 = false,
	},
	.sw_ldac = {
		.en0 = false,
		.en1 = false,
		.en2 = false,
		.en3 = false,
		.en4 = false,
		.en5 = false,
	},
	.input_value = {0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000},
};
