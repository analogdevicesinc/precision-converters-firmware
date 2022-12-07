/***************************************************************************//**
 * @file    ad717x_user_config.c
 * @brief   User Configuration source for AD717x-AD411x IIO Application
********************************************************************************
* Copyright (c) 2021-22 Analog Devices, Inc.
* All rights reserved.
*
* This software is proprietary to Analog Devices, Inc. and its licensors.
* By using this software you agree to the terms of the associated
* Analog Devices Software License Agreement.
*******************************************************************************/

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdio.h>
#include "ad717x_user_config.h"
#include "ad717x.h"
#include "ad717x_iio.h"
#include "no_os_util.h"

/******************************************************************************/
/********************* Macros and Constants Definition ************************/
/******************************************************************************/

/******************************************************************************/
/******************** Variables and User Defined Data Types *******************/
/******************************************************************************/

#if defined(DEV_AD4111) || defined(DEV_AD4112) || defined(DEV_AD4114) || \
	defined(DEV_AD4115) || defined (DEV_AD4116)
#include <ad411x_regs.h>
#define AD717X_DEVICE_MAP ad4111_regs
#define AD717x_NUM_REGS NO_OS_ARRAY_SIZE(ad4111_regs)
#elif defined(DEV_AD7172_2)
#include <ad7172_2_regs.h>
#define AD717X_DEVICE_MAP ad7172_2_regs
#define AD717x_NUM_REGS NO_OS_ARRAY_SIZE(ad7172_2_regs)
#elif defined(DEV_AD7172_4)
#include <ad7172_4_regs.h>
#define AD717X_DEVICE_MAP ad7172_4_regs
#define AD717x_NUM_REGS NO_OS_ARRAY_SIZE(ad7172_4_regs)
#elif defined(DEV_AD7173_8)
#include <ad7173_8_regs.h>
#define AD717X_DEVICE_MAP ad7173_8_regs
#define AD717x_NUM_REGS NO_OS_ARRAY_SIZE(ad7173_8_regs)
#elif defined(DEV_AD7175_2)
#include <ad7175_2_regs.h>
#define AD717X_DEVICE_MAP ad7175_2_regs
#define AD717x_NUM_REGS NO_OS_ARRAY_SIZE(ad7175_2_regs)
#elif defined(DEV_AD7175_8)
#include <ad7175_8_regs.h>
#define AD717X_DEVICE_MAP ad7175_8_regs
#define AD717x_NUM_REGS NO_OS_ARRAY_SIZE(ad7175_8_regs)
#elif defined(DEV_AD7176_2)
#include <ad7176_2_regs.h>
#define AD717X_DEVICE_MAP ad7176_2_regs
#define AD717x_NUM_REGS NO_OS_ARRAY_SIZE(ad7176_2_regs)
#else
#include <ad411x_regs.h>
#define AD717X_DEVICE_MAP ad4111_regs
#define AD717x_NUM_REGS NO_OS_ARRAY_SIZE(ad4111_regs)
#endif

/* AD717x Init Parameters */
ad717x_init_param ad717x_init_params = {
	.spi_init = {
		.max_speed_hz = 20000000,
		.chip_select = SPI_CSB,
		.mode = NO_OS_SPI_MODE_3,
		.platform_ops = &spi_platform_ops,
		.extra = &spi_extra_init_params
	},
	.num_regs = AD717x_NUM_REGS,
	.regs = AD717X_DEVICE_MAP,
	.ref_en = true,
	.active_device = ACTIVE_DEVICE_ID,
	.num_channels = NUMBER_OF_CHANNELS,
	.num_setups = NUMBER_OF_SETUPS,
	.mode =  CONTINUOUS,
	.setups = {
		{ .bi_unipolar = true, .ref_buff = false, .input_buff = true, .ref_source = INTERNAL_REF },
		{ .bi_unipolar = true, .ref_buff = false, .input_buff = true, .ref_source = INTERNAL_REF },
		{ .bi_unipolar = true, .ref_buff = false, .input_buff = true, .ref_source = INTERNAL_REF },
		{ .bi_unipolar = true, .ref_buff = false, .input_buff = true, .ref_source = INTERNAL_REF },
#if (NUMBER_OF_SETUPS != 4)
		{ .bi_unipolar = true, .ref_buff = false, .input_buff = true, .ref_source = INTERNAL_REF },
		{ .bi_unipolar = true, .ref_buff = false, .input_buff = true, .ref_source = INTERNAL_REF },
		{ .bi_unipolar = true, .ref_buff = false, .input_buff = true, .ref_source = INTERNAL_REF },
		{ .bi_unipolar = true, .ref_buff = false, .input_buff = true, .ref_source = INTERNAL_REF },
#endif
	},
	.chan_map = {
#if defined (DEV_AD4111) || defined (DEV_AD4112) || defined (DEV_AD4114) || defined (DEV_AD4115) || defined (DEV_AD4116)
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.analog_input_pairs = VIN0_VIN1 },
		{ .channel_enable = false, .setup_sel = 1, .analog_inputs.analog_input_pairs = VIN0_VIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.analog_input_pairs = VIN0_VIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.analog_input_pairs = VIN0_VIN1 },
#if (NUMBER_OF_CHANNELS != 4)
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.analog_input_pairs = VIN0_VIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.analog_input_pairs = VIN0_VIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.analog_input_pairs = VIN0_VIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.analog_input_pairs = VIN0_VIN1 },
#if (NUMBER_OF_CHANNELS != 4) && (NUMBER_OF_CHANNELS != 8)
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.analog_input_pairs = VIN0_VIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.analog_input_pairs = VIN0_VIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.analog_input_pairs = VIN0_VIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.analog_input_pairs = VIN0_VIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.analog_input_pairs = VIN0_VIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.analog_input_pairs = VIN0_VIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.analog_input_pairs = VIN0_VIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.analog_input_pairs = VIN0_VIN1 },
#endif
#endif
#else // AD717x Family
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.ainp.pos_analog_input = AIN0, .analog_inputs.ainp.neg_analog_input = AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.ainp.pos_analog_input = AIN0, .analog_inputs.ainp.neg_analog_input = AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.ainp.pos_analog_input = AIN0, .analog_inputs.ainp.neg_analog_input = AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.ainp.pos_analog_input = AIN0, .analog_inputs.ainp.neg_analog_input = AIN1 },
#if (NUMBER_OF_CHANNELS != 4)
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.ainp.pos_analog_input = AIN0, .analog_inputs.ainp.neg_analog_input = AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.ainp.pos_analog_input = AIN0, .analog_inputs.ainp.neg_analog_input = AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.ainp.pos_analog_input = AIN0, .analog_inputs.ainp.neg_analog_input = AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.ainp.pos_analog_input = AIN0, .analog_inputs.ainp.neg_analog_input = AIN1 },
#if (NUMBER_OF_CHANNELS != 4) && (NUMBER_OF_CHANNELS != 8)
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.ainp.pos_analog_input = AIN0, .analog_inputs.ainp.neg_analog_input = AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.ainp.pos_analog_input = AIN0, .analog_inputs.ainp.neg_analog_input = AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.ainp.pos_analog_input = AIN0, .analog_inputs.ainp.neg_analog_input = AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.ainp.pos_analog_input = AIN0, .analog_inputs.ainp.neg_analog_input = AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.ainp.pos_analog_input = AIN0, .analog_inputs.ainp.neg_analog_input = AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.ainp.pos_analog_input = AIN0, .analog_inputs.ainp.neg_analog_input = AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.ainp.pos_analog_input = AIN0, .analog_inputs.ainp.neg_analog_input = AIN1 },
		{ .channel_enable = false, .setup_sel = 0, .analog_inputs.ainp.pos_analog_input = AIN0, .analog_inputs.ainp.neg_analog_input = AIN1 },
#endif // (NUMBER_OF_CHANNELS != 4) && (NUMBER_OF_CHANNELS != 8)
#endif // (NUMBER_OF_CHANNELS != 4)
#endif // (DEV_AD4111),(DEV_AD4112),(DEV_AD4114),(DEV_AD4115), (DEV_AD4116)
	},
	.filter_configuration = {
		{.odr = AD717x_ODR_SEL},
		{.odr = AD717x_ODR_SEL},
		{.odr = AD717x_ODR_SEL},
		{.odr = AD717x_ODR_SEL},
#if (NUMBER_OF_SETUPS != 4)
		{.odr = AD717x_ODR_SEL},
		{.odr = AD717x_ODR_SEL},
		{.odr = AD717x_ODR_SEL},
		{.odr = AD717x_ODR_SEL},
#endif // (NUMBER_OF_SETUPS!= 4)
	}
};

/******************************************************************************/
/************************** Functions Declaration *****************************/
/******************************************************************************/

/******************************************************************************/
/************************** Functions Definition ******************************/
/******************************************************************************/
