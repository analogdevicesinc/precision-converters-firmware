/***************************************************************************//**
 *   @file   ad7689_support.h
 *   @brief  Header for AD7689 no-os driver support file
********************************************************************************
 * Copyright (c) 2022 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef _AD7689_SUPPORT_H_
#define _AD7689_SUPPORT_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "app_config.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

int32_t ad7689_read_single_sample(uint8_t input_chn, uint32_t *raw_data);
int32_t ad7689_perform_init_cnv(uint8_t first_active_chn,
				uint8_t second_active_chn, uint8_t num_of_active_channels);
int32_t ad7689_read_converted_sample(uint8_t *adc_data, uint8_t next_chn);

#endif /* _AD7689_SUPPORT_H_ */
