/*************************************************************************//**
 *   @file   ad717x_user_config.h
 *   @brief  User configuration file for AD717x-AD411x IIO firmware application
******************************************************************************
* Copyright (c) 2021-22, 2026 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD717x_USER_CONFIG_H_
#define AD717x_USER_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include "ad717x.h"
#include "app_config.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
/* Per-device register map info */
struct ad717x_device_map_info {
	const char *name;
	ad717x_st_reg *regs;
	uint8_t num_regs;
	uint8_t num_channels;
	uint8_t num_setups;
	bool use_input_pairs; /* true for AD411x family, false for AD717x family */
	float scale_factor; /* Scale factor denominator (0.1 for AD411x, 1.0 for AD717x) */
	uint8_t resolution; /* ADC resolution in bits */
	bool supports_open_wire; /* true for devices with open wire detection (AD4111, AD4113) */
};

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

extern ad717x_init_param ad717x_init_params;
extern struct ad717x_device_map_info device_map_table[];
extern struct ad717x_channel_setup default_setups[AD717x_MAX_SETUPS];
extern struct ad717x_channel_map default_ad411x_chan_maps[AD717x_MAX_CHANNELS];
extern struct ad717x_channel_map default_ad717x_chan_maps[AD717x_MAX_CHANNELS];
extern struct ad717x_filtcon default_ad717x_filtcons[AD717x_MAX_SETUPS];

#endif // AD717x_USER_CONFIG_H_
