/***************************************************************************//**
*   @file   stm32_tdm_support.h
*   @brief  Header file for STM32 TDM-DMA Data Capture Wrapper file
********************************************************************************
* Copyright (c) 2023, 2025 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef STM32_TDM_SUPPORT_H_
#define STM32_TDM_SUPPORT_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include <stdbool.h>
#include "no_os_circular_buffer.h"
#include "iio_types.h"
#include "no_os_tdm.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

extern volatile bool dma_buffer_full;
extern uint8_t *dma_buff;
int32_t start_tdm_dma_to_cb_transfer(struct no_os_tdm_desc *tdm_desc,
				     struct iio_device_data *iio_dev_data, uint32_t buffer_size,
				     uint8_t bytes_per_sample, uint32_t n_samples_tdm_read);
void update_dma_buffer_overflow(void);
int32_t end_tdm_dma_to_cb_transfer(struct no_os_tdm_desc *tdm_desc,
				   struct iio_device_data *iio_dev_data,
				   uint32_t buffer_size, uint8_t bytes_per_sample);

#endif // STM32_TDM_SUPPORT_H_
