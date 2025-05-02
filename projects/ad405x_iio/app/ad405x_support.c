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
#define MAX_LOCAL_BUF_SIZE	32000

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
	/* SPI init params */
	struct stm32_spi_init_param* spi_init_param;

	if (ad405x_interface_mode == SPI_DMA) {
		ret = ad405x_set_operation_mode(p_ad405x_dev, ad405x_operating_mode);
		if (ret) {
			return ret;
		}

		/* Switch to faster SPI SCLK and
		 * initialize Chip Select PWMs and DMA descriptors */
		ad405x_init_params.spi_init->max_speed_hz = MAX_SPI_SCLK_45MHz;
		spi_init_param = ad405x_init_params.spi_init->extra;
		spi_init_param->pwm_init = (const struct no_os_pwm_init_param *)&cs_init_params;
		spi_init_param->dma_init = &ad405x_dma_init_param;
		spi_init_param->irq_num = Rx_DMA_IRQ_ID;
		spi_init_param->rxdma_ch = &rxdma_channel;
		spi_init_param->txdma_ch = &txdma_channel;

		ret = no_os_spi_init(&p_ad405x_dev->spi_desc, ad405x_init_params.spi_init);
		if (ret) {
			return ret;
		}

		/* Use 16-bit SPI Data Frame Format during data capture */
		stm32_config_spi_data_frame_format(true);

		/* Configure CS and CNV gpios for alternate functionality as
		 * Timer PWM outputs */
		stm32_cs_output_gpio_config(false);

		ret = init_pwm();
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
static int32_t ad405x_post_disable_windowed(void *dev)
{
	int32_t ret;
	/* SPI init params */
	struct stm32_spi_init_param* spi_init_param;

	if (ad405x_interface_mode == SPI_DMA) {
		/* Revert to 8-bit SPI Data Frame Format after data capture */
		stm32_config_spi_data_frame_format(false);

		/* Abort DMA and Timers and configure CS and CNV as GPIOs */
		stm32_timer_stop();
		stm32_abort_dma_transfer();
		stm32_cs_output_gpio_config(true);

		spi_init_param = ad405x_init_params.spi_init->extra;
		spi_init_param->dma_init = NULL;
		ad405x_init_params.spi_init->max_speed_hz = MAX_SPI_SCLK;
		ret = no_os_spi_init(&p_ad405x_dev->spi_desc, ad405x_init_params.spi_init);
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
	int32_t adc_data;
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
		ret = init_pwm();
		if (ret) {
			return ret;
		}
		ret = no_os_pwm_enable(pwm_desc);
		if (ret) {
			return ret;
		}

		/*
		 * Clear any pending event that occurs from a unintended
		 * falling edge of busy pin before enabling the interrupt
		 */
		ret = no_os_irq_clear_pending(trigger_irq_desc, TRIGGER_INT_ID);
		if (ret) {
			return ret;
		}

		ret = no_os_irq_enable(trigger_irq_desc, TRIGGER_INT_ID);
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

			ret = ad405x_spi_data_read(p_ad405x_dev, &adc_data);
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

		ret = no_os_irq_disable(trigger_irq_desc, TRIGGER_INT_ID);
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
						 halfcmplt_callback);

			struct no_os_spi_msg ad405x_spi_msg = {
				.tx_buff = NULL,
				.rx_buff = local_buf,
				.bytes_number = rxdma_ndtr
			};

			struct stm32_spi_desc *sdesc = p_ad405x_dev->spi_desc->extra;
			ret = no_os_spi_transfer_dma_async(p_ad405x_dev->spi_desc,
							   &ad405x_spi_msg,
							   1,
							   NULL,
							   NULL);
			if (ret) {
				return ret;
			}

			/* Disable CS PWM and reset the counters */
			no_os_pwm_disable(sdesc->pwm_desc); // CS PWM
			htim2.Instance->CNT = 0;
			htim1.Instance->CNT = 0;
			TIM8->CNT = 0;

			dma_config_updated = true;
		}

		ad405x_conversion_flag = false;

		dma_cycle_count = (nb_of_samples + rxdma_ndtr - 1) / rxdma_ndtr;

		/* Set the callback count to twice the number of DMA cycles */
		callback_count = dma_cycle_count * 2;

		update_buff(local_buf, (uint8_t *)buff_start_addr);
		stm32_timer_enable();

		while (ad405x_conversion_flag != true && timeout > 0) {
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
	/* SPI init params */
	struct stm32_spi_init_param* spi_init_param;

	ret = ad405x_set_operation_mode(p_ad405x_dev, ad405x_operating_mode);
	if (ret) {
		return ret;
	}

	if (ad405x_interface_mode == SPI_INTR) {
		ret = init_pwm();
		if (ret) {
			return ret;
		}

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

		/* Switch to faster SPI SCLK and
		 * initialize Chip Select PWMs and DMA descriptors */
		ad405x_init_params.spi_init->max_speed_hz = MAX_SPI_SCLK_45MHz;
		spi_init_param = ad405x_init_params.spi_init->extra;
		spi_init_param->pwm_init = (const struct no_os_pwm_init_param *)&cs_init_params;
		spi_init_param->dma_init = &ad405x_dma_init_param;
		spi_init_param->irq_num = Rx_DMA_IRQ_ID;
		spi_init_param->rxdma_ch = &rxdma_channel;
		spi_init_param->txdma_ch = &txdma_channel;

		ret = no_os_spi_init(&p_ad405x_dev->spi_desc, ad405x_init_params.spi_init);
		if (ret) {
			return ret;
		}

		/* Use 16-bit SPI Data Frame Format during data capture */
		stm32_config_spi_data_frame_format(true);

		/* Configure CS and CNV gpios for alternate functionality as
		 * Timer PWM outputs */
		stm32_cs_output_gpio_config(false);

		ret = init_pwm();
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
static int32_t ad405x_post_disable_continuous(void *dev)
{
	int32_t ret;

	if (ad405x_interface_mode == SPI_DMA) {
		/* SPI init params */
		struct stm32_spi_init_param *spi_init_param;

		/* Revert to 8-bit SPI Data Frame Format after data capture */
		stm32_config_spi_data_frame_format(false);

		/* Abort DMA and Timers and configure CS and CNV as GPIOs */
		stm32_timer_stop();
		stm32_abort_dma_transfer();
		stm32_cs_output_gpio_config(true);

		spi_init_param = ad405x_init_params.spi_init->extra;
		spi_init_param->dma_init = NULL;
		ad405x_init_params.spi_init->max_speed_hz = MAX_SPI_SCLK;
		ret = no_os_spi_init(&p_ad405x_dev->spi_desc, ad405x_init_params.spi_init);
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

		ret = no_os_spi_transfer_dma_async(p_ad405x_dev->spi_desc,
						   &ad405x_spi_msg,
						   1,
						   NULL,
						   NULL);
		if (ret) {
			return ret;
		}
		struct stm32_spi_desc* sdesc = p_ad405x_dev->spi_desc->extra;
		no_os_pwm_disable(sdesc->pwm_desc); // CS PWM
		htim2.Instance->CNT = 0;
		htim1.Instance->CNT = 0;
		TIM8->CNT = 0;
		dma_config_updated = true;

		stm32_timer_enable();
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
	int32_t adc_data;

	if (!buf_size_updated) {
		/* Update total buffer size according to bytes per scan for proper
		 * alignment of multi-channel IIO buffer data */
		iio_dev_data->buffer->buf->size = ((uint32_t)(DATA_BUFFER_SIZE /
						   iio_dev_data->buffer->bytes_per_scan)) * iio_dev_data->buffer->bytes_per_scan;
		buf_size_updated = true;
	}

	/* Read the sample for channel which has been sampled recently */
	ret = ad405x_spi_data_read(p_ad405x_dev, &adc_data);
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
