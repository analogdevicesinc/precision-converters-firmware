/***************************************************************************
 *   @file   ltc2488.h
 *   @brief  Header file for the LTC2488 Driver
 *
********************************************************************************
 * Copyright (c) 2021-22 Analog Devices, Inc.
 *
 * All rights reserved.
 *
 * This software is proprietary to Analog Devices, Inc. and its licensors.
 * By using this software you agree to the terms of the associated
 * Analog Devices Software License Agreement.
 *
*****************************************************************************/

#ifndef LTC2488_H
#define LTC2488_H

/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/

#include <stdbool.h>
#include "no_os_spi.h"
#include "no_os_util.h"

/******************************************************************************/
/********************** Macros and Constants Definitions **********************/

#define LTC2488_VREF                          4.096  // Reference Volatage 
#define LTC2488_CHANNEL_CONV_TIME               150  // Timeout in millisends
#define LTC2488_CHANNEL_EOC_MASK             NO_OS_BIT(23) // EOC bit mask
#define LTC2488_CHANNEL_SIGN_BIT_MASK	     NO_OS_BIT(21) // SIGN bit mask
#define LTC2488_CHANNEL_MSB_BIT_MASK         NO_OS_BIT(20) // MSB bit mask
#define LTC2488_CHANNEL_MASK_17BITS    NO_OS_GENMASK(16,0) // 16 bit data + 1 bit sign
#define LTC2488_FS_VOLTAGE        (LTC2488_VREF*0.5) // Full Scale Range

/* Single-Ended Channel Configuration
* Channel selection for all Single-Ended Inputs
*
* MUX ADDRESS              CHANNEL SELECTION
* SGL  OS   A2   A1   A0 | 0    1    2    3    COM
* 1    0    0    0    0  | IN+  -    -    -    IN-
* 1    0    0    0    1  | -    -    IN+  -    IN-
* 1    1    0    0    0  | -    IN+  -    -    IN-
* 1    1    0    0    1  | -    -    -    IN+  IN-
*/
#define LTC2488_SINGLE_CH0            0xB0
#define LTC2488_SINGLE_CH1            0xB8
#define LTC2488_SINGLE_CH2            0xB1
#define LTC2488_SINGLE_CH3            0xB9

// Channel Configuration Enable/Disable Bits
#define LTC2488_CHANNEL_CONF_DISABLE		 0x80
#define LTC2488_CHANNEL_CONF_ENABLE		     0xA0

// Masks the read only adc code to extract only the status bits.
#define LTC2488_INPUT_RANGE(x)  \
		(enum input_status)(((x) & \
		(LTC2488_CHANNEL_SIGN_BIT_MASK | LTC2488_CHANNEL_MSB_BIT_MASK)) >> 20)

// If End Of Conversion status detected returns true
#define LTC2488_EOC_DETECT(x)  \
		((x & LTC2488_CHANNEL_EOC_MASK ) ? false : true)

// Masks the read-only adc code to extract only 17 bits conversion result
#define LTC2488_GET_ADC_DATA(x)     ((x >> 4) & LTC2488_CHANNEL_MASK_17BITS)

// Signs extends the 17 bit value to 32 bit value
#define LTC2488_SIGN_EXTEND_ADC_DATA(x)  \
		((x ^(LTC2488_CHANNEL_MSB_BIT_MASK >> 4)) - \
		(LTC2488_CHANNEL_MSB_BIT_MASK >> 4))

/**
 * @enum input_status
 * @brief Various Input range
 */
enum input_status {
	UNDER_RANGE,
	NEGATIVE_RANGE,
	POSITIVE_RANGE,
	OVER_RANGE
};

/**
 * @struct ltc2488_dev
 * @brief Device driver structure
 */
struct ltc2488_dev {
	// SPI descriptor
	struct no_os_spi_desc *spi_desc;
};

/**
 * @struct ltc2488_dev_init
 * @brief Device driver initialization parameters
 */
struct ltc2488_dev_init {
	// SPI initialization parameters
	struct no_os_spi_init_param spi_init ;
};

/******************************************************************************/
/********************** Function Declarations *********************************/

enum input_status ltc2488_data_process(const uint32_t *adc_code,
				       int32_t *adc_value);

float ltc2488_code_to_voltage(const int32_t *adc_data);

int32_t ltc2488_init(struct ltc2488_dev **device,
		     struct ltc2488_dev_init *init_param);

int32_t ltc2488_remove(struct ltc2488_dev *dev);

int32_t ltc2488_read_write(struct no_os_spi_desc *desc,
			   uint8_t buff_cmd,
			   uint32_t *adc_buff);

#endif
