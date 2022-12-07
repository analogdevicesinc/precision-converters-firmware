/*************************************************************************//**
 *   @file   ad4130_power_test_config.h
 *   @brief  Header for AD4130 power test user configuration file
******************************************************************************
* Copyright (c) 2022 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef _AD4130_POWER_TEST_CONFIG_H_
#define _AD4130_POWER_TEST_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "ad413x.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Select FS (or ODR) for power test config (applicable to all channels) */
#define AD4130_FS_CONFIG				4	// ODR = 600SPS for SINC3/4 filter

/* Filter type for power test config
 * Note: Applicable for all setups to keep the same ODR for all channels */
#define AD4130_FILTER_TYPE				AD413X_SYNC3_STANDALONE

/* Scaler factor used in FS to ODR conversion (For SINC3/4 filter) */
#define FS_TO_ODR_CONV_SCALER			(32U * AD4130_FS_CONFIG)

/* Power test channels list */
#define POWER_TEST_V_AVDD_CHN		0
#define POWER_TEST_V_IOVDD_CHN		1
#define POWER_TEST_I_AVDD_CHN		2
#define POWER_TEST_I_IOVDD_CHN		3
#define POWER_TEST_V_AVSS_DGND_CHN	4
#define POWER_TEST_V_REF_CHN		5

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct ad413x_init_param ad4130_power_test_config_params;

#endif /* end of _AD4130_POWER_TEST_CONFIG_H_ */
