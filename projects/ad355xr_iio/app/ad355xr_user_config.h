/*************************************************************************//**
 *   @file   ad355xr_user_config.h
 *   @brief  Header for AD355XR user configuration file
******************************************************************************
* Copyright (c) 2023-2024 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD355XR_USER_CONFIG_H_
#define AD355XR_USER_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "ad3552r.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
extern struct no_os_spi_init_param spi_init_params_without_sw_csb;
extern struct ad3552r_init_param ad3552r_init_params;
extern struct no_os_gpio_init_param gpio_ldac_init;

#endif /* AD355XR_USER_CONFIG_H_ */
