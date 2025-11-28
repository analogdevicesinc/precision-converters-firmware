/***************************************************************************//*
 * @file    stm32_tdm_support.c
 * @brief   Wrapper file for TDM-DMA Data capturing
******************************************************************************
 * Copyright (c) 2023 Analog Devices, Inc. All Rights Reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "stm32_tdm_support.h"

/******************************************************************************/
/********************* Macros and Constants Definitions ***********************/
/******************************************************************************/


/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/* Flag to denote if the DMA buffer is full */
volatile bool dma_buffer_full;

/* Pointer to the circular buffer */
uint8_t *dma_buff;

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/

/**
 * @brief Read TDM DMA Data into the buffer
 * @param tdm_desc[in] - TDM descriptor
 * @param iio_dev_data[in] - IIO device data instance
 * @param buffer_size[in] - Buffer Size
 * @param bytes_per_sample[in] - Byte Per Sample
 * @param n_samples_tdm_read[in] - Number of samples to be read by TDM
 * @return	0 in case of success, negative error code otherwise
 */
int32_t start_tdm_dma_to_cb_transfer(struct no_os_tdm_desc *tdm_desc,
				     struct iio_device_data *iio_dev_data, uint32_t buffer_size,
				     uint8_t bytes_per_sample, uint32_t n_samples_tdm_read)
{
	uint32_t buff_available_size;
	int32_t ret;

	/* Prepare Circular Buffer for a write operation- retrieve the
	 * pointer to the circular buffer */
	ret = no_os_cb_prepare_async_write(iio_dev_data->buffer->buf,
					   buffer_size * bytes_per_sample,
					   (void **)&dma_buff, &buff_available_size);
	if (ret) {
		return ret;
	}

	/* Trigger TDM read via DMA. Once initiated, the DMA read operation
	 * continues to fill up the buffer in the background, automatically updating
	 * the buffer index to the start after the buffer is filled up */
	ret = no_os_tdm_read(tdm_desc, dma_buff, n_samples_tdm_read << 1);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Update the TDM DMA buffer overflow flag
 * @return None
 */
void update_dma_buffer_overflow(void)
{
	dma_buffer_full = true;
}

/**
 * @brief Update circular buffer indices and prepare for next async write via DMA
 * @param tdm_desc[in] - TDM Descriptor
 * @param iio_dev_data[in] - IIO Device data instance
 * @param buffer_size[in] - Buffer Size
 * @param bytes_per_sample[in] - Byte Per Sample
 * @return 0 in case of Success, negative error code otherwise
 */
int32_t end_tdm_dma_to_cb_transfer(struct no_os_tdm_desc *tdm_desc,
				   struct iio_device_data *iio_dev_data,
				   uint32_t buffer_size, uint8_t bytes_per_sample)
{
	uint32_t buff_available_size;
	int32_t ret;

	/* End Circular Buffer write operation and update pointer index for
	 * the next cycle of write */
	ret = no_os_cb_end_async_write(iio_dev_data->buffer->buf);
	if (ret) {
		return ret;
	}

	/* Prepare Circular Buffer for a write operation- retrieve the
	 * updated pointer to the circular buffer post one cycle of buffer
	 * write overflow */
	ret = no_os_cb_prepare_async_write(iio_dev_data->buffer->buf,
					   buffer_size * bytes_per_sample,
					   (void **)&dma_buff, &buff_available_size);
	if (ret) {
		return ret;
	}

	return 0;
}
