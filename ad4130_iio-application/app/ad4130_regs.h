/***************************************************************************//**
*   @file   ad4130_regs.h
*   @brief  Global declarations for ad4130_regs module
********************************************************************************
* Copyright (c) 2022 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef _AD4130_REGS_H_
#define _AD4130_REGS_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

#define AD413X_ADDR(x)		((x) & 0xFF)

/* Max register address available (last register address defined
 * in the ad413x.h file) */
#define MAX_REGISTER_ADDRESS	(AD413X_ADDR(AD413X_REG_FIFO_DATA))

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

extern const uint32_t ad413x_regs[];

#endif /* _AD4130_REGS_H_ */
