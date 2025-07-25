/***************************************************************************//**
 *   @file    ad406x_support.c
 *   @brief   Implementation of AD406x support functions
 *   @details This module has all the support file necessary for working of AD406x
********************************************************************************
 * Copyright (c) 2025 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <stdint.h>

#include "iio.h"
#include "iio_trigger.h"
#include "app_support.h"
#include "ad405x_iio.h"

#include "no_os_error.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Maximum value the DMA NDTR register can take */
/*
 * Note: Since half transfer complete is used, the size has to be 8B aligned.
 * This is because when Burst Averaging mode is used, the sample get divided
 * between between the 2 halves of the DMA buffer.
 * Thus sizeof(uint32_t) * 2 is used .
 */
#define ALIGN_SIZE						(sizeof(uint32_t) * 2)
#define MAX_DMA_BYTES					(((uint32_t)(64000 / ALIGN_SIZE)) * \
										ALIGN_SIZE)

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* Array to store the data locally */
uint8_t local_adc_data[MAX_DMA_BYTES];

/* Flag to indicate if DMA has been configured for windowed capture */
static volatile bool dma_config_updated = false;

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/* ad405x IIO hw trigger descriptor */
extern struct iio_hw_trig *ad405x_hw_trig_desc;

extern uint8_t bytes_per_sample;

#if APP_CAPTURE_MODE == WINDOWED_DATA_CAPTURE

/**
 * @brief  Prepares the device for data transfer.
 * @param  dev[in, out]- Application descriptor.
 * @param  mask[in]- Number of bytes to transfer.
 * @return 0 in case of success, error code otherwise.
 */
static int32_t ad406x_pre_enable_windowed(void *dev, uint32_t mask)
{
	int32_t ret;
	uint32_t adc_data;

	if (ad405x_interface_mode == I3C_DMA) {
		ret = ad405x_set_operation_mode(p_ad405x_dev, ad405x_operating_mode);
		if (ret) {
			return ret;
		}

		/* Disable the PWM generation */
		ret = no_os_pwm_disable(pwm_desc);
		if (ret) {
			return ret;
		}

		/*
		 * Read the data to update the address pointer of
		 * ADC to point to the necessary data register.
		 */
		ret = ad405x_read(p_ad405x_dev,
				  AD405X_REG_CONV_READ(bytes_per_sample - 1),
				  (uint8_t *)&adc_data,
				  bytes_per_sample);
		if (ret)
			return ret;
	}

	return 0;
}

/**
 * @brief  Terminate current data transfer.
 * @param  dev[in, out]- Application descriptor.
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad406x_post_disable_windowed(void *dev)
{
	int32_t ret;

	if (ad405x_interface_mode == I3C_DMA) {
		/* Abort the I3C transaction along with the DMA */
		ret = no_os_i3c_transfer_abort(p_ad405x_dev->com_desc.i3c_desc);
		if (ret)
			return ret;

		dma_config_updated = false;
		buf_size_updated = false;
	}

	return 0;
}

/**
 * @brief Writes all the samples from the ADC buffer into the
		  IIO buffer.
 * @param iio_dev_data[in] - IIO device data instance.
 * @return Number of samples read.
 */
static int32_t ad406x_submit_windowed(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t nb_of_samples;
	uint32_t timeout = BUF_READ_TIMEOUT;
	uint32_t adc_data;
	uint32_t sample_index = 0;
	uint8_t gp1_value;

	data_ready = false;
	nb_of_samples = iio_dev_data->buffer->size / bytes_per_sample;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	if (ad405x_interface_mode == I3C_INTR) {
		ret = ad405x_set_operation_mode(p_ad405x_dev, ad405x_operating_mode);
		if (ret) {
			return ret;
		}

		ret = ad405x_get_raw(p_ad405x_dev, &adc_data);
		if (ret) {
			return ret;
		}

		ret = no_os_pwm_enable(pwm_desc);
		if (ret) {
			return ret;
		}

		while (sample_index < nb_of_samples) {
			/* Wait for the pwm completion interrupt */
			while (data_ready != true && timeout > 0) {
				timeout--;
			}
			/* Reset the data ready flag to avoid unnecessary entries into the loop */
			data_ready = false;

			if (!timeout) {
				return -EIO;
			}

			/* Check if the GP1 pin is low. Logic 0 means data ready */
			ret = no_os_gpio_get_value(p_ad405x_dev->gpio_gpio1, &gp1_value);
			if (ret) {
				return ret;
			}

			if (gp1_value == NO_OS_GPIO_HIGH) {
				/*
				 * If the control reaches here, then it means the CNV PWM pulse is faster than
				 * the device can convert a data.
				 */
				return -EBUSY;
			}

			/* Read the data */
			ret = ad405x_get_raw(p_ad405x_dev, &adc_data);
			if (ret) {
				return ret;
			}

			ret = no_os_cb_write(iio_dev_data->buffer->buf, &adc_data, bytes_per_sample);
			if (ret) {
				return ret;
			}

			sample_index++;
		}

		ret = no_os_pwm_disable(pwm_desc);
		if (ret) {
			return ret;
		}
	} else if (ad405x_interface_mode == I3C_DMA) {
		nb_of_bytes_g = (nb_of_samples + 1) * bytes_per_sample;
		iio_dev_data_g = iio_dev_data;
		data_ready = false;

		ret = no_os_cb_prepare_async_write(iio_dev_data->buffer->buf,
						   nb_of_bytes_g - bytes_per_sample,
						   (void **) &buff_start_addr,
						   &data_read);
		if (ret) {
			return ret;
		}
		/*
		 * AD406x devices (I3C devices) start a transaction when the CONV_READ
		 * register is read. So everytime we read ADC data it is the result of
		 * previous conversion. In windowed mode of data capture this would
		 * create a gap between the 1st and the 2nd data. To remove this break in
		 * continuity of the data, 1 extra sample is reserved in the beginning of
		 * adc_data_buffer and is used by I3C to read an extra sample.
		 * So effectively for a request of N samples from the application the
		 * firmware reads 1+N samples and drops the 1st sample.
		 */
		if (!dma_config_updated) {
			/* Cap I3C RX DMA NDTR to MAX_DMA_NDTR. */
			rxdma_ndtr = no_os_min(MAX_DMA_BYTES, nb_of_bytes_g);

			struct no_os_i3c_msg i3c_data_read_msg = {
				.tx_buff = NULL,
				.tx_size = 0,
				.rx_buff = local_adc_data,
				.rx_size = rxdma_ndtr
			};
			ret = no_os_i3c_transfer_dma_async(p_ad405x_dev->com_desc.i3c_desc,
							   &i3c_data_read_msg, 1, NULL, NULL);
			if (ret)
				return ret;

			/* Stop the previous CR transaction */
			ret = no_os_dma_xfer_abort(ad405x_dma_desc, &ad405x_dma_desc->channels[0]);
			if (ret)
				return ret;

			dma_config_updated = true;

			/* Disable requested interrupts */
			__HAL_I3C_DISABLE_IT(&I3C_HANDLE, (HAL_I3C_IT_FCIE | HAL_I3C_IT_ERRIE));

			AD405x_RxDMA_HANDLE.XferCpltCallback = receivecomplete_callback;

			/* Set the trigger for TC DMA to write the address */
			ret = no_os_dma_config_xfer(ad405x_dma_desc,
						    &i3c_cr_dma_xfer,
						    1,
						    &ad405x_dma_desc->channels[0]);
			if (ret)
				return ret;

			/* Start the DMA */
			ret = no_os_dma_xfer_start(ad405x_dma_desc, &ad405x_dma_desc->channels[0]);
			if (ret)
				return ret;

			/* Update the Buffers only when the DMA has been configured again */
			update_buff(local_adc_data, (uint8_t *)(buff_start_addr - bytes_per_sample));
		}

		dma_cycle_count = (nb_of_bytes_g + (rxdma_ndtr / 2) - 1) / (rxdma_ndtr / 2);
		nb_of_bytes_remaining_g = nb_of_bytes_g - ((rxdma_ndtr / 2) *
					  (dma_cycle_count - 1));

		ret = no_os_pwm_enable(pwm_desc);
		if (ret) {
			return ret;
		}

		while (!data_ready && timeout > 0) {
			timeout--;
		}

		if (!timeout) {
			return -EIO;
		}

		no_os_cb_end_async_write(iio_dev_data->buffer->buf);
	}

	return 0;
}

#endif

#if APP_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE

/**
 * @brief  Prepares the device for data transfer.
 * @param  dev[in, out]- Application descriptor.
 * @param  mask[in]- Number of bytes to transfer.
 * @return 0 in case of success, error code otherwise.
 */
static int32_t ad406x_pre_enable_continuous(void *dev, uint32_t mask)
{
	int32_t ret;
	uint32_t adc_data;

	ret = ad405x_set_operation_mode(p_ad405x_dev, ad405x_operating_mode);
	if (ret) {
		return ret;
	}

	/* Disable the PWM generation */
	ret = no_os_pwm_disable(pwm_desc);
	if (ret) {
		return ret;
	}

	if (ad405x_interface_mode == I3C_INTR) {
		/* Read the data to start a new conversion */
		ret = ad405x_get_raw(p_ad405x_dev, &adc_data);
		if (ret) {
			return ret;
		}

		ret = iio_trig_enable(ad405x_hw_trig_desc);
		if (ret) {
			return ret;
		}

		ret = no_os_pwm_enable(pwm_desc);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief  Terminate current data transfer.
 * @param  dev[in, out]- Application descriptor.
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad406x_post_disable_continuous(void *dev)
{
	int32_t ret;

	ret = no_os_pwm_disable(pwm_desc);
	if (ret) {
		return ret;
	}

	if (ad405x_interface_mode == I3C_INTR) {
		ret = iio_trig_disable(ad405x_hw_trig_desc);
		if (ret) {
			return ret;
		}
	} else if (ad405x_interface_mode == I3C_DMA) {
		/* Abort the I3C transaction along with the DMA */
		ret = no_os_i3c_transfer_abort(p_ad405x_dev->com_desc.i3c_desc);
		if (ret)
			return ret;

		dma_config_updated = false;
	}

	buf_size_updated = false;

	/*
	 * NOTE: Exit command is skipped since AD406x doesn't
	 * have any exit command
	 */

	return 0;
}

/**
 * @brief Writes all the samples from the ADC buffer into the
		  IIO buffer. Only I3C DMA function reaches here.
 * @param iio_dev_data[in] - IIO device data instance.
 * @return Number of samples read.
 */
static int32_t ad406x_submit_continuous(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t adc_data;
	uint32_t nb_of_samples;

	nb_of_samples = iio_dev_data->buffer->size / bytes_per_sample;
	nb_of_bytes_g = nb_of_samples * bytes_per_sample;
	iio_dev_data_g = iio_dev_data;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	if (!dma_config_updated) {

		ret = no_os_cb_prepare_async_write(iio_dev_data->buffer->buf,
						   nb_of_bytes_g,
						   (void **) &buff_start_addr,
						   &data_read);
		if (ret) {
			return ret;
		}

		/* Cap I3C RX DMA NDTR to MAX_DMA_NDTR. */
		rxdma_ndtr = no_os_min(MAX_DMA_BYTES, nb_of_bytes_g);

		struct no_os_i3c_msg i3c_data_read_msg = {
			.tx_buff = NULL,
			.tx_size = 0,
			.rx_buff = (uint8_t *)buff_start_addr,
			.rx_size = rxdma_ndtr
		};

		/*
		 * Read the data to start a new conversion and Update the address pointer of
		 * ADC to point to the necessary data register.
		 */
		ret = ad405x_read(p_ad405x_dev,
				  AD405X_REG_CONV_READ(bytes_per_sample - 1),
				  (uint8_t *)&adc_data,
				  bytes_per_sample);
		if (ret)
			return ret;

		ret = no_os_i3c_transfer_dma_async(p_ad405x_dev->com_desc.i3c_desc,
						   &i3c_data_read_msg, 1, NULL, NULL);
		if (ret)
			return ret;
		/* Stop the previous CR transaction */
		ret = no_os_dma_xfer_abort(ad405x_dma_desc, &ad405x_dma_desc->channels[0]);
		if (ret)
			return ret;

		/* Set the trigger for TC DMA to write the address */
		ret = no_os_dma_config_xfer(ad405x_dma_desc,
					    &i3c_cr_dma_xfer,
					    1,
					    &ad405x_dma_desc->channels[0]);
		if (ret)
			return ret;

		/* Start the DMA */
		/*
		 * Note: Since the cnv timer (trigger to TX DMA) is disabled,
		 * the DMA will not transfer any data even if requested */
		ret = no_os_dma_xfer_start(ad405x_dma_desc, &ad405x_dma_desc->channels[0]);
		if (ret)
			return ret;

		/* Disable requested interrupts */
		__HAL_I3C_DISABLE_IT(&I3C_HANDLE, (HAL_I3C_IT_FCIE | HAL_I3C_IT_ERRIE));

		ret = no_os_pwm_enable(pwm_desc);
		if (ret) {
			return ret;
		}

		dma_config_updated = true;
	}

	return 0;
}

/**
 * @brief	Reads data from the ADC and pushes it into IIO buffer when the
			IRQ is triggered. Only I3C Interrupt reaches here.
 * @param	iio_dev_data[in] - IIO device data instance.
 * @return	0 in case of success or negative value otherwise.
 */
static int32_t ad406x_trigger_handler_continuous(struct iio_device_data
		*iio_dev_data)
{
	int32_t ret;
	uint8_t gp1_value;
	uint32_t adc_data;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
						   iio_dev_data->buffer->bytes_per_scan)) * iio_dev_data->buffer->bytes_per_scan;
		buf_size_updated = true;
	}

	/* Check if the GP1 pin is low. Logic 0 means data ready */
	ret = no_os_gpio_get_value(p_ad405x_dev->gpio_gpio1, &gp1_value);
	if (ret) {
		return ret;
	}

	if (gp1_value == NO_OS_GPIO_HIGH) {
		/*
		 * If the control reaches here, then it means the CNV PWM pulse is faster than
		 * the device can convert a data.
		 */
		return -EBUSY;
	}

	/* Read the data */
	ret = ad405x_get_raw(p_ad405x_dev, &adc_data);
	if (ret) {
		return ret;
	}

	return no_os_cb_write(iio_dev_data->buffer->buf, &adc_data, bytes_per_sample);
}

#endif


#if APP_CAPTURE_MODE == WINDOWED_DATA_CAPTURE
const struct ad405x_support_desc ad406x_support_descriptor = {
	.submit = ad406x_submit_windowed,
	.pre_enable = ad406x_pre_enable_windowed,
	.post_disable = ad406x_post_disable_windowed,
	.trigger_handler = NULL
};
#endif
#if APP_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE
const struct ad405x_support_desc ad406x_support_descriptor = {
	.submit = ad406x_submit_continuous,
	.pre_enable = ad406x_pre_enable_continuous,
	.post_disable = ad406x_post_disable_continuous,
	.trigger_handler = ad406x_trigger_handler_continuous
};
#endif
