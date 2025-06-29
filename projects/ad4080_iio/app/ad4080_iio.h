/*************************************************************************//**
 *   @file   ad4080_iio.h
 *   @brief  Header file for AD4080 IIO interface
******************************************************************************
* Copyright (c) 2023-25 Analog Devices, Inc.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD4080_IIO_H
#define AD4080_IIO_H

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

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
/* Initialize the IIO interface */
int32_t ad4080_iio_initialize(void);

/* Run the IIO event handler */
void ad4080_iio_event_handler(void);

#endif /* AD4080_IIO_H */
