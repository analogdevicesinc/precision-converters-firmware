/*************************************************************************//**
 *   @file   ad7124_user_config.h
 *   @brief  Header for AD7124 user configuration file
******************************************************************************
* Copyright (c) 2021 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef _AD7124_USER_CONFIG_H
#define _AD7124_USER_CONFIG_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>

#include "ad7124.h"
#include "ad7124_regs.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct ad7124_init_param ad7124_init_params;

#endif /* end of _AD7124_USER_CONFIG_H */
