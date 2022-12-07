/*****************************************************************************
 *   @file    ad77681_user_config.h
 *
 *   @brief   User configuration settings for AD77681 on start-up
 *
 *   @details
******************************************************************************
Copyright (c) 2021-22 Analog Devices, Inc. All Rights Reserved.

This software is proprietary to Analog Devices, Inc. and its licensors.
By using this software you agree to the terms of the associated
Analog Devices Software License Agreement.
******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>

#include "app_config.h"
#include "app_config_mbed.h"
#include "ad77681_user_config.h"
#include "no_os_spi.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

// Initialize the ad77681 device structure
struct ad77681_init_param sad77681_init = {
	// Define SPI init parameters structure
	.spi_eng_dev_init =
	{
		.max_speed_hz = 22500000,		// Max SPI Speed
		.chip_select = SPI_CSB,			// Chip Select
		.mode = NO_OS_SPI_MODE_3,		// CPOL = 1, CPHA = 1
		.extra = &spi_extra_init_params,// SPI extra configurations
		.platform_ops = &spi_ops,
	},
	.power_mode = AD77681_FAST,
	.mclk_div = AD77681_MCLK_DIV_8,
	.conv_mode = AD77681_CONV_CONTINUOUS,
	.diag_mux_sel = AD77681_TEMP_SENSOR,
	.conv_diag_sel = false,
	.conv_len = AD77681_CONV_24BIT,
	.crc_sel = AD77681_NO_CRC,
	.status_bit = 0,
	.VCM_out = AD77681_VCM_0_9V,
	.AINn = AD77681_AINn_DISABLED,
	.AINp = AD77681_AINp_DISABLED,
	.REFn = AD77681_BUFn_ENABLED,
	.REFp = AD77681_BUFp_ENABLED,
	.filter = AD77681_SINC5,
	.decimate = AD77681_SINC5_FIR_DECx32,
	.sinc3_osr = 0,
	.vref = AD77681_VOLTAGE_REF,
	.mclk = AD77681_MCLK,
	.sample_rate = AD77681_DEFAULT_SAMPLING_FREQ,
	.data_frame_byte = AD77681_DECIMATION_RATE
};
