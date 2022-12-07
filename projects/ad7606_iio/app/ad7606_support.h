/***************************************************************************//**
 *   @file   ad7606_support.h
 *   @brief  Header for AD7606 No-OS driver supports
********************************************************************************
 * Copyright (c) 2020, 2022 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef AD7606_SUPPORT_H_
#define AD7606_SUPPORT_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad7606.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Offset b/w two channel selections in CHx_RANGE register */
#define CHANNEL_RANGE_MSK_OFFSET	4

/* AD7606_REG_OVERSAMPLING */
#define AD7606_OVERSAMPLING_MSK		NO_OS_GENMASK(3,0)

/* Default channel range for AD7606 devices */
#define DEFAULT_CHN_RANGE	(10.0)

/* Diagnostic channels Mux configurations */
#define AD7606_DIAGN_MUX_CH_MSK(ch)			(NO_OS_GENMASK(2, 0) << (3 * ((ch) % 2)))
#define AD7606_DIAGN_MUX_CH_VAL(ch, val)	(val << (3 * ((ch) % 2)))

#define AD7606_OPEN_DETECT_ENABLE_MSK(ch)	(NO_OS_GENMASK(7,0) & (~(1 << ch)))

/* Diagnostic channels Mux select bits */
#define ANALOG_INPUT_MUX	0X00
#define TEMPERATURE_MUX		0x01
#define VREF_MUX			0X02
#define ALDO_MUX			0X03
#define DLDO_MUX			0X04
#define	VDRIVE_MUX			0x05

/* Diagnostic Mux multiplers */
#define VREF_MUX_MULTIPLIER		4.0

/* Unipolar inputs range bits for AD7606C */
#define AD7606C_UNIPOLAR_RANGE_MIN	5
#define AD7606C_UNIPOLAR_RANGE_MAX	7

/* Number of AD7606 registers */
#define NUM_OF_REGISTERS	0x2F

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/* Analog input polarity */
typedef enum {
	UNIPOLAR,
	BIPOLAR
} polarity_e;

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

int32_t ad7606_read_converted_sample(struct ad7606_dev *dev, uint32_t *adc_data,
				     uint8_t input_chn);
int32_t ad7606_read_single_sample(struct ad7606_dev *dev,
				  uint32_t *adc_data, uint8_t chn);
polarity_e ad7606_get_input_polarity(uint8_t chn_range_bits);

#endif /* AD7606_SUPPORT_H_ */
