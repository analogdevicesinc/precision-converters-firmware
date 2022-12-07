/*************************************************************************//**
 *   @file   ad4130_user_config.c
 *   @brief  User configuration file for AD4130 device
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

#include "app_config.h"
#include "ad4130_user_config.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* AD4130 device initialization parameters */
struct ad413x_init_param ad4130_user_config_params = {
	.spi_init = &spi_init_params,

	/* Setup (Preset) Configurations */
	.preset = {
		// Setup 0
		{
			.ref_sel = AD413X_REFIN1,
			.gain = AD413X_GAIN_1,
			.filter = AD4130_FILTER_TYPE,
			.s_time = AD413X_32_MCLK
		},
	},

	/* Channel Configurations */
	.ch = {
		// Chn0 (Setup0)
		{
			.preset = AD413X_PRESET_0,
			.enable = 1,
			.ain_p = CHN0_AINP, .ain_m = CHN0_AINM
		},
		// Chn1 (Setup0)
		{
			.preset = AD413X_PRESET_0,
			.enable = 0,
			.ain_p = CHN1_AINP, .ain_m = CHN1_AINM
		},
		// Chn2 (Setup0)
		{
			.preset = AD413X_PRESET_0,
			.enable = 0,
			.ain_p = CHN2_AINP, .ain_m = CHN2_AINM
		},
		// Chn3 (Setup0)
		{
			.preset = AD413X_PRESET_0,
			.enable = 0,
			.ain_p = CHN3_AINP, .ain_m = CHN3_AINM
		},
		// Chn4 (Setup0)
		{
			.preset = AD413X_PRESET_0,
			.enable = 0,
			.ain_p = CHN4_AINP, .ain_m = CHN4_AINM
		},
		// Chn5 (Setup0)
		{
			.preset = AD413X_PRESET_0,
			.enable = 0,
			.ain_p = CHN5_AINP, .ain_m = CHN5_AINM
		},
		// Chn6 (Setup0)
		{
			.preset = AD413X_PRESET_0,
			.enable = 0,
			.ain_p = CHN6_AINP, .ain_m = CHN6_AINM
		},
		// Chn7 (Setup0)
		{
			.preset = AD413X_PRESET_0,
			.enable = 0,
			.ain_p = CHN7_AINP, .ain_m = CHN7_AINM
		},
#if (ADC_USER_CHANNELS > 8)
		// Chn8 (Setup0)
		{
			.preset = AD413X_PRESET_0,
			.enable = 0,
			.ain_p = CHN8_AINP, .ain_m = CHN8_AINM
		},
		// Chn9 (Setup0)
		{
			.preset = AD413X_PRESET_0,
			.enable = 0,
			.ain_p = CHN9_AINP, .ain_m = CHN9_AINM
		},
		// Chn10 (Setup0)
		{
			.preset = AD413X_PRESET_0,
			.enable = 0,
			.ain_p = CHN10_AINP, .ain_m = CHN10_AINM
		},
		// Chn11 (Setup0)
		{
			.preset = AD413X_PRESET_0,
			.enable = 0,
			.ain_p = CHN11_AINP, .ain_m = CHN11_AINM
		},
		// Chn12 (Setup0)
		{
			.preset = AD413X_PRESET_0,
			.enable = 0,
			.ain_p = CHN12_AINP, .ain_m = CHN12_AINM
		},
		// Chn13 (Setup0)
		{
			.preset = AD413X_PRESET_0,
			.enable = 0,
			.ain_p = CHN13_AINP, .ain_m = CHN13_AINM
		},
		// Chn14 (Setup0)
		{
			.preset = AD413X_PRESET_0,
			.enable = 0,
			.ain_p = CHN14_AINP, .ain_m = CHN14_AINM
		},
		// Chn15 (Setup0)
		{
			.preset = AD413X_PRESET_0,
			.enable = 0,
			.ain_p = CHN15_AINP, .ain_m = CHN15_AINM
		},
#endif
	},

	.chip_id = AD4130_8,
	.mclk = AD413X_INT_76_8_KHZ_OUT_OFF,
	.bipolar = true,
	.int_ref = AD413X_INTREF_DISABLED,
	.v_bias = NO_OS_BIT(0) | NO_OS_BIT(1) | NO_OS_BIT(2) | \
	NO_OS_BIT(3) | NO_OS_BIT(4) | NO_OS_BIT(5) | \
	NO_OS_BIT(6) | NO_OS_BIT(7) | NO_OS_BIT(8) | \
	NO_OS_BIT(9) | NO_OS_BIT(10) | NO_OS_BIT(11) | \
	NO_OS_BIT(12) | NO_OS_BIT(13) | NO_OS_BIT(14) | \
	NO_OS_BIT(15),
	.standby_ctrl = {
		.standby_vbias_en = true
	},
	.data_stat = 0,
	.spi_crc_en = 0
};
