/***************************************************************************//**
 *   @file    ad77681_regs.c
 *   @brief   AD77681 registers map
 *   @details
********************************************************************************
 * Copyright (c) 2021 Analog Devices, Inc.
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
#include "ad77681.h"

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

const uint32_t ad77681_regs[] = {
	AD77681_REG_CHIP_TYPE,
	AD77681_REG_PROD_ID_L,
	AD77681_REG_PROD_ID_H,
	AD77681_REG_CHIP_GRADE,
	AD77681_REG_SCRATCH_PAD,
	AD77681_REG_VENDOR_L,
	AD77681_REG_VENDOR_H,
	AD77681_REG_INTERFACE_FORMAT,
	AD77681_REG_POWER_CLOCK,
	AD77681_REG_ANALOG,
	AD77681_REG_ANALOG2,
	AD77681_REG_CONVERSION,
	AD77681_REG_DIGITAL_FILTER,
	AD77681_REG_SINC3_DEC_RATE_MSB,
	AD77681_REG_SINC3_DEC_RATE_LSB,
	AD77681_REG_DUTY_CYCLE_RATIO,
	AD77681_REG_SYNC_RESET,
	AD77681_REG_GPIO_CONTROL,
	AD77681_REG_GPIO_WRITE,
	AD77681_REG_GPIO_READ,
	AD77681_REG_OFFSET_HI,
	AD77681_REG_OFFSET_MID,
	AD77681_REG_OFFSET_LO,
	AD77681_REG_GAIN_HI,
	AD77681_REG_GAIN_MID,
	AD77681_REG_GAIN_LO,
	AD77681_REG_SPI_DIAG_ENABLE,
	AD77681_REG_ADC_DIAG_ENABLE,
	AD77681_REG_DIG_DIAG_ENABLE,
	AD77681_REG_ADC_DATA,
	AD77681_REG_MASTER_STATUS,
	AD77681_REG_SPI_DIAG_STATUS,
	AD77681_REG_ADC_DIAG_STATUS,
	AD77681_REG_DIG_DIAG_STATUS,
	AD77681_REG_MCLK_COUNTER
};

