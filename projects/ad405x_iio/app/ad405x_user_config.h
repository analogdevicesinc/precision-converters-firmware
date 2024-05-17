/*************************************************************************//**
 *   @file   ad405x_user_config.h
 *   @brief  Header for AD405X user configuration file
******************************************************************************
* Copyright (c) 2023-24 Analog Devices, Inc.
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

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
extern struct no_os_gpio_init_param gpio_reset_param;
extern struct ad405x_init_param ad405x_init_params;
extern struct no_os_gpio_init_param gpio_gpio1_param;

#endif /* AD405X_USER_CONFIG_H */
