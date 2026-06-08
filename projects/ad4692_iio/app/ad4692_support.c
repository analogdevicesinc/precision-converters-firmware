/*************************************************************************//**
 *   @file   ad4692_support.c
 *   @brief  Support file for AD4692 device
******************************************************************************
* Copyright (c) 2024 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "string.h"
#include "ad4692_support.h"
#include "ad4692_iio.h"
#include "app_config.h"

#include "no_os_error.h"
#include "no_os_delay.h"
#include "no_os_util.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* Flag to indicate if size of the buffer is updated according to requested
 * number of samples for the multi-channel IIO buffer data alignment */
volatile bool buf_size_updated = false;

/* Active mode data transfer system pointer */
static struct ad4692_data_transfer_system *ad4692_active_mode;

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief Initialize the data transfer system for a given ADC mode
 * @param desc[in] - AD4692 device descriptor
 * @param mode[in] - ADC mode to initialize
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad4692_data_transfer_init(struct ad4692_desc *desc,
				  enum ad4692_spi_mode mode)
{
	int32_t ret;

	ret = init_pwm();
	if (ret) {
		return ret;
	}

	switch (mode) {
	case AD4692_MANUAL_MODE:
		ad4692_active_mode = &ad4692_data_transfer_manual_mode;
		break;
	case AD4692_CNV_CLOCK:
		ad4692_active_mode = &ad4692_data_transfer_cnv_clock_mode;
		break;
	case AD4692_CNV_BURST:
		ad4692_active_mode = &ad4692_data_transfer_cnv_burst_mode;
		break;
	case AD4692_SPI_BURST:
		ad4692_active_mode = &ad4692_data_transfer_spi_burst_mode;
		break;
	default:
		return -EINVAL;
	}

	if (ad4692_active_mode->initialize) {
		ret = ad4692_active_mode->initialize(desc);
	}

	if (ret) {
		NO_OS_UNUSED_PARAM(remove_pwm());
		return ret;
	}

	return 0;
}

/**
 * @brief Delegation: read converted data from active mode
 * @param desc[in] - AD4692 device descriptor
 * @param chn[in] - Channel number to read
 * @param adc_data[out] - Pointer to store the converted data
 * @return 0 in case of success, negative error code otherwise
 */
int32_t ad4692_data_transfer_read_converted_data(struct ad4692_desc *desc,
		uint8_t chn, uint32_t *adc_data)
{
	if (!ad4692_active_mode) {
		return -EINVAL;
	}

	if (ad4692_active_mode->read_converted_data) {
		return ad4692_active_mode->read_converted_data(desc, chn, adc_data);
	}

	return -ENOSYS;
}


/**
 * @brief Update active channels based on sequencer mode and channel mask
 * @param ch_mask[in] - Channel mask (used in standard sequencer mode)
 */
void ad4692_update_active_channels(uint32_t ch_mask)
{
	uint8_t ch_id;

	num_of_active_channels = 0;
	memset(ad4692_active_channels, 0, NO_OF_CHANNELS);

	if (ad4692_sequencer_mode == STANDARD_SEQUENCER) {
		for (ch_id = 0; ch_id < NO_OF_CHANNELS; ch_id++) {
			if (ch_mask & NO_OS_BIT(ch_id)) {
				ad4692_active_channels[num_of_active_channels++] = ch_id;
			}
		}
		channel_mask = ch_mask;
	} else {
		for (ch_id = 0; ch_id < NO_OF_CHANNELS; ch_id++) {
			if (channel_priorities[ch_id] != 0) {
				ad4692_active_channels[num_of_active_channels++] = ch_id;
			}
		}
	}

	return;
}

/**
 * @brief Delegation: prepare transfer for active mode
 * @param dev[in] - IIO device instance
 * @param ch_mask[in] - Channel mask indicating which channels to enable
 * @return 0 in case of success, -EINVAL if no active mode, 0 if function not implemented
 */
int32_t ad4692_data_transfer_prepare(void *dev, uint32_t ch_mask)
{
	if (!ad4692_active_mode) {
		return -EINVAL;
	}

	if (ad4692_active_mode->prepare_transfer) {
		return ad4692_active_mode->prepare_transfer(dev, ch_mask);
	}

	return 0;
}

/**
 * @brief Delegation: end transfer for active mode
 * @param dev[in] - IIO device instance
 * @return 0 in case of success, -EINVAL if no active mode, 0 if function not implemented
 */
int32_t ad4692_data_transfer_end(void *dev)
{
	if (!ad4692_active_mode) {
		return -EINVAL;
	}

	if (ad4692_active_mode->end_transfer) {
		return ad4692_active_mode->end_transfer(dev);
	}

	return 0;
}

/**
 * @brief Delegation: remove active mode resources
 * @param desc[in] - AD4692 device descriptor
 * @return 0 in case of success, -EINVAL if no active mode, 0 if function not implemented
 */
int32_t ad4692_data_transfer_remove(struct ad4692_desc *desc)
{
	if (!ad4692_active_mode) {
		return -EINVAL;
	}

	if (ad4692_active_mode->remove) {
		return ad4692_active_mode->remove(desc);
	}

	return 0;
}

/**
 * @brief Delegation: submit samples for active mode
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success, -EINVAL if no active mode, 0 if function not implemented
 */
int32_t ad4692_data_transfer_submit(struct iio_device_data *iio_dev_data)
{
	if (!ad4692_active_mode) {
		return -EINVAL;
	}

	if (ad4692_active_mode->submit_samples) {
		return ad4692_active_mode->submit_samples(iio_dev_data);
	}

	return 0;
}

/**
 * @brief Delegation: trigger handler for active mode
 * @param iio_dev_data[in] - IIO device data instance
 * @return 0 in case of success, -EINVAL if no active mode, 0 if function not implemented
 */
int32_t ad4692_data_transfer_trigger_handler(struct iio_device_data
		*iio_dev_data)
{
	if (!ad4692_active_mode) {
		return -EINVAL;
	}

	if (ad4692_active_mode->trigger_handler) {
		return ad4692_active_mode->trigger_handler(iio_dev_data);
	}

	return 0;
}

/**
 * @brief Delegation: update sampling frequency for active mode
 * @param sampling_rate[in,out] - Pointer to requested/actual sampling rate
 * @return 0 in case of success, -EINVAL if no active mode, 0 if function not implemented
 */
int32_t ad4692_data_transfer_update_freq(uint32_t *sampling_rate)
{
	if (!ad4692_active_mode) {
		return -EINVAL;
	}

	if (ad4692_active_mode->update_sampling_frequency) {
		return ad4692_active_mode->update_sampling_frequency(sampling_rate);
	}

	return 0;
}

/**
 * @brief Get maximum sampling rate for a given ADC mode
 * @param mode[in] - ADC mode
 * @return Maximum sampling rate in samples per second
 */
uint32_t ad4692_get_max_sampling_rate(enum ad4692_spi_mode mode)
{
	switch (mode) {
	case AD4692_MANUAL_MODE:
		return ad4692_data_transfer_manual_mode.get_max_sampling_rate();
	case AD4692_CNV_CLOCK:
		return ad4692_data_transfer_cnv_clock_mode.get_max_sampling_rate();
	case AD4692_CNV_BURST:
		return ad4692_data_transfer_cnv_burst_mode.get_max_sampling_rate();
	case AD4692_SPI_BURST:
		return ad4692_data_transfer_spi_burst_mode.get_max_sampling_rate();
	default:
		return 0;
	}
}

/**
 * @brief  Get the Tx buffer respective to the enabled channels
 * @param local_tx_data[out] - Tx buffer to populate with channel commands
 * @return None
 */
void ad4692_get_tx_command(uint8_t *local_tx_data)
{
	uint8_t ch_id;
	uint8_t index;

	if (ad4692_interface_mode == SPI_DMA) {
		for (ch_id = 0; ch_id < num_of_active_channels; ch_id++) {
			index = ad4692_active_channels[ch_id];
			local_tx_data[ch_id * BYTES_PER_SAMPLE] = AD4692_IN_COMMAND(index);
			local_tx_data[(ch_id * BYTES_PER_SAMPLE) + 1] = 0;
		}
	} else {
		index = 0;
		for (ch_id = 0; ch_id < num_of_active_channels; ch_id++) {
			if (ad4692_readback_option == ACCUMULATOR_DATA) {
				local_tx_data[index++] = AD4692_RW_ADDR_MASK | AD4692_MSB_MASK(
								 AD4692_ACC_IN_REG(
										 ad4692_active_channels[ch_id]));
				local_tx_data[index++] = AD4692_LSB_MASK(AD4692_ACC_IN_REG(
								 ad4692_active_channels[ch_id]));
				local_tx_data[index++] = 0x0;
				local_tx_data[index++] = 0x0;
				local_tx_data[index++] = 0x0;
			} else {
				local_tx_data[index++] = AD4692_RW_ADDR_MASK | AD4692_MSB_MASK(
								 AD4692_AVG_IN_REG(
										 ad4692_active_channels[ch_id]));
				local_tx_data[index++] = AD4692_LSB_MASK(AD4692_AVG_IN_REG(
								 ad4692_active_channels[ch_id]));
				local_tx_data[index++] = 0x0;
				local_tx_data[index++] = 0x0;
			}
		}
	}
}

/**
 * @brief Configure the accumulator mask
 * @param channel_mask[in] - Channel mask (Applicable in case of standard sequencer)
 * @param sequencer[in] - Sequencer mode
 * @param chn_priorities[in] - Channel priority (Applicable in case of advanced sequencer)
 * @return 0 in case of success, negative error code otherwise.
 */
int ad4692_configure_acc_mask(uint16_t channel_mask,
			      enum ad4692_sequencer_modes sequencer, uint8_t *chn_priorities)
{
	int ret;
	uint16_t chn_mask = 0xFFFF;
	uint8_t ch_id;

	if (sequencer == STANDARD_SEQUENCER) {
		/* Invert the channel mask, as the Accumulator mask register
		 * follows the inverse logic to mask the channels in the sequencer */
		chn_mask = ~channel_mask;
	} else { // ADVANCED_SEQUENCER
		/* Build the channel mask depending on the channel priorities set */
		for (ch_id = 0; ch_id < NO_OF_CHANNELS; ch_id++) {
			if (chn_priorities[ch_id] > 0) {
				chn_mask &= ~NO_OS_BIT(ch_id);
			}
		}
	}

	/* Configure ACC Mask 1 register */
	ret = ad4692_reg_write(ad4692_dev, AD4692_ACC_MASK1_REG, (chn_mask & 0xFF));
	if (ret) {
		return ret;
	}

	/* Configure ACC mask 2 register*/
	ret = ad4692_reg_write(ad4692_dev, AD4692_ACC_MASK2_REG,
			       ((chn_mask & 0xFF00) >> 8));
	if (ret) {
		return ret;
	}

	return 0;
}


/**
 * @brief Configure the advanced sequencer slots
 * @param chn_priorities[in] - channel priorities to be configured
 * @param channel_sequence[out] - Channel sequence as configured in the AS Slots
 * @param num_of_as_slots[out] - Number of slots in the advanced sequencer
 * @param acc_count[in] - Accumulator count limit
 * @return 0 in case of success, negative error code otherwise.
 * @details This function configures the advanced sequencer slots based on the
 * priorities assigned to each channel(max 2 priorities are allowed in the application).
 * Example: If the user configures the following channels as P1 (highest priority): Ch0,Ch1,Ch2
 * and the following as P2 (Least priority): Ch3,Ch4
 * Then the advanced sequencer configurations would look like:
 * Ch0-Ch1-Ch2-Ch3-Ch0-Ch1-Ch2-Ch4
 */
int ad4692_configure_channel_priorities(uint8_t *chn_priorities,
					uint8_t *channel_sequence, uint8_t *num_of_as_slots, uint8_t *acc_count)
{
	uint8_t channel_sequence_p1[AD4692_MAX_SLOTS_AS] = { 0x0 };
	uint8_t channel_sequence_p2[AD4692_MAX_SLOTS_AS] = { 0x0 };
	uint8_t n_sequence = 0;
	uint8_t as_slot_id = 0;
	uint8_t np1_slots = 0;
	uint8_t np2_slots = 0;
	uint8_t p1_id = 0;
	uint8_t p2_id = 0;
	uint8_t ch_id;
	uint8_t i;
	uint8_t j;
	int ret;

	if (!chn_priorities) {
		return -EINVAL;
	}

	/* Set the Advanced sequencer slots to default values before configuring them */
	for (i = 0; i < AD4692_MAX_SLOTS_AS; i++) {
		ret = ad4692_reg_write(ad4692_dev, AD4692_AS_SLOT_REG(i), 0x0);
		if (ret) {
			return ret;
		}
	}

	/* Set the accumulator count to all 0, before configuring the limits */
	memset(acc_count, 0, NO_OF_CHANNELS);

	/* Build the channel sequence for p1 */
	for (ch_id = 0; ch_id < NO_OF_CHANNELS; ch_id++) {
		if (chn_priorities[ch_id] == 1) {
			channel_sequence_p1[p1_id++] = ch_id;
			np1_slots++;
		} else if (chn_priorities[ch_id] == 2) {
			channel_sequence_p2[p2_id++] = ch_id;
			np2_slots++;
		}
	}

	/* Return if no slots have been configured */
	if ((np1_slots == 0) && (np2_slots == 0)) {
		return 0;
	}

	/* Determine the number of slots needed and the sequence repeat length */
	if ((np1_slots > 0) && (np2_slots > 0)) {
		*num_of_as_slots = (np1_slots * np2_slots) + np2_slots;
		n_sequence = np2_slots;
	} else if (np2_slots == 0) {
		*num_of_as_slots = np1_slots;
		n_sequence = 1;
	} else if (np1_slots == 0) {
		*num_of_as_slots = np2_slots;
		n_sequence = 1;
	}

	/* Build the command word with P1 channels,  */
	if (np1_slots > 0) {
		for (i = 0; i < n_sequence; i++) {
			for (j = 0; j < np1_slots; j++) {
				channel_sequence[as_slot_id++] = channel_sequence_p1[j];
			}

			/* Reserve a slot for P2 after filling up the channels for P1 */
			as_slot_id += 1;
		}

		as_slot_id = 0;
		as_slot_id = np1_slots;
	}

	/* Fill up the P2 slots in the reserved slots */
	if (np2_slots > 0) {
		for (i = 0; i < np2_slots; i++) {
			channel_sequence[as_slot_id] = channel_sequence_p2[i];
			as_slot_id = as_slot_id + (np1_slots) + 1;
		}
	}

	/* Configure the number of advanced sequencer slots */
	ret = ad4692_std_seq_ch(ad4692_dev, 0, false, *num_of_as_slots - 1);
	if (ret) {
		return ret;
	}

	/* Configure the Advanced sequencer slots */
	for (i = 0; i < *num_of_as_slots; i++) {
		ret = ad4692_reg_write(ad4692_dev, AD4692_AS_SLOT_REG(i), channel_sequence[i]);
		if (ret) {
			return ret;
		}
	}

	/* Configure the accumulator count registers */
	for (i = 0; i < NO_OF_CHANNELS; i++) {
		if (chn_priorities[i] == 1) {
			acc_count[i] = np2_slots - 1;
		} else {
			acc_count[i] = 0;
		}
	}

	/* Configure the accumulator mask */
	ret = ad4692_configure_acc_mask(0x0, ADVANCED_SEQUENCER, chn_priorities);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Configure the per channel accumulator count limit and
 *        enable the desired channels in the AD4692 device.
 * @param desc[in] - AD4692 device descriptor
 * @return 0 in case of success, negative error code otherwise
 */
int ad4692_configure_channel(struct ad4692_desc *desc)
{
	int ret;
	uint8_t ch_id;

	if (!desc) {
		return -EINVAL;
	}

	if (ad4692_sequencer_mode == STANDARD_SEQUENCER) {
		/* Enable the desired channels */
		ret = ad4692_std_seq_ch(desc, channel_mask, true, 0);
		if (ret) {
			return ret;
		}

		/* Configure accumulator mask */
		ret = ad4692_configure_acc_mask(channel_mask, ad4692_sequencer_mode,
						channel_priorities);
		if (ret) {
			return ret;
		}
	}

	/* Configure the accumulator count limit */
	for (ch_id = 0; ch_id < num_of_active_channels; ch_id++) {
		ret = ad4692_reg_write(desc,
				       AD4692_ACC_COUNT_LIMIT_REG(ad4692_active_channels[ch_id]),
				       ad4692_acc_count[ad4692_active_channels[ch_id]]);
		if (ret) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Exit manual mode and switch to CNV clock mode.
 * @param desc[in] - AD4692 device descriptor
 * @return 0 in case of success, negative error code otherwise.
 */
int ad4692_exit_manual_mode(struct ad4692_desc *desc)
{
	int ret;
	uint8_t buff[BYTES_PER_SAMPLE] = { AD4692_EXIT_COMMAND, 0x0 };

	struct no_os_spi_msg ad4692_spi_msg = {
		.cs_change = CS_CHANGE,
		.tx_buff = buff,
		.rx_buff = buff,
		.bytes_number = BYTES_PER_SAMPLE
	};

	if (!desc) {
		return -EINVAL;
	}

	ret = no_os_spi_transfer(desc->comm_desc, &ad4692_spi_msg, 1);
	if (ret) {
		return ret;
	}

	/* Temporary assignment of mode */
	desc->mode = AD4692_CNV_CLOCK;

	ret = ad4692_reg_update(desc, AD4692_ADC_SETUP_REG,
				AD4692_MODE_MASK,
				desc->mode);
	if (ret) {
		return ret;
	}

	return 0;
}

/**
 * @brief Configure PWM with optimal prescaler for given sampling rate
 * @param desc[in] - PWM descriptor to configure
 * @param sampling_rate[in] - Desired sampling rate in samples per second
 * @return 0 in case of success, negative error code otherwise
 */
int ad4692_configure_pwm_rate(struct no_os_pwm_desc *desc,
			      uint32_t sampling_rate)
{
	int ret;
	uint64_t pwm_period_ns;
	uint32_t prescaler;

	pwm_period_ns = CONV_TRIGGER_PERIOD_NSEC(sampling_rate);

	ret = compute_optimal_prescaler(desc->extra,
					pwm_period_ns, &prescaler);
	if (ret) {
		return ret;
	}

	ret = set_timer_prescaler(desc, prescaler);
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_set_period(desc, pwm_period_ns);
	if (ret) {
		return ret;
	}

	ret = no_os_pwm_set_duty_cycle(desc, CNV_ON_TIME);
	if (ret) {
		return ret;
	}

	return 0;
}
