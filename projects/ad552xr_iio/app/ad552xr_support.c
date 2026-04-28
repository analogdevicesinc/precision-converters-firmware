/***************************************************************************//**
 *   @file    ad552xr_support.c
 *   @brief   Source file for the AD552XR IIO Application Support
 *   @details This module contains support functions needed for IIO application
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
#include <stdint.h>
#include "app_config.h"
#include "ad552xr_support.h"
#include "ad552xr_user_config.h"
#include "ad552xr.h"
#include "no_os_error.h"
#include "no_os_alloc.h"
#include "no_os_util.h"
#include "no_os_spi.h"
#include "stm32_pwm_ex.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/
#define TIM_COMPENSATION		(150) // Additional time for trigger settling etc.
#define TIM_BYTE_TRANSFER		(8 * HZ_NS_CONVERT(SPI_SPEED))
#define NUM_BYTES_TRANSFER		(sizeof(uint16_t) +\
									AD552XR_DAC_RESOLUTION / 8) // Address phase + Data phase

#define TIM_DMA_HIGH_TIME		(TIM_BYTE_TRANSFER)
#define TIM_DMA_LOW_TIME		(20) // Minimal time to generate duty cycle
#define TIM_DMA_PERIOD			(TIM_DMA_LOW_TIME + TIM_DMA_HIGH_TIME)

#define TIM_CS_HIGH_TIME		(100)
#define TIM_CS_LOW_TIME			(NUM_BYTES_TRANSFER * TIM_DMA_PERIOD)

#if (INTERFACE_MODE == SPI_DMA)
#define TIM_DAC_UPDATE_PERIOD	(TIM_CS_LOW_TIME + TIM_COMPENSATION)
#elif (INTERFACE_MODE == SPI_INTERRUPT)
#define TIM_DAC_UPDATE_PERIOD	HZ_NS_CONVERT(AD552XR_IIO_SAMPLE_RATE)
#endif

/* AD552XR IIO application sampling rate */
static const uint32_t ad552xr_iio_sampling_rate = HZ_NS_CONVERT(
			TIM_DAC_UPDATE_PERIOD);

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/
#if (INTERFACE_MODE == SPI_DMA)
static void ad552xr_sg_callback(void *context);
#endif

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
/* PWM DAC UPDATE init parameters */
static struct no_os_pwm_init_param pwm_dac_update_init_params = {
	.id = TIM_DAC_UPDATE_INSTANCE_ID,
	.period_ns = TIM_DAC_UPDATE_PERIOD,
	.duty_cycle_ns = TIM_CS_HIGH_TIME,
	.polarity = NO_OS_PWM_POLARITY_LOW,
#if (INTERFACE_MODE == SPI_INTERRUPT)
	.irq_id = TIM_DAC_UPDATE_IRQ_ID,
#endif
	.platform_ops = &pwm_ops,
	.extra = &pwm_dac_update_extra_init_params,
};

/* PWM TGP init parameters */
static struct no_os_pwm_init_param pwm_tgp_init_params = {
	.id = TIM_TGP_INSTANCE_ID,
	.period_ns = HZ_NS_CONVERT(AD552XR_MAX_SAMPLE_RATE),
	.duty_cycle_ns = LDAC_DUTY_CYCLE_NSEC(HZ_NS_CONVERT(AD552XR_MAX_SAMPLE_RATE)),
	.polarity = NO_OS_PWM_POLARITY_HIGH,
	.platform_ops = &pwm_ops,
	.extra = &pwm_tgp_extra_init_params,
};

#if (INTERFACE_MODE == SPI_DMA)
/* DMA Init parameters */
static struct no_os_dma_init_param dma_init_params = {
	.id = 0,
	.num_ch = DMA_NUM_CHANNELS,
	.platform_ops = (struct no_os_dma_platform_ops *)&dma_ops,
	.sg_handler = ad552xr_sg_callback,
	.ctx = NULL
};

/* Chip Select GPIO init parameters in PWM mode */
static struct no_os_gpio_init_param gpio_cs_pwm_init_params = {
	.port = SPI_CSB_PORT,
	.number = SPI_CSB,
	.pull = NO_OS_PULL_DOWN,
	.platform_ops = &gpio_ops,
	.extra = &gpio_cs_pwm_extra_init_params,
};

/* Chip Select GPIO init parameters in GPIO Output mode */
static struct no_os_gpio_init_param gpio_cs_gpio_output_init_params = {
	.port = SPI_CSB_PORT,
	.number = SPI_CSB,
	.pull = NO_OS_PULL_UP,
	.platform_ops = &gpio_ops,
	.extra = &gpio_output_extra_init_params,
};

/* PWM DMA trigger Timer Init parameters */
static struct no_os_pwm_init_param pwm_dma_trigger_init_params = {
	.id = TIM_DMA_TRIGGER_INSTANCE_ID,
	.period_ns = TIM_DMA_PERIOD,
	.duty_cycle_ns = TIM_DMA_LOW_TIME,
	.polarity = NO_OS_PWM_POLARITY_HIGH,
	.platform_ops = &pwm_ops,
	.extra = &pwm_dma_trigger_extra_init_params,
};

/* GPIO CS descriptor */
static struct no_os_gpio_desc *gpio_cs_desc;
#endif

/* GPIO TGP PWM descriptor */
static struct no_os_gpio_desc *gpio_tgp_pwm_desc;

/* PWM DAC UPDATE descriptor */
static struct no_os_pwm_desc *pwm_dac_update_desc;

/* PWM TGP descriptor */
static struct no_os_pwm_desc *pwm_tgp_desc;

#if (INTERFACE_MODE == SPI_DMA)
/* PWM DMA Trigger Timer descriptor */
static struct no_os_pwm_desc *pwm_dma_trigger_desc;

/* Flag to indicate status of DMA */
static bool spi_dma_enabled;
#endif

#if (INTERFACE_MODE == SPI_INTERRUPT)
/**
 * @struct ad552xr_spi_intr_tx_info
 * @brief Structure to hold the SPI transfer information in SPI interrupt mode.
 */
static struct ad552xr_spi_intr_tx_info {
	/* Total number of bytes to transfer */
	uint32_t total_bytes_to_transfer;
	/* Number of bytes transferred */
	uint32_t bytes_transferred;
	/* Total number of DAC data transfers over SPI in one LDAC cycle */
	uint32_t num_spi_transfers_per_cycle;
	/* Number of DAC data transfers completed */
	uint32_t num_spi_data_transfers_done;
} tx_info_spi_intr;
#endif

/* Flag to indicate configuration status */
static bool configured = false;

/* Sampling frequency */
static uint32_t sampling_frequency = ad552xr_iio_sampling_rate;

/* TGP timer usage status */
static bool use_tgp_timer = false;

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/
/**
 * @brief	Set the sampling rate.
 * @param	val[in] - Sample rate value.
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t ad552xr_set_sampling_rate(uint32_t val)
{
	int32_t ret;
	uint32_t period_ns;

	/* If user input freq is more than max sampling rate,
	 * then later will be configured */
	if (val > ad552xr_iio_sampling_rate) {
		val = ad552xr_iio_sampling_rate;
	}

	period_ns = HZ_NS_CONVERT(val);

	/* Configure the period for PWM DAC Update timer */
	ret = no_os_pwm_set_period(pwm_dac_update_desc, period_ns);
	if (ret) {
		return ret;
	}

	sampling_frequency = HZ_NS_CONVERT(period_ns);

	return 0;
}

/**
 * @brief	Get the sampling rate.
 * @param	val[out] - Sample rate value.
 * @return	0 in case of success, negative error code otherwise.
 */
int32_t ad552xr_get_sampling_rate(uint32_t *val)
{
	*val = sampling_frequency;
	return 0;
}

/**
 * @brief	Configure timer period & duty cycle.
 * @param 	desc[in] - The PWM descriptor.
 * @param 	period_ns[in] - The period value in nanoseconds.
 * @param	duty_cycle_ns[in] - The duty cycle value in nanoseconds.
 * @return	0 in case of success, error code otherwise.
 */
static int32_t pwm_set_period_and_duty_cycle(struct no_os_pwm_desc *desc,
		uint64_t period_ns, uint64_t duty_cycle_ns)
{
	int32_t ret;
	uint32_t prescaler;

	/* Compute prescaler to fit the period value within respective register */
	ret = compute_optimal_prescaler(desc->extra, period_ns, &prescaler);
	if (ret) {
		return ret;
	}

	/* Set prescaler for timer */
	ret = set_timer_prescaler(desc, prescaler);
	if (ret) {
		return ret;
	}

	/* Set the period of the timer */
	ret = no_os_pwm_set_period(desc, period_ns);
	if (ret) {
		return ret;
	}

	/* Set the duty cycle of the timer */
	ret = no_os_pwm_set_duty_cycle(desc, duty_cycle_ns);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief	Reinitialize TGP timer.
 * @details This function will be called when TGP timer is necessary to be enabled.
 * This will re-initialize TGP timer into either trigger mode or free running
 * PWM mode based on count value conditions.
 *
 * If no SPI transactions are required, TGP Timer can run independently.
 *
 * If 1 SPI transaction is required and is corresponding to SW LDAC command,
 * TGP Timer can run independently.
 *
 * If 1 SPI transaction is required and is corresponding to HW LDAC waveform
 * generation data, it operates in trigger mode and gets triggered by DAC Update timer.
 *
 * If more than 1 SPI transactions are required, TGP Timer will be operated in
 * trigger mode, triggered by DAC Update timer.
 *
 * @param	count[in] - Number of SPI transfers per LDAC cycle.
 * @return	0 in case of success, negative error code otherwise.
 */
static int32_t ad552xr_reinit_tgp_timer(struct ad552xr_dev *dev, uint32_t count)
{
	int32_t ret;
	uint64_t period_ns = HZ_NS_CONVERT(sampling_frequency);
	uint64_t duty_cycle_ns = 0;

	/* Remove the previous PWM TGP descriptor */
	ret = no_os_pwm_remove(pwm_tgp_desc);
	if (ret) {
		return ret;
	}

	/* Reconfigure the PWM TGP descriptor based on mode */
	if ((count == 0) || ((count == 1) && (dev->ldac_cfg.ldac_hw_sw_mask))) {
		/* FREE-RUNNING MODE */
		pwm_tgp_init_params.extra = &pwm_tgp_extra_init_params;
	} else {
		/* TRIGGER MODE */
		period_ns *= count;
		pwm_tgp_init_params.extra = &pwm_tgp_trigger_mode_extra_init_params;
	}

	/* Reinitialize the timer */
	ret = no_os_pwm_init(&pwm_tgp_desc, &pwm_tgp_init_params);
	if (ret) {
		return ret;
	}

	period_ns -= TIM_COMPENSATION;
	duty_cycle_ns = LDAC_DUTY_CYCLE_NSEC(period_ns);
	ret = pwm_set_period_and_duty_cycle(pwm_tgp_desc, period_ns, duty_cycle_ns);
	if (ret) {
		return ret;
	}

	return 0;
}

#if (INTERFACE_MODE == SPI_DMA)
/**
 * @brief	AD552XR DMA callback to override no-OS DMA callback.
 * @param	context[in] - Callback context.
 * @return	None
 */
static void ad552xr_sg_callback(void *context)
{
	// Nothing to do here
	// Used to bypass default_sg_handler callback of no-os DMA.
}

/**
 * @brief	Enable SPI DMA interface mode.
 * @param	count[in] - Number of SPI transfers per LDAC cycle.
 * @return	0 in case of success, negative error code otherwise.
 */
static int32_t enable_spi_dma_interface_mode(uint32_t count)
{
	int32_t ret;

	/* Enable DMA Trigger Timer */
	ret = no_os_pwm_enable(pwm_dma_trigger_desc);
	if (ret) {
		return ret;
	}

	/* Remove the previous GPIO CS descriptor */
	ret = no_os_gpio_remove(gpio_cs_desc);
	if (ret) {
		return ret;
	}

	/* Reconfigure SPI CS pin as PWM channel output */
	ret = no_os_gpio_get(&gpio_cs_desc, &gpio_cs_pwm_init_params);
	if (ret) {
		return ret;
	}

	/* Set the flag */
	spi_dma_enabled = true;

	return 0;
}

/**
 * @brief	Disable SPI DMA interface mode.
 * @param	dev[in]- Application descriptor.
 * @return	0 in case of success, negative error code otherwise.
 */
static int32_t disable_spi_dma_interface_mode(void *dev)
{
	int32_t ret;
	struct ad552xr_dev *device = dev;

	if (!spi_dma_enabled) {
		return 0;
	}

	/* Clear the flag */
	spi_dma_enabled = false;

	/* Abort SPI transfer */
	ret = no_os_spi_transfer_abort(device->spi_desc);
	if (ret) {
		return ret;
	}

	/* Disable DMA Trigger Timer */
	ret = no_os_pwm_disable(pwm_dma_trigger_desc);
	if (ret) {
		return ret;
	}

	/* Reconfigure SPI CS pin as SPI CS */
	ret = no_os_gpio_remove(gpio_cs_desc);
	if (ret) {
		return ret;
	}

	ret = no_os_gpio_get(&gpio_cs_desc, &gpio_cs_gpio_output_init_params);
	if (ret) {
		return ret;
	}

	return 0;
}
#endif

/**
 * @brief	Populate the dac data buffer with the command words
 * 			for SW LDAC update along with the data.
 * @param	iio_dev_data[in] - IIO Device Data
 * @param	dst_data_buffer[in, out] - Destination data buffer to populate the data.
 * @param	bytes_to_transfer[out] - Number of bytes to transfer over SPI
 * @param	num_spi_transfers_per_ldac_cycle[out] - Number of SPI transfers per LDAC cycle
 * @return	0 in case of success, negative error code otherwise.
 */
static int32_t ad552xr_populate_data(
	const struct iio_device_data *iio_dev_data,
	uint8_t *dst_data_buffer,
	uint32_t *bytes_to_transfer,
	uint32_t *num_spi_transfers_per_ldac_cycle)
{
	struct ad552xr_dev *device = (struct ad552xr_dev *)iio_dev_data->dev;
	uint16_t hw_sw_mask = device->ldac_cfg.ldac_hw_sw_mask;
	int8_t* iio_buff = iio_dev_data->buffer->buf->buff;
	uint8_t ch;
	uint32_t byte_id = 0;
	uint32_t pop_buff_idx = 0;
	uint8_t *pop_dac_data = dst_data_buffer;
	uint8_t byte_addr = 0;
	uint8_t local_buff[5];
	uint8_t reg_len_sw_ldac =  AD552XR_LEN(AD552XR_REG_SW_LDAC);
	uint8_t reg_len_dac_input_a = AD552XR_LEN(AD552XR_REG_DAC_INPUT_A_CH(0));
	uint32_t reg_addr_sw_ldac;
	uint32_t reg_addr_dac_input_a;
	uint32_t reg_offset_dac_input_a;
	uint32_t active_mask = iio_dev_data->buffer->active_mask;
	uint32_t num_active_channels = no_os_hweight32(active_mask);
	uint32_t num_transfers_per_cycle = 0;
	bool func_en;


	/****** Phase 1: Prepare SW LDAC command ******/
	/* If not address ascension, start accessing from next address */
	if (!device->spi_cfg.addr_asc) {
		reg_addr_sw_ldac = AD552XR_ADDR(AD552XR_REG_SW_LDAC) + reg_len_sw_ldac - 1;
		reg_offset_dac_input_a = reg_len_dac_input_a - 1;
	} else {
		reg_addr_sw_ldac = AD552XR_ADDR(AD552XR_REG_SW_LDAC);
		reg_offset_dac_input_a = 0;
	}

	/* Build the Buffer for a SW LDAC SPI transaction */
	local_buff[byte_addr++] = (reg_addr_sw_ldac >> 8);
	local_buff[byte_addr++] = reg_addr_sw_ldac;

	/* Swap the contents of data */
	no_os_memswap64(&hw_sw_mask, reg_len_sw_ldac, reg_len_sw_ldac);
	memcpy(&local_buff[byte_addr], &hw_sw_mask, reg_len_sw_ldac);

	/* Add the register data size to calculate an actual length of SPI frame */
	byte_addr += reg_len_sw_ldac;


	/****** Phase 2: Prepare DAC Input Register A commands for active channels ******/
	/* Iterate over every sample in the IIO buffer (since the data is channel-interleaved)
	 * and prepare the DAC input register commands
	 */
	while (byte_id < iio_dev_data->buffer->size) {
		active_mask = iio_dev_data->buffer->active_mask;
		/* Prepare buffer for Input Register A update for all active channels */
		while (active_mask) {
			ch = no_os_find_first_set_bit(active_mask);
			func_en = device->ldac_cfg.func_en_mask & AD552XR_CHANNEL_SEL(ch);
			/* If function generator disabled for active channel */
			if (!func_en) {
				reg_addr_dac_input_a = AD552XR_ADDR(AD552XR_REG_DAC_INPUT_A_CH(
						ch)) + reg_offset_dac_input_a;

				pop_dac_data[pop_buff_idx++] = (reg_addr_dac_input_a >> 8);
				pop_dac_data[pop_buff_idx++] = reg_addr_dac_input_a;
				pop_dac_data[pop_buff_idx++] = iio_buff[byte_id + 1];
				pop_dac_data[pop_buff_idx++] = iio_buff[byte_id];
			}

			byte_id += iio_dev_data->buffer->bytes_per_scan / num_active_channels;
			active_mask &= ~NO_OS_BIT(ch);
		}

		/* Prepare buffer for SW LDAC register update */
		if (device->ldac_cfg.ldac_hw_sw_mask &
		    iio_dev_data->buffer->active_mask) {
			memcpy(&pop_dac_data[pop_buff_idx], local_buff, byte_addr);
			pop_buff_idx += byte_addr;
		}
	}


	/****** Phase 3: Calculate total number of SPI transactions for one LDAC cycle ******/
	/* If function generator disabled for active channel, data over SPI
	 * will be transferred as a part of waveform generation */
	num_transfers_per_cycle = no_os_hweight32(iio_dev_data->buffer->active_mask &
				  (~(device->ldac_cfg.func_en_mask)));

	/* If any channel configured in SW Mode, additional SPI transaction required
	 * to send the command */
	num_transfers_per_cycle += (device->ldac_cfg.ldac_hw_sw_mask != 0);

	/****** Phase 4: Propagate the computed results ******/
	*bytes_to_transfer = pop_buff_idx;
	*num_spi_transfers_per_ldac_cycle = num_transfers_per_cycle;

	return 0;
}

#if (INTERFACE_MODE == SPI_INTERRUPT)
/**
 * @brief	Start the device data transfer for SPI interrupt interface mode.
 * @details This function will be called from IIO Trigger ISR.
 * In the first call, it will populate the data buffer with valid data and LDAC commands.
 * Then it will enable TGP timer to trigger HW LDAC if necessary.
 * The SPI data frames will be transferred for every function call.
 * In the end of the LDAC cycle, if SW LDAC is configured for any channel,
 * it will be triggered in the end of SPI transfer.
 * This cycle will be repeated until the stop function is called.
 * @param	iio_dev_data[in] - IIO device data instance.
 * @param	dst_data_buffer[in, out] - Destination data buffer to populate the data.
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad552xr_data_transfer_start(struct iio_device_data *iio_dev_data,
				    uint8_t *dst_data_buffer)
{
	int32_t ret;
	struct ad552xr_dev *device;
	struct ad552xr_spi_intr_tx_info *info = &tx_info_spi_intr;
	struct no_os_spi_msg ad552xr_spi_msg = {0};

	if (!iio_dev_data || !dst_data_buffer) {
		return -EINVAL;
	}

	if (!iio_dev_data->dev) {
		return -EINVAL;
	}

	device = iio_dev_data->dev;

	if (!configured) {
		/* Populate the destination buffer with valid data and LDAC command */
		ret = ad552xr_populate_data(iio_dev_data,
					    dst_data_buffer,
					    &info->total_bytes_to_transfer,
					    &info->num_spi_transfers_per_cycle);
		if (ret) {
			return ret;
		}

		/* Reinitialize TGP timer based on count */
		if (use_tgp_timer) {
			ret = ad552xr_reinit_tgp_timer(device, info->num_spi_transfers_per_cycle);
			if (ret) {
				return ret;
			}
		}
	}

	/* If data has to be transferred, perform a single DAC register write*/
	if (info->total_bytes_to_transfer) {
		ad552xr_spi_msg.tx_buff = &dst_data_buffer[info->bytes_transferred];
		ad552xr_spi_msg.rx_buff = NULL;
		ad552xr_spi_msg.bytes_number = NUM_BYTES_TRANSFER;

		ret = no_os_spi_transfer(device->spi_desc, &ad552xr_spi_msg, 1);
		if (ret) {
			return ret;
		}

		info->bytes_transferred += NUM_BYTES_TRANSFER;
		info->num_spi_data_transfers_done++;

		/* Reset the bytes to transfer to zero if all bytes are transferred
		 * and hence entering circular fashion */
		if (info->bytes_transferred >= info->total_bytes_to_transfer) {
			info->bytes_transferred = 0;
		}
	}

	/* If any HW LDAC is configured, enable PWM timer on TGP pin
	 * Enable the timer after SPI transactions are initialized or else
	 * LDAC happens before data is written */
	if (!configured) {
		if (use_tgp_timer) {
			ret = no_os_pwm_enable(pwm_tgp_desc);
			if (ret) {
				return ret;
			}
		}
		configured = true;
	}

	/* Trigger SW LDAC at end of all SPI transfers if any channel configured in SW Mode */
	if (device->ldac_cfg.ldac_hw_sw_mask &&
	    ((info->num_spi_transfers_per_cycle - info->num_spi_data_transfers_done) ==
	     1)) {
		ret = ad552xr_sw_ldac_trigger(device);
		if (ret) {
			return ret;
		}

		/* Reset for next LDAC cycle */
		info->num_spi_data_transfers_done = 0;
	}

	return 0;
}

#elif (INTERFACE_MODE == SPI_DMA)
/**
 * @brief	Start the device data transfer for SPI DMA interface mode.
 * @details This function will populate the SPI buffer with valid command-data frames.
 * If SPI transactions are essential, SPI DMA configuration will be performed
 * and DMA trigger timer will be enabled.
 * If HW LDAC is configured for any channel, TGP timer will be enabled.
 * @param	iio_dev_data[in] - IIO device data instance.
 * @param	dst_data_buffer[in, out] - Destination data buffer to populate the data.
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad552xr_data_transfer_start(struct iio_device_data *iio_dev_data,
				    uint8_t *dst_data_buffer)
{
	int32_t ret;
	struct ad552xr_dev *device;
	uint32_t count;
	struct no_os_spi_msg ad552xr_spi_msg = {
		.tx_buff = dst_data_buffer,
		.rx_buff = NULL,
	};

	if (!iio_dev_data || !dst_data_buffer) {
		return -EINVAL;
	}

	if (!iio_dev_data->dev) {
		return -EINVAL;
	}

	device = iio_dev_data->dev;

	if (!configured) {
		/* Populate the destination buffer with valid data and LDAC command */
		ret = ad552xr_populate_data(iio_dev_data,
					    dst_data_buffer,
					    &ad552xr_spi_msg.bytes_number,
					    &count);
		if (ret) {
			return ret;
		}

		/* If SPI transaction required */
		if (ad552xr_spi_msg.bytes_number != 0) {
			/* Start SPI DMA transfer */
			ret = no_os_spi_transfer_dma_async(device->spi_desc, &ad552xr_spi_msg, 1, NULL,
							   NULL);
			if (ret) {
				return ret;
			}

			/* Prepare for SPI DMA interface mode */
			ret = enable_spi_dma_interface_mode(count);
			if (ret) {
				return ret;
			}
		}

		/* Reinitialize TGP timer based on count */
		/* If any HW LDAC is configured, enable PWM timer on TGP pin */
		if (use_tgp_timer) {
			ret = ad552xr_reinit_tgp_timer(device, count);
			if (ret) {
				return ret;
			}

			/* If TGP Timer operates in trigger mode, enabling at this point
			 * will not actually enable the output until the DAC update timer is enabled.
			 * If TGP Timer operates as free running counter, the output will
			 * perform LDAC independent of other channels' dependency on SW LDAC
			 * command.
			 */
			ret = no_os_pwm_enable(pwm_tgp_desc);
			if (ret) {
				return ret;
			}
		}

		/* Enable DAC Update Timer */
		ret = no_os_pwm_enable(pwm_dac_update_desc);
		if (ret) {
			return ret;
		}

		/* Set the flag */
		configured = true;
	}

	return 0;
}
#endif

/**
 * @brief	Initialize the Data Transfer System.
 * @return	0 in case of success, error code otherwise.
 */
int32_t ad552xr_data_transfer_system_init(void)
{
	int32_t ret = 0;

	/* Initialize the PWM interface to generate PWM for DAC Update Timer. */
	ret = no_os_pwm_init(&pwm_dac_update_desc, &pwm_dac_update_init_params);
	if (ret) {
		goto err_pwm_init;
	}

	/* Initialize the PWM TGP Timer. */
	ret = no_os_pwm_init(&pwm_tgp_desc, &pwm_tgp_init_params);
	if (ret) {
		goto err_pwm_init;
	}

	/* Configure GPIO TGP as GPIO Output */
	gpio_ldac_tgpx_init_params[AD552XR_LDAC_TGP_0].extra =
		&gpio_output_extra_init_params;
	ret = no_os_gpio_get(&gpio_tgp_pwm_desc, &gpio_ldac_tgpx_init_params[0]);
	if (ret) {
		return ret;
	}

#if (INTERFACE_MODE == SPI_DMA)
	/* Populate the DMA init params for DMA extra init params */
	spi_extra_init_params.dma_init = &dma_init_params;

	/* Initialize the PWM interface to generate PWM for DMA Trigger. */
	ret = no_os_pwm_init(&pwm_dma_trigger_desc, &pwm_dma_trigger_init_params);
	if (ret) {
		goto err_pwm_init;
	}
#endif

	return 0;

err_pwm_init:
	ad552xr_data_transfer_system_remove();
	return ret;
}

/**
 * @brief 	De-initialize the Data Transfer System.
 * @return	0 in case of success, error code otherwise.
 */
int32_t ad552xr_data_transfer_system_remove(void)
{
	(void)no_os_gpio_remove(gpio_tgp_pwm_desc);
	(void)no_os_pwm_remove(pwm_dac_update_desc);
	(void)no_os_pwm_remove(pwm_tgp_desc);

#if (INTERFACE_MODE == SPI_DMA)
	(void)no_os_gpio_remove(gpio_cs_desc);
	(void)no_os_pwm_remove(pwm_dma_trigger_desc);
#endif

	return 0;
}

/**
 * @brief	Prepares the device for data transfer.
 * @param 	dev[in, out]- Application descriptor.
 * @param	mask[in]- Channels select mask.
 * @return	0 in case of success, error code otherwise.
 */
int32_t ad552xr_data_transfer_prepare(void *dev, uint32_t mask)
{
	int32_t ret;
	struct ad552xr_dev *device = dev;

	if (!dev) {
		return -EINVAL;
	}

	/* If any HW LDAC is configured, enable PWM timer on TGP pin */
	use_tgp_timer = (((~device->ldac_cfg.ldac_hw_sw_mask) & mask) != 0);

	if (use_tgp_timer) {
		/* Remove the previous GPIO TGP PWM descriptor */
		ret = no_os_gpio_remove(gpio_tgp_pwm_desc);
		if (ret) {
			return ret;
		}

		/* Reconfigure GPIO TGP as PWM output */
		gpio_ldac_tgpx_init_params[AD552XR_LDAC_TGP_0].extra =
			&gpio_ldac_tgp_pwm_extra_init_params;
		ret = no_os_gpio_get(&gpio_tgp_pwm_desc, &gpio_ldac_tgpx_init_params[0]);
		if (ret) {
			return ret;
		}
	}

#if (INTERFACE_MODE == SPI_INTERRUPT)
	memset(&tx_info_spi_intr, 0, sizeof(tx_info_spi_intr));

	/* Enable DAC Update Timer */
	ret = no_os_pwm_enable(pwm_dac_update_desc);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}

/**
 * @brief	Stop the device data transfer.
 * @param	dev[in, out]- Application descriptor.
 * @return	0 in case of success, error code otherwise.
 */
int32_t ad552xr_data_transfer_stop(void *dev)
{
	int32_t ret;

	/* Disable DAC Update Timer */
	ret = no_os_pwm_disable(pwm_dac_update_desc);
	if (ret) {
		return ret;
	}

	/* If any HW LDAC is configured on active, disable PWM timer on TGP pin */
	if (use_tgp_timer) {
		ret = no_os_pwm_disable(pwm_tgp_desc);
		if (ret) {
			return ret;
		}

		/* Remove the previous GPIO TGP PWM descriptor */
		ret = no_os_gpio_remove(gpio_tgp_pwm_desc);
		if (ret) {
			return ret;
		}

		/* Reconfigure GPIO TGP as GPIO Output */
		gpio_ldac_tgpx_init_params[AD552XR_LDAC_TGP_0].extra =
			&gpio_output_extra_init_params;
		ret = no_os_gpio_get(&gpio_tgp_pwm_desc, &gpio_ldac_tgpx_init_params[0]);
		if (ret) {
			return ret;
		}
	}

	/* Reset flags */
	configured = false;
	use_tgp_timer = false;

#if (INTERFACE_MODE == SPI_DMA)
	/* End SPI DMA interface mode */
	ret = disable_spi_dma_interface_mode(dev);
	if (ret) {
		return ret;
	}
#endif

	return 0;
}
