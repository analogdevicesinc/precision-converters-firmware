/***************************************************************************//**
 *   @file   ad4134_support.h
 *   @brief  Header for AD4134 IIO application
********************************************************************************
 * Copyright (c) 2020, 2023, 2025 Analog Devices, Inc.
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
#include <stdbool.h>
#include "ad713x.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/
/* Timeout count to avoid stuck into potential infinite loop while checking
 * for new data into an acquisition buffer. The actual timeout factor is determined
 * through 'sampling_frequency' attribute of IIO app, but this period here makes sure
 * we are not stuck into a forever loop in case data capture is interrupted
 * or failed in between. */
#define AD7134_CONV_TIMEOUT	10000

/*
 * AD713X_REG_DEVICE_CONFIG Readback defines
 */
#define AD713X_DEV_CONFIG_PWR_MODE_RD(x)		(((x) >> 1) & 0x1)

/*
 * AD713X_REG_DATA_PACKET_CONFIG Readback defines
 */
#define AD713X_DATA_PACKET_CONFIG_FRAME_RD(x)			(((x) >> 4) & 0x7)
#define AD713X_DATA_PACKET_CONFIG_DCLK_FREQ_MODE_RD(x)	(((x) >> 0) & 0xF)

/*
 * AD713X_REG_DIGITAL_INTERFACE_CONFIG Readback defines
 */
#define AD713X_DIG_INT_CONFIG_FORMAT_MODE_RD(x)		(((x) >> 0) & 0x3)

/*
 * AD713X_REG_CHAN_DIG_FILTER_SEL Readback defines
 */
#define AD713X_DIGFILTER_SEL_CH_MODE_RD(x, ch)		(((x) >> (2 * ch)) & 0x3)

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
/**
 * @enum ad4134_data_capture_modes
 * @brief AD4134 list of data capture modes
 */
enum ad4134_data_capture_modes {
	DATA_CAPTURE_MODE_CONTINUOUS,
	DATA_CAPTURE_MODE_BURST,
};

/**
 * @enum ad4134_interface_modes
 * @brief AD4134 list of interface modes
 */
enum ad4134_interface_modes {
	INTERFACE_MODE_TDM,
	INTERFACE_MODE_BIT_BANGING,
	INTERFACE_MODE_MINIMAL_IO,
};

/**
 * @enum ad4134_asrc_modes
 * @brief AD4134 list of ASRC modes
 */
enum ad4134_asrc_modes {
	ASRC_MODE_CONTROLLER,
	ASRC_MODE_TARGET,
};

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
enum ad4134_data_capture_modes ad4134_get_data_capture_mode(void);
enum ad4134_interface_modes ad4134_get_interface_mode();
enum ad4134_asrc_modes ad4134_get_asrc_mode(void);

int32_t ad7134_data_capture_init(struct ad713x_dev *dev);
int32_t ad7134_read_all_channels_bit_banging(uint16_t *chn_data,
		bool check_odr_state);
int32_t ad7134_perform_conv_and_read_sample(uint16_t *adc_data, uint8_t ch);

#endif /* AD4134_SUPPORT_H_ */
