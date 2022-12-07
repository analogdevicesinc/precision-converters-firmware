/*!
 *****************************************************************************
  @file:  ad7124_support.h

  @brief: Provides useful support functions for the AD7124 NoOS driver

  @details:
 -----------------------------------------------------------------------------
 Copyright (c) 2019-2022 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD7124_SUPPORT_H_
#define AD7124_SUPPORT_H_

#include "ad7124.h"
#include "no_os_util.h"

/* PGA Gain Value */
#define AD7124_PGA_GAIN(x) (1 << (x))

#define AD7124_REF_VOLTAGE 2.5
#define AD7124_ADC_N_BITS 24

/* ADC_Control Register bits */
#define AD7124_ADC_CTRL_REG_POWER_MODE_MSK    NO_OS_GENMASK(7,6)
#define AD7124_ADC_CTRL_REG_POWER_MODE_RD(x)  (((x) >> 6) & 0x3)
#define AD7124_ADC_CTRL_REG_MSK				  NO_OS_GENMASK(5,2)

/* Channel Registers 0-15 bits */
#define AD7124_CH_MAP_REG_CH_ENABLE_RD(x)     (((x) >> 15) & 0x1)
#define AD7124_CH_MAP_REG_SETUP_MSK           NO_OS_GENMASK(14, 12)
#define AD7124_CH_MAP_REG_SETUP_RD(x)         (((x) >> 12) & 0x7)
#define AD7124_CH_MAP_REG_AINP_MSK            NO_OS_GENMASK(9, 5)
#define AD7124_CH_MAP_REG_AINP_RD(x)          (((x) >> 5) & 0x1F)
#define AD7124_CH_MAP_REG_AINM_MSK            NO_OS_GENMASK(4, 0)
#define AD7124_CH_MAP_REG_AINM_RD(x)          (((x) >> 0) & 0x1F)

/* Configuration Registers 0-7 bits */
#define AD7124_CFG_REG_BIPOLAR_RD(x)          (((x) >> 11) & 0x1)
#define AD7124_CFG_REG_REF_BUFP_RD(x)         (((x) >> 8) & 0x1)
#define AD7124_CFG_REG_REF_BUFM_RD(x)         (((x) >> 7) & 0x1)
#define AD7124_CFG_REG_AIN_BUFP_RD(x)         (((x) >> 6) & 0x1)
#define AD7124_CFG_REG_AINM_BUFP_RD(x)        (((x) >> 5) & 0x1)
#define AD7124_CFG_REG_REF_SEL_MSK            NO_OS_GENMASK(4, 3)
#define AD7124_CFG_REG_REF_SEL_RD(x)          (((x) >> 3) & 0x3)
#define AD7124_CFG_REG_PGA_MSK                NO_OS_GENMASK(2, 0)
#define AD7124_CFG_REG_PGA_RD(x)              (((x) >> 0) & 0x7)

/* Filter Register 0-7 bits */
#define AD7124_FILT_REG_FILTER_MSK            NO_OS_GENMASK(23, 21)
#define AD7124_FILT_REG_FILTER_RD(x)          (((x) >> 21) & 0x7)
#define AD7124_FILT_REG_FS_MSK                NO_OS_GENMASK(10, 0)
#define AD7124_FILT_REG_FS_RD(x)              (((x) >> 0) & 0x7FF)

uint8_t ad7124_get_channel_setup(struct ad7124_dev *dev, uint8_t channel);
uint8_t ad7124_get_channel_pga(struct ad7124_dev *dev, uint8_t channel);
bool ad7124_get_channel_bipolar(struct ad7124_dev *dev, uint8_t channel);
float ad7124_convert_sample_to_voltage(struct ad7124_dev *dev, uint8_t channel,
				       uint32_t sample);

#endif /* AD7124_SUPPORT_H_ */
