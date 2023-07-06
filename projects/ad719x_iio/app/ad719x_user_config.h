/*************************************************************************//**
 *   @file   ad719x_user_config.h
 *   @brief  Header for AD719X user configuration file
******************************************************************************
* Copyright (c) 2021-22 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/
#ifndef AD719X_USER_CONFIG_H_
#define AD719X_USER_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "ad719x.h"
/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
#if (ACTIVE_MODE == NORMAL_MODE)
#define AD719X_DEFAULT_REF_VOLTAGE     5.0
#define SAMPLING_RATE_HZ               1200
#define DATA_OUTPUT_RATE_BITS          0x001
#define DEFAULT_GAIN                   AD719X_ADC_GAIN_1
#else
#define AD719X_DEFAULT_REF_VOLTAGE     5
#define DEFAULT_GAIN                   AD719X_ADC_GAIN_128
#if (ACTIVE_MODE == NOISE_TEST)
#define SAMPLING_RATE_HZ               4.7
#define DATA_OUTPUT_RATE_BITS          0x3FF
#elif (ACTIVE_MODE == FAST_50HZ_TEST)
#define SAMPLING_RATE_HZ               44
#define DATA_OUTPUT_RATE_BITS          0x06
#endif
#endif

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
extern struct ad719x_init_param ad719x_init_params;
extern struct no_os_gpio_init_param gpio_cs_init;

#endif  /* AD719X_USER_CONFIG_H_ */
