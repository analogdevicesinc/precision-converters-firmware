/***************************************************************************//**
 *   @file    ad4130_regs.c
 *   @brief   AD4130 registers map
********************************************************************************
 * Copyright (c) 2022 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>

#include "ad413x.h"
#include "ad4130_regs.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

const uint32_t ad413x_regs[] = {
	AD413X_REG_STATUS,
	AD413X_REG_ADC_CTRL,
	AD413X_REG_DATA,
	AD413X_REG_IO_CTRL,
	AD413X_REG_VBIAS_CTRL,
	AD413X_REG_ID,
	AD413X_REG_ERROR,
	AD413X_REG_ERROR_EN,
	AD413X_REG_MCLK_CNT,
	AD413X_REG_CHN(0),
	AD413X_REG_CHN(1),
	AD413X_REG_CHN(2),
	AD413X_REG_CHN(3),
	AD413X_REG_CHN(4),
	AD413X_REG_CHN(5),
	AD413X_REG_CHN(6),
	AD413X_REG_CHN(7),
	AD413X_REG_CHN(8),
	AD413X_REG_CHN(9),
	AD413X_REG_CHN(10),
	AD413X_REG_CHN(11),
	AD413X_REG_CHN(12),
	AD413X_REG_CHN(13),
	AD413X_REG_CHN(14),
	AD413X_REG_CHN(15),
	AD413X_REG_CONFIG(0),
	AD413X_REG_CONFIG(1),
	AD413X_REG_CONFIG(2),
	AD413X_REG_CONFIG(3),
	AD413X_REG_CONFIG(4),
	AD413X_REG_CONFIG(5),
	AD413X_REG_CONFIG(6),
	AD413X_REG_CONFIG(7),
	AD413X_REG_FILTER(0),
	AD413X_REG_FILTER(1),
	AD413X_REG_FILTER(2),
	AD413X_REG_FILTER(3),
	AD413X_REG_FILTER(4),
	AD413X_REG_FILTER(5),
	AD413X_REG_FILTER(6),
	AD413X_REG_FILTER(7),
	AD413X_REG_OFFSET(0),
	AD413X_REG_OFFSET(1),
	AD413X_REG_OFFSET(2),
	AD413X_REG_OFFSET(3),
	AD413X_REG_OFFSET(4),
	AD413X_REG_OFFSET(5),
	AD413X_REG_OFFSET(6),
	AD413X_REG_OFFSET(7),
	AD413X_REG_GAIN(0),
	AD413X_REG_GAIN(1),
	AD413X_REG_GAIN(2),
	AD413X_REG_GAIN(3),
	AD413X_REG_GAIN(4),
	AD413X_REG_GAIN(5),
	AD413X_REG_GAIN(6),
	AD413X_REG_GAIN(7),
	AD413X_REG_MISC,
	AD413X_REG_FIFO_CTRL,
	AD413X_REG_FIFO_STS,
	AD413X_REG_FIFO_THRSHLD,
	AD413X_REG_FIFO_DATA
};
