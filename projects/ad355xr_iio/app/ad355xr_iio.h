/***************************************************************************//**
 *   @file    ad355xr_iio.h
 *   @brief   Header file for AD355XR IIO application interfaces
********************************************************************************
 * Copyright (c) 2023-2024 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef AD355XR_IIO_H
#define AD355XR_IIO_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "ad3552r.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

extern struct ad3552r_desc *ad355xr_dev_inst;
int32_t ad355xr_iio_initialize(void);
void ad355xr_iio_event_handler(void);

#endif /* AD355XR_IIO_H */
