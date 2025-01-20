/***************************************************************************//**
* @file   ad3530r_regs.c
* @brief  Source file for the ad3530r registers map
********************************************************************************
* Copyright (c) 2023 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/

#include <stdint.h>
#include "ad3530r_regs.h"

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* AD3530R device register map */
const uint32_t ad3530r_regs [] = {
	AD3530R_REG_ADDR_INTERFACE_CONFIG_A,
	AD3530R_REG_ADDR_INTERFACE_CONFIG_B,
	AD3530R_REG_ADDR_DEVICE_CONFIG,
	AD3530R_REG_ADDR_CHIP_TYPE,
	AD3530R_REG_ADDR_PRODUCT_ID_L,
	AD3530R_REG_ADDR_PRODUCT_ID_H,
	AD3530R_REG_ADDR_CHIP_GRADE,
	AD3530R_REG_ADDR_SCRATCH_PAD,
	AD3530R_REG_ADDR_SPI_REVISION,
	AD3530R_REG_ADDR_VENDOR_L,
	AD3530R_REG_ADDR_VENDOR_H,
	AD3530R_REG_ADDR_STREAM_MODE,
	AD3530R_REG_ADDR_TRANSFER_REGISTER,
	AD3530R_REG_ADDR_INTERFACE_CONFIG_C,
	AD3530R_REG_ADDR_INTERFACE_STATUS_A,
	AD3530R_REG_ADDR_OPERATING_MODE_0,
	AD3530R_REG_ADDR_OPERATING_MODE_1,
	AD3530R_REG_ADDR_OUTPUT_CONTROL_0,
	AD3530R_REG_ADDR_REF_CONTROL_0,
	AD3530R_REG_ADDR_MUX_OUT_SELECT,
	AD3530R_REG_ADDR_STATUS_CONTROL,
	AD3530R_REG_ADDR_HW_LDAC_EN_0,
	AD3530R_REG_ADDR_SW_LDAC_EN_0,
	AD3530R_REG_ADDR_DAC_CHN(0),
	AD3530R_REG_ADDR_DAC_CHN(1),
	AD3530R_REG_ADDR_DAC_CHN(2),
	AD3530R_REG_ADDR_DAC_CHN(3),
	AD3530R_REG_ADDR_DAC_CHN(4),
	AD3530R_REG_ADDR_DAC_CHN(5),
	AD3530R_REG_ADDR_DAC_CHN(6),
	AD3530R_REG_ADDR_DAC_CHN(7),
	AD3530R_REG_ADDR_MULTI_DAC_CH,
	AD3530R_REG_ADDR_MULTI_DAC_SEL_0,
	AD3530R_REG_ADDR_SW_LDAC_TRIG_A,
	AD3530R_REG_ADDR_MULTI_INPUT_CH,
	AD3530R_REG_ADDR_MULTI_INPUT_SEL_0,
	AD3530R_REG_ADDR_SW_LDAC_TRIG_B,
	AD3530R_REG_ADDR_INPUT_CHN(0),
	AD3530R_REG_ADDR_INPUT_CHN(1),
	AD3530R_REG_ADDR_INPUT_CHN(2),
	AD3530R_REG_ADDR_INPUT_CHN(3),
	AD3530R_REG_ADDR_INPUT_CHN(4),
	AD3530R_REG_ADDR_INPUT_CHN(5),
	AD3530R_REG_ADDR_INPUT_CHN(6),
	AD3530R_REG_ADDR_INPUT_CHN(7),
	AD3530R_REG_MAP_END
};
