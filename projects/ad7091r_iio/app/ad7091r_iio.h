/*************************************************************************//**
 *   @file   ad7091r_iio.h
 *   @brief  Header file for AD7091R IIO interface
******************************************************************************
* Copyright (c) 2024 Analog Devices, Inc.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD7091R_IIO_H
#define AD7091R_IIO_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "iio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* AD7091R global device instance for accessing device specific APIs */
extern struct ad7091r8_dev *ad7091r_dev_desc;
extern volatile bool ad7091r_conversion_flag;

/* Init the IIO interface */
int ad7091r_iio_init(void);

/* Run the IIO event handler */
void ad7091r_iio_event_handler(void);

#endif /* AD7091R_IIO_H */