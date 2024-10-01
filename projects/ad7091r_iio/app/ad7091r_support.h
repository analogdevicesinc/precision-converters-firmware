/***************************************************************************/ /**
* @file   ad7091r_support.h
* @brief  AD7091R No-OS driver support functionality headers
********************************************************************************
* Copyright (c) 2024 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef AD7091R_SUPPORT_H_
#define AD7091R_SUPPORT_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad7091r8.h"

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/
enum ad7091r_conv_pin_state {
	CNV_GPIO_OUTPUT,
	CNV_PWM
};

int ad7091r_reconfig_conv(struct ad7091r8_dev *device,
			  enum ad7091r_conv_pin_state pin_state);

#endif /* AD7091R_SUPPORT_H_ */