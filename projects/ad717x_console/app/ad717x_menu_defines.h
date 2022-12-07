/*!
 *****************************************************************************
  @file:  ad717x_menu_defines_app.h

  @brief: Header for AD717x/AD411x console menu definitions.

  @details:
 -----------------------------------------------------------------------------
 Copyright (c) 2020,2022 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD717X_MENU_DEFINES_H_
#define AD717X_MENU_DEFINES_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "adi_console_menu.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

// Prototypes for the console menu functions defined in ad717x_console_app.c file
void display_main_menu_header(void);
int32_t menu_read_id(uint32_t menu_id);
int32_t menu_read_status(uint32_t menu_id);
int32_t menu_sample_channels(uint32_t menu_id);
int32_t menu_chn_enable_disable_display(uint32_t menu_id);
int32_t menu_input_chn_connect_display(uint32_t menu_id);
int32_t menu_config_and_assign_setup(uint32_t menu_id);
int32_t menu_display_setup(uint32_t menu_id);
int32_t menu_read_temperature(uint32_t menu_id);
int32_t menu_calibrate_adc(uint32_t menu_id);
int32_t menu_read_write_device_regs(uint32_t menu_id);
int32_t menu_channels_enable_disable(uint32_t action);
int32_t menu_analog_input_connect(uint32_t user_analog_input);
int32_t menu_input_type_selection(uint32_t input_type_id);
int32_t menu_select_chn_pair(uint32_t user_channel_pair);
int32_t menu_select_input_pair(uint32_t user_input_pair);
int32_t menu_open_wire_detection(uint32_t menu_id);
int32_t menu_rw_ad717x_register(uint32_t rw_id);
int32_t menu_single_conversion(uint32_t channel_id);
int32_t menu_continuous_conversion_tabular(uint32_t channel_id);
int32_t menu_continuous_conversion_stream(uint32_t channel_id);
int32_t menu_filter_select(uint32_t user_input_filter_type);
int32_t menu_postfiler_enable_disable(uint32_t user_action);
int32_t menu_postfiler_select(uint32_t user_input_post_filter_type);
int32_t menu_odr_select(uint32_t user_input_odr_val);
int32_t menu_polarity_select(uint32_t user_input_polarity);
int32_t menu_reference_source_select(uint32_t user_input_reference);
int32_t menu_ref_buffer_enable_disable(uint32_t user_action);
int32_t menu_input_buffer_enable_disable(uint32_t user_action);


// Define the standard ODR values for the AD717x/AD414x devices
// ODRx value, ODRx string name and ODRx bits

#define ODR_250000		250000.00
#define ODR_250000_STR	"250000.00"
#define ODR_250000_BITS	0x00

#define ODR_125000		125000.00
#define ODR_125000_STR	"125000.00"
#define ODR_125000_BITS	0x01

#define ODR_62500		62500.00
#define ODR_62500_STR	"62500.00"
#define ODR_62500_BITS	0x02

#define ODR_50000		50000.00
#define ODR_50000_STR	"50000.00"
#define ODR_50000_BITS	0x03

#define ODR_31250		31250.00
#define ODR_31250_STR	"31250.00"
#define ODR_31250_BITS	0x04

#define ODR_25000		25000.00
#define ODR_25000_STR	"25000.00"
#define ODR_25000_BITS	0x05

#define ODR_15625		15625.00
#define ODR_15625_STR	"15625.00"
#define ODR_15625_BITS	0x06

#define ODR_10417		10417.00
#define ODR_10417_STR	"10417.00"
#define ODR_10417_BITS	0x07

#define ODR_10000		10000.00
#define ODR_10000_STR	"10000.00"
#define ODR_10000_BITS	0x07

#define ODR_5208		5208.00
#define ODR_5208_STR	"5208.00"
#define ODR_5208_BITS	0x08

#define ODR_5000		5000.00
#define ODR_5000_STR	"5000.00"
#define ODR_5000_BITS	0x08

#define ODR_3906		3906.00
#define ODR_3906_STR	"3906.00"
#define ODR_3906_BITS	0x09

#define ODR_2604		2604.00
#define ODR_2604_STR	"2604.00"
#define ODR_2604_BITS	0x09

#define ODR_2597		2597.00
#define ODR_2597_STR	"2597.00"
#define ODR_2597_BITS	0x09

#define ODR_2500		2500.00
#define ODR_2500_STR	"2500.00"
#define ODR_2500_BITS	0x09

#define ODR_1157		1157.00
#define ODR_1157_STR	"1157.00"
#define ODR_1157_BITS	0x0A

#define ODR_1008		1008.00
#define ODR_1008_STR	"1008.00"
#define ODR_1008_BITS	0x0A

#define ODR_1007		1007.00
#define ODR_1007_STR	"1007.00"
#define ODR_1007_BITS	0x0A

#define ODR_1000		1000.00
#define ODR_1000_STR	"1000.00"
#define ODR_1000_BITS	0x0A

#define ODR_539			539.00
#define ODR_539_STR		"539.00"
#define ODR_539_BITS	0x0B

#define ODR_504			504.00
#define ODR_504_STR		"504.00"
#define ODR_504_BITS	0x0B

#define ODR_503_8		503.80
#define ODR_503_8_STR	"503.80"
#define ODR_503_8_BITS	0x0B

#define ODR_500			500.00
#define ODR_500_STR		"500.00"
#define ODR_500_BITS	0x0B

#define ODR_401			401.00
#define ODR_401_STR		"401.00"
#define ODR_401_BITS	0x0C

#define ODR_400_6		400.60
#define ODR_400_6_STR	"400.60"
#define ODR_400_6_BITS	0x0C

#define ODR_400			400.00
#define ODR_400_STR		"400.00"
#define ODR_400_BITS	0x0C

#define ODR_397_5		397.50
#define ODR_397_5_STR	"397.50"
#define ODR_397_5_BITS	0x0C

#define ODR_397			397.00
#define ODR_397_STR		"397.00"
#define ODR_397_BITS	0x0C

#define ODR_381			381.00
#define ODR_381_STR		"381.00"
#define ODR_381_BITS	0x0C

#define ODR_206			206.00
#define ODR_206_STR		"206.00"
#define ODR_206_BITS	0x0D

#define ODR_200_3		200.3
#define ODR_200_3_STR	"200.30"
#define ODR_200_3_BITS	0x0D

#define ODR_200			200.00
#define ODR_200_STR		"200.00"
#define ODR_200_BITS	0x0D

#define ODR_102			102.00
#define ODR_102_STR		"102.00"
#define ODR_102_BITS	0x0E

#define ODR_100_2		100.20
#define ODR_100_2_STR	"100.20"
#define ODR_100_2_BITS	0x0E

#define ODR_100			100.00
#define ODR_100_STR		"100.00"
#define ODR_100_BITS	0x0E

#define ODR_60			60.00
#define ODR_60_STR		"60.00"
#define ODR_60_BITS		0x0F

#define ODR_59_98		59.98
#define ODR_59_98_STR	"59.98"
#define ODR_59_98_BITS	0x0F

#define ODR_59_94		59.94
#define ODR_59_94_STR	"59.94"
#define ODR_59_94_BITS	0x0F

#define ODR_59_52		59.52
#define ODR_59_52_STR	"59.52"
#define ODR_59_52_BITS	0x0F

#define ODR_50			50.00
#define ODR_50_STR		"50.00"
#define ODR_50_BITS		0x10

#define ODR_49_96		49.96
#define ODR_49_96_STR	"49.96"
#define ODR_49_96_BITS	0x10

#define ODR_49_68		49.68
#define ODR_49_68_STR	"49.68"
#define ODR_49_68_BITS	0x10

#define ODR_20_01		20.01
#define ODR_20_01_STR	"20.01"
#define ODR_20_01_BITS	0x11

#define ODR_20			20.00
#define ODR_20_STR		"20.00"
#define ODR_20_BITS		0x11

#define ODR_16_63		16.63
#define ODR_16_63_STR	"16.63"
#define ODR_16_63_BITS	0x12

#define ODR_16_67		16.67
#define ODR_16_67_STR	"16.67"
#define ODR_16_67_BITS	0x12

#define ODR_10			10.00
#define ODR_10_STR		"10.00"
#define ODR_10_BITS		0x13

#define ODR_5			5.00
#define ODR_5_STR		"5.00"
#define ODR_5_BITS		0x14

#define ODR_2_5			2.50
#define ODR_2_5_STR		"2.50"
#define ODR_2_5_BITS	0x15

#define ODR_1_25		1.25
#define ODR_1_25_STR	"1.25"
#define ODR_1_25_BITS	0x16

#define ODR_RES_STR		"RES"
#define ODR_RES_BITS	0x17


// Analog input bits (0:9) for the AD717x/AD414x devices
#define VIN0_INPUT_BITS			0x00
#define VIN1_INPUT_BITS			0x01
#define VIN2_INPUT_BITS			0x02
#define VIN3_INPUT_BITS			0x03
#define VIN4_INPUT_BITS			0x04
#define VIN5_INPUT_BITS			0x05
#define VIN6_INPUT_BITS			0x06
#define VIN7_INPUT_BITS			0x07
#define VIN8_INPUT_BITS			0x08
#define VIN9_INPUT_BITS			0x09
#define VIN10_INPUT_BITS		0x0A
#define VIN11_INPUT_BITS		0x0B
#define VIN12_INPUT_BITS		0x0C
#define VIN13_INPUT_BITS		0x0D
#define VIN14_INPUT_BITS		0x0E
#define VIN15_INPUT_BITS		0x0F
#define VIN16_INPUT_BITS		0x10
#define VINCOM_INPUT_BITS		0x10
#define IN0N_INPUT_BITS			0x08
#define IN1N_INPUT_BITS			0x09
#define IN2N_INPUT_BITS			0x0A
#define IN3N_INPUT_BITS			0x0B
#define IN3P_INPUT_BITS			0x0C
#define IN2P_INPUT_BITS			0x0D
#define IN1P_INPUT_BITS			0x0E
#define IN0P_INPUT_BITS			0x0F
#define TEMP_SENSOR_POS_INP_BITS 0x11
#define TEMP_SENSOR_NEG_INP_BITS 0x12
#define AVDD1_AVSS_P_BITS		0x13
#define AVDD1_AVSS_N_BITS		0x14
#define REFP_INPUT_BITS			0x15
#define REFN_INPUT_BITS			0x16

// Offset to form VIN+ and VIN- pairs
#define VIN_PAIR_OFFSET			5


// Channels define for the AD717x/AD414x devices
#define ADC_CHN(x)	(x)

// Offset to form Channel pairs
#define CHN_PAIR_OFFSET		4
#define CHN_PAIR_MASK		0x0F


// Device register read/write identifiers
#define DEVICE_REG_READ_ID		(uint32_t)1
#define DEVICE_REG_WRITE_ID		(uint32_t)2

// Enable/Disable selection identifiers
#define SELECT_DISBLE			(uint32_t)0
#define SELECT_ENABLE			(uint32_t)1

// Input Type selection identifiers
#define SINGLE_ENDED_INPUT		(uint32_t)0
#define DIFF_ENDED_INPUT		(uint32_t)1

// Analog input select identifiers
#define ANALOG_INP_PAIR_SELECT	(uint32_t)0
#define POS_ANALOG_INP_SELECT	(uint32_t)1
#define NEG_ANALOG_INP_SELECT	(uint32_t)2


// Digital filter type (ORDER0) bits
#define SINC5_SINC1_FILTER	(uint32_t)0
#define SINC3_FILTER		(uint32_t)1

// SINC5+SINC1 post filter type bits
#define POST_FILTER_NA		(uint32_t)0
#define POST_FLTR_27_SPS	(uint32_t)2
#define POST_FLTR_25_SPS	(uint32_t)3
#define POST_FLTR_20_SPS	(uint32_t)5
#define POST_FLTR_16_67_SPS	(uint32_t)6

// Reference source type bits
#define EXTERNAL			(uint32_t)0
#define INTERNAL			(uint32_t)2
#define AVDD_AVSS			(uint32_t)3

// Polarity selection bits
#define UNIPOLAR			(uint32_t)0
#define BIPOLAR				(uint32_t)1

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

// Enable/Disable status names
const char *enable_disable_status[] = {
	"Disable",
	"Enable"
};

// Polarity names
const char *polarity_status[] = {
	"Unipolar",
	"Bipolar"
};

// Filter names
const char *filter_name[] = {
	"Sinc5+1",
	"Sinc3",
};

// Post filter names
const char *postfilter_name[] = {
	"NA",
	"NA",
	"27_SPS",
	"25_SPS",
	"NA",
	"20_SPS",
	"16_SPS",
};

// Reference source names
const char *reference_name[] = {
	"External",
	"External",
	"Internal",
	"AVDD-AVSS"
};


// Analog Input pin map
const char *input_pin_map[] = {
#if defined(DEV_AD4111) || defined(DEV_AD4112)
	"VIN0", "VIN1", "VIN2", "VIN3", "VIN4", "VIN5",
	"VIN6", "VIN7", "IN0-", "IN1-", "IN2-", "IN3-",
	"IN3+", "IN2+", "IN1+", "IN0+", "VINCOM",
	"TEMP+", "TEMP-", "RES", "RES", "REF+", "REF-"
#elif defined(DEV_AD4114) || defined(DEV_AD4115)
	"VIN0", "VIN1", "VIN2", "VIN3", "VIN4", "VIN5",
	"VIN6", "VIN7", "VIN8", "VIN9", "VIN10", "VIN11",
	"VIN12", "VIN13", "VIN14", "VIN15", "VINCOM",
	"TEMP+", "TEMP-", "RES", "RES", "REF+", "REF-"
#elif defined(DEV_AD7173_8) || defined(DEV_AD7175_8)
	"AIN0", "AIN1", "AIN2", "AIN3", "AIN4", "AIN5",
	"AIN6", "AIN7", "AIN8", "AIN9", "AIN10", "AIN11",
	"AIN12", "AIN13", "AIN14", "AIN15", "AIN16",
	"TEMP+", "TEMP-",
#if defined(DEV_AD7173_8)
	"RES", "RES",
#else
	"((AVDD1 ? AVSS)/5)+", "((AVDD1 ? AVSS)/5)-",
#endif
	"REF+", "REF-"
#elif defined(DEV_AD7172_2) || defined(DEV_AD7177_2) || defined(DEV_AD7175_2)
	"AIN0", "AIN1", "AIN2", "AIN3", "AIN4",
	"RES", "RES", "RES", "RES", "RES", "RES",
	"RES", "RES", "RES", "RES", "RES", "RES",
	"TEMP+", "TEMP-", "((AVDD1 ? AVSS)/5)+", "((AVDD1 ? AVSS)/5)-",
	"REF+", "REF-"
#elif defined(DEV_AD7172_4)
	"AIN0", "AIN1", "AIN2", "AIN3", "AIN4",
	"AIN5", "AIN6", "AIN7", "AIN8",
	"RES", "RES", "RES", "RES", "RES", "RES",
	"TEMP+", "TEMP-", "((AVDD1 ? AVSS)/5)+", "((AVDD1 ? AVSS)/5)-",
	"REF+", "REF-"
#elif defined(DEV_AD7176_2)
	"AIN0", "AIN1", "AIN2", "AIN3", "AIN4",
	"RES", "RES", "RES", "RES", "RES", "RES",
	"RES", "RES", "RES", "RES", "RES", "RES",
	"RES", "RES", "RES", "RES", "REF+", "REF-"
#endif
};


// SIN5+SINC1 Filter ODR map
const float sinc5_sinc1_odr_map[] = {
#if defined(DEV_AD4115)
	ODR_125000, ODR_125000, ODR_62500, ODR_62500,
	ODR_31250, ODR_25000, ODR_15625, ODR_10417, ODR_5000,
	ODR_2500, ODR_1000, ODR_500, ODR_397_5, ODR_200, ODR_100,
	ODR_59_98, ODR_49_96, ODR_20, ODR_16_67,
	ODR_10, ODR_5, ODR_2_5, ODR_2_5
#elif defined(DEV_AD4111) || defined(DEV_AD4112) || defined(DEV_AD4114) || \
	defined(DEV_AD7172_2) || defined(DEV_AD7172_4) || defined(DEV_AD7173_8)
	ODR_31250, ODR_31250, ODR_31250, ODR_31250, ODR_31250, ODR_31250,
	ODR_15625, ODR_10417, ODR_5208,  ODR_2597,  ODR_1007,  ODR_503_8,
	ODR_381,   ODR_200_3, ODR_100_2, ODR_59_52, ODR_49_68, ODR_20_01,
	ODR_16_63, ODR_10,    ODR_5,     ODR_2_5,   ODR_1_25,
#elif defined(DEV_AD7176_2) || defined(DEV_AD7175_2) || defined(DEV_AD7175_8) ||\
	  defined(DEV_AD7177_2)
#if (DEV_AD7176_2) || defined(DEV_AD7175_2) || defined(DEV_AD7175_8)
	ODR_250000, ODR_125000, ODR_62500, ODR_50000, ODR_31250, ODR_25000, ODR_15625,
#endif
	ODR_10000, ODR_5000,  ODR_2500,  ODR_1000, ODR_500,   ODR_397_5, ODR_200,
	ODR_100,   ODR_59_94, ODR_49_96, ODR_20,   ODR_16_67, ODR_10,    ODR_5
#endif
};

const float sinc3_odr_map[] = {
#if defined(DEV_AD4115)
	ODR_125000, ODR_125000, ODR_62500, ODR_62500,
	ODR_31250, ODR_25000, ODR_15625, ODR_10417, ODR_5000,
	ODR_3906, ODR_1157, ODR_539, ODR_401, ODR_206,
	ODR_102, ODR_59_98, ODR_50, ODR_20, ODR_16_67,
	ODR_10, ODR_5, ODR_2_5, ODR_2_5
#elif defined(DEV_AD4111) || defined(DEV_AD4112) || defined(DEV_AD4114) || \
	defined(DEV_AD7172_2) || defined(DEV_AD7172_4) || defined(DEV_AD7173_8)
	ODR_31250, ODR_31250, ODR_31250,
	ODR_31250, ODR_31250, ODR_31250,
	ODR_15625, ODR_10417, ODR_5208,
#if defined(DEV_AD4111) || defined(DEV_AD4112) || defined(DEV_AD4114)
	ODR_3906,  ODR_1157,  ODR_539,
	ODR_401,   ODR_206,   ODR_102,
#else
	ODR_2604,  ODR_1008,  ODR_504,
	ODR_400_6, ODR_200_3, ODR_100_2,
#endif
	ODR_59_98, ODR_50,    ODR_20_01,
	ODR_16_67, ODR_10,    ODR_5,
	ODR_2_5,   ODR_1_25,
#elif defined(DEV_AD7176_2) || defined(DEV_AD7175_2) || defined(DEV_AD7175_8) ||\
	  defined(DEV_AD7177_2)
#if (DEV_AD7176_2) || defined(DEV_AD7175_2) || defined(DEV_AD7175_8)
	ODR_250000, ODR_125000, ODR_62500,
	ODR_50000, ODR_31250, ODR_25000,
	ODR_15625,
#endif
	ODR_10000, ODR_5000, ODR_2500, ODR_1000, ODR_500,   ODR_400, ODR_200,
	ODR_100,   ODR_60,   ODR_50,   ODR_20,   ODR_16_67, ODR_10,  ODR_5
#endif
};


/*
 * Definition of the channel enable/disable menu items and menu itself
 */
console_menu_item chn_enable_disable_items[] = {
	{ "Enable Channels",	'E', menu_channels_enable_disable, NULL, SELECT_ENABLE },
	{ "Disable Channels",	'D', menu_channels_enable_disable, NULL, SELECT_DISBLE },
};

console_menu chn_enable_disable_menu = {
	.title = "Channel Enable/Disable Menu",
	.items = chn_enable_disable_items,
	.itemCount = ARRAY_SIZE(chn_enable_disable_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*
 * Definition of the analog input connection menu items and menu itself
 */
console_menu_item analog_input_connect_items[] = {
#if defined(DEV_AD4111) || defined(DEV_AD4112) || defined(DEV_AD4114) || defined(DEV_AD4115)
	// Input pin name/pair		Key		Menu Function			AINP/AINM Bits
	//
	{ "VIN0, VIN1",				'A', menu_analog_input_connect, NULL, ((VIN0_INPUT_BITS << VIN_PAIR_OFFSET) | VIN1_INPUT_BITS) },
	{ "VIN0, VINCOM",			'B', menu_analog_input_connect, NULL, ((VIN0_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN1, VIN0",				'C', menu_analog_input_connect, NULL, ((VIN1_INPUT_BITS << VIN_PAIR_OFFSET) | VIN0_INPUT_BITS) },
	{ "VIN1, VINCOM",			'D', menu_analog_input_connect, NULL, ((VIN1_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN2, VIN3",				'E', menu_analog_input_connect, NULL, ((VIN2_INPUT_BITS << VIN_PAIR_OFFSET) | VIN3_INPUT_BITS) },
	{ "VIN2, VINCOM",			'F', menu_analog_input_connect, NULL, ((VIN2_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN3, VIN2",				'G', menu_analog_input_connect, NULL, ((VIN3_INPUT_BITS << VIN_PAIR_OFFSET) | VIN2_INPUT_BITS) },
	{ "VIN3, VINCOM",			'H', menu_analog_input_connect, NULL, ((VIN3_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN4, VIN5",				'I', menu_analog_input_connect, NULL, ((VIN4_INPUT_BITS << VIN_PAIR_OFFSET) | VIN5_INPUT_BITS) },
	{ "VIN4, VINCOM",			'J', menu_analog_input_connect, NULL, ((VIN4_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN5, VIN4",				'K', menu_analog_input_connect, NULL, ((VIN5_INPUT_BITS << VIN_PAIR_OFFSET) | VIN4_INPUT_BITS) },
	{ "VIN5, VINCOM",			'L', menu_analog_input_connect, NULL, ((VIN5_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN6, VIN7",				'M', menu_analog_input_connect,NULL, ((VIN6_INPUT_BITS << VIN_PAIR_OFFSET) | VIN7_INPUT_BITS) },
	{ "VIN6, VINCOM",			'N', menu_analog_input_connect,NULL, ((VIN6_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN7, VIN6",				'O', menu_analog_input_connect, NULL, ((VIN7_INPUT_BITS << VIN_PAIR_OFFSET) | VIN6_INPUT_BITS) },
	{ "VIN7, VINCOM",			'P', menu_analog_input_connect, NULL, ((VIN7_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
#if defined(DEV_AD4111) || defined(DEV_AD4112)
	{ "IN3+, IN3-",				'Q', menu_analog_input_connect, NULL, ((IN3P_INPUT_BITS << VIN_PAIR_OFFSET) | IN3N_INPUT_BITS) },
	{ "IN2+, IN2-",				'R', menu_analog_input_connect, NULL, ((IN2P_INPUT_BITS << VIN_PAIR_OFFSET) | IN2N_INPUT_BITS) },
	{ "IN1+, IN1-",				'S', menu_analog_input_connect, NULL, ((IN1P_INPUT_BITS << VIN_PAIR_OFFSET) | IN1N_INPUT_BITS) },
	{ "IN0+, IN0-",				'T', menu_analog_input_connect, NULL, ((IN0P_INPUT_BITS << VIN_PAIR_OFFSET) | IN0N_INPUT_BITS) },
#else // mapping for AD4114/15
	{ "VIN8, VIN9",				'Q', menu_analog_input_connect, NULL, ((VIN8_INPUT_BITS << VIN_PAIR_OFFSET) | VIN9_INPUT_BITS) },
	{ "VIN8, VINCOM",			'R', menu_analog_input_connect, NULL, ((VIN8_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN9, VIN8",				'S', menu_analog_input_connect, NULL, ((VIN9_INPUT_BITS << VIN_PAIR_OFFSET) | VIN8_INPUT_BITS) },
	{ "VIN9, VINCOM",			'T', menu_analog_input_connect, NULL, ((VIN9_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN10, VIN11",			'U', menu_analog_input_connect, NULL, ((VIN10_INPUT_BITS << VIN_PAIR_OFFSET) | VIN11_INPUT_BITS) },
	{ "VIN10, VINCOM",			'V', menu_analog_input_connect, NULL, ((VIN10_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN11, VIN10",			'W', menu_analog_input_connect, NULL, ((VIN11_INPUT_BITS << VIN_PAIR_OFFSET) | VIN10_INPUT_BITS) },
	{ "VIN11, VINCOM",			'X', menu_analog_input_connect, NULL, ((VIN11_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN12, VIN13",			'Y', menu_analog_input_connect, NULL, ((VIN12_INPUT_BITS << VIN_PAIR_OFFSET) | VIN13_INPUT_BITS) },
	{ "VIN12, VINCOM",			'Z', menu_analog_input_connect, NULL, ((VIN12_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN13, VIN12",			'1', menu_analog_input_connect, NULL, ((VIN13_INPUT_BITS << VIN_PAIR_OFFSET) | VIN12_INPUT_BITS) },
	{ "VIN13, VINCOM",			'2', menu_analog_input_connect, NULL, ((VIN13_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN14, VIN15",			'3', menu_analog_input_connect, NULL, ((VIN14_INPUT_BITS << VIN_PAIR_OFFSET) | VIN15_INPUT_BITS) },
	{ "VIN14, VINCOM",			'4', menu_analog_input_connect, NULL, ((VIN14_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN15, VIN14",			'5', menu_analog_input_connect, NULL, ((VIN15_INPUT_BITS << VIN_PAIR_OFFSET) | VIN14_INPUT_BITS) },
	{ "VIN15, VINCOM",			'6', menu_analog_input_connect, NULL, ((VIN15_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
#endif
	{ "Temperature Sensor",		'7', menu_analog_input_connect, NULL, ((TEMP_SENSOR_POS_INP_BITS << VIN_PAIR_OFFSET) | TEMP_SENSOR_NEG_INP_BITS) },
	{ "Reference",				'8', menu_analog_input_connect, NULL, ((REFP_INPUT_BITS << VIN_PAIR_OFFSET) | REFN_INPUT_BITS) },
#else
	{ "AIN0",					'A', menu_analog_input_connect, NULL, VIN0_INPUT_BITS },
	{ "AIN1",					'B', menu_analog_input_connect, NULL, VIN1_INPUT_BITS },
	{ "AIN2",					'C', menu_analog_input_connect, NULL, VIN2_INPUT_BITS },
	{ "AIN3",					'D', menu_analog_input_connect, NULL, VIN3_INPUT_BITS },
	{ "AIN4",					'E', menu_analog_input_connect, NULL, VIN4_INPUT_BITS },
#if defined(DEV_AD7172_2) || defined(DEV_AD7177_2) || defined(DEV_AD7175_2)
	{ "Temperature Sensor+",	'F', menu_analog_input_connect, NULL, TEMP_SENSOR_POS_INP_BITS },
	{ "Temperature Sensor-",	'G', menu_analog_input_connect, NULL, TEMP_SENSOR_NEG_INP_BITS },
	{ "((AVDD1 - AVSS)/5)+ ",	'H', menu_analog_input_connect, NULL, AVDD1_AVSS_P_BITS },
	{ "((AVDD1 - AVSS)/5)-",	'I', menu_analog_input_connect, NULL, AVDD1_AVSS_N_BITS },
	{ "REF+",					'J', menu_analog_input_connect, NULL, REFP_INPUT_BITS },
	{ "REF-",					'K', menu_analog_input_connect, NULL, REFN_INPUT_BITS },
#elif defined(DEV_AD7172_4)
	{ "AIN5",					'F', menu_analog_input_connect, NULL, VIN5_INPUT_BITS },
	{ "AIN6",					'G', menu_analog_input_connect, NULL, VIN6_INPUT_BITS },
	{ "AIN7",					'H', menu_analog_input_connect, NULL, VIN7_INPUT_BITS },
	{ "AIN8",					'I', menu_analog_input_connect, NULL, VIN8_INPUT_BITS },
	{ "((AVDD1 - AVSS)/5)+ ",	'J', menu_analog_input_connect, NULL, AVDD1_AVSS_P_BITS },
	{ "((AVDD1 - AVSS)/5)-",	'K', menu_analog_input_connect, NULL, AVDD1_AVSS_N_BITS },
	{ "REF+",					'L', menu_analog_input_connect, NULL, REFP_INPUT_BITS },
	{ "REF-",					'M', menu_analog_input_connect, NULL, REFN_INPUT_BITS },
#elif defined(DEV_AD7176_2)
	{ "REF+",					'F', menu_analog_input_connect, NULL, REFP_INPUT_BITS },
	{ "REF-",					'G', menu_analog_input_connect, NULL, REFN_INPUT_BITS },
#elif defined(DEV_AD7173_8) || defined(DEV_AD7175_8)
	{ "AIN5",					'F', menu_analog_input_connect, NULL, VIN5_INPUT_BITS },
	{ "AIN6",					'G', menu_analog_input_connect, NULL, VIN6_INPUT_BITS },
	{ "AIN7",					'H', menu_analog_input_connect, NULL, VIN7_INPUT_BITS },
	{ "AIN8",					'I', menu_analog_input_connect, NULL, VIN8_INPUT_BITS },
	{ "AIN9",					'J', menu_analog_input_connect, NULL, VIN9_INPUT_BITS },
	{ "AIN10",					'K', menu_analog_input_connect, NULL, VIN10_INPUT_BITS },
	{ "AIN11",					'L', menu_analog_input_connect, NULL, VIN11_INPUT_BITS },
	{ "AIN12",					'M', menu_analog_input_connect, NULL, VIN12_INPUT_BITS },
	{ "AIN13",					'N', menu_analog_input_connect, NULL, VIN13_INPUT_BITS },
	{ "AIN14",					'O', menu_analog_input_connect, NULL, VIN14_INPUT_BITS },
	{ "AIN15",					'P', menu_analog_input_connect, NULL, VIN15_INPUT_BITS },
	{ "AIN16",					'Q', menu_analog_input_connect, NULL, VIN16_INPUT_BITS },
	{ "Temperature Sensor+",	'R', menu_analog_input_connect, NULL, TEMP_SENSOR_POS_INP_BITS },
	{ "Temperature Sensor-",	'S', menu_analog_input_connect, NULL, TEMP_SENSOR_NEG_INP_BITS },
#if (DEV_AD7175_8)
	{ "((AVDD1 - AVSS)/5)+ ",	'T', menu_analog_input_connect, NULL, AVDD1_AVSS_P_BITS },
	{ "((AVDD1 - AVSS)/5)-",	'U', menu_analog_input_connect, NULL, AVDD1_AVSS_N_BITS },
#endif
	{ "REF+",					'V', menu_analog_input_connect, NULL, REFP_INPUT_BITS },
	{ "REF-",					'W', menu_analog_input_connect, NULL, REFN_INPUT_BITS },
#endif
#endif
};

console_menu analog_input_connect_menu = {
	.title = "Select Analog Input",
	.items = analog_input_connect_items,
	.itemCount = ARRAY_SIZE(analog_input_connect_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*
 * Definition of the open wire detect inputs type menu items and menu itself
 */
console_menu_item open_wire_detect_input_type_items[] = {
	{ "Single Ended Input",			'S', menu_input_type_selection, NULL, SINGLE_ENDED_INPUT },
	{ "Differential Ended Input",	'D', menu_input_type_selection, NULL, DIFF_ENDED_INPUT },
};

console_menu open_wire_detect_input_type_menu = {
	.title = "Select Analog Input Type",
	.items = open_wire_detect_input_type_items,
	.itemCount = ARRAY_SIZE(open_wire_detect_input_type_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*
 * Definition of the open wire detect single ended input channel pair menu items and menu itself
 */
console_menu_item open_wire_detect_se_channel_items[] = {
	{ "CHN0, CHN15",	'A', menu_select_chn_pair, NULL, ((ADC_CHN(0) << CHN_PAIR_OFFSET)  | ADC_CHN(15)) },
	{ "CHN1, CHN2",		'B', menu_select_chn_pair, NULL, ((ADC_CHN(1) << CHN_PAIR_OFFSET)  | ADC_CHN(2))  },
	{ "CHN3, CHN4",		'C', menu_select_chn_pair, NULL, ((ADC_CHN(3) << CHN_PAIR_OFFSET)  | ADC_CHN(4))  },
	{ "CHN5, CHN6",		'D', menu_select_chn_pair, NULL, ((ADC_CHN(5) << CHN_PAIR_OFFSET)  | ADC_CHN(6))  },
	{ "CHN7, CHN8",		'E', menu_select_chn_pair, NULL, ((ADC_CHN(7) << CHN_PAIR_OFFSET)  | ADC_CHN(8))  },
	{ "CHN9, CHN10",	'F', menu_select_chn_pair, NULL, ((ADC_CHN(9) << CHN_PAIR_OFFSET)  | ADC_CHN(10)) },
	{ "CHN11, CHN12",	'G', menu_select_chn_pair, NULL, ((ADC_CHN(11) << CHN_PAIR_OFFSET) | ADC_CHN(12)) },
	{ "CHN13, CHN14",	'H', menu_select_chn_pair, NULL, ((ADC_CHN(13) << CHN_PAIR_OFFSET) | ADC_CHN(14)) },
};

console_menu open_wire_detect_se_channel_menu = {
	.title = "Select Channel Pair",
	.items = open_wire_detect_se_channel_items,
	.itemCount = ARRAY_SIZE(open_wire_detect_se_channel_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*
 * Definition of the open wire detect differential ended input channel pair menu items and menu itself
 */
console_menu_item open_wire_detect_de_channel_items[] = {
	{ "CHN1, CHN2",		'A', menu_select_chn_pair, NULL, ((ADC_CHN(1) << CHN_PAIR_OFFSET)  | ADC_CHN(2))  },
	{ "CHN5, CHN6",		'B', menu_select_chn_pair, NULL, ((ADC_CHN(5) << CHN_PAIR_OFFSET)  | ADC_CHN(6))  },
	{ "CHN9, CHN10",	'C', menu_select_chn_pair, NULL, ((ADC_CHN(9) << CHN_PAIR_OFFSET)  | ADC_CHN(10)) },
	{ "CHN13, CHN14",	'D', menu_select_chn_pair, NULL, ((ADC_CHN(13) << CHN_PAIR_OFFSET) | ADC_CHN(14)) },
};

console_menu open_wire_detect_de_channel_menu = {
	.title = "Select Channel Pair",
	.items = open_wire_detect_de_channel_items,
	.itemCount = ARRAY_SIZE(open_wire_detect_de_channel_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*
 * Definition of the open wire detect single ended analog inputs menu items and menu itself
 */
console_menu_item open_wire_detect_se_analog_input_items[] = {
	{ "VIN0, VINCOM",	'A', menu_select_input_pair, NULL, ((VIN0_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN1, VINCOM",	'B', menu_select_input_pair, NULL, ((VIN1_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN2, VINCOM",	'C', menu_select_input_pair, NULL, ((VIN2_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN3, VINCOM",	'D', menu_select_input_pair, NULL, ((VIN3_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN4, VINCOM",	'E', menu_select_input_pair, NULL, ((VIN4_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN5, VINCOM",	'F', menu_select_input_pair, NULL, ((VIN5_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN6, VINCOM",	'G', menu_select_input_pair, NULL, ((VIN6_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
	{ "VIN7, VINCOM",	'H', menu_select_input_pair, NULL, ((VIN7_INPUT_BITS << VIN_PAIR_OFFSET) | VINCOM_INPUT_BITS) },
};

console_menu open_wire_detect_se_analog_input_menu = {
	.title = "Select Analog Inputs",
	.items = open_wire_detect_se_analog_input_items,
	.itemCount = ARRAY_SIZE(open_wire_detect_se_analog_input_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*
 * Definition of the open wire detect differential ended analog inputs menu items and menu itself
 */
console_menu_item open_wire_detect_de_analog_input_items[] = {
	{ "VIN0, VIN1", 'A', menu_select_input_pair, NULL,  ((VIN0_INPUT_BITS << VIN_PAIR_OFFSET) | VIN1_INPUT_BITS) },
	{ "VIN2, VIN3", 'B', menu_select_input_pair, NULL, ((VIN2_INPUT_BITS << VIN_PAIR_OFFSET) | VIN3_INPUT_BITS) },
	{ "VIN4, VIN5", 'C', menu_select_input_pair, NULL, ((VIN4_INPUT_BITS << VIN_PAIR_OFFSET) | VIN5_INPUT_BITS) },
	{ "VIN6, VIN7", 'D', menu_select_input_pair, NULL, ((VIN6_INPUT_BITS << VIN_PAIR_OFFSET) | VIN7_INPUT_BITS) },
};

console_menu open_wire_detect_de_analog_input_menu = {
	.title = "Select Analog Inputs",
	.items = open_wire_detect_de_analog_input_items,
	.itemCount = ARRAY_SIZE(open_wire_detect_de_analog_input_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*
 * Definition of the adc register read/write menu items and menu itself
 */
console_menu_item reg_read_write_items[] = {
	{ "Read Device Register",	'R', menu_rw_ad717x_register, NULL, DEVICE_REG_READ_ID  },
	{ "Write Device Register",	'W', menu_rw_ad717x_register, NULL, DEVICE_REG_WRITE_ID },
};

console_menu reg_read_write_menu = {
	.title = "Register Read/Write Menu",
	.items = reg_read_write_items,
	.itemCount = ARRAY_SIZE(reg_read_write_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*
 * Definition of the sampling menu items and menu itself
 */
console_menu_item acquisition_menu_items[] = {
	{ "Single Conversion Mode",						'S', menu_single_conversion },
	{ "Continuous Conversion Mode - Table View",	'T', menu_continuous_conversion_tabular },
	{ "Continuous Conversion Mode - Stream Data",	'C', menu_continuous_conversion_stream },
};

console_menu acquisition_menu = {
	.title = "Data Acquisition Menu",
	.items = acquisition_menu_items,
	.itemCount = ARRAY_SIZE(acquisition_menu_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*
 * Definition of the filter select menu items and menu itself
 */
console_menu_item filter_select_items[] = {
	{ "Sinc5+1",	'A', menu_filter_select, NULL, SINC5_SINC1_FILTER },
	{ "Sinc3",		'B', menu_filter_select, NULL, SINC3_FILTER		},
};

console_menu filter_select_menu = {
	.title = "Filter Selection Menu",
	.items = filter_select_items,
	.itemCount = ARRAY_SIZE(filter_select_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*
 * Definition of the postfilter enable/disable menu items and menu itself
 */
console_menu_item postfilter_enable_disable_items[] = {
	{ "Enable",		'E', menu_postfiler_enable_disable, NULL,	SELECT_ENABLE },
	{ "Disable",	'D', menu_postfiler_enable_disable, NULL,	SELECT_DISBLE },
};

console_menu postfilter_enable_disable_menu = {
	.title = "Post-filter Enable/Disable Menu",
	.items = postfilter_enable_disable_items,
	.itemCount = ARRAY_SIZE(postfilter_enable_disable_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*
 * Definition of the post-filter select menu items and menu itself
 */
console_menu_item postfilter_select_items[] = {
	{ "27 SPS, 47 dB reject, 36.7 ms settling ",'A', menu_postfiler_select, NULL, POST_FLTR_27_SPS },
	{ "25 SPS, 62 dB reject, 40 ms settling",	'B', menu_postfiler_select, NULL, POST_FLTR_25_SPS },
	{ "20 SPS, 86 dB reject, 50 ms settling",	'C', menu_postfiler_select, NULL, POST_FLTR_20_SPS },
	{ "16.67 SPS, 92 dB reject, 60 ms settling",'D', menu_postfiler_select, NULL, POST_FLTR_16_67_SPS },
};

console_menu postfilter_select_menu = {
	.title = "Post-filter Selection Menu",
	.items = postfilter_select_items,
	.itemCount = ARRAY_SIZE(postfilter_select_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*
 * Definition of the SINC5+SINC1 ODR select menu items and menu itself
 */
console_menu_item sinc5_1_data_rate_select_items[] = {
#if defined(DEV_AD4115)
	{ ODR_125000_STR,	'A', menu_odr_select, NULL, ODR_125000_BITS	},
	{ ODR_62500_STR,	'B', menu_odr_select, NULL, ODR_62500_BITS	},
	{ ODR_31250_STR,	'C', menu_odr_select, NULL, ODR_31250_BITS	},
	{ ODR_25000_STR,	'D', menu_odr_select, NULL, ODR_25000_BITS	},
	{ ODR_15625_STR,	'E', menu_odr_select, NULL, ODR_15625_BITS	},
	{ ODR_10417_STR,	'F', menu_odr_select, NULL, ODR_10417_BITS	},
	{ ODR_5000_STR,		'G', menu_odr_select, NULL, ODR_5000_BITS		},
	{ ODR_2500_STR,		'H', menu_odr_select, NULL, ODR_2500_BITS		},
	{ ODR_1000_STR,		'I', menu_odr_select, NULL, ODR_1000_BITS		},
	{ ODR_500_STR,		'J', menu_odr_select, NULL, ODR_500_BITS		},
	{ ODR_397_5_STR,	'K', menu_odr_select, NULL, ODR_397_5_BITS	},
	{ ODR_200_STR,		'L', menu_odr_select, NULL, ODR_200_BITS		},
	{ ODR_100_STR,		'M', menu_odr_select, NULL, ODR_100_BITS		},
	{ ODR_59_98_STR,	'N', menu_odr_select, NULL, ODR_59_98_BITS	},
	{ ODR_49_96_STR,	'O', menu_odr_select, NULL, ODR_49_96_BITS	},
	{ ODR_20_STR,		'P', menu_odr_select, NULL, ODR_20_BITS		},
	{ ODR_16_67_STR,	'Q', menu_odr_select, NULL, ODR_16_67_BITS	},
	{ ODR_10_STR,		'R', menu_odr_select, NULL, ODR_10_BITS		},
	{ ODR_5_STR,		'S', menu_odr_select, NULL, ODR_5_BITS		},
	{ ODR_2_5_STR,		'T', menu_odr_select, NULL, ODR_2_5_BITS		},
#elif defined(DEV_AD4111) || defined(DEV_AD4112) || defined(DEV_AD4114) || \
	defined(DEV_AD7172_2) || defined(DEV_AD7172_4) || defined(DEV_AD7173_8)
	{ ODR_31250_STR,	'A', menu_odr_select, NULL, ODR_31250_BITS	},
	{ ODR_15625_STR,	'B', menu_odr_select, NULL, ODR_15625_BITS	},
	{ ODR_10417_STR,	'C', menu_odr_select, NULL, ODR_10417_BITS	},
	{ ODR_5208_STR,		'D', menu_odr_select, NULL, ODR_5208_BITS		},
	{ ODR_2597_STR,		'E', menu_odr_select, NULL, ODR_2597_BITS		},
	{ ODR_1007_STR,		'F', menu_odr_select, NULL, ODR_1007_BITS		},
	{ ODR_503_8_STR,	'G', menu_odr_select, NULL, ODR_503_8_BITS	},
	{ ODR_381_STR,		'H', menu_odr_select, NULL, ODR_381_BITS		},
	{ ODR_200_3_STR,	'I', menu_odr_select, NULL, ODR_200_3_BITS	},
	{ ODR_100_2_STR,	'J', menu_odr_select, NULL, ODR_100_2_BITS	},
	{ ODR_59_52_STR,	'K', menu_odr_select, NULL, ODR_59_52_BITS	},
	{ ODR_49_68_STR,	'L', menu_odr_select, NULL, ODR_49_68_BITS	},
	{ ODR_20_01_STR,	'M', menu_odr_select, NULL, ODR_20_01_BITS	},
	{ ODR_16_63_STR,	'N', menu_odr_select, NULL, ODR_16_63_BITS	},
	{ ODR_10_STR,		'O', menu_odr_select, NULL, ODR_10_BITS		},
	{ ODR_5_STR,		'P', menu_odr_select, NULL, ODR_5_BITS		},
	{ ODR_2_5_STR,		'Q', menu_odr_select, NULL, ODR_2_5_BITS		},
	{ ODR_1_25_STR,		'R', menu_odr_select, NULL, ODR_1_25_BITS		},
#elif defined(DEV_AD7176_2) || defined(DEV_AD7175_2) || \
	  defined(DEV_AD7175_8) || defined(DEV_AD7177_2)
#if (DEV_AD7176_2) || defined(DEV_AD7175_2) || defined(DEV_AD7175_8)
	{ ODR_250000_STR,	'A', menu_odr_select, NULL, ODR_250000_BITS	},
	{ ODR_125000_STR,	'B', menu_odr_select, NULL, ODR_125000_BITS	},
	{ ODR_62500_STR,	'C', menu_odr_select, NULL, ODR_62500_BITS	},
	{ ODR_50000_STR,	'D', menu_odr_select, NULL, ODR_50000_BITS	},
	{ ODR_31250_STR,	'E', menu_odr_select, NULL, ODR_31250_BITS	},
	{ ODR_25000_STR,	'F', menu_odr_select, NULL, ODR_25000_BITS	},
	{ ODR_15625_STR,	'G', menu_odr_select, NULL, ODR_15625_BITS	},
#endif
	{ ODR_10000_STR,	'H', menu_odr_select, NULL, ODR_10000_BITS	},
	{ ODR_5000_STR,		'I', menu_odr_select, NULL, ODR_5000_BITS		},
	{ ODR_2500_STR,		'J', menu_odr_select, NULL, ODR_2500_BITS		},
	{ ODR_1000_STR,		'K', menu_odr_select, NULL, ODR_1000_BITS		},
	{ ODR_500_STR,		'L', menu_odr_select, NULL, ODR_500_BITS		},
	{ ODR_397_5_STR,	'M', menu_odr_select, NULL, ODR_397_5_BITS	},
	{ ODR_200_STR,		'N', menu_odr_select, NULL, ODR_200_BITS		},
	{ ODR_100_STR,		'O', menu_odr_select, NULL, ODR_100_BITS		},
	{ ODR_59_94_STR,	'P', menu_odr_select, NULL, ODR_59_94_BITS	},
	{ ODR_49_96_STR,	'Q', menu_odr_select, NULL, ODR_49_96_BITS	},
	{ ODR_20_STR,		'R', menu_odr_select, NULL, ODR_20_BITS		},
	{ ODR_16_67_STR, 	'S', menu_odr_select, NULL, ODR_16_67_BITS	},
	{ ODR_10_STR,		'T', menu_odr_select, NULL, ODR_10_BITS		},
	{ ODR_5_STR,		'U', menu_odr_select, NULL, ODR_5_BITS		},
#endif
};

console_menu sinc5_1_data_rate_select_menu = {
	.title = "ODR Selection Menu",
	.items = sinc5_1_data_rate_select_items,
	.itemCount = ARRAY_SIZE(sinc5_1_data_rate_select_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*
 * Definition of the SINC3 ODR select menu items and menu itself
 */
console_menu_item sinc3_data_rate_select_items[] = {
#if defined(DEV_AD4115)
	{ ODR_125000_STR,	'A', menu_odr_select, NULL, ODR_125000_BITS	},
	{ ODR_62500_STR,	'B', menu_odr_select, NULL, ODR_62500_BITS	},
	{ ODR_31250_STR,	'C', menu_odr_select, NULL, ODR_31250_BITS	},
	{ ODR_25000_STR,	'D', menu_odr_select, NULL, ODR_25000_BITS	},
	{ ODR_15625_STR,	'E', menu_odr_select, NULL, ODR_15625_BITS	},
	{ ODR_10417_STR,	'F', menu_odr_select, NULL, ODR_10417_BITS	},
	{ ODR_5000_STR,		'G', menu_odr_select, NULL, ODR_5000_BITS		},
	{ ODR_3906_STR,		'H', menu_odr_select, NULL, ODR_3906_BITS		},
	{ ODR_1157_STR,		'I', menu_odr_select, NULL, ODR_1157_BITS		},
	{ ODR_539_STR,		'J', menu_odr_select, NULL, ODR_539_BITS		},
	{ ODR_401_STR,		'K', menu_odr_select, NULL, ODR_401_BITS		},
	{ ODR_206_STR,		'L', menu_odr_select, NULL, ODR_206_BITS		},
	{ ODR_102_STR,		'M', menu_odr_select, NULL, ODR_102_BITS		},
	{ ODR_59_98_STR,	'N', menu_odr_select, NULL, ODR_59_98_BITS	},
	{ ODR_50_STR,		'O', menu_odr_select, NULL, ODR_50_BITS		},
	{ ODR_20_STR,		'P', menu_odr_select, NULL, ODR_20_BITS		},
	{ ODR_16_67_STR,	'Q', menu_odr_select, NULL, ODR_16_67_BITS	},
	{ ODR_10_STR,		'R', menu_odr_select, NULL, ODR_10_BITS		},
	{ ODR_5_STR,		'S', menu_odr_select, NULL, ODR_5_BITS		},
	{ ODR_2_5_STR,		'T', menu_odr_select, NULL, ODR_2_5_BITS		},
#elif defined(DEV_AD4111) || defined(DEV_AD4112) || defined(DEV_AD4114) || \
	defined(DEV_AD7172_2) || defined(DEV_AD7172_4) || defined(DEV_AD7173_8)
	{ ODR_31250_STR,	'A', menu_odr_select, NULL, ODR_31250_BITS	},
	{ ODR_15625_STR,	'B', menu_odr_select, NULL, ODR_15625_BITS	},
	{ ODR_10417_STR,	'C', menu_odr_select, NULL, ODR_10417_BITS	},
	{ ODR_5208_STR,		'D', menu_odr_select, NULL, ODR_5208_BITS		},
#if defined(DEV_AD4111) || defined(DEV_AD4112) || defined(DEV_AD4114)
	{ ODR_3906_STR,		'E', menu_odr_select, NULL, ODR_3906_BITS		},
	{ ODR_1157_STR,		'F', menu_odr_select, NULL, ODR_1157_BITS		},
	{ ODR_539_STR,		'G', menu_odr_select, NULL, ODR_539_BITS		},
	{ ODR_401_STR,		'H', menu_odr_select, NULL, ODR_401_BITS		},
	{ ODR_206_STR,		'H', menu_odr_select, NULL, ODR_206_BITS		},
	{ ODR_102_STR,		'I', menu_odr_select, NULL, ODR_102_BITS		},
#else
	{ ODR_2604_STR,		'E', menu_odr_select, NULL, ODR_2604_BITS		},
	{ ODR_1008_STR,		'F', menu_odr_select, NULL, ODR_1008_BITS		},
	{ ODR_504_STR,		'G', menu_odr_select, NULL, ODR_504_BITS		},
	{ ODR_400_6_STR,	'H', menu_odr_select, NULL, ODR_400_6_BITS	},
	{ ODR_200_3_STR,	'H', menu_odr_select, NULL, ODR_200_3_BITS	},
	{ ODR_100_2_STR,	'I', menu_odr_select, NULL, ODR_100_2_BITS	},
#endif
	{ ODR_59_98_STR,	'J', menu_odr_select, NULL, ODR_59_98_BITS	},
	{ ODR_50_STR,		'K', menu_odr_select, NULL, ODR_50_BITS		},
	{ ODR_20_01_STR,	'L', menu_odr_select, NULL, ODR_20_01_BITS	},
	{ ODR_16_67_STR,	'M', menu_odr_select, NULL, ODR_16_67_BITS	},
	{ ODR_10_STR,		'N', menu_odr_select, NULL, ODR_10_BITS		},
	{ ODR_5_STR,		'O', menu_odr_select, NULL, ODR_5_BITS		},
	{ ODR_2_5_STR,		'P', menu_odr_select, NULL, ODR_2_5_BITS		},
	{ ODR_1_25_STR,		'Q', menu_odr_select, NULL, ODR_1_25_BITS		},
#elif defined(DEV_AD7176_2) || defined(DEV_AD7175_2) || \
	  defined(DEV_AD7175_8) || defined(DEV_AD7177_2)
#if (DEV_AD7176_2) || defined(DEV_AD7175_2) || defined(DEV_AD7175_8)
	{ ODR_250000_STR,	'A', menu_odr_select, NULL, ODR_250000_BITS	},
	{ ODR_125000_STR,	'B', menu_odr_select, NULL, ODR_125000_BITS	},
	{ ODR_62500_STR,	'C', menu_odr_select, NULL, ODR_62500_BITS	},
	{ ODR_50000_STR,	'D', menu_odr_select, NULL, ODR_50000_BITS	},
	{ ODR_31250_STR,	'E', menu_odr_select, NULL, ODR_31250_BITS	},
	{ ODR_25000_STR,	'F', menu_odr_select, NULL, ODR_25000_BITS	},
	{ ODR_15625_STR,	'G', menu_odr_select, NULL, ODR_15625_BITS	},
#endif
	{ ODR_10000_STR,	'H', menu_odr_select, NULL, ODR_10000_BITS	},
	{ ODR_5000_STR,		'I', menu_odr_select, NULL, ODR_5000_BITS		},
	{ ODR_2500_STR,		'J', menu_odr_select, NULL, ODR_2500_BITS		},
	{ ODR_1000_STR,		'K', menu_odr_select, NULL, ODR_1000_BITS		},
	{ ODR_500_STR,		'L', menu_odr_select, NULL, ODR_500_BITS		},
	{ ODR_400_STR	,	'M', menu_odr_select, NULL, ODR_400_BITS		},
	{ ODR_200_STR,		'N', menu_odr_select, NULL, ODR_200_BITS		},
	{ ODR_100_STR,		'O', menu_odr_select, NULL, ODR_100_BITS		},
	{ ODR_60_STR,		'P', menu_odr_select, NULL, ODR_60_BITS		},
	{ ODR_50_STR,		'Q', menu_odr_select, NULL, ODR_50_BITS		},
	{ ODR_20_STR,		'R', menu_odr_select, NULL, ODR_20_BITS		},
	{ ODR_16_67_STR, 	'S', menu_odr_select, NULL, ODR_16_67_BITS	},
	{ ODR_10_STR,		'T', menu_odr_select, NULL, ODR_10_BITS		},
	{ ODR_5_STR,		'U', menu_odr_select, NULL, ODR_5_BITS		},
#endif
};

console_menu sinc3_data_rate_select_menu = {
	.title = "ODR Selection Menu",
	.items = sinc3_data_rate_select_items,
	.itemCount = ARRAY_SIZE(sinc3_data_rate_select_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*
 * Definition of the polarity select menu items and menu itself
 */
console_menu_item polarity_select_items[] = {
	{ "Unipolar",	'U', menu_polarity_select, NULL, UNIPOLAR },
	{ "Bipolar",	'B', menu_polarity_select, NULL, BIPOLAR },
};

console_menu polarity_select_menu = {
	.title = "Polarity Selection Menu",
	.items = polarity_select_items,
	.itemCount = ARRAY_SIZE(polarity_select_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*
 * Definition of the refernce source selection menu items and menu itself
 */
console_menu_item reference_select_items[] = {
	{ "External",	'A', menu_reference_source_select, NULL, EXTERNAL },
	{ "Internal",	'B', menu_reference_source_select, NULL, INTERNAL	},
	{ "AVDD-AVSS",	'C', menu_reference_source_select, NULL, AVDD_AVSS },
};

console_menu reference_select_menu = {
	.title = "Reference Selection Menu",
	.items = reference_select_items,
	.itemCount = ARRAY_SIZE(reference_select_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*
 * Definition of the reference buffer enable/disable menu items and menu itself
 */
console_menu_item ref_buffer_enable_disable_items[] = {
	{ "Enable",		'E', menu_ref_buffer_enable_disable, NULL, SELECT_ENABLE },
	{ "Disable",	'D', menu_ref_buffer_enable_disable, NULL, SELECT_DISBLE },
};

console_menu ref_buffer_enable_disable_menu = {
	.title = "Reference Buffer Enable/Disable Menu",
	.items = ref_buffer_enable_disable_items,
	.itemCount = ARRAY_SIZE(ref_buffer_enable_disable_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*
 * Definition of the reference buffer enable/disable menu items and menu itself
 */
console_menu_item input_buffer_enable_disable_items[] = {
	{ "Enable",		'E', menu_input_buffer_enable_disable, NULL, SELECT_ENABLE },
	{ "Disable",	'D', menu_input_buffer_enable_disable, NULL, SELECT_DISBLE },
};

console_menu input_buffer_enable_disable_menu = {
	.title = "Input Buffer Enable/Disable Menu",
	.items = input_buffer_enable_disable_items,
	.itemCount = ARRAY_SIZE(input_buffer_enable_disable_items),
	.headerItem = NULL,
	.footerItem = NULL,
	.enableEscapeKey = true
};


/*
 * Definition of the Main Menu Items and menu itself
 */
console_menu_item main_menu_items[] = {
	{ "Read Device ID",							'A',	menu_read_id },
	{ "" },
	{ "Read Status Register",					'B',	menu_read_status },
	{ "" },
	{ "Sample Channels",						'C',	menu_sample_channels },
	{ "" },
	{ "Enable/Disable Channels",				'D',	menu_chn_enable_disable_display },
	{ "" },
	{ "Connect Analog Inputs to Channel",		'E',	menu_input_chn_connect_display },
	{ "" },
	{ "Configure and Assign Setup",				'F',	menu_config_and_assign_setup },
	{ "" },
	{ "Display setup",							'G',	menu_display_setup },
	{ "" },
	{ "Read Temperature",						'H',	menu_read_temperature },
	{ "" },
	{ "Calibrate ADC (Internal)",				'I',	menu_calibrate_adc },
	{ "" },
#if defined(DEV_AD4111)
	{ "Open Wire Detection",					'J',	menu_open_wire_detection },
	{ "" },
#endif
	{ "Read/Write Device Registers",			'K',	menu_read_write_device_regs },
};

console_menu ad717x_main_menu = {
	.title = "Main Menu",
	.items = main_menu_items,
	.itemCount = ARRAY_SIZE(main_menu_items),
	.headerItem = display_main_menu_header,
	.footerItem = NULL,
	.enableEscapeKey = false
};

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

#endif /* AD717X_MENU_DEFINES_H_ */
