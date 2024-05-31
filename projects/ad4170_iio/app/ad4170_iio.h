/***************************************************************************//**
*   @file   ad4170_iio.h
*   @brief  Header file of ad4170_iio
********************************************************************************
* Copyright (c) 2021-22 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef _AD4170_IIO_H_
#define _AD4170_IIO_H_

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

/* ADC channels assigned to sensors for the measurement (each channel per sensor) */
enum sensor_channels {
	SENSOR_CHANNEL0,
	SENSOR_CHANNEL1,
	SENSOR_CHANNEL2,
	SENSOR_CHANNEL3,
	NUM_OF_SENSOR_CHANNELS
};

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

/* AD4170 global device instance for accessing device specific APIs */
extern struct ad4170_dev *p_ad4170_dev_inst;

/* Init the IIO interface */
int32_t ad4170_iio_initialize(void);

/* Run the IIO event handler */
void ad4170_iio_event_handler(void);

#endif /* _AD4170_IIO_H_ */
