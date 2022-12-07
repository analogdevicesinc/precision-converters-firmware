/***************************************************************************//**
*   @file   ad4130_iio.h
*   @brief  Header file of AD4130 IIO interface
********************************************************************************
* Copyright (c) 2020, 2022 Analog Devices, Inc.
*
* All rights reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef AD4130_IIO_H_
#define AD4130_IIO_H_

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

/* ADC channels assigned to sensors for the measurement
 * (each channel per sensor) */
enum sensor_channels {
	SENSOR_CHANNEL0,
	SENSOR_CHANNEL1,
	NUM_OF_SENSOR_CHANNELS
};

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

/* Init the IIO interface */
int32_t ad4130_iio_initialize(void);

/* Run the IIO event handler */
void ad4130_iio_event_handler(void);

extern struct ad413x_dev *ad4130_dev_inst;

#endif /* AD4130_IIO_H_ */
