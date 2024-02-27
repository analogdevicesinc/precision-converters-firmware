/***************************************************************************//**
 *   @file    ad579x_iio.h
 *   @brief   Header file for AD579x IIO application interfaces
********************************************************************************
 * Copyright (c) 2023 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef AD579X_IIO_H
#define AD579X_IIO_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "iio.h"
#include "app_config.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct ad5791_dev *ad579x_dev_desc;
int32_t ad579x_iio_init();
void ad579x_iio_event_handler(void);

#endif /* AD579X_IIO_H_ */