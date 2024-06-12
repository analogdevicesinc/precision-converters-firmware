/***************************************************************************//**
* @file   ad355xr_regs.c
* @brief  Source file for the ad355xr registers map
********************************************************************************
* Copyright (c) 2023-2024 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/
#include <stdint.h>
#include "ad355xr_regs.h"
#include "app_config.h"

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* AD355xr device register map */
const uint8_t ad355xr_regs[] = {
	AD3552R_REG_ADDR_INTERFACE_CONFIG_A,
	AD3552R_REG_ADDR_INTERFACE_CONFIG_B,
	AD3552R_REG_ADDR_DEVICE_CONFIG,
	AD3552R_REG_ADDR_CHIP_TYPE,
	AD3552R_REG_ADDR_PRODUCT_ID_L,
	AD3552R_REG_ADDR_PRODUCT_ID_H,
	AD3552R_REG_ADDR_CHIP_GRADE,
	AD3552R_REG_ADDR_SCRATCH_PAD,
	AD3552R_REG_ADDR_SPI_REVISION,
	AD3552R_REG_ADDR_VENDOR_L,
	AD3552R_REG_ADDR_VENDOR_H,
	AD3552R_REG_ADDR_STREAM_MODE,
	AD3552R_REG_ADDR_TRANSFER_REGISTER,
	AD3552R_REG_ADDR_INTERFACE_CONFIG_C,
	AD3552R_REG_ADDR_INTERFACE_STATUS_A,
	AD3552R_REG_ADDR_INTERFACE_CONFIG_D,
	AD3552R_REG_ADDR_SH_REFERENCE_CONFIG,
	AD3552R_REG_ADDR_ERR_ALARM_MASK,
	AD3552R_REG_ADDR_ERR_STATUS,
	AD3552R_REG_ADDR_POWERDOWN_CONFIG,
	AD3552R_REG_ADDR_CH0_CH1_OUTPUT_RANGE,
	AD3552R_REG_ADDR_CH_OFFSET(0),
	AD3552R_REG_ADDR_CH_GAIN(0),
#if defined (DEV_AD3552R) || defined (DEV_AD3542R_12) || defined (DEV_AD3542R_16)
	AD3552R_REG_ADDR_CH_OFFSET(1),
	AD3552R_REG_ADDR_CH_GAIN(1),
#endif
	AD3552R_REG_ADDR_HW_LDAC_16B,
	AD3552R_REG_ADDR_CH_DAC_16B(0),
#if defined (DEV_AD3552R) || defined (DEV_AD3542R_12) || defined (DEV_AD3542R_16)
	AD3552R_REG_ADDR_CH_DAC_16B(1),
#endif
	AD3552R_REG_ADDR_DAC_PAGE_MASK_16B,
	AD3552R_REG_ADDR_CH_SELECT_16B,
	AD3552R_REG_ADDR_INPUT_PAGE_MASK_16B,
	AD3552R_REG_ADDR_SW_LDAC_16B,
	AD3552R_REG_ADDR_CH_INPUT_16B(0),
#if defined (DEV_AD3552R) || defined (DEV_AD3542R_12) || defined (DEV_AD3542R_16)
	AD3552R_REG_ADDR_CH_INPUT_16B(1),
#endif
	AD3552R_REG_ADDR_HW_LDAC_24B,
	AD3552R_REG_ADDR_CH_DAC_24B(0),
#if defined (DEV_AD3552R) || defined (DEV_AD3542R_12) || defined (DEV_AD3542R_16)
	AD3552R_REG_ADDR_CH_DAC_24B(1),
#endif
	AD3552R_REG_ADDR_DAC_PAGE_MASK_24B,
	AD3552R_REG_ADDR_CH_SELECT_24B,
	AD3552R_REG_ADDR_INPUT_PAGE_MASK_24B,
	AD3552R_REG_ADDR_SW_LDAC_24B,
	AD3552R_REG_ADDR_CH_INPUT_24B(0),
#if defined (DEV_AD3552R) || defined (DEV_AD3542R_12) || defined (DEV_AD3542R_16)
	AD3552R_REG_ADDR_CH_INPUT_24B(1)
#endif
};