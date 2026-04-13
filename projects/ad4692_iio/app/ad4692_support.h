/*************************************************************************//**
 *   @file   ad4692_support.h
 *   @brief  Support header file for AD4692
******************************************************************************
* Copyright (c) 2024 Analog Devices, Inc.
*
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD4692_SUPPORT_H
#define AD4692_SUPPORT_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "no_os_gpio.h"
#include "ad4692.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Maximum number of slots in the advanced sequencer */
#define AD4692_MAX_SLOTS_AS     128

/* Enable toggling of CS */
#define CS_CHANGE		1

/* Number of CNV toggles to register the channel ID */
#define AD4692_N_CNV_TOGGLES 2

/* Exit manual mode */
#define AD4692_EXIT_MANUAL_MODE	0x0

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/**
 * @enum ad4692_sequencer_modes
 * @brief AD4692 sequencer modes
 */
enum ad4692_sequencer_modes {
	STANDARD_SEQUENCER,
	ADVANCED_SEQUENCER
};

int ad4692_toggle_cnv(struct no_os_gpio_desc* gpio_cnv);
int ad4692_configure_channel_priorities(uint8_t* chn_priorities,
					uint8_t* channel_sequence, uint8_t* num_as_slots, uint8_t *acc_count);
int ad4692_configure_acc_mask(uint16_t channel_mask,
			      enum ad4692_sequencer_modes sequencer, uint8_t* chn_priorities);
int ad4692_exit_manual_mode(struct ad4692_desc *desc,
			    struct no_os_gpio_desc* cnv_desc);

#endif /* end of AD4692_SUPPORT_H */
