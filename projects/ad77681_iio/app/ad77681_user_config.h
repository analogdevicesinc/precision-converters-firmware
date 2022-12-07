/*************************************************************************//**
 *   @file   ad77861_user_config.h
 *   @brief  Header for AD7768-1 user configuration file
******************************************************************************
* Copyright (c) 2021 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef _AD77681_USER_CONFIG_H_
#define _AD77681_USER_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "ad77681.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* AD77681 Voltage reference */
#define AD77681_VOLTAGE_REF				(4096)

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

extern struct ad77681_init_param sad77681_init;

#endif //_AD77681_USER_CONFIG_H_
