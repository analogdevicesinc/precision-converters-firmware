/*************************************************************************//**
 *   @file   ad4170_user_config.h
 *   @brief  Header for AD4170 default user configurations file
******************************************************************************
* Copyright (c) 2021-22,24 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef _AD4170_USER_CONFIG_H_
#define _AD4170_USER_CONFIG_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <stdlib.h>

#include "app_config.h"
#include "ad4170.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

#define DIFFERENTIAL_CHN_CFG	// comment to select single-ended chn config

/* Select filter type for default user config (same for all channels) */
/* The filter type is chosen depending on the interface mode as it decides
/* output data rate that appears on the DIG_AUX1 pin and the upper limit
/* for the ODR is higher for TDM Mode compared to the SPI.
 * Example -:
 * Sinc5+averaging filter allows FS_CONFIG_VALUE to be configured from
 * 4 (125ksps) to 8330 (60.4sps) in steps of 4,
 * Sinc5 filter allows the same to be configured from 1 (500ksps) to 256 (1953sps) in
 * steps of 2
 * Please refer to the data sheet for more details on digital filters */
#if (INTERFACE_MODE == SPI_INTERRUPT_MODE)
#define AD4170_FILTER_CONFIG			AD4170_FILT_SINC5_AVG
#else // TDM_MODE and SPI_DMA Mode
#define AD4170_FILTER_CONFIG			AD4170_FILT_SINC5
#endif

/* Select FS (or ODR) for default user config (same for all channels) */
#define AD4170_FS_CONFIG				FS_CONFIG_VALUE

/* Scaler factor used in FS value to ODR conversion (for SINC5+Avg filter) */
#define FS_TO_ODR_CONV_SCALER			(32U * AD4170_FS_CONFIG)

/* Select the ADC continuous conversion mode for default config
 * Note: When supplying FIR coefficients, set this macro to 'AD4170_MODE_CONT_FIR' */
#define AD4170_CONT_CONV_MODE_CONFIG	AD4170_MODE_CONT

/* Select the positive and negative analog inputs for each channel */
#if defined(DIFFERENTIAL_CHN_CFG)
/* Differential-ended channel configuration */
#define CHN0_AINP	AD4170_AIN0
#define CHN0_AINM	AD4170_AIN1
#define CHN1_AINP	AD4170_AIN3
#define CHN1_AINM	AD4170_AIN4
#define CHN2_AINP	AD4170_AIN5
#define CHN2_AINM	AD4170_AIN6
#if (DIFFERENTIAL_CHNS > 3)
#define CHN3_AINP	AD4170_AIN7
#define CHN3_AINM	AD4170_AIN8
#endif
#if (DIFFERENTIAL_CHNS > 4)
#define CHN4_AINP	AD4170_AIN8
#define CHN4_AINM	AD4170_AIN9
#define CHN5_AINP	AD4170_AIN10
#define CHN5_AINM	AD4170_AIN11
#define CHN6_AINP	AD4170_AIN12
#define CHN6_AINM	AD4170_AIN13
#define CHN7_AINP	AD4170_AIN14
#define CHN7_AINM	AD4170_AIN15
#endif
#define TOTAL_CHANNELS	DIFFERENTIAL_CHNS
#else
/* Single-ended channel configuration */
#define CHN0_AINP	AD4170_AIN0
#define CHN0_AINM	AD4170_DGND
#define CHN1_AINP	AD4170_AIN1
#define CHN1_AINM	AD4170_DGND
#define CHN2_AINP	AD4170_AIN2
#define CHN2_AINM	AD4170_DGND
#define CHN3_AINP	AD4170_AIN3
#define CHN3_AINM	AD4170_DGND
#define CHN4_AINP	AD4170_AIN4
#define CHN4_AINM	AD4170_DGND
#define CHN5_AINP	AD4170_AIN5
#define CHN5_AINM	AD4170_DGND
#if (SINGLE_ENDED_CHNS > 6)
#define CHN6_AINP	AD4170_AIN6
#define CHN6_AINM	AD4170_DGND
#define CHN7_AINP	AD4170_AIN7
#define CHN7_AINM	AD4170_DGND
#endif
#if (SINGLE_ENDED_CHNS > 8)
#define CHN8_AINP	AD4170_AIN8
#define CHN8_AINM	AD4170_DGND
#define CHN9_AINP	AD4170_AIN9
#define CHN9_AINM	AD4170_DGND
#define CHN10_AINP	AD4170_AIN10
#define CHN10_AINM	AD4170_DGND
#define CHN11_AINP	AD4170_AIN11
#define CHN11_AINM	AD4170_DGND
#define CHN12_AINP	AD4170_AIN12
#define CHN12_AINM	AD4170_DGND
#define CHN13_AINP	AD4170_AIN13
#define CHN13_AINM	AD4170_DGND
#define CHN14_AINP	AD4170_AIN14
#define CHN14_AINM	AD4170_DGND
#define CHN15_AINP	AD4170_AIN15
#define CHN15_AINM	AD4170_DGND
#endif
#define TOTAL_CHANNELS	SINGLE_ENDED_CHNS
#endif

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

extern struct ad4170_init_param ad4170_user_config_params;

#endif /* end of _AD4170_USER_CONFIG_H_ */
