/***************************************************************************//**
 *   @file    ad717x_system_config.h
 *   @brief   Header file for AD717x System Configuration
 ********************************************************************************
 * Copyright (c) 2026 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
 ******************************************************************************/
#ifndef AD717X_SYSTEM_CONFIG_H_
#define AD717X_SYSTEM_CONFIG_H_

#include "iio.h"
#include "ad717x.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

/* Initialize the IIO device descriptor for AD717x system configuration */
int32_t iio_ad717x_system_config_init(struct iio_device **desc,
				      ad717x_dev *device);

/* Check if the AD717x system reconfigure is requested */
bool iio_ad717x_is_system_reconfigured(void);

/* Remove the AD717x system configuration device */
int32_t iio_ad717x_system_config_remove(struct iio_device *desc);

/* Get the effective sampling frequency with all channels enabled */
int32_t ad717x_get_sampling_frequency(ad717x_dev *device,
				      float *sampling_freq);

#endif // AD717X_SYSTEM_CONFIG_H_
