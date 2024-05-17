/***************************************************************************//**
*   @file   ad469x_iio.h
*   @brief  Header file of ad469x_iio
********************************************************************************
* Copyright (c) 2021-22 Analog Devices, Inc.
*
* All rights reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef IIO_AD469X_H_
#define IIO_AD469X_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include <stdbool.h>

#include "iio.h"
#include "iio_types.h"

/******************************************************************************/
/****************************** Macros ****************************************/
/******************************************************************************/

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/*  AD469x global device instance for accessing device specific APIs */
extern struct ad469x_dev *p_ad469x_dev;

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/* Init the IIO interface */
int32_t ad469x_iio_initialize(void);

/* Run the IIO event handler */
void ad469x_iio_event_handler(void);

#endif /* IIO_AD469X_H_ */