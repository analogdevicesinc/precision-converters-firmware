/*************************************************************************//**
 *   @file   ad717x_support.h
 *   @brief  Header for AD717X support configurations
******************************************************************************
* Copyright (c) 2022, 2026 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/
#ifndef AD717X_SUPPORT_H_
#define AD717X_SUPPORT_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad717x.h"
#include "no_os_util.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Enhanced filter mask */
#define AD717X_FILT_CONF_REG_ENHFILT_MSK      NO_OS_GENMASK(10,8)

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

int32_t ad717x_write_filter_order(ad717x_dev *device,
				  enum ad717x_order filter_order,
				  uint8_t setup_id);
int32_t ad717x_write_post_filter(ad717x_dev *device,
				 bool enable,
				 enum ad717x_enhfilt post_filter,
				 uint8_t setup_id);
int32_t ad717x_enable_cont_read(ad717x_dev *device, bool cont_read_en);
int32_t ad717x_adc_read_converted_sample(uint32_t *adc_data);

#endif  /* AD717X_SUPPORT_H_ */
