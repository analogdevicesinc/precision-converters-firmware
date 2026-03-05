/***************************************************************************//**
*   @file   ad7134_iio.h
*   @brief  Header file AD7134 IIO interface
********************************************************************************
* Copyright (c) 2020-21, 2025 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef AD7134_IIO_H_
#define AD7134_IIO_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "iio.h"
#include "iio_types.h"
#include "ad713x.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

/* AD7134 global device instance for accessing device specific APIs */
extern struct ad713x_dev *p_ad7134_dev_inst;

/* Init the IIO interface */
int32_t ad7134_iio_initialize(void);

/* Run the IIO event handler */
void ad7134_iio_event_handler(void);

#endif /* AD7134_IIO_H_ */
