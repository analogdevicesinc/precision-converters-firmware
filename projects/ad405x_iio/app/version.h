/*************************************************************************//**
 *   @file   version.h
 *   @brief  Version macros for ADI AD405x
******************************************************************************
* Copyright (c) 2025 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef VERSION_H
#define VERSION_H

#include "adi_version.h"

/******************************************************************************/
/* Define firmware_version string */
#define MAJOR_VERSION		1
#define MINOR_VERSION		1
#define PATCH_VERSION		0
#define QUALITY_LEVEL          QUALITY_LEVEL_RC
#define STATE_VERSION		0

/* Create firmware_version string */
#define FIRMWARE_VERSION ADI_CONSTRUCT_VERSION(MAJOR_VERSION, \
                                               MINOR_VERSION, \
                                               PATCH_VERSION, \
                                               QUALITY_LEVEL, \
                                               STATE_VERSION)

#endif //VERSION_H