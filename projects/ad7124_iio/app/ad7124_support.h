/*****************************************************************************
  *@file  ad7124_support.h
  *@brief Provides useful support functions for the AD7124 NoOS driver
******************************************************************************
* Copyright (c) 2023-25 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD7124_SUPPORT_H_
#define AD7124_SUPPORT_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <errno.h>

#include "no_os_util.h"
#include "ad7124.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* Timeout count to avoid stuck into potential infinite loop while checking
 * for new data into an acquisition buffer. The actual timeout factor is determined
 * through 'sampling_frequency' attribute of IIO app, but this period here makes sure
 * we are not stuck into a forever loop in case data capture is interrupted
 * or failed in between.
 * Note: This timeout factor is dependent upon the MCU clock frequency. Below timeout
 * is tested for SDP-K1 platform @180Mhz default core clock */
#define AD7124_CONV_TIMEOUT 0xffffffff

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

enum ad7124_input_polarity {
	AD7124_UNIPOLAR,
	AD7124_BIPOLAR
};

/******************************************************************************/
/********************** Public Declarations ***********************************/
/******************************************************************************/

int ad7124_get_polarity(struct ad7124_dev *dev,
			uint8_t chn,
			enum ad7124_input_polarity *polarity);
int ad7124_single_read(struct ad7124_dev* device,
		       uint8_t id,
		       int32_t *adc_raw_data);
int ad7124_read_converted_data(struct ad7124_dev *dev, uint32_t *sd_adc_code);
int ad7124_trigger_data_capture(struct ad7124_dev *ad7124_dev_inst);
int ad7124_enable_cont_read(struct ad7124_dev *device, bool cont_read_en);
int ad7124_stop_data_capture(struct ad7124_dev *ad7124_dev_inst);
int ad7124_get_3db_frequency(struct ad7124_dev *ad7124_dev_inst, uint8_t chn,
			     uint16_t *frequency);
int ad7124_set_3db_frequency(struct ad7124_dev* ad7124_dev_inst, uint8_t chn,
			     uint16_t frequency);
int ad7124_update_sampling_rate(struct ad7124_dev *ad7124_dev_inst,
				uint16_t *frequency);

#endif /* AD7124_SUPPORT_H_ */
