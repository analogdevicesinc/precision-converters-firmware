/***************************************************************************//**
 *   @file    ad4692_support_cnv_clock_mode.c
 *   @brief   AD4692 CNV Clock Mode data transfer implementation
 *   @details Contains all data capture logic specific to CNV Clock Mode
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
#include "app_config.h"
#include "ad4692.h"
#include "iio_trigger.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

#define BUF_READ_TIMEOUT	0xffffffff

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* Maximum sampling frequency for CNV clock mode */
static uint32_t ad4692_sampling_frequency_max;

/* Accumulator data buffer */
static uint8_t acc_data_buff[AD4692_N_BYTES_TXN_24BIT *
						      AD4692_MAX_CHANNELS] = { 0x0 };

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

static uint32_t ad4692_cnv_clock_get_max_sampling_rate(void);
static int32_t ad4692_cnv_clock_update_sampling_frequency(
	uint32_t *sampling_rate);

/**
 * @brief Initialize CNV clock mode
 * @param desc[in] - AD4692 device descriptor
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad4692_cnv_clock_data_capture_init(struct ad4692_desc *desc)
{
	int32_t ret;

	if (!desc) {
		return -EINVAL;
	}

	ad4692_sampling_frequency_max = ad4692_cnv_clock_get_max_sampling_rate();

	ret = ad4692_cnv_clock_update_sampling_frequency(
		      &ad4692_sampling_frequency_max);
	if (ret) {
		return ret;
	}

	/* Reset manual mode bit */
	ret = ad4692_reg_update(desc,
				AD4692_DEVICE_SETUP_REG,
				AD4692_MANUAL_MODE_MASK,
				AD4692_EXIT_MANUAL_MODE);
	if (ret) {
		return ret;
	}

	/* Enter CNV Clock Mode */
	ret = ad4692_reg_update(desc,
				AD4692_ADC_SETUP_REG,
				AD4692_MODE_MASK,
				AD4692_CNV_CLOCK);
	if (ret) {
		return ret;
	}

	/* Configure GPIO0 as DATA_READYb */
	ret = ad4692_reg_update(desc,
				AD4692_GPIO_MODE1_REG,
				AD4692_GPIO0_MASK,
				AD4692_GPIO_OUTPUT_DATA_READYb);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Prepare for ADC data transfer in CNV clock mode
 * @param dev[in] - IIO device instance
 * @param ch_mask[in] - Channels select mask
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad4692_cnv_clock_prepare_transfer(void *dev,
		uint32_t ch_mask)
{
	int32_t ret;

	if (!dev) {
		return -EINVAL;
	}

	ad4692_update_active_channels(ch_mask);

	if (ad4692_data_capture_mode == CONTINUOUS_DATA_CAPTURE) {
		ret = ad4692_configure_channel(ad4692_dev);
		if (ret) {
			return ret;
		}
	}

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

		ret = ad4692_config_and_start_pwm(ad4692_dev);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Submit samples in CNV clock mode
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad4692_cnv_clock_submit_samples(
	struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t timeout = BUF_READ_TIMEOUT;
	uint32_t sample_index = 0;
	uint32_t nb_of_samples;
	ad4692_conversion_flag = false;
	uint32_t data_read_local;
	uint8_t i;
	uint8_t offset = 0;

	if (!iio_dev_data) {
		return -EINVAL;
	}

	nb_of_samples = iio_dev_data->buffer->size / n_data_bytes;

	if (!buf_size_updated) {
		iio_dev_data->buffer->buf->size = iio_dev_data->buffer->size;
		buf_size_updated = true;
	}

	/* Start ADC data capture */
	ret = ad4692_configure_channel(ad4692_dev);
	if (ret) {
		return ret;
	}

	/* Clear any pending interrupts */
	ret = no_os_irq_clear_pending(trigger_irq_desc,
				      trigger_gpio_irq_params.irq_ctrl_id);
	if (ret) {
		return ret;
	}

	ret = no_os_irq_enable(trigger_irq_desc,
			       trigger_gpio_irq_params.irq_ctrl_id);
	if (ret) {
		return ret;
	}

	/* Config CNV PWM and start */
	ret = ad4692_config_and_start_pwm(ad4692_dev);
	if (ret) {
		goto ccm_timer_trig_disable;
	}

	while (sample_index < nb_of_samples) {
		/* Build the Tx Command */
		ad4692_get_tx_command(acc_data_buff);

		/* Check for status of conversion flag */
		while (!ad4692_conversion_flag && timeout > 0) {
			timeout--;
		}

		if (timeout == 0) {
			ret = -ETIMEDOUT;
			goto ccm_timer_trig_disable;
		}
		timeout = BUF_READ_TIMEOUT;

		ad4692_conversion_flag = false;

		ret = no_os_spi_write_and_read(ad4692_dev->comm_desc, acc_data_buff,
					       num_of_active_channels * n_bytes_per_transaction);
		if (ret) {
			goto ccm_timer_trig_disable;
		}

		for (i = 0; i < num_of_active_channels; i++) {
			offset = (i * n_bytes_per_transaction) + AD4692_N_BYTES_TXN_OFFSET;
			if (ad4692_readback_option == ACCUMULATOR_DATA) {
				data_read_local = no_os_get_unaligned_be24(&acc_data_buff[offset]);
			} else {
				data_read_local = no_os_get_unaligned_be16(&acc_data_buff[offset]);
			}

			ret = no_os_cb_write(iio_dev_data->buffer->buf,
					     &data_read_local,
					     n_data_bytes);
			if (ret) {
				goto ccm_timer_trig_disable;
			}
		}

		/* Reset the state of accumulator */
		ret = ad4692_reg_write(ad4692_dev,
				       AD4692_STATE_RESET_REG,
				       AD4692_STATE_RESET_ALL);
		if (ret) {
			goto ccm_timer_trig_disable;
		}

		sample_index += num_of_active_channels;
	}

ccm_timer_trig_disable:
	/* Stop timer */
	ad4692_stop_timer();

	/* Disable triggers */
	ret |= no_os_irq_disable(trigger_irq_desc,
				 trigger_gpio_irq_params.irq_ctrl_id);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Trigger handler for CNV clock mode
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success or negative value otherwise
 */
static int32_t ad4692_cnv_clock_trigger_handler(
	struct iio_device_data *iio_dev_data)
{
	int32_t ret;
	uint32_t data_read_local = 0;
	uint8_t offset = 0;
	uint8_t i;

	/* Populate the Tx command */
	ad4692_get_tx_command(acc_data_buff);

	ret = no_os_spi_write_and_read(ad4692_dev->comm_desc, acc_data_buff,
				       num_of_active_channels * n_bytes_per_transaction);
	if (ret) {
		return ret;
	}

	for (i = 0; i < num_of_active_channels; i++) {
		offset = (i * n_bytes_per_transaction) + AD4692_N_BYTES_TXN_OFFSET;
		if (ad4692_readback_option == ACCUMULATOR_DATA) {
			data_read_local = no_os_get_unaligned_be24(&acc_data_buff[offset]);
		} else {
			data_read_local = no_os_get_unaligned_be16(&acc_data_buff[offset]);
		}

		ret = no_os_cb_write(iio_dev_data->buffer->buf,
				     &data_read_local,
				     n_data_bytes);
		if (ret) {
			return ret;
		}
	}

	/* Reset the state of accumulator */
	ret = ad4692_reg_write(ad4692_dev,
			       AD4692_STATE_RESET_REG,
			       AD4692_STATE_RESET_ALL);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief End data transfer in CNV clock mode
 * @param dev[in] - IIO device instance
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad4692_cnv_clock_end_transfer(void *dev)
{
	int32_t ret;

	if (!dev) {
		return -EINVAL;
	}

	if (ad4692_data_capture_mode == CONTINUOUS_DATA_CAPTURE) {
		ad4692_stop_timer();

		ret = iio_trig_disable(ad4692_hw_trig_desc);
		if (ret) {
			return ret;
		}
	}

	buf_size_updated = false;

	return 0;
}

/**
 * @brief Read single-shot ADC data in CNV clock mode
 * @param desc[in] - AD4692 device descriptor
 * @param chn[in] - Channel number
 * @param adc_data[out] - ADC converted data
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad4692_cnv_clock_read_converted_data(struct ad4692_desc *desc,
		uint8_t chn, uint32_t *adc_data)
{
	int32_t ret;
	uint8_t eoc_status;
	uint32_t timeout = BUF_READ_TIMEOUT;

	ret = ad4692_configure_channel(desc);
	if (ret) {
		return ret;
	}

	/* Enable CNV PWM */
	ret = no_os_pwm_enable(desc->conv_desc);
	if (ret) {
		return ret;
	}

	eoc_status = NO_OS_GPIO_HIGH;

	/* Poll for DATA_READYb Low */
	do {
		ret = no_os_gpio_get_value(desc->gpio0_desc, &eoc_status);
		if (ret) {
			goto ccm_disable_cnv_pwm;
		}
	} while (eoc_status != NO_OS_GPIO_LOW && --timeout > 0);

	if (timeout == 0) {
		ret = -ETIMEDOUT;
		goto ccm_disable_cnv_pwm;
	}

	ret = ad4692_reg_read(desc,
			      AD4692_AVG_IN_REG(chn),
			      adc_data);
	if (ret) {
		goto ccm_disable_cnv_pwm;
	}

	/* Reset the state of accumulator to start a new burst of conversion */
	ret = ad4692_reg_write(desc,
			       AD4692_STATE_RESET_REG,
			       AD4692_STATE_RESET_ALL);
	if (ret) {
		goto ccm_disable_cnv_pwm;
	}

ccm_disable_cnv_pwm:
	/* Disable CNV PWM */
	ret |= no_os_pwm_disable(desc->conv_desc);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Update sampling frequency for CNV clock mode
 * @param sampling_rate[in,out] - Requested/actual sampling rate
 * @return 0 in case of success, negative error code otherwise
 */
static int32_t ad4692_cnv_clock_update_sampling_frequency(
	uint32_t *sampling_rate)
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
 * @brief Get maximum sampling rate for CNV clock mode
 * @return Maximum sampling rate in samples per second
 */
static uint32_t ad4692_cnv_clock_get_max_sampling_rate(void)
{
	if (ad4692_readback_option == AVERAGED_DATA) {
		return (ad4692_sequencer_mode == STANDARD_SEQUENCER) ?
		       S_RATE_CNV_CLOCK_INTR_STD_AVG :
		       S_RATE_CNV_CLOCK_INTR_ADV_AVG;
	}

	return (ad4692_sequencer_mode == STANDARD_SEQUENCER) ?
	       S_RATE_CNV_CLOCK_INTR_STD_ACC :
	       S_RATE_CNV_CLOCK_INTR_ADV_ACC;
}

/* CNV clock mode data transfer system instance */
struct ad4692_data_transfer_system ad4692_data_transfer_cnv_clock_mode = {
	.initialize = ad4692_cnv_clock_data_capture_init,
	.prepare_transfer = ad4692_cnv_clock_prepare_transfer,
	.submit_samples = ad4692_cnv_clock_submit_samples,
	.trigger_handler = ad4692_cnv_clock_trigger_handler,
	.end_transfer = ad4692_cnv_clock_end_transfer,
	.remove = NULL,
	.read_converted_data = ad4692_cnv_clock_read_converted_data,
	.update_sampling_frequency = ad4692_cnv_clock_update_sampling_frequency,
	.get_max_sampling_rate = ad4692_cnv_clock_get_max_sampling_rate,
};
