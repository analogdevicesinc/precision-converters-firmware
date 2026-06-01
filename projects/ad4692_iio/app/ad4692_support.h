/*************************************************************************//**
 *   @file   ad4692_support.h
 *   @brief  Support header file for AD4692
******************************************************************************
* Copyright (c) 2024, 2026 Analog Devices, Inc.
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
#include "iio.h"
#include "iio_trigger.h"
#include "ad4692.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Maximum number of slots in the advanced sequencer */
#define AD4692_MAX_SLOTS_AS     128

/* Enable toggling of CS */
#define CS_CHANGE		1

/* Number of bytes per transaction for accumulator data */
#define AD4692_N_BYTES_TXN_24BIT	5

/* Number of bytes per transaction for averaged data */
#define AD4692_N_BYTES_TXN_16BIT	4

/* Number of bytes of command for data capture */
#define AD4692_N_BYTES_TXN_OFFSET	2

/* Number of CNV toggles to register the channel ID */
#define AD4692_N_CNV_TOGGLES 2

/* Exit manual mode */
#define AD4692_EXIT_MANUAL_MODE	0x0

/* Converts pwm period in nanoseconds to sampling frequency in samples per second */
#define PWM_PERIOD_TO_FREQUENCY(x)       (1E9 / x)

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

/**
 * @enum ad4692_readback_options
 * @brief AD4692 readback options
 */
enum ad4692_readback_options {
	AVERAGED_DATA,
	ACCUMULATOR_DATA
};

/**
 * @struct ad4692_data_transfer_system
 * @brief AD4692 Data transfer system function pointers (per ADC mode)
 */
struct ad4692_data_transfer_system {
	int32_t (*initialize)(struct ad4692_desc *desc);
	int32_t (*prepare_transfer)(void *dev, uint32_t ch_mask);
	int32_t (*submit_samples)(struct iio_device_data *iio_dev_data);
	int32_t (*trigger_handler)(struct iio_device_data *iio_dev_data);
	int32_t (*end_transfer)(void *dev);
	int32_t (*remove)(struct ad4692_desc *desc);
	int32_t (*read_converted_data)(struct ad4692_desc *desc, uint8_t chn,
				       uint32_t *adc_data);
	int32_t (*update_sampling_frequency)(uint32_t *sampling_rate);
	uint32_t (*get_max_sampling_rate)(void);
};

/* Mode-specific data transfer system instances */
extern struct ad4692_data_transfer_system ad4692_data_transfer_manual_mode;
extern struct ad4692_data_transfer_system ad4692_data_transfer_cnv_clock_mode;
extern struct ad4692_data_transfer_system ad4692_data_transfer_cnv_burst_mode;
extern struct ad4692_data_transfer_system ad4692_data_transfer_spi_burst_mode;

/* Shared state accessed by mode files */
extern uint8_t ad4692_active_channels[];
extern uint8_t num_of_active_channels;
extern uint16_t channel_mask;
extern uint8_t ad4692_acc_count[];
extern uint8_t channel_priorities[];
extern uint8_t n_data_bytes;
extern uint8_t n_bytes_per_transaction;
extern volatile bool ad4692_conversion_flag;
extern volatile bool buf_size_updated;
extern enum ad4692_sequencer_modes ad4692_sequencer_mode;
extern enum ad4692_readback_options ad4692_readback_option;
extern enum ad4692_int_osc_sel ad4692_osc_freq_id;
extern struct iio_hw_trig *ad4692_hw_trig_desc;

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

/* Utility functions */
int ad4692_configure_channel(struct ad4692_desc *desc);
int ad4692_configure_channel_priorities(uint8_t *chn_priorities,
					uint8_t* channel_sequence, uint8_t *num_as_slots, uint8_t *acc_count);
int ad4692_configure_acc_mask(uint16_t channel_mask,
			      enum ad4692_sequencer_modes sequencer, uint8_t *chn_priorities);
int ad4692_exit_manual_mode(struct ad4692_desc *desc);
int ad4692_configure_pwm_rate(struct no_os_pwm_desc *desc,
			      uint32_t sampling_rate);
void ad4692_get_tx_command(uint8_t *local_tx_data);

/* Common utility functions */
void ad4692_update_active_channels(uint32_t ch_mask);

/* Delegation functions */
int32_t ad4692_data_transfer_init(struct ad4692_desc *desc,
				  enum ad4692_spi_mode mode);
int32_t ad4692_data_transfer_prepare(void *dev, uint32_t ch_mask);
int32_t ad4692_data_transfer_submit(struct iio_device_data *iio_dev_data);
int32_t ad4692_data_transfer_trigger_handler(struct iio_device_data
		*iio_dev_data);
int32_t ad4692_data_transfer_end(void *dev);
int32_t ad4692_data_transfer_remove(struct ad4692_desc *desc);
int32_t ad4692_data_transfer_read_converted_data(struct ad4692_desc *desc,
		uint8_t chn, uint32_t *adc_data);
int32_t ad4692_data_transfer_update_freq(uint32_t *sampling_rate);
uint32_t ad4692_get_max_sampling_rate(enum ad4692_spi_mode mode);

#endif /* end of AD4692_SUPPORT_H */
