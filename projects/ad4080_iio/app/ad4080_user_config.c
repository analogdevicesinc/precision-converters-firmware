/***************************************************************************//**
 *   @file    ad4080_user_config.c
 *   @brief   User configuration source for the AD4080 IIO Application
********************************************************************************
 * Copyright (c) 2023-25 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include "ad4080_user_config.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* Initialize the AD4080 device structure */
struct ad4080_init_param ad4080_init_params = {
	.spi_init = NULL,
	.spi3wire = 1,
	.addr_asc = AD4080_ADDR_INCR,
	.single_instr = AD4080_SINGLE_INST,
	.short_instr = AD4080_15_BIT_ADDR,
	.strict_reg = AD4080_REG_NORMAL_MODE,
	.intf_chk_en = AD4080_DATA,
	.cnv_spi_lvds_lanes = AD4080_ONE_LANE,
	.conv_data_spi_lvds = AD4080_CONV_DATA_SPI,
	.lvds_cnv_clk_cnt = 0,
	.lvds_self_clk_mode = AD4080_SELF_CLK_MODE,
	.cnv_clk_mode = AD4080_CNV_CMOS_MODE,
	.lvds_vod = AD4080_185mVPP,
	.ana_dig_ldo_pd = AD4080_AD_LDO_EN,
	.intf_ldo_pd = AD4080_INTF_LDO_EN,
	.fifo_mode = AD4080_FIFO_DISABLE,
	.op_mode = AD4080_OP_NORMAL,
	.gpio_op_enable = { AD4080_GPIO_OUTPUT, AD4080_GPIO_INPUT, AD4080_GPIO_INPUT, AD4080_GPIO_INPUT },
	.gpio_op_func_sel = { AD4080_GPIO_ADI_NSPI_SDO_DATA, AD4080_GPIO_ADI_NSPI_SDO_DATA, AD4080_GPIO_ADI_NSPI_SDO_DATA, AD4080_GPIO_ADI_NSPI_SDO_DATA}
};

