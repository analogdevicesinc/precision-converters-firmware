/***************************************************************************//**
* @file   ad7689_iio.h
* @brief  Header file for AD7689 IIO interface
********************************************************************************
* Copyright (c) 2021 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef _AD7689_IIO_H_
#define _AD7689_IIO_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "iio.h"
#include "iio_types.h"
#include "ad7689.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

/* AD7689 global device instance for accessing device specific APIs */
extern struct ad7689_dev *p_ad7689_dev_inst;

/* AD7689 current device config */
extern struct ad7689_config ad7689_current_config;

/* Init the IIO interface */
int32_t ad7689_iio_initialize(void);

/* Run the IIO event handler */
void ad7689_iio_event_handler(void);

#endif /* _AD7689_IIO_H_ */
