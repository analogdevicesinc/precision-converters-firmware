/***************************************************************************//**
*   @file   ad77681_iio.h
*   @brief  Header file of ad77681_iio
********************************************************************************
* Copyright (c) 2021 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef _AD77681_IIO_H_
#define _AD77681_IIO_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include <stdbool.h>

#include "iio.h"
#include "iio_types.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

/* AD77681 global device instance for accessing device specific APIs */
extern struct ad77681_dev *p_ad77681_dev_inst;

/* Init the IIO interface */
int32_t ad77681_iio_initialize(void);

/* Run the IIO event handler */
void ad77681_iio_event_handler(void);

#endif /* _AD77681_IIO_H_ */
