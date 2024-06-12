/***************************************************************************//**
 *   @file   ad355xr_support.h
 *   @brief  Header file for AD3552R No-OS driver supports
********************************************************************************
 * Copyright (c) 2023-2024 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef AD355XR_SUPPORT_H_
#define AD355XR_SUPPORT_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "ad3552r.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

int32_t ad355xr_write_one_sample_all_ch(struct ad3552r_desc *desc,
					uint16_t *data);
int32_t ad355xr_write_one_sample_one_ch(struct ad3552r_desc *desc,
					uint16_t *data, uint8_t ch_num);
#endif /* AD355XR_SUPPORT_H_ */
