/*************************************************************************//**
 *   @file   ad405x_iio.h
 *   @brief  Header for AD405X IIO application
******************************************************************************
* Copyright (c) 2023-2025 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD405X_IIO_H
#define AD405X_IIO_H

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
/* Enum of interface modes */
enum ad405x_interface_modes {
	SPI_DMA,
	SPI_INTR,
	I3C_DMA,
	I3C_INTR
};

extern struct ad405x_dev *p_ad405x_dev;
extern enum ad405x_interface_modes ad405x_interface_mode;
extern enum ad405x_operation_mode ad405x_operating_mode;

int32_t iio_ad405x_initialize(void);
void iio_ad405x_event_handler(void);
void data_capture_callback(void *context);
#endif /* AD405X_IIO_H */
