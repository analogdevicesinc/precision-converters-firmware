/***************************************************************************//**
*   @file   ad4692_iio.h
*   @brief  Header file of ad4692_iio
********************************************************************************
* Copyright (c) 2024, 2026 Analog Devices, Inc.
*
* All rights reserved.
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/
#ifndef AD4692_IIO_H_
#define AD4692_IIO_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include <stdbool.h>

#include "iio.h"
#include "iio_types.h"

/******************************************************************************/
/****************************** Macros ****************************************/
/******************************************************************************/

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/

/* Enum of interface modes */
enum ad4692_interface_modes {
	SPI_DMA,
	SPI_INTR
};

/* Enum of data capture modes */
enum ad4692_data_capture_modes {
	CONTINUOUS,
	BURST
};

/* AD4692 attribute unique IDs */
enum ad4692_attribute_ids {
	ADC_RAW_ATTR_ID,
	ADC_SCALE_ATTR_ID,
	ADC_OFFSET_ATTR_ID,
	ACC_COUNT_ATTR_ID,
	ADC_CHN_PRIORITY_ATTR_ID,
	NUM_OF_CHN_ATTR,

	ADC_SAMPLING_FREQUENCY_ATTR_ID,
	ADC_MODE_ATTR_ID,
	SEQUENCER_MODE_ATTR_ID,
	INTERFACE_MODE_ATTR_ID,
	DATA_CAPTURE_MODE_ATTR_ID,
	OSC_FREQUENCY_ATTR_ID,
	SEQUENCE_LENGTH_ATTR_ID,
	READBACK_OPTION_ATTR_ID,
	RESTART_IIO_ATTR_ID,
	NUM_OF_DEV_ATTR = RESTART_IIO_ATTR_ID - NUM_OF_CHN_ATTR
};

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

int ad4692_iio_attr_available_set(void *device, char *buf, uint32_t len,
				  const struct iio_ch_info *channel, intptr_t priv);
int ad4692_iio_attr_available_get(void *device, char *buf, uint32_t len,
				  const struct iio_ch_info *channel, intptr_t priv);
int ad4692_iio_attr_set(void *device, char *buf, uint32_t len,
			const struct iio_ch_info *channel, intptr_t priv);
int ad4692_iio_attr_get(void *device, char *buf, uint32_t len,
			const struct iio_ch_info *channel, intptr_t priv);

extern struct ad4692_desc *ad4692_dev;
extern uint32_t ad4692_sampling_frequency;
extern enum ad4692_interface_modes ad4692_interface_mode;
extern enum ad4692_data_capture_modes ad4692_data_capture_mode;
extern uint8_t buf_offset;

#endif /* AD4692_IIO_H_ */
