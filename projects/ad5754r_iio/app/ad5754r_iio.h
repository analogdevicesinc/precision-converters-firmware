/*************************************************************************//**
 *   @file   ad5754r_iio.h
 *   @brief  Header file for AD5754R IIO interface
******************************************************************************
* Copyright (c) 2024 Analog Devices, Inc.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD5754R_IIO_H
#define AD5754R_IIO_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include "app_config.h"
/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* AD5754R global device instance for accessing device specific APIs */
extern struct ad5754r_dev *ad5754r_dev_inst;

/* Init the IIO interface */
int32_t ad5754r_iio_init(void);

/* Run the IIO event handler */
void ad5754r_iio_event_handler(void);

#endif /* AD5754R_IIO_H */