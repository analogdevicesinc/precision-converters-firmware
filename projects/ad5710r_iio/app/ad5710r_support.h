/***************************************************************************//**
 *   @file    ad5710r_support.h
 *   @brief   AD5710r No-OS driver support header file
********************************************************************************
 * Copyright (c) 2024-25 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef AD5710R_SUPPORT_H_
#define AD5710R_SUPPORT_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "ad5710r.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

enum ad5710r_ldac_pin_state {
	AD5710R_LDAC_GPIO_OUTPUT,
	AD5710R_LDAC_PWM
};

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

int ad5710r_spi_read_mask(struct ad5710r_desc *desc,
			  uint32_t addr,
			  uint32_t mask,
			  uint16_t *val);
int ad5710r_reconfig_ldac(struct ad5710r_desc* device,
			  enum ad5710r_ldac_pin_state pin_state);

#endif /* AD5710R_SUPPORT_H_ */
