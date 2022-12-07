/***************************************************************************//**
 *   @file   ad4696_support.h
 *   @brief  Header for AD469x No-OS driver supports
********************************************************************************
 * Copyright (c) 2021-22 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
*******************************************************************************/

#ifndef AD4696_SUPPORT_H_
#define AD4696_SUPPORT_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "ad469x.h"
#include "no_os_util.h"

/******************************************************************************/
/********************** Macros and Constants Definition ***********************/
/******************************************************************************/

/* Number of AD469x registers */
#define NUM_OF_REGISTERS	0x17F

/* Default channel range for AD4696 devices */
#define DEFAULT_VREF                (5.0)

/* AD469x_REG_TEMPERATURE */
#define AD469x_TEMPERATURE_MSK		NO_OS_GENMASK(0,0)

/* AD469x Sequencer Lower Byte Configuration */
#define AD469x_SEQ_LB_CONFIG(x)    ( x & NO_OS_GENMASK(7,0))

/* AD469x Sequencer Upper Byte Configuration */
#define AD469x_SEQ_UB_CONFIG(x)    ( x >> 8)

/* AD469x Sequencer Lower Byte Register */
#define AD469x_REG_SEQ_LB          AD469x_REG_STD_SEQ_CONFIG

/* AD469x Sequencer Upper Byte Register */
#define AD469x_REG_SEQ_UB          (AD469x_REG_STD_SEQ_CONFIG + 0x01)

/* AD469x Sequencer Lower Byte Configuration */
#define AD469x_SINGLE_CHANNEL_EN(x)    AD469x_CHANNEL(x)

/* AD469x Enable Autocycle Mode*/
#define AD469x_SEQ_CHANNELS_RESET      0x00

/* AD469x Sequencer disable all channels */
#define AD469x_EN_AUTOCYLE_MODE	       0x01

/* AD469x Manual Trigger Configurations */
#define AD469x_REG_SETUP_RESET         0x10
#define AD469x_REG_SEQ_CTRL_RESET      0x80

/* AD469x Sequencer disable all channels */
#define AD469x_SEQ_CHANNEL_EN          1
#define AD469x_SEQ_CHANNEL_DI          0

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/
/**
 * @enum ad469x_polarity_select
 * @brief Channel polarity modes
 */
enum ad469x_polarity_select {
	AD469x_UNIPOLAR_MODE,
	AD469x_PSEUDO_BIPOLAR_MODE
};

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/
int32_t ad469x_polarity_mode_select(struct ad469x_dev *device,
				    enum ad469x_polarity_select polarity_sel);
int32_t ad469x_reference_config(struct ad469x_dev *device);
int32_t ad469x_trigger_conversion(struct ad469x_dev *device);
int32_t ad469x_read_single_sample(struct ad469x_dev *device,
				  uint8_t chn_num,
				  uint32_t *data);

#endif /* AD4696_SUPPORT_H_ */
