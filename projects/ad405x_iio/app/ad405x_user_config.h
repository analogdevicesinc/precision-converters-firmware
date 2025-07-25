/*************************************************************************//**
 *   @file   ad405x_user_config.h
 *   @brief  Header for AD405X user configuration file
******************************************************************************
* Copyright (c) 2023-2024 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD405X_USER_CONFIG_H
#define AD405X_USER_CONFIG_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "ad405x.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

// Set the PID Instance ID to match the pins ADDR[2:0] (valid range: 0-7) (I3C only)
#define AD405X_INSTANCE_ID						0
#define AD405X_I3C_GEN_DYN_ADDR_DEFAULT			0x32
#define AD405X_I3C_GEN_PID_(x)					(0x02ee00700000 | ((x & 0xF) << 16))
#define AD405X_I3C_GEN_INSTANCE_ID(x)			(((uint64_t)x & 0x7) << 12)
#define AD405X_I3C_GEN_PID(dev, x)				(AD405X_I3C_GEN_PID_(dev) | AD405X_I3C_GEN_INSTANCE_ID(x))

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
extern struct ad405x_init_param ad405x_init_params;

#endif /* AD405X_USER_CONFIG_H */
