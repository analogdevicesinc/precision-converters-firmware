/***************************************************************************//**
 *   @file    ad579x_support.h
 *   @brief   AD579x No-OS driver support header file
********************************************************************************
 * Copyright (c) 2023 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef AD579X_SUPPORT_H
#define AD579X_SUPPORT_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "ad5791.h"

/******************************************************************************/
/********************* Macros and Constants Definitions ***********************/
/******************************************************************************/

/* Mask for register address bits in the register */
#define  AD579X_ADDRESS_MASK    NO_OS_GENMASK(23, 20)

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/
int ad579x_reconfig_ldac(struct ad5791_dev *device);

#endif /* AD579X_SUPPORT_H_ */
