/*************************************************************************//**
 *   @file   app_config.h
 *   @brief  Configuration file for LTC268X device applications
******************************************************************************
* Copyright (c) 2022 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/
#ifndef APP_CONFIG_H
#define APP_CONFIG_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* List of supported platforms*/
#define	MBED_PLATFORM		1

/* Select the active platform */
#define ACTIVE_PLATFORM		MBED_PLATFORM

#if (ACTIVE_PLATFORM == MBED_PLATFORM)
#include "app_config_mbed.h"

/* Redefine the init params structure mapping w.r.t. platform */
#define spi_extra_init_params mbed_spi_extra_init_params
#define spi_ops mbed_spi_ops
#else
#error "No/Invalid active platform selected"
#endif

/* Supported LTC268x devices (One selected at a time, default is LTC2688) */
#define		DEV_LTC2688

#if defined(DEV_LTC2688)
#define ACTIVE_DEVICE	"LTC2688"
#elif defined(DEV_LTC2686)
#define ACTIVE_DEVICE	"LTC2686"
#else
#define ACTIVE_DEVICE	"LTC2688"
#warning "No active device selected. LTC2688 is assumed as default"
#endif

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

#endif //APP_CONFIG_H
