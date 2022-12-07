/*************************************************************************//**
 *   @file   ad4130_thermistor_config.c
 *   @brief  Thermistor user configurations file for AD4130 device
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
struct ad413x_init_param ad4130_thermistor_config_params = {
	.spi_init = &spi_init_params,

	/* Setup (Preset) Configurations */
	.preset = {
		// Setup 0
		{
			.ref_buf = {
				.ref_buf_p_en = true,
				.ref_buf_m_en = true,
			},
			.ref_sel = AD413X_REFOUT_AVSS,
			.gain = AD413X_GAIN_1,
			.filter = AD4130_FILTER_TYPE,
			.s_time = AD413X_32_MCLK
		},
	},

	/* Chnnel Configurations */
	.ch = {
		// Chn0
		{
			.preset = AD413X_PRESET_0,
			.enable = 1,
			.ain_p = AD413X_AIN4,
			.ain_m = AD413X_AIN5
		},
	},

	.chip_id = AD4130_8,
	.mclk = AD413X_INT_76_8_KHZ_OUT_OFF,
	.bipolar = true,
	.int_ref = AD413X_INTREF_1_25V,
	.standby_ctrl = {
		.standby_int_ref_en = true,
	},
	.data_stat = 0,
	.spi_crc_en = 0
};
