/*************************************************************************//**
 *   @file   ltc268x_user_config.c
 *   @brief  User configuration file for LTC268X device
******************************************************************************
* Copyright (c) 2022 Analog Devices, Inc.
*
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
#include "ltc268x_user_config.h"

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
/* Initialize the ltc268x device structure */
struct ltc268x_init_param ltc268x_dev_init = {
	{
		.max_speed_hz = 44000000,			// Max SPI Speed
		.chip_select = SPI_CSB,				// Chip Select
		.mode = NO_OS_SPI_MODE_3,			// CPOL = 1, CPHA = 1
		.platform_ops = &spi_ops,
		.extra = &spi_extra_init_params		// SPI extra configurations
	},
	.pwd_dac_setting = 0x0000,
	.dither_toggle_en = false,
#if defined(DEV_LTC2688)
	.dev_id = LTC2688,
#else
	.dev_id = LTC2686,
#endif
	.dither_mode = {
		false, false, false, false,
		false, false, false, false,
		false, false, false, false,
		false, false, false, false,
	},
	.crt_range = {
		LTC268X_VOLTAGE_RANGE_0V_5V, LTC268X_VOLTAGE_RANGE_0V_5V,
		LTC268X_VOLTAGE_RANGE_0V_5V, LTC268X_VOLTAGE_RANGE_0V_5V,
		LTC268X_VOLTAGE_RANGE_0V_5V, LTC268X_VOLTAGE_RANGE_0V_5V,
		LTC268X_VOLTAGE_RANGE_0V_5V, LTC268X_VOLTAGE_RANGE_0V_5V,
		LTC268X_VOLTAGE_RANGE_0V_5V, LTC268X_VOLTAGE_RANGE_0V_5V,
		LTC268X_VOLTAGE_RANGE_0V_5V, LTC268X_VOLTAGE_RANGE_0V_5V,
		LTC268X_VOLTAGE_RANGE_0V_5V, LTC268X_VOLTAGE_RANGE_0V_5V
	},
	.dither_phase = {
		LTC268X_DITH_PHASE_0, LTC268X_DITH_PHASE_0,
		LTC268X_DITH_PHASE_0, LTC268X_DITH_PHASE_0,
		LTC268X_DITH_PHASE_0, LTC268X_DITH_PHASE_0,
		LTC268X_DITH_PHASE_0, LTC268X_DITH_PHASE_0,
		LTC268X_DITH_PHASE_0, LTC268X_DITH_PHASE_0,
		LTC268X_DITH_PHASE_0, LTC268X_DITH_PHASE_0,
		LTC268X_DITH_PHASE_0, LTC268X_DITH_PHASE_0,
		LTC268X_DITH_PHASE_0, LTC268X_DITH_PHASE_0
	},
	.dither_period = {
		LTC268X_DITH_PERIOD_4, LTC268X_DITH_PERIOD_4,
		LTC268X_DITH_PERIOD_4, LTC268X_DITH_PERIOD_4,
		LTC268X_DITH_PERIOD_4, LTC268X_DITH_PERIOD_4,
		LTC268X_DITH_PERIOD_4, LTC268X_DITH_PERIOD_4,
		LTC268X_DITH_PERIOD_4, LTC268X_DITH_PERIOD_4,
		LTC268X_DITH_PERIOD_4, LTC268X_DITH_PERIOD_4,
		LTC268X_DITH_PERIOD_4, LTC268X_DITH_PERIOD_4,
		LTC268X_DITH_PERIOD_4, LTC268X_DITH_PERIOD_4
	},
	.clk_input = {
		LTC268X_SOFT_TGL, LTC268X_SOFT_TGL,
		LTC268X_SOFT_TGL, LTC268X_SOFT_TGL,
		LTC268X_SOFT_TGL, LTC268X_SOFT_TGL,
		LTC268X_SOFT_TGL, LTC268X_SOFT_TGL,
		LTC268X_SOFT_TGL, LTC268X_SOFT_TGL,
		LTC268X_SOFT_TGL, LTC268X_SOFT_TGL,
		LTC268X_SOFT_TGL, LTC268X_SOFT_TGL,
		LTC268X_SOFT_TGL, LTC268X_SOFT_TGL
	},
	.reg_select = {
		LTC268X_SELECT_A_REG, LTC268X_SELECT_A_REG,
		LTC268X_SELECT_A_REG, LTC268X_SELECT_A_REG,
		LTC268X_SELECT_A_REG, LTC268X_SELECT_A_REG,
		LTC268X_SELECT_A_REG, LTC268X_SELECT_A_REG,
		LTC268X_SELECT_A_REG, LTC268X_SELECT_A_REG,
		LTC268X_SELECT_A_REG, LTC268X_SELECT_A_REG,
		LTC268X_SELECT_A_REG, LTC268X_SELECT_A_REG,
		LTC268X_SELECT_A_REG, LTC268X_SELECT_A_REG
	},
};
