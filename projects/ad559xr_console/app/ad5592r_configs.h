/*!
 *****************************************************************************
  @file:  ad5592r_configs.h
  @brief: user config header for ad5592r
  @details:
 -----------------------------------------------------------------------------
 Copyright (c) 2020, 2022 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
*****************************************************************************/


#ifndef AD5592R_INIT_CONFIG_H
#define AD5592R_INIT_CONFIG_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "ad5592r-base.h"

/******************************************************************************/
/*****************************  Public Declarations  *******************************/
/******************************************************************************/
extern struct ad5592r_init_param ad5592r_user_param;
extern struct no_os_i2c_init_params i2c_user_params;
extern struct no_os_spi_init_params spi_user_params;
extern struct no_os_i2c_desc i2c_user_descr;
extern struct no_os_spi_desc spi_user_descr;
extern struct ad5592r_dev ad5592r_dev_reset;
extern struct ad5592r_dev ad5592r_dev_user;

#endif /*AD5592R_INIT_CONFIG_H*/
