/*!
 *****************************************************************************
  @file:  nanodac_console_app.h

  @brief: defines the console menu structure for the nanodac example code

  @details:
 -----------------------------------------------------------------------------
 Copyright (c) 2020 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef NANODAC_CONSOLE_APP_H_
#define NANODAC_CONSOLE_APP_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "adi_console_menu.h"
#include "app_config.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

// Define the DAC channel menu selection. The following devices from nanodac
// family have only single channel and does not need menu to select DAC channel.
#if	!(defined(DEV_AD5683) || defined(DEV_AD5683R) || defined(DEV_AD5682R) || \
		defined(DEV_AD5681R) || defined(DEV_AD5693)  || defined(DEV_AD5693R) || \
		defined(DEV_AD5692R) || defined(DEV_AD5691R))
#define DISPLAY_DAC_CHANNEL_SELECT_MENU
// Define the number of DAC channels (for devices having more than 1 channel)
#if (defined(DEV_AD5687) || defined(DEV_AD5687R) || defined(DEV_AD5697R) || \
	 defined(DEV_AD5689) || defined(DEV_AD5689R))
#define DAC_CHANNEL_COUNT 2
#elif (defined(DEV_AD5686) || defined(DEV_AD5686R) || defined(DEV_AD5685R) || \
	 defined(DEV_AD5684) || defined(DEV_AD5684R) || \
	 defined(DEV_AD5696) || defined(DEV_AD5696R) || defined(DEV_AD5695R) || \
	 defined(DEV_AD5694) || defined(DEV_AD5694R))
#define DAC_CHANNEL_COUNT 4
#elif (defined(DEV_AD5676) || defined(DEV_AD5676R) || defined(DEV_AD5672R) || \
	   defined(DEV_AD5675R) || defined(DEV_AD5671R))
#define DAC_CHANNEL_COUNT 8
#elif (defined(DEV_AD5679) || defined(DEV_AD5679R) || defined(DEV_AD5673R) || \
	   defined(DEV_AD5674) || defined(DEV_AD5674R) || defined(DEV_AD5677R))
#define DAC_CHANNEL_COUNT 16
#endif
#endif

// Define the LDAC masking menu. The following devices from nanodac
// family have only single channel and does not need menu to select LDAC masking.
#if	!(defined(DEV_AD5683) || defined(DEV_AD5683R) || \
		defined(DEV_AD5682R) || defined(DEV_AD5681R) || defined(DEV_AD5693) || \
		defined(DEV_AD5693R) || defined(DEV_AD5692R) || defined(DEV_AD5691R))
#define DISPLAY_LDAC_MASK_SELECT_MENU
#endif

// Define the Vref source selections. The following devices from nanodac
// family have only external Vref source.
#if (defined(DEV_AD5674) || defined(DEV_AD5676) || defined(DEV_AD5686) || \
	 defined(DEV_AD5684) || defined(DEV_AD5696) || defined(DEV_AD5694) || \
	 defined(DEV_AD5683) || defined(DEV_AD5693) || defined(DEV_AD5679) || \
	 defined(DEV_AD5687) || defined(DEV_AD5689))
#define EXT_VREF_SOURCE_ONLY
#endif

// Define the gain source (software controlled or hardware controlled)
#if	(defined(DEV_AD5683) || defined(DEV_AD5683R) || defined(DEV_AD5682R) || \
	 defined(DEV_AD5681R) || defined(DEV_AD5693)  || defined(DEV_AD5693R) || \
	 defined(DEV_AD5692R) || defined(DEV_AD5691R))
#define SOFTWARE_CONTROLLED_GAIN
#else
#define HARDWARE_CONTROLLED_GAIN
#endif

// Define the operating mode selections
#if	(defined(DEV_AD5674) || defined(DEV_AD5674R) || defined(DEV_AD5673R) || \
	 defined(DEV_AD5679) || defined(DEV_AD5677R))
#define _1K_TO_GND_POWER_DOWN
#elif (defined(DEV_AD5676) || defined(DEV_AD5676R) || defined(DEV_AD5675R) || \
	   defined(DEV_AD5671R) || defined(DEV_AD5672R))
#define _1K_TO_GND_POWER_DOWN
#define THREE_STATE_POWER_DOWN
#else
#define _1K_TO_GND_POWER_DOWN
#define _100K_TO_GND_POWER_DOWN
#define THREE_STATE_POWER_DOWN
#endif

// define the DAC resolution
#if (defined(DEV_AD5671R) || defined(DEV_AD5672R) || defined(DEV_AD5673R) || \
	 defined(DEV_AD5674) || defined(DEV_AD5674R) || defined(DEV_AD5684R) || \
	 defined(DEV_AD5687) || defined(DEV_AD5687R) || defined(DEV_AD5697R) || \
	 defined(DEV_AD5694) || defined(DEV_AD5694R) || defined(DEV_AD5681R) || \
	 defined(DEV_AD5691R))
#define TOTAL_OUTPUT_CODES		((1U << 12) - 1)	// For 12-bit DAC (2^12 - 1)
#elif (defined(DEV_AD5685R) || defined(DEV_AD5695R) || defined(DEV_AD5682R) || \
	 defined(DEV_AD5692R))
#define TOTAL_OUTPUT_CODES		((1U << 14) - 1)	// For 14-bit DAC (2^14 - 1)
#else
#define TOTAL_OUTPUT_CODES		((1U << 16) - 1)	// For 16-bit DAC (2^16 - 1)
#endif

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

extern console_menu nanodac_main_menu;

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

int32_t nanodac_app_initialize(void);


#endif /* NANODAC_CONSOLE_APP_H_ */
