/***************************************************************************//**
 *   @file    app_support.h
 *   @brief   Implementation of AD405x support functions
 *   @details This module has all the support file necessary for working of AD405x
********************************************************************************
 * Copyright (c) 2022-2025 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef AD405X_SUPPORT_H_
#define AD405X_SUPPORT_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>

#include "iio.h"
#include "app_config.h"
#include "ad405x.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* Timeout count to avoid stuck into potential infinite loop while checking
 * for new data whenever the BUSY pin goes low. The actual timeout factor is determined
 * through 'sampling_frequency' attribute of IIO app, but this period here makes sure
 * we are not stuck into a forever loop in case data capture is interrupted
 * or failed in between.
 * Note: This timeout factor is dependent upon the MCU clock frequency. Below timeout
 * is tested for SDP-K1 platform @180Mhz default core clock */
#define BUF_READ_TIMEOUT	0xffffffff

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
struct ad405x_support_desc {
	/** Called when buffer ready to transfer. Write/read to/from dev */
	int32_t	(*submit)(struct iio_device_data *dev);
	/** Called before enabling buffer */
	int32_t (*pre_enable)(void *dev, uint32_t mask);
	/** Called after disabling buffer */
	int32_t (*post_disable)(void *dev);
	/** Called after a trigger signal has been received by iio */
	int32_t (*trigger_handler)(struct iio_device_data *dev);
};

extern const struct ad405x_support_desc *support_desc[];

#endif /* AD405X_SUPPORT_H_ */
