/*!
 *****************************************************************************
  @file:  ad717x_console_app.h

  @brief: Header for AD717x/AD411x console application interface.

  @details:
 -----------------------------------------------------------------------------
 Copyright (c) 2020 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD717X_CONSOLE_APP_H_
#define AD717X_CONSOLE_APP_H_

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include "adi_console_menu.h"
#include "app_config.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

#define ADC_REF_VOLTAGE		2.5		// in volts

#if defined(DEV_AD7177_2)
#define ADC_RESOLUTION		32		// in bits
#else
#define ADC_RESOLUTION		24		// in bits
#endif


// Define the number of channels for selected device
#if defined(DEV_AD4111) || defined(DEV_AD4112) || \
	defined(DEV_AD4114) || defined(DEV_AD4115) || \
	defined(DEV_AD7173_8) || defined(DEV_AD7175_8)
#define NUMBER_OF_CHANNELS	16U
#elif defined(DEV_AD7172_4)
#define NUMBER_OF_CHANNELS	8U
#else
#define NUMBER_OF_CHANNELS	4U
#endif


// Define the number of setups for selected device
#if defined(DEV_AD4111) || defined(DEV_AD4112) || \
	defined(DEV_AD4114) || defined(DEV_AD4115) || \
	defined(DEV_AD7173_8) || defined(DEV_AD7172_4) || \
	defined(DEV_AD7175_8)
#define NUMBER_OF_SETUPS	8U
#else
#define NUMBER_OF_SETUPS	4U
#endif

/******************************************************************************/
/********************** Variables and User Defined Data Types *****************/
/******************************************************************************/

extern console_menu ad717x_main_menu;

/* AD717x Setup Configuration Structure */
typedef struct {
	uint32_t setup;                  // Selected setup
	uint32_t filter;                 // Filter type
	uint32_t postfilter;             // Post filter type for SINC5+1 Filter
	uint32_t post_filter_enabled;    // Post filter enable status
	uint32_t odr_bits;               // Output data rate register bits
	uint32_t polarity;               // Bipolar or Unipolar analog input
	uint32_t reference;              // Reference source for ADC
	uint32_t input_buffers;          // Buffers on analog inputs
	uint32_t reference_buffers;      // Buffers on reference source
	uint32_t pos_analog_input;       // Positive analog input
	uint32_t neg_analog_input;       // Negative analog input
	uint32_t channel_enabled;        // Channel Enable/Disable flag
	uint32_t setup_assigned;         // Setup assigned to a channel
} ad717x_setup_config;

/******************************************************************************/
/************************ Public Declarations *********************************/
/******************************************************************************/

int32_t ad717x_app_initialize(void);

#endif /* AD717X_CONSOLE_APP_H_ */
