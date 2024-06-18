/**************************************************************************//**
 * @file    ad7124_user_config.c
 * @brief   User Configurations for AD7124
 * @details This module contains the configurations needed for IIO application
*******************************************************************************
* Copyright (c) 2023-24 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
******************************************************************************/

/*****************************************************************************/
/***************************** Include Files *********************************/
/*****************************************************************************/

#include "ad7124_user_config.h"
#include "ad7124_regs.h"
#include "app_config.h"

/*****************************************************************************/
/********************* Macros and Constants Definition ***********************/
/*****************************************************************************/

/*****************************************************************************/
/******************** Variables and User Defined Data Types ******************/
/*****************************************************************************/

struct no_os_spi_init_param spi_init_params = {
	.max_speed_hz = 5000000,
	.mode = NO_OS_SPI_MODE_3,
	.chip_select = SPI_CSB,
	.device_id = SPI_DEVICE_ID,
	.platform_ops = &spi_platform_ops,
	.extra = &spi_extra_init_params
};

/* AD7124- Init Parameters */
struct ad7124_init_param ad7124_init_params = {
	.spi_init = &spi_init_params,
	.regs = ad7124_regs,
	.spi_rdy_poll_cnt = 10000,
	.ref_en = false,
	.mode = AD7124_CONTINUOUS,
	.power_mode = AD7124_HIGH_POWER,
#if defined(DEV_AD7124_8)
	.active_device = ID_AD7124_8
#else
	.active_device = ID_AD7124_4
#endif
	,
	.setups = {
		{ .bi_unipolar = true, .ref_buff = false, .ain_buff = true, .ref_source = EXTERNAL_REFIN1 },
		{ .bi_unipolar = true, .ref_buff = false, .ain_buff = true, .ref_source = EXTERNAL_REFIN1 },
		{ .bi_unipolar = true, .ref_buff = false, .ain_buff = true, .ref_source = EXTERNAL_REFIN1 },
		{ .bi_unipolar = true, .ref_buff = false, .ain_buff = true, .ref_source = EXTERNAL_REFIN1},
		{ .bi_unipolar = true, .ref_buff = false, .ain_buff = true, .ref_source = EXTERNAL_REFIN1 },
		{ .bi_unipolar = true, .ref_buff = false, .ain_buff = true, .ref_source = EXTERNAL_REFIN1 },
		{ .bi_unipolar = true, .ref_buff = false, .ain_buff = true, .ref_source = EXTERNAL_REFIN1 },
		{ .bi_unipolar = true, .ref_buff = false, .ain_buff = true, .ref_source = EXTERNAL_REFIN1 },
	},
	.chan_map = {
		{ .channel_enable = true, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 }
	}
};
