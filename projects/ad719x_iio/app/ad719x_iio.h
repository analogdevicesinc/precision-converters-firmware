/***************************************************************************//**
* @file   ad719x_iio.h
* @brief  Header file for ad719x IIO interface
********************************************************************************
* Copyright (c) 2021-22,2024 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef AD719X_IIO_H_
#define AD719X_IIO_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "iio.h"
#include "iio_types.h"
#include "ad719x.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* Additional bit macros */
#define REAL_BITS               ADC_RESOLUTION
#define STORAGE_BITS            32
#define MODE_BIT_POSTION        21
#define BPDSW_BIT_POSTION       6
#define CNV_START_CMD           0x5C
#define CNV_STOP_CMD            0x58
#define BYTES_TRANSFER_THREE    3
/* For AD7190/2/4/5 the channel mask needs to shifted by 4,
 * when operating in pseudo differential mode.*/
#define AD719X_CHN_SHIFT    4

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

/* ad719x global device instance for accessing device specific APIs */
extern struct ad719x_dev *p_ad719x_dev_inst;

/* Init the IIO interface */
int32_t ad719x_iio_initialize(void);

/* Run the IIO event handler */
void ad719x_iio_event_handler(void);

#endif /* AD719X_IIO_H_ */
