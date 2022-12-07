/*************************************************************************//**
 *   @file   ad4130_loadcell_config.h
 *   @brief  Header for AD4130 loadcell user configuration file
******************************************************************************
* Copyright (c) 2022 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef _AD4130_LOADCELL_CONFIG_H_
#define _AD4130_LOADCELL_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "ad413x.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Select FS (or ODR) for loadcell config (applicable to all channels) */
#define AD4130_FS_CONFIG				48	// ODR = 50SPS for SINC3/4 filter

/* Filter type for default loadcell config
 * Note: Applicable for all setups to keep the same ODR for all channels */
#define AD4130_FILTER_TYPE				AD413X_SYNC3_STANDALONE

/* Scaler factor used in FS to ODR conversion (For SINC3/4 filter) */
#define FS_TO_ODR_CONV_SCALER			(32U * AD4130_FS_CONFIG)

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct ad413x_init_param ad4130_loadcell_config_params;

#endif /* end of _AD4130_LOADCELL_CONFIG_H_ */
