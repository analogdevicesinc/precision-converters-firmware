/***************************************************************************//**
 *   @file    ad405x_support.c
 *   @brief   Implementation of AD405x support functions
 *   @details This module has all the support file necessary for working of AD405x
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
#include <stdbool.h>

#include "iio.h"
#include "iio_trigger.h"
#include "app_support.h"
#include "ad405x_iio.h"
#include "ad405x_user_config.h"

#include "no_os_error.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Maximum size of the local SRAM buffer */
#define MAX_LOCAL_BUF_SIZE	64000

/* Maximum value the DMA NDTR register can take */
#define MAX_DMA_NDTR		(no_os_min(65535, MAX_LOCAL_BUF_SIZE/2))

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* Local SRAM buffer */
uint8_t local_buf[MAX_LOCAL_BUF_SIZE];

/* Flag to indicate if DMA has been configured for windowed capture */
static volatile bool dma_config_updated = false;

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
static volatile bool buf_size_updated = false;

/* ad405x IIO hw trigger descriptor */
extern struct iio_hw_trig *ad405x_hw_trig_desc;

extern uint8_t bytes_per_sample;

/*
 * @brief  Reconfigures the SPI interface for data transfer.
 * @param  enable_stream[in] - Flag to indicate streaming.
 * @return 0 in case of success, error code otherwise.
 */
static int32_t ad405x_spi_reconfigure(bool enable_stream)
{
	int32_t ret = 0;
	/* SPI init params */
	struct no_os_spi_init_param *spi_init_param;
	struct stm32_spi_init_param *stm32_spi_init_param;

	spi_init_param = ad405x_init_params.comm_init.spi_init;
	stm32_spi_init_param = (struct stm32_spi_init_param *) spi_init_param->extra;

	if (enable_stream) {
		spi_init_param->max_speed_hz = MAX_SPI_SCLK_45MHz;
		stm32_spi_init_param->dma_init = &ad405x_dma_init_param;
		stm32_spi_init_param->irq_num = Rx_DMA_IRQ_ID;
		stm32_spi_init_param->rxdma_ch = &spi_dma_rxdma_channel;
		stm32_spi_init_param->txdma_ch = &spi_dma_txdma_channel;
	} else {
		spi_init_param->max_speed_hz = MAX_SPI_SCLK;
		stm32_spi_init_param->dma_init = NULL;
	}

	if (p_ad405x_dev->com_desc.spi_desc) {
		ret = no_os_spi_remove(p_ad405x_dev->com_desc.spi_desc);
		if (ret) {
			return ret;
		}
	}
	ret = no_os_spi_init(&p_ad405x_dev->com_desc.spi_desc, spi_init_param);
	if (ret) {
		return ret;
	}

	/* Use 16-bit SPI Data Frame Format during data capture */
	/* Revert to 8-bit SPI Data Frame Format after data capture */
	stm32_config_spi_data_frame_format(enable_stream);

	return 0;
}

#if APP_CAPTURE_MODE == WINDOWED_DATA_CAPTURE
/**
 * @brief  Prepares the device for data transfer.
 * @param  dev[in, out]- Application descriptor.
 * @param  mask[in]- Number of bytes to transfer.
 * @return 0 in case of success, error code otherwise.
 */
static int32_t ad405x_pre_enable_windowed(void *dev, uint32_t mask)
{
	int32_t ret;

	if (ad405x_interface_mode == SPI_DMA) {
		ret = ad405x_set_operation_mode(p_ad405x_dev, ad405x_operating_mode);
		if (ret) {
			return ret;
		}

		ret = ad405x_spi_reconfigure(true);
		if (ret) {
			return ret;
		}

		/* Configure CS and CNV gpios for alternate functionality as
		 * Timer PWM outputs */
		stm32_cs_output_gpio_config(false);
	}

	return 0;
}

/**
 * @brief  Terminate current data transfer.
 * @param  dev[in, out]- Application descriptor.
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad405x_post_disable_windowed(void *dev)
{
	int32_t ret;

	if (ad405x_interface_mode == SPI_DMA) {
		stm32_timer_stop();

		/* Abort DMA and Timers and configure CS and CNV as GPIOs */
		no_os_spi_transfer_abort(p_ad405x_dev->com_desc.spi_desc);
		stm32_cs_output_gpio_config(true);

		ret = ad405x_spi_reconfigure(false);
		if (ret) {
			return ret;
		}

		dma_config_updated = false;
		buf_size_updated = false;

		ret = ad405x_exit_command(p_ad405x_dev);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Writes all the samples from the ADC buffer into the
		  IIO buffer.
 * @param iio_dev_data[in] - IIO device data instance.
 * @return Number of samples read.
 */
static int32_t ad405x_submit_windowed(struct iio_device_data *iio_dev_data)
{
	uint32_t timeout = BUF_READ_TIMEOUT;
	uint32_t adc_data;
	int32_t ret;
	uint32_t nb_of_samples;

	data_ready = false;
	nb_of_samples = iio_dev_data->buffer->size / bytes_per_sample;
	nb_of_bytes_g = nb_of_samples * bytes_per_sample;
	iio_dev_data_g = iio_dev_data;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	if (ad405x_interface_mode == SPI_INTR) {
		ret = ad405x_set_operation_mode(p_ad405x_dev, ad405x_operating_mode);
		if (ret) {
			return ret;
		}

		/*
		 * Clear any pending event that occurs from a unintended
		 * falling edge of busy pin before enabling the interrupt
		 */
		ret = no_os_irq_clear_pending(trigger_irq_desc, TRIGGER_INT_ID_SPI_INTR);
		if (ret) {
			return ret;
		}

		ret = no_os_irq_enable(trigger_irq_desc, TRIGGER_INT_ID_SPI_INTR);
		if (ret) {
			return ret;
		}

		ret = no_os_pwm_enable(pwm_desc);
		if (ret) {
			return ret;
		}
		while (nb_of_samples--) {
			while (data_ready != true && timeout > 0) {
				timeout--;
			}

			if (!timeout) {
				return -EIO;
			}

			ret = ad405x_get_raw(p_ad405x_dev, &adc_data);
			if (ret) {
				return ret;
			}

			ret = no_os_cb_write(iio_dev_data->buffer->buf, &adc_data, bytes_per_sample);
			if (ret) {
				return ret;
			}

			data_ready = false;
		}

		ret = no_os_pwm_disable(pwm_desc);
		if (ret) {
			return ret;
		}

		ret = no_os_irq_disable(trigger_irq_desc, TRIGGER_INT_ID_SPI_INTR);
		if (ret) {
			return ret;
		}

		buf_size_updated = false;

		ret = ad405x_exit_command(p_ad405x_dev);
		if (ret) {
			return ret;
		}
	} else {
		ret = no_os_cb_prepare_async_write(iio_dev_data->buffer->buf,
						   nb_of_samples * (bytes_per_sample),
						   (void **) &buff_start_addr,
						   &data_read);
		if (ret) {
			return ret;
		}

		if (!dma_config_updated) {
			/* Cap SPI RX DMA NDTR to MAX_DMA_NDTR. */
			rxdma_ndtr = no_os_min(MAX_DMA_NDTR, nb_of_samples);

			/* Register half complete callback, for ping-pong buffers implementation. */
			HAL_DMA_RegisterCallback(&hdma_spi1_rx,
						 HAL_DMA_XFER_HALFCPLT_CB_ID,
						 receivecomplete_callback);

			struct no_os_spi_msg ad405x_spi_msg = {
				.tx_buff = NULL,
				.rx_buff = local_buf,
				.bytes_number = rxdma_ndtr
			};

			ret = no_os_spi_transfer_dma_async(p_ad405x_dev->com_desc.spi_desc,
							   &ad405x_spi_msg,
							   1,
							   NULL,
							   NULL);
			if (ret) {
				return ret;
			}

			dma_config_updated = true;
			update_buff(local_buf, (uint8_t *)buff_start_addr);

			stm32_timer_enable();
		}

		dma_cycle_count = (nb_of_bytes_g + rxdma_ndtr - 1) / rxdma_ndtr;
		nb_of_bytes_remaining_g = nb_of_bytes_g - (rxdma_ndtr * (dma_cycle_count - 1));

		/* Enable TIM DMA request */
		no_os_pwm_enable(tx_trigger_desc);

		while (data_ready != true && timeout > 0) {
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
static int32_t ad405x_pre_enable_continuous(void *dev, uint32_t mask)
{
	int32_t ret;

	ret = ad405x_set_operation_mode(p_ad405x_dev, ad405x_operating_mode);
	if (ret) {
		return ret;
	}

	if (ad405x_interface_mode == SPI_INTR) {
		ret = no_os_pwm_enable(pwm_desc);
		if (ret) {
			return ret;
		}

		/* Clear any pending event that occurs from a unintended
		 * falling edge of busy pin before enabling the interrupt
		 */
		ret = no_os_irq_clear_pending(ad405x_hw_trig_desc->irq_ctrl,
					      ad405x_hw_trig_desc->irq_id);
		if (ret) {
			return ret;
		}

		ret = iio_trig_enable(ad405x_hw_trig_desc);
		if (ret) {
			return ret;
		}
	} else {

		ret = ad405x_spi_reconfigure(true);
		if (ret) {
			return ret;
		}

		/* Configure CS and CNV gpios for alternate functionality as
		 * Timer PWM outputs */
		stm32_cs_output_gpio_config(false);

	}

	return 0;
}

/**
 * @brief  Terminate current data transfer.
 * @param  dev[in, out]- Application descriptor.
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad405x_post_disable_continuous(void *dev)
{
	int32_t ret;

	if (ad405x_interface_mode == SPI_DMA) {

		/* Abort DMA and Timers and configure CS and CNV as GPIOs */
		stm32_timer_stop();
		no_os_spi_transfer_abort(p_ad405x_dev->com_desc.spi_desc);
		stm32_cs_output_gpio_config(true);

		ret = ad405x_spi_reconfigure(false);
		if (ret) {
			return ret;
		}

		dma_config_updated = false;

	} else {
		ret = no_os_pwm_disable(pwm_desc);
		if (ret) {
			return ret;
		}

		ret = iio_trig_disable(ad405x_hw_trig_desc);
		if (ret) {
			return ret;
		}
	}

	buf_size_updated = false;

	ret = ad405x_exit_command(p_ad405x_dev);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Writes all the samples from the ADC buffer into the
		  IIO buffer.
 * @param iio_dev_data[in] - IIO device data instance.
 * @return Number of samples read.
 */
static int32_t ad405x_submit_continuous(struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t nb_of_samples;

	data_ready = false;
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
						   nb_of_samples * (bytes_per_sample),
						   (void **) &buff_start_addr,
						   &data_read);
		if (ret) {
			return ret;
		}

		/* Cap SPI RX DMA NDTR to MAX_DMA_NDTR. */
		rxdma_ndtr = no_os_min(MAX_DMA_NDTR, nb_of_samples);

		struct no_os_spi_msg ad405x_spi_msg = {
			.tx_buff = NULL,
			.rx_buff = (uint8_t *)buff_start_addr,
			.bytes_number = rxdma_ndtr
		};

		ret = no_os_spi_transfer_dma_async(p_ad405x_dev->com_desc.spi_desc,
						   &ad405x_spi_msg,
						   1,
						   NULL,
						   NULL);
		if (ret) {
			return ret;
		}

		AD405x_RxDMA_HANDLE.XferCpltCallback = receivecomplete_callback;

		dma_config_updated = true;
		stm32_timer_enable();

		no_os_pwm_enable(tx_trigger_desc);
	}

	return 0;
}

/**
 * @brief	Reads data from the ADC and pushes it into IIO buffer when the
			IRQ is triggered.
 * @param	iio_dev_data[in] - IIO device data instance.
 * @return	0 in case of success or negative value otherwise.
 */
static int32_t ad405x_trigger_handler_continuous(struct iio_device_data
		*iio_dev_data)
{
	int32_t ret;
	uint32_t adc_data;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
						   iio_dev_data->buffer->bytes_per_scan)) * iio_dev_data->buffer->bytes_per_scan;
		buf_size_updated = true;
	}

	/* Read the sample for channel which has been sampled recently */
	ret = ad405x_get_raw(p_ad405x_dev, &adc_data);
	if (ret) {
		return ret;
	}

	return no_os_cb_write(iio_dev_data->buffer->buf, &adc_data, bytes_per_sample);
}

#endif


#if APP_CAPTURE_MODE == WINDOWED_DATA_CAPTURE
const struct ad405x_support_desc ad405x_support_descriptor = {
	.submit = ad405x_submit_windowed,
	.pre_enable = ad405x_pre_enable_windowed,
	.post_disable = ad405x_post_disable_windowed,
	.trigger_handler = NULL
};
#endif
#if APP_CAPTURE_MODE == CONTINUOUS_DATA_CAPTURE
const struct ad405x_support_desc ad405x_support_descriptor = {
	.submit = ad405x_submit_continuous,
	.pre_enable = ad405x_pre_enable_continuous,
	.post_disable = ad405x_post_disable_continuous,
	.trigger_handler = ad405x_trigger_handler_continuous
};
#endif
