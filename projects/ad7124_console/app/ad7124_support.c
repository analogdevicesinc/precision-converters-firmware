/*!
 *****************************************************************************
  @file:  ad7124_support.c

  @brief: Provides useful support functions for the AD7124 NoOS driver

  @details:
 -----------------------------------------------------------------------------
 Copyright (c) 2019, 2020 Analog Devices, Inc.
 All rights reserved.

 This software is proprietary to Analog Devices, Inc. and its licensors.
 By using this software you agree to the terms of the associated
 Analog Devices Software License Agreement.
*****************************************************************************/

#include <stdbool.h>
#include "ad7124_support.h"

// Public Functions
/*
 * @brief helper function get the setup setting for an ADC channel
 *
 * @param dev The device structure.
 *
 * @param channel ADC channel to get Setup for.
 *
 * @return value of setup field in channel configuration.
 */
uint8_t ad7124_get_channel_setup(struct ad7124_dev *dev, uint8_t channel)
{
	return (dev->regs[AD7124_Channel_0 + channel].value >> 12) & 0x7;
}


/*
 * @brief helper function get the PGA setting for an ADC channel
 *
 * @param dev The device structure.
 *
 * @param channel ADC channel to get Setup for.
 *
 * @return value of PGA field in the setup for an ADC channel.
 */
uint8_t ad7124_get_channel_pga(struct ad7124_dev *dev, uint8_t channel)
{
	uint8_t setup = ad7124_get_channel_setup(dev, channel);

	return (dev->regs[AD7124_Config_0 + setup].value) & 0x07;
}


/*
 * @brief helper function get the bipolar setting for an ADC channel
 *
 * @param dev The device structure.
 *
 * @param channel ADC channel to get bipolar mode for.
 *
 * @return value of bipolar field in the setup for an ADC channel.
 */
bool ad7124_get_channel_bipolar(struct ad7124_dev *dev, uint8_t channel)
{
	uint8_t setup = ad7124_get_channel_setup(dev, channel);

	return ((dev->regs[AD7124_Config_0 + setup].value >> 11) & 0x1) ? true : false;
}


/*
 * @brief converts ADC sample value to voltage based on gain setting
 *
 * @param dev The device structure.
 *
 * @param channel ADC channel to get Setup for.
 *
 * @param sample Raw ADC sample
 *
 * @return Sample ADC value converted to voltage.
 *
 * @note The conversion equation is implemented for simplicity,
 *       not for accuracy or performance
 *
 */
float ad7124_convert_sample_to_voltage(struct ad7124_dev *dev, uint8_t channel,
				       uint32_t sample)
{
	bool isBipolar = ad7124_get_channel_bipolar(dev, channel);
	uint8_t channelPGA = ad7124_get_channel_pga(dev, channel);

	float convertedValue;

	if (isBipolar) {
		convertedValue = ( ((float)sample / (1 << (AD7124_ADC_N_BITS -1))) -1 ) * \
				 (AD7124_REF_VOLTAGE / AD7124_PGA_GAIN(channelPGA));
	} else {
		convertedValue = ((float)sample * AD7124_REF_VOLTAGE)/(AD7124_PGA_GAIN(
					 channelPGA) * \
				 (1 << AD7124_ADC_N_BITS));
	}

	return (convertedValue);
}
