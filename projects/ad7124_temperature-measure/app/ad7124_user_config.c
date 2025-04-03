/*************************************************************************//**
 *   @file   ad7124_user_config.c
 *   @brief  User configuration file for AD7124 device
******************************************************************************
* Copyright (c) 2021-22, 2025 Analog Devices, Inc.
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

#include <stdint.h>

#include "app_config.h"
#include "ad7124_user_config.h"
#include "no_os_spi.h"
#include "no_os_uart.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* Used to create the ad7124 device */
struct  ad7124_init_param ad7124_user_init_params = {
	.spi_init = &spi_init_params,  // spi_init_param type
	.regs = ad7124_regs,
	.spi_rdy_poll_cnt = 10000, // count for polling RDY
	.power_mode = AD7124_HIGH_POWER,
	.ref_en = true,
#if defined(DEV_AD7124_4)
	.active_device = ID_AD7124_4,
#else
	.active_device = ID_AD7124_8,
#endif
	.setups = {
		{ .bi_unipolar = true, .ref_buff = true, .ain_buff = true, .ref_source =  INTERNAL_REF },
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
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN2, .ain.ainm = AD7124_AIN3 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN4, .ain.ainm = AD7124_AIN5 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN4, .ain.ainm = AD7124_AIN5 },
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
