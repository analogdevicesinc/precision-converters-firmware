/***************************************************************************//**
 * @file     ad7124_regs_config_rtd.c
 * @brief    AD7124 register configuration file for RTD temperature sensor interface
 * @details
********************************************************************************
* Copyright (c) 2021, 2025 Analog Devices, Inc.
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

/* 2-wire multiple RTD sensor configurations */
const struct ad7124_st_reg ad7124_regs_config_2wire_rtd[AD7124_REG_NO] = {
	{ AD7124_STATUS_REG, 0x0, 1, AD7124_R },
	{
		AD7124_ADC_CTRL_REG,
		AD7124_ADC_CTRL_REG_MODE(2) |									// ADC in Standby mode
		AD7124_ADC_CTRL_REG_POWER_MODE(2) |								// Full power mode
		AD7124_ADC_CTRL_REG_CS_EN | AD7124_ADC_CTRL_REG_DATA_STATUS,	// CS mode enable, Status along data
		2, AD7124_RW
	},
	{ AD7124_DATA_REG, 0x0, 3, AD7124_R },
	{
		AD7124_IO_CTRL1_REG,
		AD7124_IO_CTRL1_REG_IOUT_CH0(RTD1_2WIRE_IOUT0),
		3, AD7124_RW
	},
	{ AD7124_IO_CTRL2_REG, 0x0, 2, AD7124_RW },
	{ AD7124_ID_REG, 0x0, 1, AD7124_R },
	{ AD7124_ERR_REG, 0x0, 3, AD7124_R },
	{
		AD7124_ERREN_REG,
		AD7124_ERR_REG_SPI_IGNORE_ERR |
		AD7124_ERREN_REG_ADC_CAL_ERR_EN, 		// Monitor ADC calibration error
		3,
		AD7124_RW
	},
	{ 0x08, 0x00, 1, AD7124_R },
	{
		AD7124_CH0_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(RTD1_2WIRE_AINP) | AD7124_CH_MAP_REG_AINM(RTD1_2WIRE_AINM),
		2, AD7124_RW
	},
	{
		AD7124_CH1_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(RTD2_2WIRE_AINP) | AD7124_CH_MAP_REG_AINM(RTD2_2WIRE_AINM),
		2, AD7124_RW
	},
	{
		AD7124_CH2_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(RTD3_2WIRE_AINP) | AD7124_CH_MAP_REG_AINM(RTD3_2WIRE_AINM),
		2, AD7124_RW
	},
	{
		AD7124_CH3_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(RTD4_2WIRE_AINP) | AD7124_CH_MAP_REG_AINM(RTD4_2WIRE_AINM),
		2, AD7124_RW
	},
	{
		AD7124_CH4_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(RTD5_2WIRE_AINP) | AD7124_CH_MAP_REG_AINM(RTD5_2WIRE_AINM),
		2, AD7124_RW
	},
	{ AD7124_CH5_MAP_REG, 0x0001, 2, AD7124_RW },
	{ AD7124_CH6_MAP_REG, 0x0001, 2, AD7124_RW },
	{ AD7124_CH7_MAP_REG, 0x0001, 2, AD7124_RW },
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
		AD7124_CFG_REG_PGA(RTD_2WIRE_GAIN_VALUE) | AD7124_CFG_REG_REF_SEL(0) |  	// External REFIN
		AD7124_CFG_REG_BIPOLAR |								// Bipolar inputs
		AD7124_CFG_REG_AINN_BUFM | AD7124_CFG_REG_AIN_BUFP |	// Input buffers enabled
		AD7124_CFG_REG_REF_BUFM | AD7124_CFG_REG_REF_BUFP,		// Ref buffers enabled
		2, AD7124_RW
	},
	{ AD7124_CFG1_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG2_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG3_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG4_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG5_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG6_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG7_REG, 0x0860, 2, AD7124_RW },

	{
		AD7124_FILT0_REG,
		AD7124_FILT_REG_FS(48) | AD7124_FILT_REG_POST_FILTER(3),  // ODR= 50SPS, Post filter= 25SPS
		3, AD7124_RW
	},
	{ AD7124_FILT1_REG, 0x060180, 3, AD7124_RW },
	{ AD7124_FILT2_REG, 0x060180, 3, AD7124_RW },
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


/* 3-wire multiple RTD sensor configurations */
const struct ad7124_st_reg ad7124_regs_config_3wire_rtd[AD7124_REG_NO] = {
	{ AD7124_STATUS_REG, 0x0, 1, AD7124_R },
	{
		AD7124_ADC_CTRL_REG,
		AD7124_ADC_CTRL_REG_REF_EN |
		AD7124_ADC_CTRL_REG_MODE(2) | AD7124_ADC_CTRL_REG_REF_EN | 		// ADC in Standby mode, Int Ref enabled
		AD7124_ADC_CTRL_REG_POWER_MODE(2) |								// Full power mode
		AD7124_ADC_CTRL_REG_CS_EN | AD7124_ADC_CTRL_REG_DATA_STATUS, 	// CS mode enable, Status along data
		2, AD7124_RW
	},
	{ AD7124_DATA_REG, 0x0, 3, AD7124_R },
	{
		AD7124_IO_CTRL1_REG,
		AD7124_IO_CTRL1_REG_IOUT_CH0(RTD1_3WIRE_IOUT0) |
		AD7124_IO_CTRL1_REG_IOUT_CH1(RTD1_3WIRE_IOUT1),
		3, AD7124_RW
	},
	{ AD7124_IO_CTRL2_REG, 0x0, 2, AD7124_RW },
	{ AD7124_ID_REG, 0x0, 1, AD7124_R },
	{ AD7124_ERR_REG, 0x0, 3, AD7124_R },
	{
		AD7124_ERREN_REG,
		AD7124_ERR_REG_SPI_IGNORE_ERR |
		AD7124_ERREN_REG_ADC_CAL_ERR_EN, 		// Monitor ADC calibration error
		3,
		AD7124_RW
	},
	{ 0x08, 0x00, 1, AD7124_R },
	{
		AD7124_CH0_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(RTD1_3WIRE_AINP) | AD7124_CH_MAP_REG_AINM(RTD1_3WIRE_AINM),
		2, AD7124_RW
	},
	{
		AD7124_CH1_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(RTD2_3WIRE_AINP) | AD7124_CH_MAP_REG_AINM(RTD2_3WIRE_AINM),
		2, AD7124_RW
	},
	{
		AD7124_CH2_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(RTD3_3WIRE_AINP) | AD7124_CH_MAP_REG_AINM(RTD3_3WIRE_AINM),
		2, AD7124_RW
	},
	{
		AD7124_CH3_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(RTD4_3WIRE_AINP) | AD7124_CH_MAP_REG_AINM(RTD4_3WIRE_AINM),
		2, AD7124_RW
	},
	{
		AD7124_CH4_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(1) |
		AD7124_CH_MAP_REG_AINP(RTD_3WIRE_EXC_MEASURE_AINP) | AD7124_CH_MAP_REG_AINM(RTD_3WIRE_EXC_MEASURE_AINM),
		2, AD7124_RW
	},
	{ AD7124_CH5_MAP_REG, 0x0001, 2, AD7124_RW },
	{ AD7124_CH6_MAP_REG, 0x0001, 2, AD7124_RW },
	{ AD7124_CH7_MAP_REG, 0x0001, 2, AD7124_RW },
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
		AD7124_CFG_REG_PGA(SINGLE_3WIRE_RTD_GAIN) | AD7124_CFG_REG_REF_SEL(0) | 	// External REFIN
		AD7124_CFG_REG_BIPOLAR |								// Bipolar inputs
		AD7124_CFG_REG_AINN_BUFM | AD7124_CFG_REG_AIN_BUFP | 	// Input buffers enabled
		AD7124_CFG_REG_REF_BUFM | AD7124_CFG_REG_REF_BUFP,		// Ref buffers enabled
		2, AD7124_RW
	},
	{
		AD7124_CFG1_REG,
		AD7124_CFG_REG_PGA(RTD_3WIRE_EXC_MEASURE_GAIN) | AD7124_CFG_REG_REF_SEL(2) |    // Internal REFIN
		AD7124_CFG_REG_BIPOLAR | 								// Bipolar inputs
		AD7124_CFG_REG_AINN_BUFM | AD7124_CFG_REG_AIN_BUFP | 	// Input buffers enabled
		AD7124_CFG_REG_REF_BUFM | AD7124_CFG_REG_REF_BUFP,		// Ref buffers enabled
		2, AD7124_RW
	},
	{ AD7124_CFG2_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG3_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG4_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG5_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG6_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG7_REG, 0x0860, 2, AD7124_RW },

	{
		AD7124_FILT0_REG,
		AD7124_FILT_REG_FS(48) | AD7124_FILT_REG_POST_FILTER(3),   // ODR= 50SPS, Post filter= 25SPS
		3, AD7124_RW
	},
	{
		AD7124_FILT1_REG,
		AD7124_FILT_REG_FS(48) | AD7124_FILT_REG_POST_FILTER(3),   // ODR= 50SPS, Post filter= 25SPS
		3, AD7124_RW
	},
	{ AD7124_FILT2_REG, 0x060180, 3, AD7124_RW },
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


/* 4-wire multiple RTD sensor configurations */
const struct ad7124_st_reg ad7124_regs_config_4wire_rtd[AD7124_REG_NO] = {
	{ AD7124_STATUS_REG, 0x0, 1, AD7124_R },
	{
		AD7124_ADC_CTRL_REG,
		AD7124_ADC_CTRL_REG_MODE(2) | 									// ADC in Standby mode
		AD7124_ADC_CTRL_REG_POWER_MODE(2) |								// Full power mode
		AD7124_ADC_CTRL_REG_CS_EN | AD7124_ADC_CTRL_REG_DATA_STATUS, 	// CS mode enable, Status along data
		2, AD7124_RW
	},
	{ AD7124_DATA_REG, 0x0, 3, AD7124_R },
	{
		AD7124_IO_CTRL1_REG,
		AD7124_IO_CTRL1_REG_IOUT_CH0(RTD1_4WIRE_IOUT0),
		3, AD7124_RW
	},
	{ AD7124_IO_CTRL2_REG, 0x0, 2, AD7124_RW },
	{ AD7124_ID_REG, 0x0, 1, AD7124_R },
	{ AD7124_ERR_REG, 0x0, 3, AD7124_R },
	{
		AD7124_ERREN_REG,
		AD7124_ERR_REG_SPI_IGNORE_ERR |
		AD7124_ERREN_REG_ADC_CAL_ERR_EN, 		// Monitor ADC calibration error
		3,
		AD7124_RW
	},
	{ 0x08, 0x00, 1, AD7124_R },
	{
		AD7124_CH0_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(RTD1_4WIRE_AINP) | AD7124_CH_MAP_REG_AINM(RTD1_4WIRE_AINM),
		2, AD7124_RW
	},
	{
		AD7124_CH1_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(RTD2_4WIRE_AINP) | AD7124_CH_MAP_REG_AINM(RTD2_4WIRE_AINM),
		2, AD7124_RW
	},
	{
		AD7124_CH2_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(RTD3_4WIRE_AINP) | AD7124_CH_MAP_REG_AINM(RTD3_4WIRE_AINM),
		2, AD7124_RW
	},
	{
		AD7124_CH3_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(RTD4_4WIRE_AINP) | AD7124_CH_MAP_REG_AINM(RTD4_4WIRE_AINM),
		2, AD7124_RW
	},
	{
		AD7124_CH4_MAP_REG,
		AD7124_CH_MAP_REG_SETUP(0) |
		AD7124_CH_MAP_REG_AINP(RTD5_4WIRE_AINP) | AD7124_CH_MAP_REG_AINM(RTD5_4WIRE_AINM),
		2, AD7124_RW
	},
	{ AD7124_CH5_MAP_REG, 0x0001, 2, AD7124_RW },
	{ AD7124_CH6_MAP_REG, 0x0001, 2, AD7124_RW },
	{ AD7124_CH7_MAP_REG, 0x0001, 2, AD7124_RW },
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
		AD7124_CFG_REG_PGA(RTD_4WIRE_GAIN_VALUE) | AD7124_CFG_REG_REF_SEL(0) |   	// External REFIN
		AD7124_CFG_REG_BIPOLAR |								// Bipolar inputs
		AD7124_CFG_REG_AINN_BUFM | AD7124_CFG_REG_AIN_BUFP | 	// Input buffers enabled
		AD7124_CFG_REG_REF_BUFM | AD7124_CFG_REG_REF_BUFP,	 	// Ref buffers enabled
		2, AD7124_RW
	},
	{ AD7124_CFG1_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG2_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG3_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG4_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG5_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG6_REG, 0x0860, 2, AD7124_RW },
	{ AD7124_CFG7_REG, 0x0860, 2, AD7124_RW },

	{
		AD7124_FILT0_REG,
		AD7124_FILT_REG_FS(48) | AD7124_FILT_REG_POST_FILTER(3),   // ODR= 50SPS, Post filter= 25SPS
		3, AD7124_RW
	},
	{ AD7124_FILT1_REG, 0x060180, 3, AD7124_RW },
	{ AD7124_FILT2_REG, 0x060180, 3, AD7124_RW },
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

/* Used to create the ad7124 device */
struct  ad7124_init_param ad7124_rtd_init_params = {
	.spi_init = &spi_init_params,  // spi_init_param type
	.regs = ad7124_regs_config_2wire_rtd,
	.spi_rdy_poll_cnt = 10000, //  count for polling RDY
	.power_mode = AD7124_HIGH_POWER,
#if defined(DEV_AD7124_4)
	.active_device = ID_AD7124_4,
#else
	.active_device = ID_AD7124_8,
#endif
	.setups = {
		{ .bi_unipolar = true, .ref_buff = false, .ain_buff = true, .ref_source = EXTERNAL_REFIN1 },
		{ .bi_unipolar = true, .ref_buff = false, .ain_buff = true, .ref_source = EXTERNAL_REFIN1 },
		{ .bi_unipolar = true, .ref_buff = false, .ain_buff = true, .ref_source = EXTERNAL_REFIN1 },
		{ .bi_unipolar = true, .ref_buff = false, .ain_buff = true, .ref_source = EXTERNAL_REFIN1},
		{ .bi_unipolar = true, .ref_buff = false, .ain_buff = true, .ref_source = EXTERNAL_REFIN1 },
		{ .bi_unipolar = true, .ref_buff = false, .ain_buff = true, .ref_source = EXTERNAL_REFIN1 },
		{ .bi_unipolar = true, .ref_buff = false, .ain_buff = true, .ref_source = EXTERNAL_REFIN1 },
		{ .bi_unipolar = true, .ref_buff = false, .ain_buff = true, .ref_source = EXTERNAL_REFIN1 },
	},
	.chan_map = {
		{ .channel_enable = true, .setup_sel = 0, .ain.ainp = AD7124_AIN2, .ain.ainm = AD7124_AIN3 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN4, .ain.ainm = AD7124_AIN5 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN6, .ain.ainm = AD7124_AIN7 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN9, .ain.ainm = AD7124_AIN10 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN12, .ain.ainm = AD7124_AIN13 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .ain.ainp = AD7124_AIN0, .ain.ainm = AD7124_AIN1 }
	}
};

