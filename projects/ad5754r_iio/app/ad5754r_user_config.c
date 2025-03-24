/*************************************************************************//**
 *   @file   ad5754r_user_config.c
 *   @brief  User configuration file for AD5754R devices
******************************************************************************
* Copyright (c) 2024, 2025 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad5754r_user_config.h"
#include "app_config.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* SPI init parameters */
struct no_os_spi_init_param spi_init_params = {
	.device_id = SPI_DEVICE_ID,
	.max_speed_hz = MAX_SPI_CLK,
	.mode = NO_OS_SPI_MODE_2,
	.chip_select = SPI_CSB,
	.platform_ops = &spi_ops,
	.extra = &spi_extra_init_params
};

/* Device initialization parameters */
struct ad5754r_init_param ad5754r_init_params = {
	.spi_init = &spi_init_params,
	.gpio_clear_init = &clear_gpio_params,
	.gpio_ldac_init = &ldac_gpio_params,
	.clamp_en = AD5754R_CTRL_CLAMP_DIS,
	.tsd_en = AD5754R_CTRL_TSD_DIS,
	.clear_sel = AD5754R_CTRL_CLEAR_MIDSCALE_CODE,
	.sdo_dis = AD5754R_CTRL_SDO_EN,
	.dac_ch_pwr_states = {
		AD5754R_PWR_DAC_CH_POWERDOWN,
		AD5754R_PWR_DAC_CH_POWERDOWN,
		AD5754R_PWR_DAC_CH_POWERDOWN,
		AD5754R_PWR_DAC_CH_POWERDOWN,
	},
	.dac_ch_range = {
		AD5754R_SPAN_0V_TO_5V,
		AD5754R_SPAN_0V_TO_5V,
		AD5754R_SPAN_0V_TO_5V,
		AD5754R_SPAN_0V_TO_5V
	},
	.int_ref_pwrup = AD5754R_PWR_INT_REF_POWERDOWN,
#if defined(USE_BINARY_CODING)
	.encoding = AD5754R_ENCODING_BINARY,
#else
	.encoding = AD5754R_ENCODING_TWOSCOMPLEMENT,
#endif
	.vref_mv = AD5754R_VREF * 1000
};