/***************************************************************************//**
 *   @file    ad4692_support_manual_mode.c
 *   @brief   AD4692 Manual Mode data transfer implementation
 *   @details Contains all data capture logic specific to Manual Mode
********************************************************************************
 * Copyright (c) 2026 Analog Devices, Inc.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <string.h>

#include "ad4692_support.h"
#include "ad4692_iio.h"
#include "ad4692_user_config.h"
#include "no_os_error.h"
#include "no_os_delay.h"
#include "no_os_util.h"
#include "no_os_alloc.h"
#include "app_config.h"
#include "ad4692.h"
#include "iio_trigger.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/* Timeout count to avoid stuck into potential infinite loop */
#define BUF_READ_TIMEOUT	0xffffffff

/* Maximum value the DMA NDTR register can take */
#define MAX_LOCAL_BUF_SIZE	65536
#define MAX_DMA_NDTR		(no_os_min(65532, (MAX_LOCAL_BUF_SIZE)))

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* SPI message for manual mode capture */
static struct no_os_spi_msg ad4692_spi_msg_manual_mode = {
	.cs_change = CS_CHANGE,
	.tx_buff = NULL,
	.rx_buff = NULL,
	.bytes_number = BYTES_PER_SAMPLE
};

/* Data buffer for SPI transactions */
static uint8_t data_buff[BYTES_PER_SAMPLE] = { 0x0 };

/* Flag to indicate if DMA has been configured for capture */
static volatile bool dma_config_updated = false;

/* Flag for checking DMA buffer overflow */
volatile bool ad4692_dma_buff_full = false;

/* Variable to store start of buffer address */
volatile uint32_t *buff_start_addr;

/* Local buffer for DMA */
__attribute__((aligned(32))) static uint8_t local_buf[MAX_LOCAL_BUF_SIZE +
				   (NO_OF_CHANNELS * N_CYCLE_OFFSET)];

/* Variable to track the number of entries to the complete callback */
uint32_t callback_count = 0;

/* Global Pointer for IIO Device Data (used by DMA callbacks) */
volatile struct iio_device_data *iio_dev_data_g;

/* Global variable for number of samples (used by DMA callbacks) */
uint32_t nb_of_samples_g;

/* Channel ID during data capture */
static volatile uint8_t chan_id = 0;

/* Chip Select GPIO init parameters */
static struct no_os_gpio_init_param csb_gpio_init_param = {
	.port = SPI_CS_PORT_NUM,
	.number = SPI_CSB,
	.pull = NO_OS_PULL_NONE,
	.platform_ops = &gpio_ops,
	.extra = &gpio_output_extra_init_params
};

/* CNV GPIO Init params */
static struct no_os_gpio_init_param cnv_gpio_init_param = {
	.port = CNV_PORT_NUM,
	.number = CNV_PIN_NUM,
	.platform_ops = &gpio_ops,
	.extra = &gpio_input_extra_init_params
};

/* Tx Trigger Init params */
static struct no_os_pwm_init_param tx_trigger_init_param = {
	.id = TX_TRIGGER_TIMER_ID,
	.period_ns = TX_TRIGGER_PERIOD,
	.duty_cycle_ns = TX_TRIGGER_DUTY_RATIO,
	.polarity = NO_OS_PWM_POLARITY_HIGH,
	.platform_ops = &pwm_ops,
	.extra = &tx_trigger_extra_init_params,
};

/* Tx trigger descriptor */
static struct no_os_pwm_desc *tx_trigger_desc = NULL;

/* CNV GPIO descriptor */
static struct no_os_gpio_desc *cnv_gpio_desc = NULL;

/* Chip Select GPIO descriptor */
static struct no_os_gpio_desc *csb_gpio_desc = NULL;

/* Maximum sampling frequency for manual mode */
static uint32_t ad4692_sampling_frequency_max;

/* DMA NDTR register value */
extern uint32_t rxdma_ndtr;

/* DMA cycle count */
extern uint32_t dma_cycle_count;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

static uint32_t ad4692_manual_get_max_sampling_rate(void);
static int32_t ad4692_manual_update_sampling_frequency(uint32_t *sampling_rate);
static int32_t ad4692_stop_data_capture(struct ad4692_desc *desc);

/**
 * @brief Reconfigure CNV GPIO as PWM or GPIO based on the mode
 *
 * @param gpio_cnv[in, out] - GPIO descriptor for CNV, which will be reconfigured as PWM or GPIO
 * @param pwm_gpio[in] - boolean flag to indicate if CNV is to be configured as PWM (true) or GPIO (false)
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad4692_reconfigure_cnv(struct no_os_gpio_desc **gpio_cnv,
				      bool pwm_gpio)
{
	int32_t ret;

	if (!gpio_cnv) {
		return -EINVAL;
	}

	/* Remove existing GPIO descriptor (if any) */
	if (*gpio_cnv) {
		NO_OS_UNUSED_PARAM(no_os_gpio_remove(*gpio_cnv));
		*gpio_cnv = NULL;
	}

	if (pwm_gpio) {
		/* Configure CNV as PWM */
		ret = no_os_gpio_get(gpio_cnv, &cnv_pwm_gpio_params);
		if (ret) {
			return ret;
		}
	} else {
		/* Configure CNV as GPIO */
		ret = no_os_gpio_get(gpio_cnv, &cnv_gpio_init_param);
		if (ret) {
			return ret;
		}

		ret = no_os_gpio_direction_output(*gpio_cnv, NO_OS_GPIO_LOW);
		if (ret) {
			NO_OS_UNUSED_PARAM(no_os_gpio_remove(*gpio_cnv));
			*gpio_cnv = NULL;
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Toggle CNV GPIO
 * @param gpio_cnv[in, out] - GPIO CNV Descriptor
 * @return 0 in case of success, negative error code otherwise.
 */
static int32_t ad4692_toggle_cnv(struct no_os_gpio_desc *gpio_cnv)
{
	int32_t ret;

	if (!gpio_cnv) {
		return -EINVAL;
	}

	ret = no_os_gpio_set_value(gpio_cnv, NO_OS_GPIO_HIGH);
	if (ret) {
		return ret;
	}

	no_os_udelay(1);

	ret = no_os_gpio_set_value(gpio_cnv, NO_OS_GPIO_LOW);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Initialize device for data capture in manual mode
 * @param desc[in] - AD4692 device descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad4692_start_data_capture(struct ad4692_desc *desc)
{
	int32_t ret;
	uint8_t toggle_n;
	struct no_os_spi_msg ad4692_spi_msg = {
		.cs_change = CS_CHANGE,
		.tx_buff = data_buff,
		.rx_buff = data_buff,
		.bytes_number = BYTES_PER_SAMPLE
	};

	if (!desc) {
		return -EINVAL;
	}

	if (ad4692_interface_mode == SPI_DMA) {
		ret = no_os_gpio_get(&csb_gpio_desc, &csb_gpio_init_param);
		if (ret) {
			return ret;
		}

		ret = no_os_gpio_direction_output(csb_gpio_desc, NO_OS_GPIO_LOW);
		if (ret) {
			goto mm_stop_data_capture;
		}
	}

	/* Enter Manual Mode */
	ret = ad4692_reg_write(desc,
			       AD4692_DEVICE_SETUP_REG,
			       AD4692_DEVICE_MANUAL);
	if (ret) {
		goto mm_stop_data_capture;
	}

	/* Skip 2 samples at the beginning of manual mode as the data is 2 sample delayed */
	if (ad4692_interface_mode != SPI_DMA) {
		ret = ad4692_reconfigure_cnv(&cnv_gpio_desc, false);
		if (ret) {
			goto mm_stop_data_capture;
		}

		/* Toggle CNV at a gap of 5us */
		for (toggle_n = 0; toggle_n < AD4692_N_CNV_TOGGLES; toggle_n++) {
			no_os_udelay(5);
			ret = ad4692_toggle_cnv(cnv_gpio_desc);
			if (ret) {
				goto mm_stop_data_capture;
			}

			/* Register the channel ID */
			data_buff[0] = AD4692_IN_COMMAND(ad4692_active_channels[chan_id]);
			data_buff[1] = 0x0;

			if (num_of_active_channels > 1) {
				chan_id++;
			}

			ret = no_os_spi_transfer(desc->comm_desc, &ad4692_spi_msg, 1);
			if (ret) {
				goto mm_stop_data_capture;
			}
		}

		ret = ad4692_reconfigure_cnv(&cnv_gpio_desc, true);
		if (ret) {
			goto mm_stop_data_capture;
		}
	}

	return 0;
mm_stop_data_capture:
	NO_OS_UNUSED_PARAM(ad4692_stop_data_capture(desc));
	return ret;
}

/**
 * @brief Stop data capture in manual mode
 * @param desc[in] - AD4692 device descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad4692_stop_data_capture(struct ad4692_desc *desc)
{
	int32_t ret;
	uint8_t toggle_n;

	if (!desc) {
		return -EINVAL;
	}

	ret = ad4692_reconfigure_cnv(&cnv_gpio_desc, false);
	if (ret) {
		return ret;
	}

	/* Toggle CNV at a gap of 5us */
	for (toggle_n = 0; toggle_n < AD4692_N_CNV_TOGGLES; toggle_n++) {
		no_os_udelay(5);
		ret = ad4692_toggle_cnv(cnv_gpio_desc);
		if (ret) {
			return ret;
		}
	}

	ret = ad4692_reconfigure_cnv(&cnv_gpio_desc, true);
	if (ret) {
		return ret;
	}

	ret = ad4692_exit_manual_mode(desc);
	if (ret) {
		return ret;
	}

	if (cnv_gpio_desc) {
		NO_OS_UNUSED_PARAM(no_os_gpio_remove(cnv_gpio_desc));
		cnv_gpio_desc = NULL;
	}

	if (csb_gpio_desc) {
		NO_OS_UNUSED_PARAM(no_os_gpio_remove(csb_gpio_desc));
		csb_gpio_desc = NULL;
	}

	return 0;
}

/**
 * @brief Read data in burst mode via SPI DMA
 * @param nb_of_samples_req[in] - Number of samples requested by IIO
 * @param iio_dev_data[in] - IIO Device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad4692_read_data_spi_dma(uint32_t nb_of_samples_req,
					struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t timeout = BUF_READ_TIMEOUT;
	uint32_t spirxdma_ndtr;
	uint32_t data_read;
	static uint8_t local_tx_data[32] = { 0x0 };

	if (ad4692_data_capture_mode == BURST_DATA_CAPTURE) {
		nb_of_samples_req = nb_of_samples_req * BYTES_PER_SAMPLE;

		ret = no_os_cb_prepare_async_write(iio_dev_data->buffer->buf,
						   nb_of_samples_req,
						   (void **) &buff_start_addr,
						   &data_read);
		if (ret) {
			return ret;
		}

		/* Manipulate the number of samples considering the 2-cycle offset */
		if (num_of_active_channels <= 2) {
			nb_of_samples_req += (N_CYCLE_OFFSET * BYTES_PER_SAMPLE);
		} else {
			nb_of_samples_req += (num_of_active_channels * BYTES_PER_SAMPLE);
		}

		if (!dma_config_updated) {
			/* Build the Tx command with respect to the enabled channels */
			ad4692_get_tx_command(local_tx_data);

			/* Cap SPI RX DMA NDTR to MAX_DMA_NDTR */
			spirxdma_ndtr = no_os_min(MAX_DMA_NDTR, nb_of_samples_req);
			rxdma_ndtr = spirxdma_ndtr;

			/* Register half complete callback */
			HAL_DMA_RegisterCallback(&hdma_spi1_rx,
						 HAL_DMA_XFER_HALFCPLT_CB_ID,
						 ad4692_spi_dma_rx_half_cplt_callback);

			struct no_os_spi_msg  ad4692_spi_msg = {
				.tx_buff = local_tx_data,
				.rx_buff = local_buf,
				.bytes_number = spirxdma_ndtr,
			};

			ret = no_os_spi_transfer_dma_async(ad4692_dev->comm_desc,
							   &ad4692_spi_msg,
							   1, NULL, NULL);
			if (ret) {
				return ret;
			}

			/* DMA to be disabled while reconfiguring the NDTR register */
			DMA2_Stream2->CR &= ~1;
			DMA2_Stream2->NDTR = num_of_active_channels * 2;
			DMA2_Stream2->CR |= 1;

			dma_config_updated = true;
		}

		if (nb_of_samples_req == rxdma_ndtr) {
			dma_cycle_count = 1;
		} else {
			dma_cycle_count = ((nb_of_samples_req) / rxdma_ndtr) + 1;
		}
		callback_count = dma_cycle_count * 2;
		update_buff(local_buf, (uint8_t *)buff_start_addr);

		/* Configure CNV PWM and start */
		ret = ad4692_config_and_start_pwm(ad4692_dev);
		if (ret) {
			return ret;
		}

		while (ad4692_dma_buff_full != true && timeout > 0) {
			timeout--;
		}

		if (!timeout) {
			return -EIO;
		}

		ad4692_dma_buff_full = false;
		ret = no_os_cb_end_async_write(iio_dev_data->buffer->buf);
		if (ret) {
			return ret;
		}
		ad4692_stop_timer();
	} else { // CONTINUOUS_DATA_CAPTURE
		if (!dma_config_updated) {
			/* Build the Tx command with respect to the enabled channels */
			ad4692_get_tx_command(local_tx_data);

			nb_of_samples_req = nb_of_samples_req * BYTES_PER_SAMPLE;

			/* Manipulate the number of samples considering the 2-cycle offset */
			if (num_of_active_channels <= 2) {
				nb_of_samples_req += (N_CYCLE_OFFSET * BYTES_PER_SAMPLE);
			} else {
				nb_of_samples_req += (num_of_active_channels * BYTES_PER_SAMPLE);
			}

			spirxdma_ndtr = no_os_min(MAX_DMA_NDTR, nb_of_samples_req);
			rxdma_ndtr = spirxdma_ndtr;

			nb_of_samples_g = spirxdma_ndtr;
			iio_dev_data_g = iio_dev_data;

			/* SPI Message */
			struct no_os_spi_msg ad4692_spi_msg = {
				.tx_buff = local_tx_data,
				.rx_buff = local_buf,
				.bytes_number = spirxdma_ndtr
			};

			ret = no_os_cb_prepare_async_write(iio_dev_data_g->buffer->buf,
							   nb_of_samples_req,
							   (void **) &buff_start_addr,
							   &data_read);
			if (ret) {
				return ret;
			}

			ret = no_os_spi_transfer_dma_async(ad4692_dev->comm_desc,
							   &ad4692_spi_msg, 1,
							   NULL, NULL);
			if (ret) {
				return ret;
			}

			/* DMA to be disabled while configuring the NDTR Register */
			DMA2_Stream2->CR &= ~1;
			DMA2_Stream2->NDTR = num_of_active_channels * BYTES_PER_SAMPLE;
			DMA2_Stream2->CR |= 1;

			dma_config_updated = true;

			/* Update Buffer indices */
			update_buff(local_buf, (uint8_t *)buff_start_addr);

			/* Configure CNV PWM and start */
			ret = ad4692_config_and_start_pwm(ad4692_dev);
			if (ret) {
				return ret;
			}
		}
	}

	return 0;
}

/**
 * @brief Initialize manual mode
 * @param desc[in] - AD4692 device descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad4692_manual_init(struct ad4692_desc *desc)
{
	int32_t ret;

	if (!desc) {
		return -EINVAL;
	}

	ad4692_sampling_frequency_max = ad4692_manual_get_max_sampling_rate();

	ret = ad4692_manual_update_sampling_frequency(&ad4692_sampling_frequency_max);
	if (ret) {
		return ret;
	}

	/* Configure GPIO0 as BUSY */
	ret = ad4692_gpio_set(desc,
			      AD4692_GPIO0,
			      AD4692_GPIO_OUTPUT_ADC_BUSY);
	if (ret) {
		return ret;
	}

	if (ad4692_interface_mode == SPI_DMA) {
		ret = no_os_pwm_init(&tx_trigger_desc, &tx_trigger_init_param);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Prepare for ADC data transfer in manual mode
 * @param dev[in] - IIO device instance
 * @param ch_mask[in] - Channels select mask
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad4692_manual_prepare_transfer(void *dev,
		uint32_t ch_mask)
{
	int32_t ret;

	if (!dev) {
		return -EINVAL;
	}

	chan_id = 0;
	ad4692_update_active_channels(ch_mask);

	if ((ad4692_data_capture_mode == CONTINUOUS_DATA_CAPTURE)
	    || (ad4692_interface_mode == SPI_DMA)) {
		/* Start ADC Data capture */
		ret = ad4692_start_data_capture(ad4692_dev);
		if (ret) {
			return ret;
		}
	}

	if (ad4692_interface_mode == SPI_INTR) {
		if (ad4692_data_capture_mode == CONTINUOUS_DATA_CAPTURE) {
#if (ACTIVE_PLATFORM == STM32_PLATFORM)
			ret = no_os_irq_clear_pending(trigger_irq_desc,
						      trigger_gpio_irq_params.irq_ctrl_id);
			if (ret) {
				return ret;
			}
#endif
			ret = iio_trig_enable(ad4692_hw_trig_desc);
			if (ret) {
				return ret;
			}

			/* Configure CNV PWM and start */
			ret = ad4692_config_and_start_pwm(ad4692_dev);
			if (ret) {
				return ret;
			}
		}
	} else { // SPI DMA
		/* Pull the SPI CS line low to enable the data on SDO */
		ret = no_os_gpio_set_value(csb_gpio_desc, NO_OS_GPIO_LOW);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Submit samples in manual mode
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad4692_manual_submit_samples(struct iio_device_data
		*iio_dev_data)
{
	int32_t ret;
	uint32_t timeout = BUF_READ_TIMEOUT;
	uint32_t sample_index = 0;
	uint32_t nb_of_samples;
	ad4692_conversion_flag = false;
	uint8_t ch = 0;

	if (!iio_dev_data) {
		return -EINVAL;
	}

	nb_of_samples = iio_dev_data->buffer->size / n_data_bytes;

	if (!buf_size_updated) {
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	if (ad4692_interface_mode == SPI_INTR) {
		/* Start ADC data capture */
		ret = ad4692_start_data_capture(ad4692_dev);
		if (ret) {
			return ret;
		}

		/* Clear any pending interrupts */
		ret = no_os_irq_clear_pending(trigger_irq_desc,
					      trigger_gpio_irq_params.irq_ctrl_id);
		if (ret) {
			goto mm_submit_samples_exit;
		}

		ret = no_os_irq_enable(trigger_irq_desc,
				       trigger_gpio_irq_params.irq_ctrl_id);
		if (ret) {
			goto mm_submit_samples_exit;
		}

		/* Config CNV PWM and start */
		ret = ad4692_config_and_start_pwm(ad4692_dev);
		if (ret) {
			goto mm_submit_samples_exit;
		}

		while (sample_index < nb_of_samples) {
			/* Check for status of conversion flag */
			while (!ad4692_conversion_flag && timeout > 0) {
				timeout--;
			}

			if (timeout == 0) {
				ret = -ETIMEDOUT;
				goto mm_submit_samples_exit;
			}
			timeout = BUF_READ_TIMEOUT;

			ad4692_conversion_flag = false;

			/* Reset the channel ID back to the first enabled channel */
			if (ch >= num_of_active_channels) {
				ch = 0;
			}

			data_buff[0] = AD4692_IN_COMMAND(ad4692_active_channels[ch++]);
			data_buff[1] = 0x0;

			ad4692_spi_msg_manual_mode.tx_buff = data_buff;
			ad4692_spi_msg_manual_mode.rx_buff = data_buff;

			ret = no_os_spi_transfer(ad4692_dev->comm_desc,
						 &ad4692_spi_msg_manual_mode, 1);
			if (ret) {
				goto mm_submit_samples_exit;
			}

			ret = no_os_cb_write(iio_dev_data->buffer->buf,
					     ad4692_spi_msg_manual_mode.rx_buff,
					     n_data_bytes);
			if (ret) {
				goto mm_submit_samples_exit;
			}

			sample_index++;
		}

mm_submit_samples_exit:
		/* Stop timer */
		ad4692_stop_timer();

		/* Disable triggers */
		ret |= no_os_irq_disable(trigger_irq_desc,
					 trigger_gpio_irq_params.irq_ctrl_id);

		/* Stop ADC Data capture */
		ret |= ad4692_stop_data_capture(ad4692_dev);
		if (ret) {
			return ret;
		}
	} else { // SPI_DMA
		ret = ad4692_read_data_spi_dma(nb_of_samples, iio_dev_data);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Trigger handler for manual mode (continuous capture via ISR)
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad4692_manual_trigger_handler(
	struct iio_device_data *iio_dev_data)
{
	int32_t ret;

	/* Reset the channel ID back to the first enabled channel */
	if (chan_id >= num_of_active_channels) {
		chan_id = 0;
	}

	data_buff[0] = AD4692_IN_COMMAND(ad4692_active_channels[chan_id++]);
	data_buff[1] = 0x0;

	ad4692_spi_msg_manual_mode.tx_buff = data_buff;
	ad4692_spi_msg_manual_mode.rx_buff = data_buff;

	ret = no_os_spi_transfer(ad4692_dev->comm_desc,
				 &ad4692_spi_msg_manual_mode, 1);
	if (ret) {
		return ret;
	}

	ret = no_os_cb_write(iio_dev_data->buffer->buf,
			     ad4692_spi_msg_manual_mode.rx_buff,
			     n_data_bytes);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief End data transfer in manual mode
 * @param dev[in] - IIO device instance
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad4692_manual_end_transfer(void *dev)
{
	int32_t ret;

	if (!dev) {
		return -EINVAL;
	}

	if (ad4692_interface_mode == SPI_INTR) {
		if (ad4692_data_capture_mode == CONTINUOUS_DATA_CAPTURE) {
			/* Stop timer */
			ad4692_stop_timer();

			/* Disable triggers */
			iio_trig_disable(ad4692_hw_trig_desc);
		}
	} else { // SPI_DMA
		/* Stop timers */
		ad4692_stop_timer();

		no_os_spi_transfer_abort(ad4692_dev->comm_desc);

		/* Pull the SPI CS line back high to enable reg Access */
		ret = no_os_gpio_set_value(csb_gpio_desc, NO_OS_GPIO_HIGH);
		if (ret) {
			return ret;
		}

		/* De initialize the Tx Trigger PWM */
		ret = no_os_pwm_disable(tx_trigger_desc);
		if (ret) {
			return ret;
		}

		dma_config_updated = false;
	}

	/* Stop ADC Data capture */
	ret = ad4692_stop_data_capture(ad4692_dev);
	if (ret) {
		return ret;
	}

	buf_size_updated = false;

	return 0;
}

/**
 * @brief Remove manual mode resources
 * @param desc[in] - AD4692 device descriptor
 * @return 0 in case of success
 */
static int32_t ad4692_manual_remove(struct ad4692_desc *desc)
{
	if (tx_trigger_desc) {
		no_os_pwm_remove(tx_trigger_desc);
		tx_trigger_desc = NULL;
	}

	return 0;
}

/**
 * @brief Read single-shot ADC data in manual mode
 * @param desc[in] - AD4692 device descriptor
 * @param chn[in] - Channel number
 * @param adc_data[out] - ADC converted data
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad4692_manual_read_converted_data(struct ad4692_desc *desc,
		uint8_t chn, uint32_t *adc_data)
{
	int32_t ret;
	uint8_t eoc_status;
	uint8_t toggle_n;
	uint32_t timeout = BUF_READ_TIMEOUT;

	ret = ad4692_start_data_capture(desc);
	if (ret) {
		return ret;
	}

	data_buff[0] = AD4692_IN_COMMAND(chn);
	data_buff[1] = 0x0;
	eoc_status = NO_OS_GPIO_HIGH;

	ret = no_os_gpio_direction_input(desc->gpio0_desc);
	if (ret) {
		goto mm_stop_read_converted_data;
	}

	ret = ad4692_reconfigure_cnv(&cnv_gpio_desc, false);
	if (ret) {
		goto mm_stop_read_converted_data;
	}

	/* Toggle CNV to skip the initial dummy data */
	for (toggle_n = 0; toggle_n <= AD4692_N_CNV_TOGGLES; toggle_n++) {
		timeout = BUF_READ_TIMEOUT;

		ret = ad4692_toggle_cnv(cnv_gpio_desc);
		if (ret) {
			goto mm_stop_read_converted_data;
		}

		/* Poll for BSY Low */
		do {
			ret = no_os_gpio_get_value(desc->gpio0_desc, &eoc_status);
			if (ret) {
				goto mm_stop_read_converted_data;
			}
		} while ((eoc_status != NO_OS_GPIO_LOW) && (--timeout > 0));

		if (timeout == 0) {
			ret = -ETIMEDOUT;
			goto mm_stop_read_converted_data;
		}
	}

	ret = ad4692_reconfigure_cnv(&cnv_gpio_desc, true);
	if (ret) {
		goto mm_stop_read_converted_data;
	}

	ad4692_spi_msg_manual_mode.tx_buff = data_buff;
	ad4692_spi_msg_manual_mode.rx_buff = data_buff;

	ret = no_os_spi_transfer(desc->comm_desc, &ad4692_spi_msg_manual_mode,
				 1);
	if (ret) {
		goto mm_stop_read_converted_data;
	}

	*adc_data = no_os_get_unaligned_be16(ad4692_spi_msg_manual_mode.rx_buff);

mm_stop_read_converted_data:
	/* Stop data capture after single-shot read */
	ret |= ad4692_stop_data_capture(desc);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Update sampling frequency for manual mode
 * @param sampling_rate[in,out] - Requested/actual sampling rate
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad4692_manual_update_sampling_frequency(uint32_t *sampling_rate)
{
	int32_t ret;
	uint32_t period_ns_readback;

	if (*sampling_rate > ad4692_sampling_frequency_max) {
		*sampling_rate = ad4692_sampling_frequency_max;
	}

	ret = ad4692_configure_pwm_rate(ad4692_dev->conv_desc, *sampling_rate);
	if (ret) {
		return ret;
	}

	/* Get the actual period of the PWM */
	ret = no_os_pwm_get_period(ad4692_dev->conv_desc, &period_ns_readback);
	if (ret) {
		return ret;
	}

	ad4692_sampling_frequency = PWM_PERIOD_TO_FREQUENCY(period_ns_readback);

	return 0;
}

/**
 * @brief Get maximum sampling rate for manual mode
 * @return Maximum sampling rate in samples per second
 */
static uint32_t ad4692_manual_get_max_sampling_rate(void)
{
	if (ad4692_interface_mode == SPI_DMA) {
		return S_RATE_MANUAL_DMA;
	}

	return S_RATE_MANUAL_INTR;
}

/* Manual mode data transfer system instance */
struct ad4692_data_transfer_system ad4692_data_transfer_manual_mode = {
	.initialize = ad4692_manual_init,
	.prepare_transfer = ad4692_manual_prepare_transfer,
	.submit_samples = ad4692_manual_submit_samples,
	.trigger_handler = ad4692_manual_trigger_handler,
	.end_transfer = ad4692_manual_end_transfer,
	.remove = ad4692_manual_remove,
	.read_converted_data = ad4692_manual_read_converted_data,
	.update_sampling_frequency = ad4692_manual_update_sampling_frequency,
	.get_max_sampling_rate = ad4692_manual_get_max_sampling_rate,
};
