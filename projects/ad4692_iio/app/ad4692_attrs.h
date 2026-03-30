/***************************************************************************//**
*   @file   ad4692_attrs.h
*   @brief  Header file of ad4692 IIO attributes
********************************************************************************
* Copyright (c) 2025 Analog Devices, Inc.
*
* All rights reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef AD4692_ATTRS_H_
#define AD4692_ATTRS_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "app_config.h"
#include "iio.h"
#include "ad4692_iio.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* IIO Channel attribute definition */
#define AD4692_CHN_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = ad4692_iio_attr_get,\
		.store = ad4692_iio_attr_set\
}

/* IIO Channel available attribute definition */
#define AD4692_CHN_AVAIL_ATTR(_name, _priv) {\
		.name = _name,\
		.priv = _priv,\
		.show = ad4692_iio_attr_available_get,\
		.store = ad4692_iio_attr_available_set\
}

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* AD4692 channel attributes
 * Sequencer: Standard Sequencer
 * ADC Modes: All Modes */
static struct iio_attribute
	ad4692_std_seq_ch_attr[NUM_OF_IIO_DEVICES][4] = {
	{
		AD4692_CHN_ATTR("raw", ADC_RAW_ATTR_ID),
		AD4692_CHN_ATTR("scale", ADC_SCALE_ATTR_ID),
		AD4692_CHN_ATTR("offset", ADC_OFFSET_ATTR_ID),

		END_ATTRIBUTES_ARRAY,
	},
};

/* AD4692 channel attributes
 * Sequencer: Advanced Sequencer
 * ADC Modes: All Modes */
static struct iio_attribute
	ad4692_adv_seq_ch_attr[NUM_OF_IIO_DEVICES][5] = {
	{
		AD4692_CHN_ATTR("raw", ADC_RAW_ATTR_ID),
		AD4692_CHN_ATTR("scale", ADC_SCALE_ATTR_ID),
		AD4692_CHN_ATTR("offset", ADC_OFFSET_ATTR_ID),
		AD4692_CHN_ATTR("acc_depth", ACC_COUNT_ATTR_ID),

		END_ATTRIBUTES_ARRAY,
	},
	{
		AD4692_CHN_ATTR("priority", ADC_CHN_PRIORITY_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("priority_available", ADC_CHN_PRIORITY_ATTR_ID),

		END_ATTRIBUTES_ARRAY,
	}
};

/* AD4692 Global attributes
 * Sequencer: Standard Sequencer (manual mode supports only standard sequencer)
 * ADC Modes: Manual Mode only */
static struct iio_attribute
	ad4692_manual_global_attr[NUM_OF_IIO_DEVICES][12] = {
	{
		AD4692_CHN_ATTR("sampling_frequency", ADC_SAMPLING_FREQUENCY_ATTR_ID),

		END_ATTRIBUTES_ARRAY
	},
	{
		AD4692_CHN_ATTR("adc_mode", ADC_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("adc_mode_available", ADC_MODE_ATTR_ID),
		AD4692_CHN_ATTR("sequencer_mode", SEQUENCER_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("sequencer_mode_available", SEQUENCER_MODE_ATTR_ID),
#if defined (TEST_MODE)
		AD4692_CHN_ATTR("interface_mode", INTERFACE_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("interface_mode_available", INTERFACE_MODE_ATTR_ID),
#endif
		AD4692_CHN_ATTR("data_capture_mode", DATA_CAPTURE_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("data_capture_mode_available", DATA_CAPTURE_MODE_ATTR_ID),
		AD4692_CHN_ATTR("readback_option", READBACK_OPTION_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("readback_option_available", READBACK_OPTION_ATTR_ID),
		AD4692_CHN_ATTR("reconfigure_system", RESTART_IIO_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("reconfigure_system_available", RESTART_IIO_ATTR_ID),

		END_ATTRIBUTES_ARRAY
	}
};

/* AD4692 Global attributes
 * Sequencer: Advanced Sequencer
 * ADC Modes: CNV Burst and SPI Burst Mode only */
static struct iio_attribute
	ad4692_adv_seq_burst_global_attr[NUM_OF_IIO_DEVICES][12] = {
	{
		AD4692_CHN_ATTR("sampling_frequency", ADC_SAMPLING_FREQUENCY_ATTR_ID),
		AD4692_CHN_ATTR("channel_seq_length", SEQUENCE_LENGTH_ATTR_ID),
		AD4692_CHN_ATTR("oscillator_frequency", OSC_FREQUENCY_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("oscillator_frequency_available", OSC_FREQUENCY_ATTR_ID),

		END_ATTRIBUTES_ARRAY
	},
	{
		AD4692_CHN_ATTR("adc_mode", ADC_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("adc_mode_available", ADC_MODE_ATTR_ID),
		AD4692_CHN_ATTR("sequencer_mode", SEQUENCER_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("sequencer_mode_available", SEQUENCER_MODE_ATTR_ID),
#if defined (TEST_MODE)
		AD4692_CHN_ATTR("interface_mode", INTERFACE_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("interface_mode_available", INTERFACE_MODE_ATTR_ID),
#endif
		AD4692_CHN_ATTR("data_capture_mode", DATA_CAPTURE_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("data_capture_mode_available", DATA_CAPTURE_MODE_ATTR_ID),
		AD4692_CHN_ATTR("readback_option", READBACK_OPTION_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("readback_option_available", READBACK_OPTION_ATTR_ID),
		AD4692_CHN_ATTR("reconfigure_system", RESTART_IIO_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("reconfigure_system_available", RESTART_IIO_ATTR_ID),

		END_ATTRIBUTES_ARRAY
	}
};

/* AD4692 Global attributes
 * Sequencer: Advanced Sequencer
 * ADC Modes: CNV clock Mode only */
static struct iio_attribute
	ad4692_adv_seq_cnv_clock_global_attr[NUM_OF_IIO_DEVICES][12] = {
	{
		AD4692_CHN_ATTR("sampling_frequency", ADC_SAMPLING_FREQUENCY_ATTR_ID),
		AD4692_CHN_ATTR("channel_seq_length", SEQUENCE_LENGTH_ATTR_ID),

		END_ATTRIBUTES_ARRAY
	},
	{
		AD4692_CHN_ATTR("adc_mode", ADC_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("adc_mode_available", ADC_MODE_ATTR_ID),
		AD4692_CHN_ATTR("sequencer_mode", SEQUENCER_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("sequencer_mode_available", SEQUENCER_MODE_ATTR_ID),
#if defined (TEST_MODE)
		AD4692_CHN_ATTR("interface_mode", INTERFACE_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("interface_mode_available", INTERFACE_MODE_ATTR_ID),
#endif
		AD4692_CHN_ATTR("data_capture_mode", DATA_CAPTURE_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("data_capture_mode_available", DATA_CAPTURE_MODE_ATTR_ID),
		AD4692_CHN_ATTR("readback_option", READBACK_OPTION_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("readback_option_available", READBACK_OPTION_ATTR_ID),
		AD4692_CHN_ATTR("reconfigure_system", RESTART_IIO_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("reconfigure_system_available", RESTART_IIO_ATTR_ID),

		END_ATTRIBUTES_ARRAY
	}
};

/* AD4692 Global attributes
 * Sequencer: Standard Sequencer
 * ADC Modes: CNV clock Mode only */
static struct iio_attribute
	ad4692_std_seq_cnv_clock_global_attr[NUM_OF_IIO_DEVICES][12] = {
	{
		AD4692_CHN_ATTR("sampling_frequency", ADC_SAMPLING_FREQUENCY_ATTR_ID),
		AD4692_CHN_ATTR("acc_depth", ACC_COUNT_ATTR_ID),

		END_ATTRIBUTES_ARRAY
	},
	{
		AD4692_CHN_ATTR("adc_mode", ADC_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("adc_mode_available", ADC_MODE_ATTR_ID),
		AD4692_CHN_ATTR("sequencer_mode", SEQUENCER_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("sequencer_mode_available", SEQUENCER_MODE_ATTR_ID),
#if defined (TEST_MODE)
		AD4692_CHN_ATTR("interface_mode", INTERFACE_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("interface_mode_available", INTERFACE_MODE_ATTR_ID),
#endif
		AD4692_CHN_ATTR("data_capture_mode", DATA_CAPTURE_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("data_capture_mode_available", DATA_CAPTURE_MODE_ATTR_ID),
		AD4692_CHN_ATTR("readback_option", READBACK_OPTION_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("readback_option_available", READBACK_OPTION_ATTR_ID),
		AD4692_CHN_ATTR("reconfigure_system", RESTART_IIO_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("reconfigure_system_available", RESTART_IIO_ATTR_ID),

		END_ATTRIBUTES_ARRAY
	}
};


/* AD4692 Global attributes
 * Sequencer: Standard Sequencer
 * ADC Modes: CNV Burst and SPI Burst Mode */
static struct iio_attribute
	ad4692_std_seq_burst_global_attr[NUM_OF_IIO_DEVICES][12] = {
	{
		AD4692_CHN_ATTR("sampling_frequency", ADC_SAMPLING_FREQUENCY_ATTR_ID),
		AD4692_CHN_ATTR("acc_depth", ACC_COUNT_ATTR_ID),
		AD4692_CHN_ATTR("oscillator_frequency", OSC_FREQUENCY_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("oscillator_frequency_available", OSC_FREQUENCY_ATTR_ID),

		END_ATTRIBUTES_ARRAY
	},
	{
		AD4692_CHN_ATTR("adc_mode", ADC_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("adc_mode_available", ADC_MODE_ATTR_ID),
		AD4692_CHN_ATTR("sequencer_mode", SEQUENCER_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("sequencer_mode_available", SEQUENCER_MODE_ATTR_ID),
#if defined (TEST_MODE)
		AD4692_CHN_ATTR("interface_mode", INTERFACE_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("interface_mode_available", INTERFACE_MODE_ATTR_ID),
#endif
		AD4692_CHN_ATTR("data_capture_mode", DATA_CAPTURE_MODE_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("data_capture_mode_available", DATA_CAPTURE_MODE_ATTR_ID),
		AD4692_CHN_ATTR("readback_option", READBACK_OPTION_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("readback_option_available", READBACK_OPTION_ATTR_ID),
		AD4692_CHN_ATTR("reconfigure_system", RESTART_IIO_ATTR_ID),
		AD4692_CHN_AVAIL_ATTR("reconfigure_system_available", RESTART_IIO_ATTR_ID),

		END_ATTRIBUTES_ARRAY
	}
};

#endif // AD4692_ATTRS_H_