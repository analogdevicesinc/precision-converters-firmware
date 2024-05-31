/***************************************************************************//*
 * @file    ad4170_support.h
 * @brief   AD4170 No-OS driver support header file
******************************************************************************
 * Copyright (c) 2021-24 Analog Devices, Inc. All Rights Reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/
#ifndef AD4170_SUPPORT_H_
#define AD4170_SUPPORT_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdlib.h>
#include "ad4170.h"

/******************************************************************************/
/********************* Macros and Constants Definitions ***********************/
/******************************************************************************/

#define AD4170_PGA_GAIN(x)	(1 << (x))

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/

int32_t ad4170_read_single_sample(uint8_t input_chn, uint32_t *raw_data);
int32_t ad4170_read_converted_sample(uint32_t *adc_data);
int32_t perform_sign_conversion(uint32_t adc_raw_data, uint8_t chn);
float convert_adc_sample_into_voltage(uint32_t adc_raw, uint8_t chn);
float convert_adc_data_to_voltage_without_vref(int32_t data, uint8_t chn);
float convert_adc_data_to_voltage_wrt_vref(int32_t data, uint8_t chn);
float convert_adc_raw_into_rtd_resistance(uint32_t adc_raw, float rtd_res,
		uint8_t chn);
float ad4170_get_reference_voltage(uint8_t chn);
float ad4170_get_gain_value(uint8_t chn);
int32_t ad4170_disable_conversion(void);
int32_t ad4170_enable_input_chn(uint8_t input_chn);
int32_t ad4170_disable_input_chn(uint8_t input_chn);
int32_t ad4170_apply_excitation(uint8_t input_chn);
int32_t ad4170_remove_excitation(uint8_t input_chn);
int32_t ad4170_set_filter(struct ad4170_dev *dev, uint8_t chn,
			  enum ad4170_filter_type filt_type);
int32_t ad4170_set_reference(struct ad4170_dev *dev, uint8_t chn,
			     enum ad4170_ref_select ref);
int32_t ad4170_set_fs(struct ad4170_dev *dev, uint8_t setup, uint8_t chn,
		      uint16_t fs_val);
#endif	/* end of AD4170_SUPPORT_H_ */
