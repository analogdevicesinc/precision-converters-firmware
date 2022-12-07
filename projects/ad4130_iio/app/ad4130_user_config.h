/*************************************************************************//**
 *   @file   ad4130_user_config.h
 *   @brief  Header for AD4130 user configuration file
******************************************************************************
* Copyright (c) 2020, 2022 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef _AD4130_USER_CONFIG_H_
#define _AD4130_USER_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "ad413x.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Select channel config for default user config (applicable to all channels) */
//#define DIFFERENTIAL_CHN_CFG	// Uncomment to select differential config

/* Select FS (or ODR) for default user config (applicable to all channels) */
#if (FS_CONFIG_VALUE != 0)
#define AD4130_FS_CONFIG				FS_CONFIG_VALUE
#else
#define AD4130_FS_CONFIG				1	// ODR = 2.4KSPS (max)
#endif

/* Filter type for default user config
 * Note: Applicable for all setups to keep the same ODR for all channels */
#define AD4130_FILTER_TYPE				AD413X_SYNC3_STANDALONE

/* Scaler factor used in FS to ODR conversion (For SINC3/4 filter) */
#define FS_TO_ODR_CONV_SCALER			(32U * AD4130_FS_CONFIG)

/* Select the positive and negative analog inputs for each channel */
#if defined(DIFFERENTIAL_CHN_CFG)
#define CHN0_AINP	AD413X_AIN0
#define CHN0_AINM	AD413X_AIN1
#define CHN1_AINP	AD413X_AIN2
#define CHN1_AINM	AD413X_AIN3
#define CHN2_AINP	AD413X_AIN4
#define CHN2_AINM	AD413X_AIN5
#define CHN3_AINP	AD413X_AIN6
#define CHN3_AINM	AD413X_AIN7
#define CHN4_AINP	AD413X_AIN8
#define CHN4_AINM	AD413X_AIN9
#define CHN5_AINP	AD413X_AIN10
#define CHN5_AINM	AD413X_AIN11
#define CHN6_AINP	AD413X_AIN12
#define CHN6_AINM	AD413X_AIN13
#define CHN7_AINP	AD413X_AIN14
#define CHN7_AINM	AD413X_AIN15
#define ADC_USER_CHANNELS	ADC_DIFFERENTIAL_CHNS
#else
#define CHN0_AINP	AD413X_AIN0
#define CHN0_AINM	AD413X_AVSS
#define CHN1_AINP	AD413X_AIN1
#define CHN1_AINM	AD413X_AVSS
#define CHN2_AINP	AD413X_AIN2
#define CHN2_AINM	AD413X_AVSS
#define CHN3_AINP	AD413X_AIN3
#define CHN3_AINM	AD413X_AVSS
#define CHN4_AINP	AD413X_AIN4
#define CHN4_AINM	AD413X_AVSS
#define CHN5_AINP	AD413X_AIN5
#define CHN5_AINM	AD413X_AVSS
#define CHN6_AINP	AD413X_AIN6
#define CHN6_AINM	AD413X_AVSS
#define CHN7_AINP	AD413X_AIN7
#define CHN7_AINM	AD413X_AVSS
#define CHN8_AINP	AD413X_AIN8
#define CHN8_AINM	AD413X_AVSS
#define CHN9_AINP	AD413X_AIN9
#define CHN9_AINM	AD413X_AVSS
#define CHN10_AINP	AD413X_AIN10
#define CHN10_AINM	AD413X_AVSS
#define CHN11_AINP	AD413X_AIN11
#define CHN11_AINM	AD413X_AVSS
#define CHN12_AINP	AD413X_AIN12
#define CHN12_AINM	AD413X_AVSS
#define CHN13_AINP	AD413X_AIN13
#define CHN13_AINM	AD413X_AVSS
#define CHN14_AINP	AD413X_AIN14
#define CHN14_AINM	AD413X_AVSS
#define CHN15_AINP	AD413X_AIN15
#define CHN15_AINM	AD413X_AVSS
#define ADC_USER_CHANNELS	ADC_PSEUDO_DIFF_CHNS
#endif

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct ad413x_init_param ad4130_user_config_params;

#endif /* end of _AD4130_USER_CONFIG_H_ */
