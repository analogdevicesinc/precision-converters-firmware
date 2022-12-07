/*************************************************************************//**
 *   @file   ad4130_power_test_config.c
 *   @brief  Power test user configurations file for AD4130 device
******************************************************************************
* Copyright (c) 2022 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "app_config.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* AD4130 device initialization parameters */
struct ad413x_init_param ad4130_power_test_config_params = {
	.spi_init = &spi_init_params,

	/* Setup (Preset) Configurations */
	.preset = {
		// Setup 0
		{
			.ref_buf = {
				.ref_buf_p_en = true,
				.ref_buf_m_en = true,
			},
			.ref_sel = AD413X_AVDD_AVSS,
			.gain = AD413X_GAIN_1,
			.filter = AD4130_FILTER_TYPE,
			.s_time = AD413X_32_MCLK
		},
	},

	/* Chnnel Configurations */
	.ch = {
		// Chn0 (V_AVDD)
		{
			.preset = AD413X_PRESET_0,
			.enable = 1,
			.ain_p = AD413X_AVDD_AVSS_6P,
			.ain_m = AD413X_AVDD_AVSS_6M
		},
		// Chn1 (V_IOVDD)
		{
			.preset = AD413X_PRESET_0,
			.enable = 1,
			.ain_p = AD413X_IOVDD_DGND_6P,
			.ain_m = AD413X_IOVDD_DGND_6M
		},
		// Chn2 (I_AVDD)
		{
			.preset = AD413X_PRESET_0,
			.enable = 1,
			.ain_p = AD413X_AIN12,
			.ain_m = AD413X_AIN13
		},
		// Chn3 (I_IOVDD)
		{
			.preset = AD413X_PRESET_0,
			.enable = 1,
			.ain_p = AD413X_AIN10,
			.ain_m = AD413X_AIN11
		},
		// Chn4 (V_AVSS-DGND)
		{
			.preset = AD413X_PRESET_0,
			.enable = 1,
			.ain_p = AD413X_AVSS,
			.ain_m = AD413X_DGND
		},
		// Chn5 (V_REF)
		{
			.preset = AD413X_PRESET_0,
			.enable = 1,
			.ain_p = AD413X_AIN14,
			.ain_m = AD413X_AIN15
		},
	},

	.chip_id = AD4130_8,
	.mclk = AD413X_INT_76_8_KHZ_OUT_OFF,
	.bipolar = true,
	.int_ref = AD413X_INTREF_DISABLED,
	.v_bias = 0,
	.data_stat = 0,
	.spi_crc_en = 0
};
