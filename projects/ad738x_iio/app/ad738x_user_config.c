/*************************************************************************//**
 *   @file   ad738x_user_config.c
 *   @brief  User configuration file for AD738x devices
******************************************************************************
* Copyright (c) 2022 Analog Devices, Inc.
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
#include "ad738x_user_config.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* Device initialization parameters */
struct ad738x_init_param ad738x_init_params = {
	.spi_param = &spi_init_params,
	.conv_mode = ONE_WIRE_MODE,
	.ref_sel = INT_REF,
};
