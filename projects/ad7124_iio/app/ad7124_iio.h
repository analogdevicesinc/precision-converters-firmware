/*************************************************************************//**
 *   @file   ad7124_iio.h
 *   @brief  IIO Header file for AD7124
******************************************************************************
* Copyright (c) 2023 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD7124_IIO_H_
#define AD7124_IIO_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>

#include "iio.h"
#include "ad7124.h"
#include "app_config.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

extern struct ad7124_dev *ad7124_dev_inst;
int32_t ad7124_iio_initialize(void);
void ad7124_iio_event_handler(void);

#endif // AD7124_IIO_H_
