/*************************************************************************//**
 *   @file   ad4130_rtd_config.c
 *   @brief  RTD user configurations file for AD4130 device
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
struct ad413x_init_param ad4130_rtd_config_params = {
	.spi_init = &spi_init_params,

	/* Setup (Preset) Configurations */
	.preset = {
		// Setup 0
		{
			.ref_buf = {
				.ref_buf_p_en = true,
				.ref_buf_m_en = true,
			},
			.ref_sel = AD413X_REFIN1,
			.gain = AD413X_GAIN_16,
			.filter = AD4130_FILTER_TYPE,
			.iout0_exc_current = AD413X_EXC_200UA,
#if (ACTIVE_DEMO_MODE_CONFIG == RTD_3WIRE_CONFIG)
			.iout1_exc_current = AD413X_EXC_200UA,
#endif
			.s_time = AD413X_32_MCLK
		},
	},

	/* Chnnel Configurations */
	.ch = {
		// Chn0
		{
			.preset = AD413X_PRESET_0,
			.enable = 1,
			.ain_p = AD413X_AIN2,
			.ain_m = AD413X_AIN3,
			.iout0_exc_input = AD413X_AIN0,
#if (ACTIVE_DEMO_MODE_CONFIG == RTD_3WIRE_CONFIG)
			.iout1_exc_input = AD413X_AIN1,
#endif
		},
	},

	.chip_id = AD4130_8,
	.mclk = AD413X_INT_76_8_KHZ_OUT_OFF,
	.bipolar = true,
	.int_ref = AD413X_INTREF_DISABLED,
	.standby_ctrl = {
		.standby_iexc_en = true
	},
	.data_stat = 0,
	.spi_crc_en = 0
};
