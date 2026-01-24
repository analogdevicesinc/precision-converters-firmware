/*************************************************************************//**
 *   @file   ad5706r_iio.h
 *   @brief  Header file for AD5706R IIO interface
******************************************************************************
* Copyright (c) 2024-2026 Analog Devices, Inc.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD5706_IIO_H
#define AD5706_IIO_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include "ad5706r.h"
/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

#define NUM_IIO_DEVICES				2

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* AD5706R global device instance for accessing device specific APIs */
extern struct ad5706r_dev *ad5706r_dev_inst[NUM_IIO_DEVICES];

/* Init the IIO interface */
int32_t ad5706r_iio_initialize(void);

/* Run the IIO event handler */
void ad5706r_iio_event_handler(void);

#endif /* AD5706_IIO_H */
