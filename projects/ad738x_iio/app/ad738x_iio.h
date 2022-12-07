/***************************************************************************//**
 *   @file    ad738x_iio.h
 *   @brief   Header file for AD738x IIO application interfaces
********************************************************************************
 * Copyright (c) 2022 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef AD738X_IIO_H_
#define AD738X_IIO_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "ad738x.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

int32_t ad738x_iio_initialize(void);
void ad738x_iio_event_handler(void);

extern struct ad738x_dev *ad738x_dev_inst;

#endif /* AD738X_IIO_H_ */
