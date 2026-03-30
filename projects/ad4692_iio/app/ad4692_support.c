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

#include "ad4692_support.h"
#include "no_os_error.h"
#include "no_os_delay.h"
#include "app_config.h"
#include "ad4692_iio.h"
#include "string.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/**
 * @brief Toggle CNV GPIO
 * @param gpio_cnv[in, out] - GPIO CNV Descriptor
 * @return 0 in case of success, negative error code otherwise.
 */
int ad4692_toggle_cnv(struct no_os_gpio_desc* gpio_cnv)
{
	int ret;

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
 * @brief Configure the accumulator mask
 * @param channel_mask[in] - Channel mask (Applicable in case of standard sequencer)
 * @param sequencer[in] - Sequencer mode
 * @param chn_priorities[in] - Channel priority (Applicable in case of advanced sequencer)
 * @return 0 in case of success, negative error code otherwise.
 */
int ad4692_configure_acc_mask(uint16_t channel_mask,
			      enum ad4692_sequencer_modes sequencer, uint8_t* chn_priorities)
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
				chn_mask &= ~(1 << ch_id);
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
int ad4692_configure_channel_priorities(uint8_t* chn_priorities,
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
