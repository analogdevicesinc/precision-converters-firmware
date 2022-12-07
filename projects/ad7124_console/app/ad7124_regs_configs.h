/*!
 *****************************************************************************
  @file:  ad7124_reg_app_config.h

  @brief:

  @details:
 -----------------------------------------------------------------------------
 Copyright (c) 2018-20 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD7124_REGS_CONFIGS_H_
#define AD7124_REGS_CONFIGS_H_

#include "ad7124.h"

/*
 * Arrays holding the info for the AD7124 registers - address, initial value,
 * size and access type.
 */
extern struct ad7124_st_reg ad7124_regs_config_a[AD7124_REG_NO];
extern struct ad7124_st_reg ad7124_regs_config_b[AD7124_REG_NO];

#endif /* AD7124_REGS_CONFIGS_H_ */
