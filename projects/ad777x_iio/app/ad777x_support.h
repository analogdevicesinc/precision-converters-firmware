/***************************************************************************//*
 * @file    ad777x_support.h
 * @brief   AD777x No-OS driver support header file
******************************************************************************
 * Copyright (c) 2022 Analog Devices, Inc. All Rights Reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
******************************************************************************/
#ifndef AD777x_SUPPORT_H_
#define AD777x_SUPPORT_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "ad7779.h"

/******************************************************************************/
/********************* Macros and Constants Definitions ***********************/
/******************************************************************************/

/* Timeout count to avoid stuck into potential infinite loop while checking
 * for new data into an acquisition buffer. The actual timeout factor is determined
 * through 'sampling_frequency' attribute of IIO app, but this period here makes sure
 * we are not stuck into a forever loop in case data capture is interrupted
 * or failed in between.
 * Note: This timeout factor is dependent upon the MCU clock frequency. Below timeout
 * is tested for SDP-K1 platform @180Mhz default core clock */
#define AD777x_CONV_TIMEOUT	10000

/******************************************************************************/
/********************** Public/Extern Declarations ****************************/
/******************************************************************************/
extern int32_t ad777x_raw_data_read(ad7779_dev *dev, uint8_t ch_num,
				    uint32_t *sd_adc_code);

extern int32_t ad777x_read_all_channels(ad7779_dev *dev, uint32_t *sd_adc_code);

extern int32_t ad777x_enable_single_dout(ad7779_dev *dev);

int32_t ad7779_sar_data_read(ad7779_dev *dev, ad7779_sar_mux mux,
			     uint16_t *sar_code);

#endif	/* end of AD777x_SUPPORT_H_ */
