/*************************************************************************//**
 *   @file   ad4170_loadcell_config.h
 *   @brief  Header for AD4170 Loadcell configurations module
******************************************************************************
* Copyright (c) 2021-22,25 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef _AD4170_LOADCELL_CONFIG_H_
#define _AD4170_LOADCELL_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <stdlib.h>
#include "ad4170.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Select the excitation type for load cell (AC/DC) - one at a time
 * Note- AC excitation is not applicable in case of AD4190 */
#define LOADCELL_DC_EXCITATION
//#define LOADCELL_AC_EXCITATION

/* Select between 4/6 wire loadcell - one at a time */
#define FOUR_WIRE_LOAD_CELL
//#define SIX_WIRE_LOAD_CELL

/* Select filter type for loadcell config (same for all channels) */
#define AD4170_FILTER_CONFIG			AD4170_FILT_SINC3

/* Select FS (or ODR) for loadcell config (for SINC3 filter) */
#define AD4170_FS_CONFIG				625		// ODR = 50SPS

/* Scaler factor used in FS value to ODR conversion (for SINC3 filter) */
#define FS_TO_ODR_CONV_SCALER			(512U * AD4170_FS_CONFIG)

/* Select continuous conversion mode for loadcell config */
#define AD4170_CONT_CONV_MODE_CONFIG	AD4170_MODE_CONT

#if defined (FOUR_WIRE_LOAD_CELL)
#define TOTAL_CHANNELS                  4
#else // SIX_WIRE_LOAD_CELL
#define TOTAL_CHANNELS                  2
#endif

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

extern struct ad4170_init_param ad4170_loadcell_config_params;

#endif /* end of _AD4170_LOADCELL_CONFIG_H_ */
