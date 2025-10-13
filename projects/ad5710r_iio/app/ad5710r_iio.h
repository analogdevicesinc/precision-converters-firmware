/*************************************************************************//**
 *   @file   ad5710r_iio.h
 *   @brief  Header for AD5710R IIO aplication
******************************************************************************
* Copyright (c) 2024-25 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD5710R_IIO_H
#define AD5710R_IIO_H

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
extern struct ad5710r_desc *ad5710r_dev_desc;
extern uint8_t streaming_option;
extern uint8_t* global_iio_buff;
extern uint32_t num_of_samples;

int32_t ad5710r_iio_initialize(void);
void ad5710r_iio_event_handler(void);

#endif /* AD5710R_IIO_H */
