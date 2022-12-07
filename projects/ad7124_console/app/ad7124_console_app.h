/*!
 *****************************************************************************
  @file:  ad7124_console_app.h

  @brief: defines the console menu structure for the AD7124 example code

  @details:
 -----------------------------------------------------------------------------
 Copyright (c) 2019, 2020 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
*****************************************************************************/

#ifndef AD7124_CONSOLE_APP_H_
#define AD7124_CONSOLE_APP_H_

#include "adi_console_menu.h"

/* #defines */
#define AD7124_CONFIG_A       0
#define AD7124_CONFIG_B       1

/* Public Declarations */
int32_t ad7124_app_initialize(uint8_t configID);

extern console_menu ad7124_main_menu;


/* AD7124 Filter types */
typedef enum {
	SINC4_FILTER               = 0,
	SINC3_FILTER               = 2,
	FAST_SETTLING_SINC4_FILTER = 4,
	FAST_SETTLING_SINC3_FILTER = 5
} filter_type;

/* AD7124 Reference Source */
typedef enum {
	REFIN1,
	REFIN2,
	INT,
	AVDD
} reference_type;

/* AD7124 power mode */
typedef enum {
	LOW_POWER_MODE,
	MED_POWER_MODE,
	FULL_POWER_MODE
} power_mode_t;

// Available adc master clock for particular power mode
#define LOW_POWER_MODE_FREQUENCY  76800    // 76.8Khz
#define MED_POWER_MODE_FREQUENCY  153600   // 153.6Khz
#define FUL_POWER_MODE_FREQUENCY  614400   // 614.4Khz

#define DEVICE_REG_READ_ID         1
#define DEVICE_REG_WRITE_ID        2

/* AD7124 Setup Configuration Structure */
typedef struct {
	filter_type filter;             // Filter type
	uint16_t data_rate_fs_val;      // Output data rate value
	uint8_t programmable_gain_bits; // PGA bits value
	uint8_t polarity;               // Bipolar or Unipolar analog input
	reference_type reference;       // Reference source for ADC
	uint8_t input_buffers;          // Buffers on analog inputs
	uint8_t reference_buffers;      // Buffers on reference source
	uint8_t channel_enabled;        // Channel Enable/Disable flag
	uint8_t setup_assigned;         // Assigned setup to channel
	uint8_t pos_analog_input;       // Positive analog input
	uint8_t neg_analog_input;       // Negative analog input
} ad7124_setup_config;


#endif /* AD7124_CONSOLE_APP_H_ */
