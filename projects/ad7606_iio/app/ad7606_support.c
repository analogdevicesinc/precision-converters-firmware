/*************************************************************************//**
 *   @file   ad7606_support.c
 *   @brief  AD7606 No-OS drivers support functionality
******************************************************************************
* Copyright (c) 2020, 2022 Analog Devices, Inc.
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

#include <stdint.h>

#include "app_config.h"
#include "ad7606_support.h"
#include "no_os_error.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

#if (AD7606X_ADC_RESOLUTION == 18)
#define	SAMPLE_SIZE_IN_BYTE		3
#else
#define	SAMPLE_SIZE_IN_BYTE		2
#endif

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/*!
 * @brief	Function to get the polarity of analog input
 * @param	chn_range_bits[in] - Bits from the channel range register
 * @return	UNIPOLAR or BIPOLAR
 */
polarity_e ad7606_get_input_polarity(uint8_t chn_range_bits)
{
	polarity_e polarity;

	if (chn_range_bits >= AD7606C_UNIPOLAR_RANGE_MIN
	    && chn_range_bits <= AD7606C_UNIPOLAR_RANGE_MAX) {
		polarity = UNIPOLAR;
	} else {
		polarity = BIPOLAR;
	}

	return polarity;
}

/*!
 * @brief	Perform conversion and read single conversion sample
 * @param	dev[in] - AD7606 device instance
 * @param	adc_data[in, out] - Pointer to adc data read variable
 * @param	chn[in] - Channel for which data is to read
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad7606_read_single_sample(struct ad7606_dev *dev,
				  uint32_t *adc_data, uint8_t chn)
{
	uint32_t adc_raw[AD7606X_ADC_CHANNELS] = { 0 };
	int32_t ret;

	if (!adc_data) {
		return -EINVAL;
	}

	/* This function monitors BUSY line for EOC and read ADC result post that */
	ret = ad7606_read(dev, adc_raw);
	if (ret) {
		return ret;
	}
	
	*adc_data = adc_raw[chn];

	return 0;
}

/*!
 * @brief	Read ADC raw data for recently sampled channel
 * @param	dev[in] - AD7606 device instance
 * @param	adc_data[in, out] - Pointer to adc data read variable
 * @param	input_chn[in] - Input channel
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad7606_read_converted_sample(struct ad7606_dev *dev, uint32_t *adc_data,
				     uint8_t input_chn)
{
	uint8_t bytes_to_read;
	uint8_t buffer_offset;
	int32_t ret;

	if (!adc_data) {
		return -EINVAL;
	}

	/* Get number of bytes to read count = chn_cnt * bytes per sample */
	bytes_to_read = AD7606X_ADC_CHANNELS * SAMPLE_SIZE_IN_BYTE;
	buffer_offset = input_chn * SAMPLE_SIZE_IN_BYTE;

	/* Read data over spi interface for all ADC channels */
	memset(dev->data, 0, sizeof(dev->data));
	ret = no_os_spi_write_and_read(dev->spi_desc, dev->data, bytes_to_read);
	if (ret) {
		return ret;
	}

#if (AD7606X_ADC_RESOLUTION == 18)
	*adc_data =
		(((uint32_t)dev->data[buffer_offset] << 16) | 		// MSB
		 ((uint32_t)dev->data[buffer_offset + 1] << 8) |
		 ((uint32_t)dev->data[buffer_offset + 2])); 		// LSB
#else
	*adc_data =
		(uint16_t)(((uint16_t)dev->data[buffer_offset] << 8) |  	// MSB
			   dev->data[buffer_offset + 1]);  		// LSB
#endif

	return 0;
}
