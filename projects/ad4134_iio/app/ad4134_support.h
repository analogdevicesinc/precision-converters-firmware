/***************************************************************************//**
 *   @file   ad4134_support.h
 *   @brief  Header for AD4134 No-OS driver supports
********************************************************************************
 * Copyright (c) 2020, 2023 Analog Devices, Inc.
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef AD4134_SUPPORT_H_
#define AD4134_SUPPORT_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include <stdbool.h>
#include "ad4134.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Timeout count to avoid stuck into potential infinite loop while checking
 * for new data into an acquisition buffer. The actual timeout factor is determined
 * through 'sampling_frequency' attribute of IIO app, but this period here makes sure
 * we are not stuck into a forever loop in case data capture is interrupted
 * or failed in between. */
#define AD4134_CONV_TIMEOUT	10000

/*
 * AD4134_REG_DEVICE_CONFIG Readback defines
 */
#define AD4134_DEV_CONFIG_PWR_MODE_RD(x)		(((x) >> 1) & 0x1)

/*
 * AD4134_REG_DATA_PACKET_CONFIG Readback defines
 */
#define AD4134_DATA_PACKET_CONFIG_FRAME_RD(x)			(((x) >> 4) & 0x7)
#define AD4134_DATA_PACKET_CONFIG_DCLK_FREQ_MODE_RD(x)	(((x) >> 0) & 0xF)

/*
 * AD4134_REG_DIGITAL_INTERFACE_CONFIG Readback defines
 */
#define AD4134_DIG_INT_CONFIG_FORMAT_MODE_RD(x)		(((x) >> 0) & 0x3)

/*
 * AD4134_REG_CHAN_DIG_FILTER_SEL Readback defines
 */
#define AD4134_DIGFILTER_SEL_CH_MODE_RD(x, ch)		(((x) >> (2 * ch)) & 0x3)

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

int32_t ad4134_data_capture_init(struct ad4134_dev *dev);
int32_t ad4134_read_data(uint16_t *adc_data, uint8_t curr_chn);
int32_t ad4134_perform_conv_and_read_sample(uint8_t input_chn,
		uint16_t *adc_data);
int32_t ad4134_read_all_channels(uint16_t *chn_data);

#endif /* AD4134_SUPPORT_H_ */