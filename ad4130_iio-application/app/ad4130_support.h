/***************************************************************************//**
 *   @file   ad4130_support.h
 *   @brief  Header for AD4130 No-OS driver supports
********************************************************************************
 * Copyright (c) 2020-2022 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef AD4130_SUPPORT_H_
#define AD4130_SUPPORT_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdint.h>
#include "ad413x.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

#define AD413X_ADDR(x)		((x) & 0xFF)

#define AD4130_INT_SRC_SEL_MSK		NO_OS_GENMASK(9, 8)
#define AD4130_FILTER_FS_MSK		NO_OS_GENMASK(10, 0)
#define AD4130_FIFO_MODE_MSK		NO_OS_GENMASK(17, 16)
#define AD413X_WATERMARK_MSK		NO_OS_GENMASK(7, 0)

#define AD413X_COMM_REG_RD			NO_OS_BIT(6)

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* FIFO modes */
typedef enum {
	FIFO_DISABLED,
	FIFO_OLDEST_SAVE_MODE,
	FIFO_STREAM_MODE
} fifo_mode_e;

/* ADC conversion interrupt source */
typedef enum {
	INT_PIN,
	CLK_PIN,
	GPIO1_PIN
} adc_conv_int_source_e;

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

float ad4130_get_reference_voltage(struct ad413x_dev *dev, uint8_t chn);
int32_t perform_sign_conversion(struct ad413x_dev *dev, uint32_t adc_raw_data,
				uint8_t chn);
float convert_adc_sample_into_voltage(void *dev, uint32_t adc_raw,
				      uint8_t chn);
float convert_adc_raw_into_rtd_resistance(void *dev, uint32_t adc_raw,
		float rtd_ref, uint8_t chn);
int32_t ad4130_read_fifo(struct ad413x_dev *dev, uint32_t *data,
			 uint32_t adc_samples);
int32_t ad413x_read_single_sample(struct ad413x_dev *dev, uint8_t input_chn,
				  uint32_t *adc_raw);
int32_t ad413x_mon_conv_and_read_data(struct ad413x_dev *dev,
				      uint32_t *raw_data);
int32_t ad413x_set_int_source(struct ad413x_dev *dev,
			      adc_conv_int_source_e conv_int_source);
int32_t ad413x_set_filter_fs(struct ad413x_dev *dev, uint32_t fs,
			     uint8_t preset);

#endif /* AD4130_SUPPORT_H_ */
