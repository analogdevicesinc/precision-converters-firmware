/*************************************************************************//**
 *   @file   ad4692_user_config.h
 *   @brief  Header for AD4692 user configuration file
******************************************************************************
* Copyright (c) 2024 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD4692_USER_CONFIG_H
#define AD4692_USER_CONFIG_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad4692.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

extern struct ad4692_init_param ad4692_init_params;
extern struct no_os_pwm_init_param pwm_init_convst;

#endif /* end of AD4692_USER_CONFIG_H */
