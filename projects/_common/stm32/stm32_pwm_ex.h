/***************************************************************************//**
 * @file    stm32_pwm_ex.h
 * @brief   Extended support for STM32 PWM peripheral.
********************************************************************************
* Copyright (c) 2026 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef _STM32_PWM_EX_H_
#define _STM32_PWM_EX_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "stm32_pwm.h"

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
int32_t compute_optimal_prescaler(struct stm32_pwm_desc *desc,
				  uint64_t period_ns,
				  uint32_t *prescaler);

#endif // _STM32_PWM_EX_H_
