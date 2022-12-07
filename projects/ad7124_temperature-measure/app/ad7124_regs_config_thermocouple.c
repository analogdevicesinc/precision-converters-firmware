/***************************************************************************//**
 * @file     ad7124_regs_config_thermocouple.c
 * @brief    AD7124 register configuration file for thermocouple sensor interface
 * @details
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

#include "ad7124_regs_configs.h"

/******************************************************************************/
/********************* Macros and Constants Definitions ***********************/
/******************************************************************************/

const struct ad7124_st_reg ad7124_regs_config_thermocouple[AD7124_REG_NO] = {
	{ AD7124_STATUS_REG, 0x0, 1, AD7124_R },
	{
		AD7124_ADC_CTRL_REG,
		AD7124_ADC_CTRL_REG_MODE(2) | AD7124_ADC_CTRL_REG_REF_EN | 		// ADC in Standby mode, Int Ref enabled
		AD7124_ADC_CTRL_REG_POWER_MODE(2) |								// Full power mode
		AD7124_ADC_CTRL_REG_CS_EN | AD7124_ADC_CTRL_REG_DATA_STATUS,  	// CS mode enable, Status along data
		2, AD7124_RW
	},
	{ AD7124_DATA_REG, 0x0, 3, AD7124_R },
	{ AD7124_IO_CTRL1_REG, 0x0, 3, AD7124_RW },
	{ AD7124_IO_CTRL2_REG, 0x0, 2, AD7124_RW },
	{ AD7124_ID_REG, 0x0, 1, AD7124_R },
	{ AD7124_ERR_REG, 0x0, 3, AD7124_R },
	{
		AD7124_ERREN_REG,
		AD7124_ERR_REG_SPI_CRC_ERR | AD7124_ERR_REG_SPI_IGNORE_ERR |
		AD7124_ERREN_REG_ADC_CAL_ERR_EN, 		// Monitor ADC calibration error
		3,
		AD7124_RW
	},
	{ 0x08, 0x00, 1, AD7124_R },
	{
		AD7124_CH0_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(THERMOCOUPLE1_AINP) | AD7124_CH_MAP_REG_AINM(THERMOCOUPLE1_AINM),
		2, AD7124_RW
	},
	{
		AD7124_CH1_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(THERMOCOUPLE2_AINP) | AD7124_CH_MAP_REG_AINM(THERMOCOUPLE2_AINM),
		2, AD7124_RW
	},
	{
		AD7124_CH2_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(THERMOCOUPLE3_AINP) | AD7124_CH_MAP_REG_AINM(THERMOCOUPLE3_AINM),
		2,
		AD7124_RW
	},
	{
		AD7124_CH3_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(THERMOCOUPLE4_AINP) | AD7124_CH_MAP_REG_AINM(THERMOCOUPLE4_AINM),
		2,
		AD7124_RW
	},
	{
		AD7124_CH4_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(THERMOCOUPLE5_AINP) | AD7124_CH_MAP_REG_AINM(THERMOCOUPLE5_AINM),
		2,
		AD7124_RW
	},
	{
		AD7124_CH5_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(THERMOCOUPLE6_AINP) | AD7124_CH_MAP_REG_AINM(THERMOCOUPLE6_AINM),
		2,
		AD7124_RW
	},
	{
		AD7124_CH6_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(1) |
		AD7124_CH_MAP_REG_AINP(CJC_RTD_AINP) | AD7124_CH_MAP_REG_AINM(CJC_RTD_AINM),
		2, AD7124_RW
	},
	{
		AD7124_CH7_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(2) |
		AD7124_CH_MAP_REG_AINP(CJC_PTC_THERMISTOR_AINP) | AD7124_CH_MAP_REG_AINM(CJC_PTC_THERMISTOR_AINM),
		2, AD7124_RW
	},
	{ AD7124_CH8_MAP_REG, 0x0001, 2, AD7124_RW },
	{ AD7124_CH9_MAP_REG, 0x0001, 2, AD7124_RW },
	{ AD7124_CH10_MAP_REG, 0x0001, 2, AD7124_RW },
	{ AD7124_CH11_MAP_REG, 0x0001, 2, AD7124_RW },
	{ AD7124_CH12_MAP_REG, 0x0001, 2, AD7124_RW },
	{ AD7124_CH13_MAP_REG, 0x0001, 2, AD7124_RW },
	{ AD7124_CH14_MAP_REG, 0x0001, 2, AD7124_RW },
	{ AD7124_CH15_MAP_REG, 0x0001, 2, AD7124_RW },

	{
		AD7124_CFG0_REG,
		AD7124_CFG_REG_PGA(THERMOCOUPLE_GAIN_VALUE) | AD7124_CFG_REG_REF_SEL(2) |	// Internal Ref
		AD7124_CFG_REG_BIPOLAR |								// Bipolar inputs
		AD7124_CFG_REG_AINN_BUFM | AD7124_CFG_REG_AIN_BUFP |  	// Input buffers enabled
		AD7124_CFG_REG_REF_BUFM | AD7124_CFG_REG_REF_BUFP,		// Ref buffers enabled
		2, AD7124_RW
	},
	{
		AD7124_CFG1_REG,
		AD7124_CFG_REG_PGA(RTD_2WIRE_GAIN_VALUE) | AD7124_CFG_REG_REF_SEL(0) |   		// External REFIN
		AD7124_CFG_REG_BIPOLAR |								// Bipolar inputs
		AD7124_CFG_REG_AINN_BUFM | AD7124_CFG_REG_AIN_BUFP |   	// Input buffers enabled
		AD7124_CFG_REG_REF_BUFM | AD7124_CFG_REG_REF_BUFP,	   	// Ref buffers enabled
		2, AD7124_RW
	},
	{
		AD7124_CFG2_REG,
		AD7124_CFG_REG_PGA(THERMISTOR_GAIN_VALUE) | AD7124_CFG_REG_REF_SEL(0) |  // External REFIN
		AD7124_CFG_REG_BIPOLAR | 								// Bipolar inputs
		AD7124_CFG_REG_AINN_BUFM | AD7124_CFG_REG_AIN_BUFP |    // Input buffers enabled
		AD7124_CFG_REG_REF_BUFM | AD7124_CFG_REG_REF_BUFP, 	   	// Ref buffers enabled
		2, AD7124_RW
	},
	{ AD7124_CFG3_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG4_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG5_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG6_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG7_REG, 0x0860, 2, AD7124_RW },

	{
		AD7124_FILT0_REG,
		AD7124_FILT_REG_FS(48) | AD7124_FILT_REG_POST_FILTER(3),    // ODR= 50SPS, Post filter= 25SPS
		3, AD7124_RW
	},
	{
		AD7124_FILT1_REG,
		AD7124_FILT_REG_FS(48) | AD7124_FILT_REG_POST_FILTER(3),     // ODR= 50SPS, Post filter= 25SPS
		3, AD7124_RW
	},
	{
		AD7124_FILT2_REG,
		AD7124_FILT_REG_FS(48) | AD7124_FILT_REG_POST_FILTER(3),     // ODR= 50SPS, Post filter= 25SPS
		3, AD7124_RW
	},
	{ AD7124_FILT3_REG, 0x060180, 3, AD7124_RW },
	{ AD7124_FILT4_REG, 0x060180, 3, AD7124_RW },
	{ AD7124_FILT5_REG, 0x060180, 3, AD7124_RW },
	{ AD7124_FILT6_REG, 0x060180, 3, AD7124_RW },
	{ AD7124_FILT7_REG, 0x060180, 3, AD7124_RW },

	{AD7124_OFFS0_REG, 0x800000, 3, AD7124_RW },
	{AD7124_OFFS1_REG, 0x800000, 3, AD7124_RW },
	{AD7124_OFFS2_REG, 0x800000, 3, AD7124_RW },
	{AD7124_OFFS3_REG, 0x800000, 3, AD7124_RW },
	{AD7124_OFFS4_REG, 0x800000, 3, AD7124_RW },
	{AD7124_OFFS5_REG, 0x800000, 3, AD7124_RW },
	{AD7124_OFFS6_REG, 0x800000, 3, AD7124_RW },
	{AD7124_OFFS7_REG, 0x800000, 3, AD7124_RW },

	{AD7124_GAIN0_REG, 0x500000, 3, AD7124_RW },
	{AD7124_GAIN1_REG, 0x500000, 3, AD7124_RW },
	{AD7124_GAIN2_REG, 0x500000, 3, AD7124_RW },
	{AD7124_GAIN3_REG, 0x500000, 3, AD7124_RW },
	{AD7124_GAIN4_REG, 0x500000, 3, AD7124_RW },
	{AD7124_GAIN5_REG, 0x500000, 3, AD7124_RW },
	{AD7124_GAIN6_REG, 0x500000, 3, AD7124_RW },
	{AD7124_GAIN7_REG, 0x500000, 3, AD7124_RW },
};
