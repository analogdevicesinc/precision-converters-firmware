/***************************************************************************//**
 *   @file    ad3530r_support.h
 *   @brief   AD3530r No-OS driver support header file
********************************************************************************
 * Copyright (c) 2023-24 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef AD3530R_SUPPORT_H_
#define AD3530R_SUPPORT_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "ad3530r.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

enum ad3530r_ldac_pin_state {
	AD3530R_LDAC_GPIO_OUTPUT,
	AD3530R_LDAC_PWM
};

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

int ad3530r_spi_read_mask(struct ad3530r_desc *desc,
			  uint32_t addr,
			  uint32_t mask,
			  uint16_t *val);
int ad3530r_reconfig_ldac(struct ad3530r_desc* device,
			  enum ad3530r_ldac_pin_state pin_state);

#endif /* AD3530R_SUPPORT_H_ */