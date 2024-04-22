/*************************************************************************//**
 *   @file   ltc2672_iio.h
 *   @brief  Header file for LTC2672 IIO interface
******************************************************************************
* Copyright (c) 2023 Analog Devices, Inc.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef LTC2672_IIO_H
#define LTC2672_IIO_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* LTC2672 global device instance for accessing device specific APIs */
extern struct ltc2672_dev *ltc2672_dev_desc;

/* Init the IIO interface */
int32_t ltc2672_iio_init(void);

/* Run the IIO event handler */
void ltc2672_iio_event_handler(void);

#endif /* LTC2672_IIO_H */