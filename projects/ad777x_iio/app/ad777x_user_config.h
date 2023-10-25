/*************************************************************************//**
 *   @file   ad777x_user_config.h
 *   @brief  Header for AD777x default user configurations file
******************************************************************************
* Copyright (c) 2022 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef _AD777x_USER_CONFIG_H_
#define _AD777x_USER_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "app_config.h"
#include "ad7779.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Available Power Modes */
#define AD777x_LOW_POWER	0
#define AD777x_HIGH_RES		1

/* Select the desired power mode */
#define AD777x_POWER_MODE	AD777x_HIGH_RES

/* MCLK Divisor */
#if (AD777x_POWER_MODE == AD777x_HIGH_RES)
#define AD777x_MCLK_DIV					4
#else
#define AD777x_MCLK_DIV					8
#endif

/* Calculate the decimation factor */
#define AD777x_DEC_RATE_INT				(int) (AD777x_MCLK_FREQ/AD777x_MCLK_DIV/AD777x_SAMPLING_FREQUENCY)
#define AD777x_DEC_RATE_DEC				0

/* Gain correction factor. This default value represents a gain of 1 */
#define AD777x_GAIN_CORR				0x555555

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

extern ad7779_init_param ad777x_init_params;

#endif /* end of _AD777x_USER_CONFIG_H_ */
