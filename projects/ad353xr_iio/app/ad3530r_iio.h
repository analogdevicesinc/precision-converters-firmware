/*************************************************************************//**
 *   @file   ad3530r_iio.h
 *   @brief  Header for AD3530R IIO aplication
******************************************************************************
* Copyright (c) 2022-23 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD3530R_IIO_H
#define AD3530R_IIO_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include "iio.h"
#include "app_config.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/**
 * @enum reg_access_mode
 * @brief Register access modes
 */
enum reg_access_mode {
	SINGLE_INSTRUCTION_MODE,
	STREAMING_MODE
};

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* AD3530r global device instance for accessing device specific APIs */
extern struct ad3530r_desc *ad3530r_dev_desc;
extern uint8_t streaming_option;
extern uint8_t* global_iio_buff;
extern uint32_t num_of_samples;

int32_t ad3530r_iio_initialize(void);
void ad3530r_iio_event_handler(void);

#endif /* AD3530R_IIO_H */
