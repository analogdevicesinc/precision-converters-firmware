/*************************************************************************//**
 *   @file   ad4170_rtd_config.h
 *   @brief  Header for AD4170 RTD configurations module
******************************************************************************
* Copyright (c) 2021-22 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef _AD4170_RTD_CONFIG_H_
#define _AD4170_RTD_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include "ad4170.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Select filter type for RTD config (same for all channels) */
#define AD4170_FILTER_CONFIG			AD4170_FILT_SINC3

/* Select FS (or ODR) for RTD config (for SINC3 filter) */
#define AD4170_FS_CONFIG				625		// ODR = 50SPS

/* Scaler factor used in FS value to ODR conversion (for SINC3 filter) */
#define FS_TO_ODR_CONV_SCALER			(512U * AD4170_FS_CONFIG)

/* Select continuous conversion mode for RTD config */
#define AD4170_CONT_CONV_MODE_CONFIG	AD4170_MODE_CONT

#define TOTAL_CHANNELS                  3

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

extern struct ad4170_init_param ad4170_rtd_config_params;

#endif /* end of _AD4170_RTD_CONFIG_H_ */
