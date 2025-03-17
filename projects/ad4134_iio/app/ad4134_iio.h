/***************************************************************************//**
*   @file   ad4134_iio.h
*   @brief  Header file AD4134 IIO interface
********************************************************************************
* Copyright (c) 2020-21 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef AD4134_IIO_H_
#define AD4134_IIO_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "iio.h"
#include "iio_types.h"
#include "ad4134.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

/* AD4134 global device instance for accessing device specific APIs */
extern struct ad4134_dev *p_ad4134_dev_inst;

/* Init the IIO interface */
int32_t ad4134_iio_initialize(void);

/* Run the IIO event handler */
void ad4134_iio_event_handler(void);

#endif /* AD4134_IIO_H_ */
