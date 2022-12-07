/***************************************************************************//**
 *   @file    ad7689_support.c
 *   @brief   AD7689 no-os drivers support file
********************************************************************************
 * Copyright (c) 2022 Analog Devices, Inc.
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

#include "ad7689_support.h"
#include "ad7689_iio.h"
#include "ad7689_user_config.h"
#include "no_os_delay.h"
#include "no_os_spi.h"
#include "no_os_util.h"
#include "no_os_gpio.h"
#include "no_os_error.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

#define BYTE_SIZE		8

/* Config register bit positions */
#define CONFIG_OVERRIDE_BIT_POS		13
#define CHN_CONFIG_SELECT_BIT_POS	10
#define CHN_SELECT_BIT_POS			7
#define REF_SRC_SELECT_BIT_POS		3

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
 * @brief	Read the single ADC sample (raw data) for input channel
 * @param	input_chn[in] - Input channel to be sampled and read data for
 * @param	raw_data[in, out]- ADC raw data
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad7689_read_single_sample(uint8_t input_chn, uint32_t *raw_data)
{
	uint16_t adc_raw = 0;
	int32_t ret;

	/* Configure 1st channel (n) for acquisition, data is read for (n-2) channel
	* (undefined conversion result) */
	if (input_chn == TEMPERATURE_CHN) {
		ad7689_current_config.inx = input_chn;
		ad7689_current_config.incc = AD7689_TEMPERATURE_SENSOR;
		ad7689_current_config.ref = AD7689_REF_INTERNAL_4p096V;
		input_chn -= 1;
	} else {
		ad7689_current_config.inx = input_chn;
		ad7689_current_config.incc = ADC_INPUT_TYPE_CFG;
		ad7689_current_config.ref = ADC_REF_VOLTAGE_CFG;
	}

	ret = ad7689_write_config(p_ad7689_dev_inst, &ad7689_current_config);
	if (ret) {
		return ret;
	}

	/* Previous conversion wait delay */
	no_os_udelay(10);

	/* Configure 2nd channel (n+1) for acquisition, data is read for (n-1) channel */
	if (input_chn == TEMPERATURE_CHN) {
		/* Load the inx bit to a next valid channel */
		ad7689_current_config.inx = input_chn;
		ad7689_current_config.incc = AD7689_TEMPERATURE_SENSOR;
		ad7689_current_config.ref = AD7689_REF_INTERNAL_4p096V;
	} else {
		ad7689_current_config.inx = input_chn;
		ad7689_current_config.incc = ADC_INPUT_TYPE_CFG;
		ad7689_current_config.ref = ADC_REF_VOLTAGE_CFG;
	}

	ret = ad7689_write_config(p_ad7689_dev_inst, &ad7689_current_config);
	if (ret) {
		return ret;
	}

	/* Previous conversion wait delay */
	no_os_udelay(10);

	/* The acquisition for channel (n) started from
	 * 'ad7689_enable_single_read_conversion' function. Data for that channel
	 * is available here (after 2 dummy reads).
	 **/
	ret = ad7689_read(p_ad7689_dev_inst, &adc_raw, 1);
	if (ret) {
		return ret;
	}

	*raw_data = adc_raw;

	return 0;
}

/*!
 * @brief	Read ADC raw data for recently sampled channel and trigger new conversion
 * @param	adc_data[in, out] - Pointer to adc data read variable
 * @param	next_chn[in] - Next channel in sequence
 * @return	0 in case of success, negative error code otherwise
 * @note	This function is intended to call from the conversion end trigger
 *			event. Therefore, this function should just read raw ADC data
 *			without further monitoring conversion end event
 */
int32_t ad7689_read_converted_sample(uint8_t *adc_data, uint8_t next_chn)
{
	uint16_t config_reg;
	uint8_t buf[2] = { 0, 0 };
	int32_t ret;

	if (!adc_data) {
		return -EINVAL;
	}

	/* Form the config register with new channel configuration */
	config_reg = (1 << CONFIG_OVERRIDE_BIT_POS);
	config_reg |= (next_chn << CHN_SELECT_BIT_POS);
	if (next_chn == TEMPERATURE_CHN) {
		config_reg |= ((AD7689_TEMPERATURE_SENSOR << CHN_CONFIG_SELECT_BIT_POS) |
			       (AD7689_REF_INTERNAL_4p096V
				<< REF_SRC_SELECT_BIT_POS));
	} else {
		config_reg |= ((ad7689_current_config.incc << CHN_CONFIG_SELECT_BIT_POS) |
			       (ad7689_current_config.ref
				<< REF_SRC_SELECT_BIT_POS));
	}

	/* Config word must to be sent during first 14 (MSBbits) clocks, therefore left
	 * shifted by 2 */
	config_reg <<= 2;

	buf[0] = config_reg >> BYTE_SIZE;
	buf[1] = config_reg;

	/* Read the conversion result */
	ret = no_os_spi_write_and_read(p_ad7689_dev_inst->spi_desc,
				       buf,
				       sizeof(buf));
	if (ret) {
		return ret;
	}

	adc_data[0] = buf[1];
	adc_data[1] = buf[0];

	return 0;
}

/*!
 * @brief	Perform the 2 dummy conversions after device initialization
 * @param	first_active_chn[in] - First ADC channel in sequencer
 * @param	second_active_chn[in] - Second ADC channel in sequencer
 * @param	num_of_active_channels[in] - Number of ADC channels in sequencer
 * @return	0 in case of success, negative error code otherwise
 */
int32_t ad7689_perform_init_cnv(uint8_t first_active_chn,
				uint8_t second_active_chn, uint8_t num_of_active_channels)
{
	int32_t ret;

	/* From power-up, in any read/write mode, the first three conversion results are
	 * undefined because a valid CFG does not take place until the second EOC;
	 * therefore, two dummy conversions are required
	 **/

	/* Configure 1st channel (n) for acquisition, data is read for (n-2) channel
	 * (undefined conversion reult) */
	ad7689_current_config.inx = first_active_chn;
	ret = ad7689_write_config(p_ad7689_dev_inst, &ad7689_current_config);
	if (ret) {
		return ret;
	}

	/* Previous conversion wait delay */
	no_os_udelay(10);

	if (num_of_active_channels > 1) {
		/* Configure 2nd channel (n+1) for acquisition, data is read for (n-1) channel */
		ad7689_current_config.inx = second_active_chn;
		ret = ad7689_write_config(p_ad7689_dev_inst, &ad7689_current_config);
		if (ret) {
			return ret;
		}

		/* Previous conversion wait delay */
		no_os_udelay(10);
	}

	return 0;
}
