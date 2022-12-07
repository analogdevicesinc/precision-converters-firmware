/***************************************************************************//**
 * @file     ad7124_support.h
 * @brief    Provides useful support functions for the AD7124 No-OS driver
 * @details
********************************************************************************
* Copyright (c) 2019-2021 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef AD7124_SUPPORT_H_
#define AD7124_SUPPORT_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad7124.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

#define AD7124_PGA_GAIN(x)		(1 << (x))

/* ADC_Control Register bits */
#define AD7124_ADC_CTRL_REG_POWER_MODE_MSK    NO_OS_GENMASK(7,6)
#define AD7124_ADC_CTRL_REG_POWER_MODE_RD(x)  (((x) >> 6) & 0x3)
#define AD7124_ADC_CTRL_REG_MSK				  NO_OS_GENMASK(5,2)

/* Channel Registers 0-15 bits */
#define AD7124_CH_MAP_REG_SETUP_RD(x)         (((x) >> 12) & 0x7)
#define AD7124_CH_MAP_REG_AINP_RD(x)          (((x) >> 5) & 0x1F)
#define AD7124_CH_MAP_REG_AINM_RD(x)          (((x) >> 0) & 0x1F)

/* Configuration Registers 0-7 bits */
#define AD7124_CFG_REG_PGA_MSK                NO_OS_GENMASK(2, 0)

#define AD7124_REF_VOLTAGE		(2.5)
#define AD7124_ADC_N_BITS		(24)

/* AD7124 IOUT0 excitation current selection mask */
#define AD7124_IO_CTRL1_REG_IOUT_CH0_MSK	NO_OS_GENMASK(3,0)
#define AD7124_IO_CTRL1_REG_IOUT0_MSK		NO_OS_GENMASK(10,8)

/* AD7124 IOUT1 excitation current selection mask */
#define AD7124_IO_CTRL1_REG_IOUT_CH1_MSK	NO_OS_GENMASK(7,4)
#define AD7124_IO_CTRL1_REG_IOUT1_MSK		NO_OS_GENMASK(13,11)

/* AD7124 calibration bit read mask */
#define AD7124_ERR_REG_ADC_CAL_ERR_RD(x)	((x >> 18) & 0x1)

enum adc_control_modes {
	CONTINUOUS_CONV_MODE,
	SINGLE_CONV_MODE,
	STANDBY_MODE,
	POWER_DOWN_MODE,
	IDLE_MODE,
	INTERNAL_ZERO_SCALE_CALIBRATE_MODE,
	INTERNAL_FULL_SCALE_CALIBRATE_MODE,
	SYSTEM_ZERO_SCALE_CALIBRATE_MODE,
	SYSTEM_FULL_SCALE_CALIBRATE_MODE,
};

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

uint8_t ad7124_get_channel_setup(struct ad7124_dev *dev, uint8_t channel);
uint8_t ad7124_get_channel_pga(struct ad7124_dev *dev, uint8_t channel);
bool ad7124_get_channel_bipolar(struct ad7124_dev *dev, uint8_t channel);
float ad7124_convert_sample_to_voltage(struct ad7124_dev *dev, uint8_t channel,
				       int32_t sample);

#endif /* AD7124_SUPPORT_H_ */
