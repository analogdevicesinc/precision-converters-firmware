/***************************************************************************//**
 *   @file    ad552xr_support.h
 *   @brief   Header file for the AD552XR IIO Application Support
********************************************************************************
 * Copyright (c) 2026 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef AD552XR_SUPPORT_H_
#define AD552XR_SUPPORT_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>
#include "iio.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
/* Support functions */
int32_t ad552xr_set_sampling_rate(uint32_t val);
int32_t ad552xr_get_sampling_rate(uint32_t *val);

/* Data transfer functions */
int32_t ad552xr_data_transfer_system_init();
int32_t ad552xr_data_transfer_prepare(void *dev, uint32_t mask);
int32_t ad552xr_data_transfer_start(struct iio_device_data *iio_dev_data,
				    uint8_t *dst_data_buffer);
int32_t ad552xr_data_transfer_stop(void *dev);
int32_t ad552xr_data_transfer_system_remove();

#endif /* AD552XR_SUPPORT_H_ */
