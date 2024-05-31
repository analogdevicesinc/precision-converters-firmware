/***************************************************************************//**
*   @file   ad4170_regs.h
*   @brief  Global declarations for ad4170_regs module
********************************************************************************
* Copyright (c) 2021 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef _AD4170_REGS_H_
#define _AD4170_REGS_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

/* Get the AD4170 registers count (actual single + multi-byte entity registers
 * available and defined in ad4170.h file) */
#define ADC_REGISTER_COUNT		(148)

/* Max register address available (last register address defined in ad4170.h file) */
#define MAX_REGISTER_ADDRESS	(AD4170_REG_INPUT_DATA)

extern const uint32_t ad4170_regs[];

#endif /* _AD4170_REGS_H_ */
