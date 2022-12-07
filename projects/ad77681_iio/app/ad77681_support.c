/***************************************************************************//**
 *   @file    ad77681_support.c
 *   @brief   AD7768-1 No-OS driver support file
********************************************************************************
 * Copyright (c) 2021-22 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "app_config.h"
#include "ad77681_iio.h"
#include "ad77681_support.h"
#include "no_os_error.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* AD77681 Buffer Length */
#define AD77681_SAMPLE_DATA_BUFF_LEN	6
/* AD77681 24 Bits Sign Extension Value */
#define AD77681_24_BITS_SIGN_EXTENSION	0xFFFFFF
/* AD77681 2 Bytes Shift Value */
#define	AD77681_2_BYTES_SHIFT			16
/* AD77681 1 Byte Shift Value */
#define	AD77681_1_BYTE_SHIFT			8

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/

/******************************************************************************/
/************************ Functions Definitions *******************************/
/******************************************************************************/

/*!
 * @brief	Enable continuous conversion mode
 * @return	0 in case of success or negative value otherwise
 */
int32_t ad77681_enable_cont_conv_mode(void)
{
	if (ad77681_set_conv_mode
	    (p_ad77681_dev_inst,
	     AD77681_CONV_CONTINUOUS,
	     AD77681_AIN_SHORT,
	     false) != 0) {
		return -EIO;
	}

	return 0;
}

/*!
 * @brief	Read ADC raw data for recently sampled channel
 * @param	adc_raw[in, out] - Pointer to adc data read variable
 * @return	0 in case of success or negative value otherwise
 */
int32_t ad77681_read_converted_sample(uint32_t *adc_raw)
{
	uint32_t	ad77681_sample_data = 0;
	uint8_t		ad77681_sample_data_buff[AD77681_SAMPLE_DATA_BUFF_LEN] = { 0, 0, 0, 0, 0, 0 };

	if (!adc_raw) {
		return -EINVAL;
	}

	if (ad77681_spi_read_adc_data(p_ad77681_dev_inst, ad77681_sample_data_buff,
				      AD77681_REGISTER_DATA_READ) != 0) {
		return -EIO;
	}

	ad77681_sample_data = (uint32_t)
			      (
				      (ad77681_sample_data_buff[1] << AD77681_2_BYTES_SHIFT) |
				      (ad77681_sample_data_buff[2] << AD77681_1_BYTE_SHIFT ) |
				      (ad77681_sample_data_buff[3])
			      );
	*adc_raw = ad77681_sample_data & AD77681_24_BITS_SIGN_EXTENSION;

	return 0;
}

/*!
 * @brief	Read ADC single sample data
 * @param	adc_raw[out] - Pointer to adc data read variable
 * @return	0 in case of success or negative value otherwise
 */
int32_t ad77681_read_single_sample(uint32_t *adc_raw)
{
	if (ad77681_set_conv_mode
	    (p_ad77681_dev_inst,
	     AD77681_CONV_SINGLE,
	     AD77681_AIN_SHORT,
	     false) != 0) {
		return -EIO;
	}

	if (ad77681_read_converted_sample(adc_raw) != 0) {
		return -EIO;
	}

	return 0;
}
