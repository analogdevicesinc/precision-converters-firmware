/***************************************************************************//**
 *   @file    cn0586_support.c
 *   @brief   CN0586 support module
 *   @details This module performs the system configurations for CN0586
********************************************************************************
 * Copyright (c) 2024 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdbool.h>
#include "app_config.h"
#include "cn0586_support.h"
#include "no_os_util.h"
#include "no_os_error.h"
#include "no_os_gpio.h"
#include "no_os_alloc.h"

/******************************************************************************/
/************************ Macros/Constants ************************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Declarations ****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definitions *****************************/
/******************************************************************************/

/**
 * @brief Initialize the CFTL. Power up channels A, B, D
 * @param dev[in,out] - cn0586 device structure.
 * @param ad5754r_device[in,out] - Pointer to ad5754r device structure
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0586_init(struct cn0586_dev **dev, struct ad5754r_dev *ad5754r_device)
{
	int ret;
	struct cn0586_dev *cftl;

	if (!dev || !ad5754r_device) {
		return -EINVAL;
	}

	cftl = (struct cn0586_dev *)no_os_malloc(sizeof(struct cn0586_dev));
	if (!cftl) {
		return -ENOMEM;
	}

	cftl->dev = ad5754r_device;

	ret = ad5754r_set_ch_pwrup(cftl->dev,
				   AD5754R_DAC_CH_A,
				   AD5754R_PWR_DAC_CH_POWERUP);
	if (ret) {
		return ret;
	}

	ret = ad5754r_set_ch_pwrup(cftl->dev,
				   AD5754R_DAC_CH_B,
				   AD5754R_PWR_DAC_CH_POWERUP);
	if (ret) {
		return ret;
	}

	ret = ad5754r_set_ch_pwrup(cftl->dev,
				   AD5754R_DAC_CH_D,
				   AD5754R_PWR_DAC_CH_POWERUP);
	if (ret) {
		return ret;
	}

	cftl->hvout_volts = 0;
	cftl->state = HVOUT_DISABLED;
	cftl->range = HVOUT_0V_100V;

	*dev = cftl;

	/* Change range to [-100V, 100V] by default */
	return cn0586_set_hvout_range(*dev, HVOUT_M100V_100V);
}

/**
 * @brief Set HVOUT volts for the CFTL
 * @param dev[in,out] - The CFTL device structure.
 * @param volts[in] - Volts to appear at hvout when enabled.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0586_set_hvout_volts(struct cn0586_dev *dev, float volts)
{
	float dac_a_volts;
	float dac_a_volts_ul;  //Upper limit for DAC Ch A output voltage
	uint32_t dac_a_code;
	float dac_b_volts;
	int ret;

	if (!dev) {
		return -EINVAL;
	}

	switch (dev->range) {
	case HVOUT_0V_100V:
		if (volts < 0 || volts > 100) {
			return -EINVAL;
		}

		dac_b_volts = 0;
		dac_a_volts_ul = 5;
		break;

	case HVOUT_M100V_100V:
		if (volts < -100 || volts > 100) {
			return -EINVAL;
		}

		dac_b_volts = 5;
		dac_a_volts_ul = 10;

		break;

	case HVOUT_M50V_50V:
		if (volts < -50 || volts > 50) {
			return -EINVAL;
		}

		dac_b_volts = 2.5;
		dac_a_volts_ul = 5;

		break;

	case HVOUT_0V_200V:
		if (volts < 0 || volts > 200) {
			return -EINVAL;
		}

		dac_b_volts = 0;
		dac_a_volts_ul = 10;

		break;
	}

	dac_a_volts = volts/20 + dac_b_volts;

	dac_a_code = (1 << AD5754R_MAX_RESOLUTION) * dac_a_volts;
	dac_a_code /= (((float)dev->dev->vref_mv / AD5754R_GAIN_SCALE) *
		       (ad5754r_gain_values_scaled[dev->dev->dac_ch_range[AD5754R_DAC_CH_A]]
			/ AD5754R_GAIN_SCALE));

	if (dac_a_volts == dac_a_volts_ul) {
		dac_a_code = 0xFFFF;
	}

	/* Write codes to DAC register */
	ret = ad5754r_write(dev->dev,
			    AD5754R_PREP_INSTR_ADDR(AD5754R_REG_DAC, AD5754R_DAC_CH_A),
			    (uint16_t)dac_a_code);
	if (ret) {
		return ret;
	}

	/* Update AD5754R outputs using SW LDAC */
	ret = ad5754r_write(dev->dev, AD5754R_INSTR_LOAD, 0x0000);
	if (ret) {
		return ret;
	}

	dev->hvout_volts = volts;

	return 0;
}

/**
 * @brief Get HVOUT volts for the CFTL
 * @param dev[in,out] - The CFTL device structure.
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0586_get_hvout_volts(struct cn0586_dev *dev)
{
	int32_t ret;
	uint16_t dac_a_code, dac_b_code;
	float volts_a, volts_b;

	if (!dev) {
		return -EINVAL;
	}

	ret = ad5754r_read_dac_ch_register(dev->dev, AD5754R_DAC_CH_A, &dac_a_code);
	if (ret) {
		return ret;
	}

	ret = ad5754r_read_dac_ch_register(dev->dev, AD5754R_DAC_CH_B, &dac_b_code);
	if (ret) {
		return ret;
	}

	volts_a =
		(ad5754r_gain_values_scaled[dev->dev->dac_ch_range[AD5754R_DAC_CH_A]] /
		 AD5754R_GAIN_SCALE);
	volts_a *= ((float)dev->dev->vref_mv / AD5754R_GAIN_SCALE);
	volts_a *= dac_a_code;
	volts_a	/= (1 << AD5754R_MAX_RESOLUTION);

	volts_b =
		(ad5754r_gain_values_scaled[dev->dev->dac_ch_range[AD5754R_DAC_CH_B]] /
		 AD5754R_GAIN_SCALE);
	volts_b *= ((float)dev->dev->vref_mv / AD5754R_GAIN_SCALE);
	volts_b *= dac_b_code;
	volts_b	/= (1 << AD5754R_MAX_RESOLUTION);

	dev->hvout_volts = 20 * (volts_a - volts_b);

	return 0;
}

/**
 * @brief Set HVOUT range for the CFTL
 * @param dev[in,out] - cn0586 device structure.
 * @param range[in] - hvout range to be applied
 * @return 0 in case of success, negative error code otherwise.
 */
int32_t cn0586_set_hvout_range(struct cn0586_dev *dev, enum cn0586_range range)
{
	int ret;
	enum ad5754r_dac_ch_range dac_a_range;
	uint16_t dac_a_code;
	uint16_t dac_b_code;

	if (!dev) {
		return -EINVAL;
	}

	switch (range) {
	case HVOUT_0V_100V:
		dac_a_range = AD5754R_SPAN_0V_TO_5V;
		dac_a_code = 0;
		dac_b_code = 0;

		break;

	case HVOUT_M100V_100V:
		dac_a_range = AD5754R_SPAN_0V_TO_10V;
		dac_a_code = 0x8000;
		dac_b_code = 0xFFFF;

		break;

	case HVOUT_M50V_50V:
		dac_a_range = AD5754R_SPAN_0V_TO_5V;
		dac_a_code = 0x8000;
		dac_b_code = 0x8000;

		break;

	case HVOUT_0V_200V:
		dac_a_range = AD5754R_SPAN_0V_TO_10V;
		dac_a_code = 0;
		dac_b_code = 0;

		break;
	}

	/* Apply range */
	ret = ad5754r_set_ch_range(dev->dev,
				   AD5754R_DAC_CH_A,
				   dac_a_range);
	if (ret) {
		return ret;
	}

	/* Write codes to DAC register */
	ret = ad5754r_write(dev->dev,
			    AD5754R_PREP_INSTR_ADDR(AD5754R_REG_DAC, AD5754R_DAC_CH_A),
			    dac_a_code);
	if (ret) {
		return ret;
	}

	ret = ad5754r_write(dev->dev,
			    AD5754R_PREP_INSTR_ADDR(AD5754R_REG_DAC, AD5754R_DAC_CH_B),
			    dac_b_code);
	if (ret) {
		return ret;
	}

	/* Update AD5754R outputs using SW LDAC */
	ret = ad5754r_write(dev->dev, AD5754R_INSTR_LOAD, 0x0000);
	if (ret) {
		return ret;
	}

	dev->range = range;
	dev->hvout_volts = 0;

	return 0;
}